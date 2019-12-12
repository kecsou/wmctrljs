#include "../../libwmctrl/src/wmctrl.h"
#include "../../node_modules/node-addon-api/src/node_api.h"

//UTILS
bool create_window(napi_env env, napi_value *win_js, struct window_info *wi);

//WINDOW-EDIT
void activeWindowById(napi_env env, napi_callback_info info);

//WINDOW-MOVE
/*int window_to_desktop (Display *disp, Window win, int desktop);
int window_to_current_desktop(Display *disp, Window win);
int window_move_resize (Display *disp, Window win, unsigned long grav, 
    unsigned long x, unsigned long y, 
    unsigned long w, unsigned long h);*/