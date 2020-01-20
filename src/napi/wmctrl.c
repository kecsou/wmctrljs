#include "wmctrl-napi.h"
#include <stdio.h>

napi_value initialise_wmctrl_lib(napi_env env, napi_callback_info info) {
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (init_wmctrl_lib())
        return success;

    return failure;
}

napi_value init_all (napi_env env, napi_value exports) {
    napi_value getScreenFn;
    napi_create_function(env, NULL, 0, getScreen, NULL, &getScreenFn);
    napi_set_named_property(env, exports, "getScreen", getScreenFn);

    napi_value initialise_wmctrl_libFn;
    napi_create_function(env, NULL, 0, initialise_wmctrl_lib, NULL, &initialise_wmctrl_libFn);
    napi_set_named_property(env, exports, "initialise_wmctrl_lib", initialise_wmctrl_libFn);

    napi_value getWindowListFn, 
    getActiveWindowFn,
    getWindowsByPidFn, 
    getWindowsByClassNameFn;
    napi_create_function(env, NULL, 0, getWindowList, NULL, &getWindowListFn);
    napi_set_named_property(env, exports, "getWindowList", getWindowListFn);
    napi_create_function(env, NULL, 0, getActiveWindow, NULL, &getActiveWindowFn);
    napi_set_named_property(env, exports, "getActiveWindow", getActiveWindowFn);
    napi_create_function(env, NULL, 0, getWindowsByPid, NULL, &getWindowsByPidFn);
    napi_set_named_property(env, exports, "getWindowsByPid", getWindowsByPidFn);
    napi_create_function(env, NULL, 0, getWindowsByClassName, NULL, &getWindowsByClassNameFn);
    napi_set_named_property(env, exports, "getWindowsByClassName", getWindowsByClassNameFn);

    napi_value activeWindowByIdFn, 
    activeWindowsByClassNameFn, 
    activeWindowsByPidFn;
    napi_create_function(env, NULL, 0, activeWindowById, NULL, &activeWindowByIdFn);
    napi_set_named_property(env, exports, "activeWindowById", activeWindowByIdFn);
    napi_create_function(env, NULL, 0, activeWindowsByClassName, NULL, &activeWindowsByClassNameFn);
    napi_set_named_property(env, exports, "activeWindowsByClassName", activeWindowsByClassNameFn);
    napi_create_function(env, NULL, 0, activeWindowsByPid, NULL, &activeWindowsByPidFn);
    napi_set_named_property(env, exports, "activeWindowsByPid", activeWindowsByPidFn);

    napi_value closeWindowByIdFn,
    closeWindowsByClassNameFn, 
    closeWindowsByPidFn;
    napi_create_function(env, NULL, 0, closeWindowById, NULL, &closeWindowByIdFn);
    napi_set_named_property(env, exports, "closeWindowById", closeWindowByIdFn);
    napi_create_function(env, NULL, 0, closeWindowsByClassName, NULL, &closeWindowsByClassNameFn);
    napi_set_named_property(env, exports, "closeWindowsByClassName", closeWindowsByClassNameFn);
    napi_create_function(env, NULL, 0, closeWindowsByPid, NULL, &closeWindowsByPidFn);
    napi_set_named_property(env, exports, "closeWindowsByPid", closeWindowsByPidFn);

    napi_value windowMoveResizeFn, windowStateFn, windowMinimizeFn, windowAllowAllSizesFn;
    napi_create_function(env, NULL, 0, windowMoveResize, NULL, &windowMoveResizeFn);
    napi_set_named_property(env, exports, "windowMoveResize", windowMoveResizeFn);
    napi_create_function(env, NULL, 0, windowState, NULL, &windowStateFn);
    napi_set_named_property(env, exports, "windowState", windowStateFn);
    napi_create_function(env, NULL, 0, windowMinimize, NULL, &windowMinimizeFn);
    napi_set_named_property(env, exports, "windowMinimize", windowMinimizeFn);
    napi_create_function(env, NULL, 0, windowAllowAllSizes, NULL, &windowAllowAllSizesFn);
    napi_set_named_property(env, exports, "windowAllowAllSizes", windowAllowAllSizesFn);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init_all)