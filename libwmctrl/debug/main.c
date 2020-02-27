#include "../src/wmctrl.h"

int main() {
    Display *disp = XOpenDisplay(NULL);
    close_window_by_id(disp, 0x02200003);
    XSync(disp, false);
    sleep(5);
    XCloseDisplay(disp);
    return 0;
}