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

#pragma once

#include <stdbool.h>
#include "variant8.h"
#include "eeprom.h"

static const constexpr uint8_t EEPROM_LLAMA__PADDING = 3;

// Llama eeprom map
static const eeprom_entry_t eeprom_llama_map[] = {
    { "VERSION",             VARIANT8_UI16,  1, EEVAR_FLG_READONLY }, // EEVAR_LLAMA_VERSION
    { "EXTRUDER_TYPE",       VARIANT8_UI8,   1, 0 }, // EEVAR_LLAMA_EXTRUDER_TYPE
    { "EXTRUDER_ESTEPS",     VARIANT8_FLT,   1, 0 }, // EEVAR_LLAMA_EXTRUDER_ESTEPS
    { "HOTEND_FAN_SPD",      VARIANT8_UI8,   1, 0 }, // EEVAR_LLAMA_HOTEND_FAN_SPEED
    { "SKEW_ENABLED",        VARIANT8_UI8,   1, 0 }, // EEPROM_LLAMA_SKEW_ENABLED
    { "SKEW_XY",             VARIANT8_FLT,   1, 0 }, // EEPROM_LLAMA_SKEW_XY
    { "SKEW_XZ",             VARIANT8_FLT,   1, 0 }, // EEPROM_LLAMA_SKEW_XZ
    { "SKEW_YZ",             VARIANT8_FLT,   1, 0 }, // EEPROM_LLAMA_SKEW_YZ
    { "_PADDING",            VARIANT8_PCHAR, EEPROM_LLAMA__PADDING, 0 }, // EEVAR_LLAMA__PADDING32
    { "CRC32",               VARIANT8_UI32,  1, 0 }, // EEVAR_LLAMA_CRC32
};

enum {
    EEPROM_LLAMA_ADDRESS = 0x0600,
    EEPROM_LLAMA_VERSION = 1
};

enum {
    EEVAR_LLAMA_VERSION = 0x00,         // uint16_t eeprom version
    EEVAR_LLAMA_EXTRUDER_TYPE = 0x01,
    EEVAR_LLAMA_EXTRUDER_ESTEPS = 0x02,
    EEVAR_LLAMA_HOTEND_FAN_SPEED = 0x03,
    EEVAR_LLAMA_SKEW_ENABLED = 0x04,
    EEVAR_LLAMA_SKEW_XY = 0x05,
    EEVAR_LLAMA_SKEW_XZ = 0x06,
    EEVAR_LLAMA_SKEW_YZ = 0x07,
    EEVAR_LLAMA__PADDING = 0x08,            // 1..4 chars, to ensure (DATASIZE % 4 == 0)
    EEVAR_LLAMA_CRC32 = 0x09,               // uint32_t crc32
};

typedef struct __attribute__((__packed__)) _eeprom_llama_vars_t {
    uint16_t VERSION;
    uint8_t EXTRUDER_TYPE;
    float EXTRUDER_ESTEPS;
    uint8_t HOTEND_FAN_SPEED;
    uint8_t SKEW_ENABLED;
    float SKEW_XY;
    float SKEW_XZ;
    float SKEW_YZ;
    char _PADDING[EEPROM_LLAMA__PADDING];
    uint32_t CRC32;
} eeprom_llama_vars_t;

static_assert(sizeof(eeprom_llama_vars_t) % 4 == 0, "EEPROM_LLAMA__PADDING needs to be adjusted so CRC32 could work.");
static const constexpr uint32_t EEPROM_LLAMA_VARCOUNT = sizeof(eeprom_llama_map) / sizeof(eeprom_entry_t);
static const constexpr uint32_t EEPROM_LLAMA_DATASIZE = sizeof(eeprom_llama_vars_t);


// Llama eeprom variable defaults
static const eeprom_llama_vars_t eeprom_llama_var_defaults = {
    EEPROM_LLAMA_VERSION,   // EEPROM_LLAMA_VERSION
    0,                      // EEPROM_LLAMA_EXTRUDER_TYPE
    325.f,                  // EEPROM_LLAMA_EXTRUDER_ESTEPS
    0,                      // EEPROM_LLAMA_HOTEND_FAN_SPEED
    0,                      // EEPROM_LLAMA_SKEW_ENABLED
    0.f,                    // EEPROM_LLAMA_SKEW_XY
    0.f,                    // EEPROM_LLAMA_SKEW_XZ
    0.f,                    // EEPROM_LLAMA_SKEW_YZ
    "",                     // EEVAR_LLAMA__PADDING
    0xffffffff,             // EEVAR_LLAMA_CRC32
};


extern uint8_t eeprom_llama_get_var_count(void);
// equivalent functions for Llama
extern void eeprom_llama_init(void);
extern void eeprom_llama_defaults(void);
extern variant8_t eeprom_llama_get_var(uint8_t id);
extern void eeprom_llama_set_var(uint8_t id, variant8_t var, bool update_crc32);

void llama_apply_fan_settings();
float llama_compute_extruder_esteps();
void llama_apply_extruder_settings();
void llama_apply_skew_settings();
