#include "../../lib/Marlin/Marlin/src/inc/MarlinConfig.h"
#include "../../lib/Marlin/Marlin/src/gcode/gcode.h"
#include "../../lib/Marlin/Marlin/src/gcode/queue.h"

#include "PrusaGcodeSuite.hpp"
#include "../common/marlin_server.h"

#include "M330.h"
#include "M50.hpp"

bool GcodeSuite::process_parsed_command_custom(bool no_ok) {
    switch (parser.command_letter) {
    case 'M':
        switch (parser.codenum) {
        case 50:
            PrusaGcodeSuite::M50(); //selftest
            return true;
        case 117:
            PrusaGcodeSuite::M117();
            if (!no_ok) queue.ok_to_send();
            return true;
        case 300:
            PrusaGcodeSuite::M300();
            if (!no_ok) queue.ok_to_send();
            return true;
        case 301:
            M301();
            marlin_server_settings_save_noz_pid();
            if (!no_ok) queue.ok_to_send();
            return true;
        case 303: {
            M303();
            const int16_t e = parser.intval('E');
            const bool u = parser.boolval('U');
            if (u) {
                if (e == -1)
                    marlin_server_settings_save_bed_pid();
                else if (e == 0)
                    marlin_server_settings_save_noz_pid();
            }
            if (!no_ok)
                queue.ok_to_send();
            return true;
        }
        case 304:
            M304();
            marlin_server_settings_save_bed_pid();
            if (!no_ok) queue.ok_to_send();
            return true;
#if defined(_DEBUG)
        case 330:
            PrusaGcodeSuite::M330();
            return true;
        case 331:
            PrusaGcodeSuite::M331();
            return true;
        case 332:
            PrusaGcodeSuite::M332();
            return true;
        case 333:
            PrusaGcodeSuite::M333();
            return true;
        case 334:
            PrusaGcodeSuite::M334();
            return true;
#endif // _DEBUG

        case 997:
            PrusaGcodeSuite::M997();
            return true;

#ifdef M999_MCU_RESET
        case 999:
            if (parser.seen('R')) {
                PrusaGcodeSuite::M999();
                return true;
            } else {
                return false;
            }
        case 1400:
            PrusaGcodeSuite::M1400();
            return true;
#endif

        default:
            return false;
        }
        return false;
    case 'G':
        switch (parser.codenum) {
        case 26:
            PrusaGcodeSuite::G26();
            return true;
        case 162:
            PrusaGcodeSuite::G162();
            return true;
        }
        return false;
    default:
        return false;
    }
}
