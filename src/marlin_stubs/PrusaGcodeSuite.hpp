#pragma once

namespace PrusaGcodeSuite {
void G26();  /// first layer calibration
void G162(); /// handler-specific configuration

void M117(); /// M117 LCD message.

void M300(); /// M300 play sound.
}
