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

#include "sound.hpp"
#include "../../lib/Marlin/Marlin/src/gcode/gcode.h"

#include "M300.hpp"
#include <stdint.h>

/**
 * M300: Play beep sound S<frequency Hz> P<duration ms>
 */
void PrusaGcodeSuite::M300() {
    uint16_t const frequency = parser.ushortval('S', 260);
    uint16_t duration = parser.ushortval('P', 1000);

    // Limits the tone duration to 0-5 seconds.
    //NOMORE(duration, 5000U);

    Sound::getInstance().buzz(frequency, duration);
}
