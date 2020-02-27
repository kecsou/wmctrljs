#include "wmctrl-napi.hpp"
#include <stdio.h>

using namespace Napi;

namespace wmctrljs {
    Display *disp = NULL;
    void sync() {
        if (wmctrljs::disp)
            XSync(wmctrljs::disp, false);
    }
}

napi_value initialise_wmctrl_lib(const CallbackInfo &info) {  
    Env env = info.Env();
    
    //init_wmctrl_lib();
    
    /*if (!thread_initialized) {
        thread_initialized = 
        if (!thread_initialized)
            return Boolean::New(env, false);
    }

    if (!disp_initialized) {
        wmctrljs::disp = XOpenDisplay(NULL);
        if (!wmctrljs::disp)
            return Boolean::New(env, false);
        disp_initialized = true;
    }*/

    return Number::New(env, true);
}

void closeDisplay(const CallbackInfo &info) {
    XCloseDisplay(wmctrljs::disp);
    wmctrljs::disp = NULL;
}

Object Init(Env env, Object exports) {
    XInitThreads();
    XSetErrorHandler(handler_x11_error);
    wmctrljs::disp = XOpenDisplay(NULL);
    exports.Set("initialise_wmctrl_lib", Function::New(env, initialise_wmctrl_lib));
    exports.Set("closeDisplay", Function::New(env, closeDisplay));
    exports.Set("getScreenSync", Function::New(env, getScreenSync));
    exports.Set("getScreenAsync", Function::New(env, getScreenAsync));

    exports.Set("getWindowByIdAsync", Function::New(env, getWindowByIdAsync));

    exports.Set("getActiveWindowSync", Function::New(env, getActiveWindowSync));
    exports.Set("getActiveWindowAsync", Function::New(env, getActiveWindowAsync));

    exports.Set("getWindowListSync", Function::New(env, getWindowListSync));
    exports.Set("getWindowListAsync", Function::New(env, getWindowListAsync));

    exports.Set("getWindowsByClassNameSync", Function::New(env, getWindowsByClassNameSync));
    exports.Set("getWindowsByClassNameAsync", Function::New(env, getWindowsByClassNameAsync));

    exports.Set("getWindowsByPidSync", Function::New(env, getWindowsByPidSync));
    exports.Set("getWindowsByPidAsync", Function::New(env, getWindowsByPidAsync));

    exports.Set("activeWindowByIdSync", Function::New(env, activeWindowByIdSync));
    exports.Set("activeWindowByIdAsync", Function::New(env, activeWindowByIdAsync));
    exports.Set("activeWindowsByPidSync", Function::New(env, activeWindowsByPidSync));
    exports.Set("activeWindowsByPidAsync", Function::New(env, activeWindowsByPidAsync));
    exports.Set("activeWindowsByClassNameSync", Function::New(env, activeWindowsByClassNameSync));
    exports.Set("activeWindowsByClassNameAsync", Function::New(env, activeWindowsByClassNameAsync));

    exports.Set("closeWindowByIdSync", Function::New(env, closeWindowByIdSync));
    exports.Set("closeWindowByIdAsync", Function::New(env, closeWindowByIdAsync));
    exports.Set("closeWindowsByPidSync", Function::New(env, closeWindowsByPidSync));
    exports.Set("closeWindowsByPidAsync", Function::New(env, closeWindowsByPidAsync));
    exports.Set("closeWindowsByClassNameSync", Function::New(env, closeWindowsByClassNameSync));
    exports.Set("closeWindowsByClassNameAsync", Function::New(env, closeWindowsByClassNameAsync));

    exports.Set("windowMoveResizeSync", Function::New(env, windowMoveResizeSync));
    exports.Set("windowMoveResizeAsync", Function::New(env, windowMoveResizeAsync));

    exports.Set("windowMoveResizeTransitionAsync", Function::New(env, windowMoveResizeTransitionAsync));

    exports.Set("windowStateSync", Function::New(env, windowStateSync));
    exports.Set("windowStateAsync", Function::New(env, windowStateAsync));

    exports.Set("windowMinimizeSync", Function::New(env, windowMinimizeSync));
    exports.Set("windowMinimizeAsync", Function::New(env, windowMinimizeAsync));

    exports.Set("windowAllowAllSizesSync", Function::New(env, windowAllowAllSizesSync));
    exports.Set("windowAllowAllSizesAsync", Function::New(env, windowAllowAllSizesAsync));

    exports.Set("windowRaiseSync", Function::New(env, windowRaiseSync));
    exports.Set("windowRaiseAsync", Function::New(env, windowRaiseAsync));
    return exports;
}

NODE_API_MODULE(wmctrl, Init)