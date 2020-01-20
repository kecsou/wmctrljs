#include "../../libwmctrl/src/wmctrl.h"
#include <node_api.h>

//SCREEN
napi_value getScreen(napi_env env, napi_callback_info infos);

//UTILS
bool create_window(napi_env env, napi_value *win_js, struct window_info *wi);
void handling_libwmctrl_error(napi_env env, char *fnName, enum STATES st);
bool set_key_value_int(napi_env env, napi_value *obj, const char *key, int64_t value);

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
*/
napi_value windowMoveResize(napi_env env, napi_callback_info info);
napi_value windowMinimize(napi_env env, napi_callback_info info);

//WINDOW-EDIT-PROPERTIES
napi_value windowState(napi_env env, napi_callback_info info);
napi_value windowAllowAllSizes(napi_env env, napi_callback_info info);