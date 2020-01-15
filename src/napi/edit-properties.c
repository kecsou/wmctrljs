#include "wmctrl-napi.h"

napi_value windowState(napi_env env, napi_callback_info info) {
        napi_value args[4];
    size_t argc = 4;

    napi_value success, failure;
    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 4) {
        napi_throw_error(env, "EINVAL", "[windowState] Must have args [win_id, action, prop1, prop2]");
        return failure;
    }

    int32_t win_id;
    size_t len_action, len_prop1, len_prop2;
    char action[64];
    char prop1[64];
    char prop2[64];

    if (napi_get_value_int32(env, args[0], &win_id) != napi_ok) {
        napi_throw_error(env, NULL, "[windowState] Can't get win_id_js");
        return failure;
    }

    if (napi_get_value_string_utf8(env, args[1], (char *)&action, 64, &len_action) != napi_ok) {
        napi_throw_error(env, NULL, "[windowState] Can't get action_js");
        return failure;
    }

    if (napi_get_value_string_utf8(env, args[2], (char *)&prop1, 64, &len_prop1) != napi_ok) {
        napi_throw_error(env, NULL, "[windowState] Can't get prop1_js");
        return failure;
    }

    if (napi_get_value_string_utf8(env, args[3], (char *)&prop2, 64, &len_prop2) != napi_ok) {
        napi_throw_error(env, NULL, "[windowState] Can't get prop2_js");
        return failure;
    }

    unsigned long action_number;
    if (strcmp(action, "REMOVE") == 0)
        action_number = _NET_WM_STATE_REMOVE;
    else if (strcmp(action, "ADD") == 0)
        action_number = _NET_WM_STATE_ADD;
    else if (strcmp(action, "TOGGLE") == 0)
        action_number = _NET_WM_STATE_TOGGLE;
    else {
        char msg[128];
        sprintf(msg, "[windowState] bad action provided (%s)", action);
        napi_throw_error(env, NULL, msg);
        return failure;
    }

    enum STATES st = window_state(NULL, win_id, action_number, prop1, 
        strcmp(prop2, "") == 0 ? NULL : prop2);

    if (st != WINDOW_STATE_SET) {
        handling_libwmctrl_error(env, "windowState", st);
        return failure;
    }

    return success;
}