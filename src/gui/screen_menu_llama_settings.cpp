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

#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "screen_menus.hpp"
#include "menu_spin_config.hpp"
#include "llama.h"
#include "extruder_enum.h"


using Screen = ScreenMenu<EHeader::Off, EFooter::On, HelpLines_None, MI_RETURN,
    MI_EXTRUDER_TYPE, MI_EXTRUDER_ESTEPS, MI_HOTEND_FAN_SPEED,
    MI_SKEW_ENABLED, MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ>
;

class ScreenMenuLlamaSettings : public Screen {
public:
    constexpr static const char *label = N_("LLAMA");
    ScreenMenuLlamaSettings()
        : Screen(_(label)) {
        last_extruder_type = variant_get_ui8(eeprom_llama_get_var(EEVAR_LLAMA_EXTRUDER_TYPE));
    }

    uint8_t last_extruder_type;

    virtual void windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) override {
        if (ev != GUI_event_t::CHILD_CLICK) {
            SuperWindowEvent(sender, ev, param);
            return;
        }

        int extruder_type = variant_get_ui8(eeprom_llama_get_var(EEVAR_LLAMA_EXTRUDER_TYPE));
        if (extruder_type != last_extruder_type) {
            last_extruder_type = extruder_type;
            Item<MI_EXTRUDER_ESTEPS>().SetVal(llama_compute_extruder_esteps());
            if (extruder_type == eEXTRUDER_TYPE::EXTRUDER_TYPE_USER_USE_M92)
                Item<MI_EXTRUDER_ESTEPS>().Enable();
            else
                Item<MI_EXTRUDER_ESTEPS>().Disable();
            menu.unconditionalDrawItem(1);
            menu.unconditionalDrawItem(2);
        }
    }
};

ScreenFactory::UniquePtr GetScreenMenuLlamaSettings() {
    return ScreenFactory::Screen<ScreenMenuLlamaSettings>();
}
