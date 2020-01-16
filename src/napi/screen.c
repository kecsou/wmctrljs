#include "wmctrl-napi.h"

napi_value getScreen(napi_env env, napi_callback_info infos) {
    enum STATES st;
    Screen *sc = get_screen(NULL, &st);
    if (!sc || st != SCREEN_GET) {
        handling_libwmctrl_error(env, "getScreen", st);
        return NULL;
    }

    napi_value screen;
    if (napi_create_object(env, &screen) != napi_ok) {
        napi_throw_error(env, NULL, "Can't create screen_js object");
        return NULL;
    }

    if (!set_key_value_int(env, &screen, "backing_store", sc->backing_store))
        return NULL;

    if (!set_key_value_int(env, &screen, "black_pixel", sc->black_pixel))
        return NULL;

    if (!set_key_value_int(env, &screen, "cmap", sc->cmap))
        return NULL;

    if (!set_key_value_int(env, &screen, "height", sc->height))
        return NULL;

    if (!set_key_value_int(env, &screen, "max_maps", sc->max_maps))
        return NULL;

    if (!set_key_value_int(env, &screen, "mheight", sc->mheight))
        return NULL;

    if (!set_key_value_int(env, &screen, "min_maps", sc->min_maps))
        return NULL;

    if (!set_key_value_int(env, &screen, "mwidth", sc->mwidth))
        return NULL;

    if (!set_key_value_int(env, &screen, "ndepths", sc->ndepths))
        return NULL;

    if (!set_key_value_int(env, &screen, "root_depth", sc->root_depth))
        return NULL;

    if (!set_key_value_int(env, &screen, "root_input_mask", sc->root_input_mask))
        return NULL;

    if (!set_key_value_int(env, &screen, "save_unders", sc->save_unders))
        return NULL;

    if (!set_key_value_int(env, &screen, "white_pixel", sc->white_pixel))
        return NULL;

    if (!set_key_value_int(env, &screen, "width", sc->width))
        return NULL;

    free_screen(sc);
    return screen;
}