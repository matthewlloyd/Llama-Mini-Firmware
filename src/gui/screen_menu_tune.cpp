// screen_menu_tune.cpp

#include "gui.hpp"
#include "screen_menu.hpp"
#include "screen_menus.hpp"
#include "marlin_client.h"
#include "MItem_print.hpp"
#include "MItem_tools.hpp"
#include "MItem_menus.hpp"

/*****************************************************************************/
//parent alias
using Screen = ScreenMenu<EHeader::Off, EFooter::On, HelpLines_None, MI_RETURN, MI_BABYSTEP, MI_M600, MI_SPEED, MI_NOZZLE,
    MI_HEATBED, MI_PRINTFAN, MI_FLOWFACT, MI_SOUND_MODE, MI_LAN_SETTINGS, MI_TIMEZONE, MI_VERSION_INFO,
#ifdef _DEBUG
    MI_TEST,
#endif //_DEBUG
    MI_MESSAGES>;

class ScreenMenuTune : public Screen {
public:
    constexpr static const char *label = N_("TUNE");
    ScreenMenuTune()
        : Screen(_(label)) {
        //todo test if needed
        //marlin_update_vars(MARLIN_VAR_MSK_TEMP_TARG | MARLIN_VAR_MSK(MARLIN_VAR_Z_OFFSET) | MARLIN_VAR_MSK(MARLIN_VAR_FANSPEED) | MARLIN_VAR_MSK(MARLIN_VAR_PRNSPEED) | MARLIN_VAR_MSK(MARLIN_VAR_FLOWFACT));
    }
    virtual void windowEvent(window_t *sender, uint8_t ev, void *param) override;
};

void ScreenMenuTune::windowEvent(window_t *sender, uint8_t event, void *param) {
    if (marlin_all_axes_homed() && marlin_all_axes_known() && (marlin_command() != MARLIN_CMD_G28) && (marlin_command() != MARLIN_CMD_G29) && (marlin_command() != MARLIN_CMD_M109) && (marlin_command() != MARLIN_CMD_M190)) {
        Item<MI_M600>().Enable();
    } else {
        Item<MI_M600>().Disable();
    }
    Screen::windowEvent(sender, event, param);
}

ScreenFactory::UniquePtr GetScreenMenuTune() {
    return ScreenFactory::Screen<ScreenMenuTune>();
}
