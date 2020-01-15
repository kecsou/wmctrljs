#include "wmctrl-napi.h"

napi_value windowMoveResize(napi_env env, napi_callback_info info) {
    napi_value args[6];
    size_t argc = 6;

    napi_value success, failure;
    napi_create_int32(env, 1, &success);
    napi_create_int32(env, 0, &failure);

    if (napi_get_cb_info(env, info, &argc, args, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "Can't get function parameters");
        return failure;
    }

    if (argc != 6) {
        napi_throw_error(env, "EINVAL", "[windowMoveResize] Must have args [win_id, gravity, x, y, width, height]");
        return failure;
    }

    int32_t win_id, gravity, x, y, width, height;
    if (napi_get_value_int32(env, args[0], &win_id) != napi_ok) {
        napi_throw_error(env, NULL, "[windowMoveResize] Can't get win_id_js");
        return failure;
    }

    if (napi_get_value_int32(env, args[1], &gravity) != napi_ok) {
        napi_throw_error(env, NULL, "[windowMoveResize] Can't get gravity_js");
        return failure;
    }

    if (napi_get_value_int32(env, args[2], &x) != napi_ok) {
        napi_throw_error(env, NULL, "[windowMoveResize] Can't get x_js");
        return failure;
    }

    if (napi_get_value_int32(env, args[3], &y) != napi_ok) {
        napi_throw_error(env, NULL, "[windowMoveResize] Can't get y_js");
        return failure;
    }

    if (napi_get_value_int32(env, args[4], &width) != napi_ok) {
        napi_throw_error(env, NULL, "[windowMoveResize] Can't get width_js");
        return failure;
    }

    if (napi_get_value_int32(env, args[5], &height) != napi_ok) {
        napi_throw_error(env, NULL, "[windowMoveResize] Can't get height_js");
        return failure;
    }

    enum STATES st = window_move_resize(NULL, win_id, gravity, x, y, width, height);
    if (st != WINDOW_MOVED_RESIZED) {
        handling_libwmctrl_error(env, "windowMoveResize", st);
        return failure;
    }

    return success;
}