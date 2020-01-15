#include "wmctrl-napi.h"
#include <stdio.h>

Display *disp_client_read = NULL;

napi_value initialise_client_read(napi_env env, napi_callback_info info) {
    if (! (disp_client_read = XOpenDisplay(NULL))) {
        napi_throw_error(env, NULL, "Can't open dispay");
        return NULL;
    }
    return NULL;
}

napi_value free_disp_client_read(napi_env env, napi_callback_info info) {
    if (disp_client_read) {
        XCloseDisplay(disp_client_read);
        disp_client_read = NULL;
    }
    return NULL;
}

napi_value init_all (napi_env env, napi_value exports) {
    napi_value getScreenFn;
    napi_create_function(env, NULL, 0, getScreen, NULL, &getScreenFn);
    napi_set_named_property(env, exports, "getScreen", getScreenFn);

    napi_value initialise_clientFn, 
    free_disp_client_readFn;
    napi_create_function(env, NULL, 0, initialise_client_read, NULL, &initialise_clientFn);
    napi_set_named_property(env, exports, "initialiseClient", initialise_clientFn);
    napi_create_function(env, NULL, 0, free_disp_client_read, NULL, &free_disp_client_readFn);
    napi_set_named_property(env, exports, "freeDispClient", free_disp_client_readFn);

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

    napi_value windowMoveResizeFn, windowStateFn;
    napi_create_function(env, NULL, 0, windowMoveResize, NULL, &windowMoveResizeFn);
    napi_set_named_property(env, exports, "windowMoveResize", windowMoveResizeFn);
    napi_create_function(env, NULL, 0, windowState, NULL, &windowStateFn);
    napi_set_named_property(env, exports, "windowState", windowStateFn);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init_all)