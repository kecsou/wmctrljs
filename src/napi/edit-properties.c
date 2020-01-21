#include "wmctrl-napi.h"

napi_value windowState(napi_env env, napi_callback_info info) {
    napi_value args[4];
    napi_value success, failure;
    int32_t win_id;
    size_t len_action, len_prop1, len_prop2;
    char action[64];
    char prop1[64];
    char prop2[64];
    char msg[128];
    unsigned long action_number;
    enum STATES st;
    size_t argc = 4;

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

    if (strcmp(action, "REMOVE") == 0)
        action_number = _NET_WM_STATE_REMOVE;
    else if (strcmp(action, "ADD") == 0)
        action_number = _NET_WM_STATE_ADD;
    else if (strcmp(action, "TOGGLE") == 0)
        action_number = _NET_WM_STATE_TOGGLE;
    else {
        sprintf(msg, "[windowState] bad action provided (%s)", action);
        napi_throw_error(env, NULL, msg);
        return failure;
    }

    st = window_state(NULL, win_id, action_number, prop1, 
        strcmp(prop2, "") == 0 ? NULL : prop2);

    if (st != WINDOW_STATE_SET) {
        handling_libwmctrl_error(env, "windowState", st);
        return failure;
    }

    return success;
}

napi_value windowAllowAllSizes(napi_env env, napi_callback_info info) {
    napi_value args[1];
    size_t argc = 1;
    int32_t win_id;
    enum STATES st;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "[windowAllowAllSizes] Must have arg [win_id]");
        return failure;
    }

    if (napi_get_value_int32(env, args[0], &win_id) != napi_ok) {
        napi_throw_error(env, NULL, "[windowAllowAllSizes] Can't get win_id from js");
        return failure;
    }

    st = window_allow_all_sizes(NULL, win_id);
    if (st != WINDOW_ALLOWED_ALL_SIZE) {
        handling_libwmctrl_error(env, "windowAllowAllSizes", st);
        return failure;
    }

    return success;
}

napi_value windowRaise(napi_env env, napi_callback_info info) {
    napi_value args[1];
    size_t argc = 1;
    int32_t win_id;
    enum STATES st;
    napi_value success, failure;

    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 1) {
        napi_throw_error(env, "EINVAL", "[windowRaise] Must have arg [win_id]");
        return failure;
    }

    if (napi_get_value_int32(env, args[0], &win_id) != napi_ok) {
        napi_throw_error(env, NULL, "[windowRaise] Can't get win_id from js");
        return failure;
    }

    st = window_raise(NULL, win_id);
    if (st != WINDOW_RAISED) {
        handling_libwmctrl_error(env, "windowRaise", st);
        return failure;
    }

    return success;
}