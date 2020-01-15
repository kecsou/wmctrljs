#include "wmctrl.h"

Screen *get_screen(Display *disp, enum STATES *st) {
    bool dispLocal = false;
    disp = create_display(disp, &dispLocal);
    if (!disp) {
        *st = CAN_NOT_OPEN_DISPLAY;
        return NULL;
    }
    Screen *sc = DefaultScreenOfDisplay(disp);
    free_local_display(disp, dispLocal);
    *st = SCREEN_GET;
    return sc;
}