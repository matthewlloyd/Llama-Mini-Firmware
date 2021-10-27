/*
 * Copyright (c) 2021 Matthew Lloyd <github@matthewlloyd.net>
 * All rights reserved.
 *
 * This file is part of Llama Mini.
 *
 * Llama Mini is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Llama Mini is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Llama Mini. If not, see <https://www.gnu.org/licenses/>.
 */

#include "llama.h"
#include "extruder_enum.h"
#include "hwio.h"
#include "eeprom.h"
#include "crc32.h"
#include "app.h"
#include "config_a3ides2209_02.h"
#include "marlin_client.h"
#include "marlin_server.h"
#include "st25dv64k.h"
#include "cmsis_os.h"
#include "../../lib/Marlin/Marlin/src/module/planner.h"

static bool _eeprom_llama_cache_loaded = false;
static eeprom_llama_vars_t _eeprom_llama_cache;

static uint16_t eeprom_llama_var_size(uint8_t id);
static uint16_t eeprom_llama_var_offset(uint8_t id);

static int eeprom_llama_check_crc32(void);
static void eeprom_llama_update_crc32();

// semaphore handle (lock/unlock)
static osSemaphoreId eeprom_llama_sema = 0;

static inline void eeprom_llama_lock(void) {
    osSemaphoreWait(eeprom_llama_sema, osWaitForever);
}

static inline void eeprom_llama_unlock(void) {
    osSemaphoreRelease(eeprom_llama_sema);
}

void eeprom_llama_init(void) {
    if (_eeprom_llama_cache_loaded) return;

    osSemaphoreDef(eepromLlamaSema);
    eeprom_llama_sema = osSemaphoreCreate(osSemaphore(eepromLlamaSema), 1);

    // check for Llama at the new EEPROM address first
    st25dv64k_user_read_bytes(EEPROM_LLAMA_ADDRESS, &_eeprom_llama_cache, sizeof(eeprom_llama_vars_t));
    if (!eeprom_llama_check_crc32()) {
        // if invalid, check for Llama at the old EEPROM address
        st25dv64k_user_read_bytes(EEPROM_LLAMA_OLD_ADDRESS, &_eeprom_llama_cache, sizeof(eeprom_llama_vars_t));
        if (eeprom_llama_check_crc32()) {
            // if found, move to the new EEPROM address
            st25dv64k_user_write_bytes(EEPROM_LLAMA_ADDRESS, (void *)&_eeprom_llama_cache, EEPROM_LLAMA_DATASIZE);
        } else {
            // if not found, use defaults
            eeprom_llama_defaults();
        }
    } else if (_eeprom_llama_cache.VERSION < 1 || _eeprom_llama_cache.VERSION > EEPROM_LLAMA_VERSION) {
        eeprom_llama_defaults();
    } else {
        if (_eeprom_llama_cache.VERSION == 1) {
            // upgrade to version 2: adds extruder reverse
            // move data forward to account for addition of datasize field
            memcpy(((void*)&_eeprom_llama_cache) + 4, ((void*)&_eeprom_llama_cache) + 2, 19);
            _eeprom_llama_cache.EXTRUDER_REVERSE = 0;
            if (_eeprom_llama_cache.EXTRUDER_TYPE == 2) {  // Bond-Rev
                _eeprom_llama_cache.EXTRUDER_TYPE = 1;
                _eeprom_llama_cache.EXTRUDER_REVERSE = 1;
            } else if (_eeprom_llama_cache.EXTRUDER_TYPE == 3) {  // Custom
                _eeprom_llama_cache.EXTRUDER_TYPE = 2;
            }
            _eeprom_llama_cache.VERSION = 2;
            _eeprom_llama_cache.DATASIZE = EEPROM_LLAMA_DATASIZE;
            _eeprom_llama_cache.CRC32 = crc32_eeprom((uint32_t *)(&_eeprom_llama_cache), (EEPROM_LLAMA_DATASIZE - 4) / 4);
            st25dv64k_user_write_bytes(EEPROM_LLAMA_ADDRESS, (void *)&_eeprom_llama_cache, EEPROM_LLAMA_DATASIZE);
        }
    }
    _eeprom_llama_cache_loaded = true;
}

void eeprom_llama_defaults(void) {
    eeprom_llama_vars_t vars = eeprom_llama_var_defaults;
    // calculate crc32
    vars.CRC32 = crc32_eeprom((uint32_t *)(&vars), (EEPROM_LLAMA_DATASIZE - 4) / 4);
    // write data to eeprom
    st25dv64k_user_write_bytes(EEPROM_LLAMA_ADDRESS, (void *)&vars, EEPROM_LLAMA_DATASIZE);
    memcpy(&_eeprom_llama_cache, (void*)&vars, EEPROM_LLAMA_DATASIZE);
}

variant8_t eeprom_llama_get_var(uint8_t id) {
    uint16_t offset;
    uint16_t size;
    uint16_t data_size;
    void *data_ptr;
    variant8_t var = variant8_empty();
    const eeprom_entry_t *map = eeprom_llama_map;
    if (id < EEPROM_LLAMA_VARCOUNT) {
        var = variant8_init(map[id].type, map[id].count, 0);
        size = eeprom_llama_var_size(id);
        data_size = variant8_data_size(&var);
        if (size == data_size) {
            offset = eeprom_llama_var_offset(id);
            data_ptr = variant8_data_ptr(&var);
            memcpy((void*) data_ptr, ((void*) &_eeprom_llama_cache) + offset, size);
        }
    }
    return var;
}

void eeprom_llama_set_var(uint8_t id, variant8_t var, bool update_crc32) {
    uint16_t size;
    uint16_t data_size;
    if (id < EEPROM_LLAMA_VARCOUNT) {
        eeprom_llama_lock();
        if (variant8_get_type(var) == eeprom_llama_map[id].type) {
            size = eeprom_llama_var_size(id);
            data_size = variant8_data_size(&var);
            if ((size == data_size) || ((variant8_get_type(var) == VARIANT8_PCHAR) && (data_size <= size))) {
                uint16_t offset = eeprom_llama_var_offset(id);
                void* data_ptr = variant8_data_ptr(&var);
                st25dv64k_user_write_bytes(EEPROM_LLAMA_ADDRESS + offset, data_ptr, data_size);
                memcpy(((void*)&_eeprom_llama_cache) + offset, data_ptr, data_size);

                if (update_crc32) {
                    eeprom_llama_update_crc32();
                }
            }
        }
        eeprom_llama_unlock();
    }
}

static uint16_t eeprom_llama_var_size(uint8_t id) {
    if (id < EEPROM_LLAMA_VARCOUNT)
        return variant8_type_size(eeprom_llama_map[id].type & ~VARIANT8_PTR) * eeprom_llama_map[id].count;
    return 0;
}

static uint16_t eeprom_llama_var_offset(uint8_t id) {
    uint16_t addr = 0;
    while (id)
        addr += eeprom_llama_var_size(--id);
    return addr;
}

static int eeprom_llama_check_crc32() {
    uint16_t datasize = _eeprom_llama_cache.DATASIZE;
    if (_eeprom_llama_cache.VERSION == 1)
        datasize = EEPROM_LLAMA_MIN_DATASIZE;  // before datasize field was added
    if (datasize < EEPROM_LLAMA_MIN_DATASIZE || datasize > EEPROM_LLAMA_DATASIZE) return 0;
    uint32_t crc2 = crc32_eeprom((uint32_t *)&_eeprom_llama_cache, (datasize - 4) / 4);
    uint32_t eeprom_crc2 = *((uint32_t*)(((uint8_t*)&_eeprom_llama_cache) + datasize - 4));
    return eeprom_crc2 == crc2 ? 1 : 0;
}

static void eeprom_llama_update_crc32() {
    uint16_t datasize = _eeprom_llama_cache.DATASIZE;
    if (datasize < EEPROM_LLAMA_MIN_DATASIZE || datasize > EEPROM_LLAMA_DATASIZE) return;
    // calculate crc32
    _eeprom_llama_cache.CRC32 = crc32_eeprom((uint32_t *)&_eeprom_llama_cache, (datasize - 4) / 4);
    // write crc to eeprom
    st25dv64k_user_write_bytes(EEPROM_LLAMA_ADDRESS + sizeof(eeprom_llama_vars_t) - 4, &(_eeprom_llama_cache.CRC32), 4);
}


void llama_apply_fan_settings() {
    switch (_eeprom_llama_cache.HOTEND_FAN_SPEED) {
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_100:
        hwio_fan_control_set_hotend_fan_speed_percent(100);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_90:
        hwio_fan_control_set_hotend_fan_speed_percent(90);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_80:
        hwio_fan_control_set_hotend_fan_speed_percent(80);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_70:
        hwio_fan_control_set_hotend_fan_speed_percent(70);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_60:
        hwio_fan_control_set_hotend_fan_speed_percent(60);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_50:
        hwio_fan_control_set_hotend_fan_speed_percent(50);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_DEFAULT:
    default:
        hwio_fan_control_set_hotend_fan_speed_percent(38);
        break;
    }
}

void llama_apply_skew_settings() {
    double xy, xz, yz;
    if (_eeprom_llama_cache.SKEW_ENABLED) {
        xy = _eeprom_llama_cache.SKEW_XY;
        xz = _eeprom_llama_cache.SKEW_XZ;
        yz = _eeprom_llama_cache.SKEW_YZ;
    } else {
        xy = xz = yz = 0.f;
    }
    if (marlin_is_client_thread()) {
        // client thread - set variables remotely
        marlin_set_var(MARLIN_VAR_SKEW_XY, xy);
        marlin_set_var(MARLIN_VAR_SKEW_XZ, xz);
        marlin_set_var(MARLIN_VAR_SKEW_YZ, yz);
    } else {
        // server thread - set variables directly
        marlin_server_vars()->skew_xy = xy;
        marlin_server_handle_var_change(MARLIN_VAR_SKEW_XY);
        marlin_server_vars()->skew_xz = xz;
        marlin_server_handle_var_change(MARLIN_VAR_SKEW_XZ);
        marlin_server_vars()->skew_yz = yz;
        marlin_server_handle_var_change(MARLIN_VAR_SKEW_YZ);
    }
}
