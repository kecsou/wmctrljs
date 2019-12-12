#include "wmctrl-napi.h"

bool set_key_value_int(napi_env env, napi_value *obj, const char *key, int64_t value) {
    napi_value key_js;
    napi_value int_value_js;

    if (napi_create_int64(env, value, &int_value_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't create int_value_js [set_key_value_int]");
        return false;
    }

    if (napi_create_string_utf8(env, key, strlen(key), &key_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't create string key_js [set_key_value_int]");
        return false;
    }

    if (napi_set_property(env, *obj, key_js, int_value_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't set int_value_js [set_key_value_int]");
        return false;
    }
    return true;
}

bool set_key_value_str(napi_env env, napi_value *obj, const char *key, const char *value) {
    napi_value str_key_js;
    napi_value str_value_js;
    const char *val = value ? value : "";

    if (napi_create_string_utf8(env, val, strlen(val), &str_value_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't create str_value_js [set_key_value_str]");
        return false;
    }

    if (napi_create_string_utf8(env, key, strlen(key), &str_key_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't create str_key_js [set_key_value_str]");
        return false;
    }

    if (napi_set_property(env, *obj, str_key_js, str_value_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't set str_value_js [set_key_value_str]");
        return false;
    }
    return true;
}

bool set_object(napi_env env, napi_value obj1, const char *key, napi_value obj2) {
    if (!key) {
        napi_throw_error(env, NULL, "Can't use null key [set_object]");
        return false;
    }

    if (napi_set_named_property(env, obj1, key, obj2) != napi_ok) {
        char msg[strlen(key) + 512];
        sprintf(msg, "Can't set property %s to obj1 [set_object]", key);
        napi_throw_error(env, NULL, msg);
        return false;
    }

    return true;
}

struct napi_desc {
        char *flag;
        Atom number;
};
bool set_key_value_array_obj_desc(napi_env env, napi_value *obj, const char *key, struct napi_desc *arr, size_t size) {
    napi_value array_js;
    if (!arr) {
        napi_throw_error(env, NULL, "Can't use NULL arr [set_key_value_array_obj_desc]");
        return false;
    }

    if (napi_create_array_with_length(env, size, &array_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't create array_js [set_key_value_array_obj_desc]");        
        return false;
    }

    for (size_t i = 0; i < size; i++) {
        napi_value obj_desc_js, flag_js, number_js;
        struct napi_desc *desc = arr + i;
        if (napi_create_object(env, &obj_desc_js) != napi_ok) {
            napi_throw_error(env, NULL, "Can't create obj_desc_js [set_key_value_array_obj_desc]");
            return false;
        }

        if (napi_create_int64(env, desc->number, &number_js) != napi_ok) {
            napi_throw_error(env, NULL, "Can't create number_js [set_key_value_array_obj_desc]");
            return false;
        }

        if (napi_create_string_utf8(env, desc->flag, strlen(desc->flag), &flag_js) != napi_ok) {
            napi_throw_error(env, NULL, "Can't create flag_js [set_key_value_array_obj_desc]");
            return false;
        }

        if (!set_object(env, obj_desc_js, "flag", flag_js)) {
            napi_throw_error(env, NULL, "Can't set flag as property [set_key_value_array_obj_desc]");
            return false;
        }

        if (!set_object(env, obj_desc_js, "number", number_js) != napi_ok) {
            napi_throw_error(env, NULL, "Can't set number_js to property number [set_key_value_array_obj_desc]");
            return false;
        }
        napi_set_element(env, array_js, i, obj_desc_js);
    }

    if (!set_object(env, *obj, key, array_js)) {
        napi_throw_error(env, NULL, "Can't set array_js to object [set_key_value_array_obj_desc]");
        return false;
    }
    return true;
}

bool create_window(napi_env env, napi_value *win_js, struct window_info *wi) {
    if (napi_create_object(env, win_js) != napi_ok) {
        napi_throw_error(env, NULL, "Can't create win_js object");
        return false;
    }

    if (!set_key_value_int(env, win_js, "win_id", wi->win_id))
        return false;

    if (!set_key_value_int(env, win_js, "win_pid", wi->win_pid))
        return false;

    if (!set_key_value_int(env, win_js, "desktop_number", wi->desktop_number))
        return false;

    if (!set_key_value_int(env, win_js, "showing_desktop", wi->showing_desktop))
        return false;

    if (!set_key_value_str(env, win_js, "win_client_machine", wi->win_client_machine))
        return false;

    if (!set_key_value_str(env, win_js, "win_class", wi->win_class))
        return false;

    if (wi->nbr_type)
        if (!set_key_value_array_obj_desc(env, win_js, "win_types", (struct napi_desc *)wi->win_types, wi->nbr_type))
            return false;

    if (wi->nbr_action)
        if (!set_key_value_array_obj_desc(env, win_js, "win_actions", (struct napi_desc *)wi->win_actions, wi->nbr_action))
            return false;

    if (wi->nbr_state)
        if (!set_key_value_array_obj_desc(env, win_js, "win_states", (struct napi_desc *)wi->win_states, wi->nbr_state))
            return false;

    if (!set_key_value_str(env, win_js, "win_name", wi->win_name))
        return false;

    if (!set_key_value_str(env, win_js, "win_visible_name", wi->win_visible_name))
        return false;

    if (!set_key_value_str(env, win_js, "win_icon_name", wi->win_icon_name))
        return false;

    if (!set_key_value_str(env, win_js, "win_visible_icon_name", wi->win_visible_icon_name))
        return false;

    if (!set_key_value_int(env, win_js, "wm_normal_hints_supplied", wi->wm_normal_hints_supplied))
        return false;

    if (!set_key_value_int(env, win_js, "wm_hints_supplied", wi->wm_hints_supplied))
        return false;

    if (wi->win_geometry) {
        napi_value obj_geometry_js;
        if (napi_create_object(env, &obj_geometry_js) != napi_ok) {
            napi_throw_error(env, NULL, "Can't create object obj_geometry_js [create_window]");
            return false;
        }

        if (!set_key_value_int(env, &obj_geometry_js, "width", wi->win_geometry->width))
            return false;

        if (!set_key_value_int(env, &obj_geometry_js, "height", wi->win_geometry->height))
            return false;

        if (!set_key_value_int(env, &obj_geometry_js, "x", wi->win_geometry->x))
            return false;

        if (!set_key_value_int(env, &obj_geometry_js, "y", wi->win_geometry->y))
            return false;

        if (!set_object(env, *win_js, "win_geometry", obj_geometry_js))
            return false;
    }

    if (wi->WM_NORMAL_HINTS) {
        napi_value obj_wm_normal_hints;
        if (napi_create_object(env, &obj_wm_normal_hints) != napi_ok) {
            napi_throw_error(env, NULL, "Can't create object obj_wm_normal_hints [create_window]");
            return false;
        }

        if (!set_key_value_int(env, &obj_wm_normal_hints, "win_gravity", wi->WM_NORMAL_HINTS->win_gravity))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "height", wi->WM_NORMAL_HINTS->height))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "width", wi->WM_NORMAL_HINTS->width))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "x", wi->WM_NORMAL_HINTS->x))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "y", wi->WM_NORMAL_HINTS->y))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "base_width", wi->WM_NORMAL_HINTS->base_width))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "base_height", wi->WM_NORMAL_HINTS->base_height))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "min_width", wi->WM_NORMAL_HINTS->min_width))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "min_height", wi->WM_NORMAL_HINTS->min_height))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "width_inc", wi->WM_NORMAL_HINTS->width_inc))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "height_inc", wi->WM_NORMAL_HINTS->height_inc))
            return false;

        napi_value wm_normal_min_aspect_js;
            if (napi_create_object(env, &wm_normal_min_aspect_js) != napi_ok) {
                napi_throw_error(env, NULL, "Can't create objecr wm_normal_min_aspect_js [create_window]");
                return false;
            }

            if (!set_key_value_int(env, &wm_normal_min_aspect_js, "numerator", wi->WM_NORMAL_HINTS->min_aspect.x))
                return false;

            if (!set_key_value_int(env, &wm_normal_min_aspect_js, "denominator", wi->WM_NORMAL_HINTS->min_aspect.y))
                return false;
            if (!set_object(env, obj_wm_normal_hints, "min_aspect", wm_normal_min_aspect_js))
                return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "min_height", wi->WM_NORMAL_HINTS->min_height))
            return false;

        if (!set_key_value_int(env, &obj_wm_normal_hints, "min_height", wi->WM_NORMAL_HINTS->min_height))
            return false;

        napi_value wm_normal_max_aspect_js;
            if (napi_create_object(env, &wm_normal_max_aspect_js) != napi_ok) {
                napi_throw_error(env, NULL, "Can't create objecr wm_normal_max_aspect_js [create_window]");
                return false;
            }

            if (!set_key_value_int(env, &wm_normal_max_aspect_js, "numerator", wi->WM_NORMAL_HINTS->max_aspect.x))
                return false;

            if (!set_key_value_int(env, &wm_normal_max_aspect_js, "denominator", wi->WM_NORMAL_HINTS->max_aspect.y))
                return false;

            if (!set_key_value_int(env, &obj_wm_normal_hints, "min_height", wi->WM_NORMAL_HINTS->min_height))
                return false;

            if (!set_key_value_int(env, &obj_wm_normal_hints, "min_height", wi->WM_NORMAL_HINTS->min_height))
                return false;

            if (!set_object(env, obj_wm_normal_hints, "max_aspect", wm_normal_max_aspect_js))
                return false;

        if (!set_object(env, *win_js, "WM_NORMAL_HINTS", obj_wm_normal_hints))
            return false;
    }

    if (wi->WM_HINTS) {
        napi_value obj_wm_hints;
        if (napi_create_object(env, &obj_wm_hints) != napi_ok) {
            napi_throw_error(env, NULL, "Can't create object obj_wm_hints [create_window]");
            return false;
        }

        if (!set_key_value_int(env, &obj_wm_hints, "win_gravity", wi->WM_HINTS->win_gravity))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "height", wi->WM_HINTS->height))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "width", wi->WM_HINTS->width))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "x", wi->WM_HINTS->x))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "y", wi->WM_HINTS->y))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "base_width", wi->WM_HINTS->base_width))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "base_height", wi->WM_HINTS->base_height))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "min_width", wi->WM_HINTS->min_width))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "min_height", wi->WM_HINTS->min_height))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "width_inc", wi->WM_HINTS->width_inc))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "height_inc", wi->WM_HINTS->height_inc))
            return false;

        napi_value wm_min_aspect_js;
            if (napi_create_object(env, &wm_min_aspect_js) != napi_ok) {
                napi_throw_error(env, NULL, "Can't create objecr wm_min_aspect_js [create_window]");
                return false;
            }

            if (!set_key_value_int(env, &wm_min_aspect_js, "numerator", wi->WM_HINTS->min_aspect.x))
                return false;

            if (!set_key_value_int(env, &wm_min_aspect_js, "denominator", wi->WM_HINTS->min_aspect.y))
                return false;
            if (!set_object(env, obj_wm_hints, "min_aspect", wm_min_aspect_js))
                return false;

        if (!set_key_value_int(env, &obj_wm_hints, "min_height", wi->WM_HINTS->min_height))
            return false;

        if (!set_key_value_int(env, &obj_wm_hints, "min_height", wi->WM_HINTS->min_height))
            return false;

        napi_value wm_max_aspect_js;
            if (napi_create_object(env, &wm_max_aspect_js) != napi_ok) {
                napi_throw_error(env, NULL, "Can't create objecr wm_max_aspect_js [create_window]");
                return false;
            }

            if (!set_key_value_int(env, &wm_max_aspect_js, "numerator", wi->WM_HINTS->max_aspect.x))
                return false;

            if (!set_key_value_int(env, &wm_max_aspect_js, "denominator", wi->WM_HINTS->max_aspect.y))
                return false;

            if (!set_key_value_int(env, &obj_wm_hints, "min_height", wi->WM_HINTS->min_height))
                return false;

            if (!set_key_value_int(env, &obj_wm_hints, "min_height", wi->WM_HINTS->min_height))
                return false;

            if (!set_object(env, obj_wm_hints, "max_aspect", wm_max_aspect_js))
                return false;

        if (!set_object(env, *win_js, "WM_HINTS", obj_wm_hints))
            return false;
    }
    return true;
}