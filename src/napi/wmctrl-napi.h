#include "../../libwmctrl/src/wmctrl.h"
#include "../../node_modules/node-addon-api/src/node_api.h"

extern Display *disp_client_read;

//UTILS
bool create_window(napi_env env, napi_value *win_js, struct window_info *wi);
//void create_display(napi_env env, Display **disp);

//PROVIDE-WINDOW
napi_value getWindowList(napi_env env, napi_callback_info info);
napi_value getActiveWindow(napi_env env, napi_callback_info info);
napi_value getWindowsByPid(napi_env env, napi_callback_info info);
napi_value getWindowsByClassName(napi_env env, napi_callback_info info);

//ACTIVE-CLOSE
napi_value activeWindowById(napi_env env, napi_callback_info info);
napi_value activeWindowsByClassName(napi_env env, napi_callback_info info);
napi_value activeWindowsByPid(napi_env env, napi_callback_info info);

napi_value closeWindowById(napi_env env, napi_callback_info info);
napi_value closeWindowsByClassName(napi_env env, napi_callback_info info);
napi_value closeWindowsByPid(napi_env env, napi_callback_info info);

//WINDOW-MOVE
/*int window_to_desktop (Display *disp, Window win, int desktop);
int window_to_current_desktop(Display *disp, Window win);
int window_move_resize (Display *disp, Window win, unsigned long grav, 
    unsigned long x, unsigned long y, 
    unsigned long w, unsigned long h);*/