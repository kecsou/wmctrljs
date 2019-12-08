#include "./wmctrl.h"

#define _NET_WM_WINDOW_TYPES ((char const*[]) { \
    "_NET_WM_WINDOW_TYPE_DESKTOP", \
    "_NET_WM_WINDOW_TYPE_DOCK", \
    "_NET_WM_WINDOW_TYPE_TOOLBAR", \
    "_NET_WM_WINDOW_TYPE_MENU", \
    "_NET_WM_WINDOW_TYPE_UTILITY", \
    "_NET_WM_WINDOW_TYPE_SPLASH", \
    "_NET_WM_WINDOW_TYPE_DIALOG", \
    "_NET_WM_WINDOW_TYPE_NORMAL" \
})
#define NB_ALLOWED_TYPES 8

#define _NET_WM_ALLOWED_ACTIONS ((char const*[]) { \
    "_NET_WM_ACTION_MOVE", "_NET_WM_ACTION_RESIZE", \
    "_NET_WM_ACTION_MINIMIZE", "_NET_WM_ACTION_SHADE", \
    "_NET_WM_ACTION_STICK", "_NET_WM_ACTION_MAXIMIZE_HORZ", \
    "_NET_WM_ACTION_MAXIMIZE_VERT", "_NET_WM_ACTION_FULLSCREEN",\
    "_NET_WM_ACTION_CHANGE_DESKTOP", "_NET_WM_ACTION_CLOSE" \
})
#define NB_ALLOWED_ACTIONS 10

#define _NET_WM_STATE ((char const*[]) { \
    "_NET_WM_STATE_MODAL", "_NET_WM_STATE_STICKY",\
    "_NET_WM_STATE_MAXIMIZED_VERT", "_NET_WM_STATE_MAXIMIZED_HORZ",\
    "_NET_WM_STATE_SHADED", "_NET_WM_STATE_SKIP_TASKBAR",\
    "_NET_WM_STATE_SKIP_PAGER", "_NET_WM_STATE_HIDDEN",\
    "_NET_WM_STATE_FULLSCREEN", "_NET_WM_STATE_ABOVE",\
    "_NET_WM_STATE_BELOW", "_NET_WM_STATE_DEMANDS_ATTENTION"\
})
#define NB_WM_STATE 12


gchar *get_property (Display *disp, Window win,
        Atom xa_prop_type, gchar *prop_name, size_t *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    size_t tmp_size;
    unsigned char *ret_prop;
    gchar *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    /* MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):
     *
     * long_length = Specifies the length in 32-bit multiples of the
     *               data to be retrieved.
     *
     * NOTE:  see 
     * http://mail.gnome.org/archives/wm-spec-list/2003-March/msg00067.html
     * In particular:
     *
     * 	When the X window system was ported to 64-bit architectures, a
     * rather peculiar design decision was made. 32-bit quantities such
     * as Window IDs, atoms, etc, were kept as longs in the client side
     * APIs, even when long was changed to 64 bits.
     *
     */
    if (XGetWindowProperty(disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
            xa_prop_type, &xa_ret_type, &ret_format,     
            &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        printf("Cannot get %s property.\n", prop_name);
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        //printf("Invalid type of %s property.\n", prop_name);
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / 8) * ret_nitems;
    /* Correct 64 Architecture implementation of 32 bit data */
    if(ret_format==32) tmp_size *= sizeof(long)/4;
    ret = g_malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

//_NET_WM_PID
unsigned long get_window_pid(Display *disp, Window win) {
    unsigned long *wm_pid = NULL;    
    if (! (wm_pid = (unsigned long *)get_property(disp, win,
                    XA_CARDINAL, "_NET_WM_PID", NULL))) {
        return 0;
    }

    unsigned long pid = *wm_pid;
    g_free(wm_pid);
    return pid;
}

//_NET_WM_DESKTOP
unsigned long get_window_desktop(Display *disp, Window win) {
    unsigned long *net_wm_desktop = NULL;
    net_wm_desktop = (unsigned long *)get_property(disp, win, 
        XA_CARDINAL, "_NET_WM_DESKTOP", NULL);
    if (!net_wm_desktop) {
        net_wm_desktop = (unsigned long *)get_property(disp, win,
                    XA_CARDINAL, "_WIN_WORKSPACE", NULL);
        return 0;
    }
    unsigned long desktop = *net_wm_desktop;
    g_free(net_wm_desktop);
    return desktop;
}

//_NET_SHOWING_DESKTOP
unsigned long get_window_shwing_desktop(Display *disp, Window win) {
    unsigned long *ret = NULL;
    if (! (ret = (unsigned long *)get_property(disp, win,
                    XA_CARDINAL, "_NET_SHOWING_DESKTOP", NULL))) {
        return 0;
    }

    unsigned long showing_desktop = *ret;
    g_free(ret);
    return showing_desktop;
}

//WM_CLIENT_MACHINE
gchar *get_window_client_machine(Display *disp, Window win) {
    gchar *wm_client_machine = get_property(disp, win,
                XA_STRING, "WM_CLIENT_MACHINE", NULL);
    return wm_client_machine;
}

//WM_CLASS
gchar *get_window_class(Display *disp, Window win) {
    gchar *class_utf8;
    gchar *wm_class;
    size_t size;

    wm_class = get_property(disp, win, XA_STRING, "WM_CLASS", &size);
    if (wm_class) {
        gchar *p_0 = strchr(wm_class, '\0');
        if (wm_class + size - 1 > p_0) {
            *(p_0) = '.';
        }
        class_utf8 = g_locale_to_utf8(wm_class, -1, NULL, NULL, NULL);
    }
    else {
        class_utf8 = NULL;
    }

    g_free(wm_class);
    return class_utf8;
}

//_NET_WM_WINDOW_TYPE
struct type_desc *get_window_types(Display *disp, Window win, size_t *size) {
    struct type_desc *types = NULL;
    Atom *atoms_type = (Atom *)get_property(disp, win, XA_ATOM,
            "_NET_WM_WINDOW_TYPE", size);
    if (atoms_type) {
        *size = *size / 8;
        size_t counter = 0;
        types = g_malloc(sizeof(struct type_desc) * (*size));
        for (size_t i = 0; i < *size; i++) {
            Atom atom_type = atoms_type[i];
            for (size_t j = 0; j < NB_ALLOWED_TYPES; j++) {
                const char *type = _NET_WM_WINDOW_TYPES[j];
                Atom type_atom = XInternAtom(disp, type, False);
                if (atom_type == type_atom) {
                    (types + counter)->flag = g_strdup(type);
                    (types + counter)->number = type_atom;
                    counter++;
                }
            }
        }

        if (counter == 0) {
            g_free(types);
            types = NULL;
        }
        *size = counter;
    }
    else 
        *size = 0;

    g_free(atoms_type);
    return types;
}

//_NET_WM_ALLOWED_ACTIONS
struct action_desc *get_window_allowed_actions(Display *disp, Window win, size_t *size) {
    struct action_desc *actions = NULL;
    Atom *atoms_action = (Atom *)get_property(disp, win, XA_ATOM,
            "_NET_WM_ALLOWED_ACTIONS", size);
    if (atoms_action) {
        *size = *size / 8;
        size_t counter = 0;
        actions = g_malloc(sizeof(struct action_desc) * (*size));
        for (size_t i = 0; i < *size; i++) {
            Atom atom_action = atoms_action[i];
            for (size_t j = 0; j < NB_ALLOWED_ACTIONS; j++) {
                const char *action = _NET_WM_ALLOWED_ACTIONS[j];
                Atom action_atom = XInternAtom(disp, action, False);
                if (atom_action == action_atom) {
                    (actions + counter)->flag = g_strdup(action);
                    (actions + counter)->number = action_atom;
                    counter++;
                }
            }
        }

        if (counter == 0) {
            g_free(actions);
            actions = NULL;
        }
        *size = counter;
    }
    else 
        *size = 0;

    g_free(atoms_action);
    return actions;
}

//_NET_WM_STATE
struct state_desc *get_window_states(Display *disp, Window win, size_t *size) {
    struct state_desc *states = NULL;
    Atom *atoms_state = (Atom *)get_property(disp, win, XA_ATOM,
            "_NET_WM_STATE", size);
    if (atoms_state) {
        *size = *size / 8;
        size_t counter = 0;
        states = g_malloc(sizeof(struct state_desc) * (*size));
        for (size_t i = 0; i < *size; i++) {
            Atom atom_state = atoms_state[i];
            for (size_t j = 0; j < NB_WM_STATE; j++) {
                const char *state = _NET_WM_STATE[j];
                Atom state_atom = XInternAtom(disp, state, False);
                if (atom_state == state_atom) {
                    (states + counter)->flag = g_strdup(state);
                    (states + counter)->number = state_atom;
                    counter++;
                }
            }
        }

        if (counter == 0) {
            g_free(states);
            states = NULL;
        }
        *size = counter;
    }
    else 
        *size = 0;

    g_free(atoms_state);
    return states;
}

//_NET_WM_NAME
gchar *get_window_name(Display *disp, Window win) {
    gchar *title_utf8 = NULL;
    gchar *wm_name = NULL;
    gchar *net_wm_name = NULL;

    wm_name = get_property(disp, win, XA_STRING, "WM_NAME", NULL);
    net_wm_name = get_property(disp, win, 
            XInternAtom(disp, "UTF8_STRING", False), "_NET_WM_NAME", NULL);

    if (net_wm_name) {
        title_utf8 = g_strdup(net_wm_name);
    }
    else {
        if (wm_name) {
            title_utf8 = g_locale_to_utf8(wm_name, -1, NULL, NULL, NULL);
        }
        else {
            title_utf8 = NULL;
        }
    }

    g_free(wm_name);
    g_free(net_wm_name);

    return title_utf8;
}

//_NET_WM_VISIBLE_NAME
gchar *get_window_visible_name(Display *disp, Window win) {
    gchar *net_wm_visible_name = NULL;
    gchar *wm_visible_name = NULL;
    gchar *visible_name = NULL;

    wm_visible_name = get_property(disp, win, XA_STRING, "WM_VISIBLE_NAME", NULL);
    net_wm_visible_name = get_property(disp, win, 
        XInternAtom(disp, "UTF8_STRING", False), "_NET_WM_VISIBLE_NAME", NULL);

    if (net_wm_visible_name) {
        visible_name = net_wm_visible_name;
    }else {
        if (wm_visible_name) {
            visible_name = g_locale_to_utf8(wm_visible_name, -1, NULL, NULL, NULL);
            g_free(wm_visible_name);
        }
        else {
            visible_name = NULL;
        }
    }

    return visible_name;
}

//_NET_WM_ICON_NAME
gchar *get_window_icon_name(Display *disp, Window win) {
    gchar *net_wm_icon_name = NULL;
    net_wm_icon_name = get_property(disp, win, 
        XInternAtom(disp, "UTF8_STRING", False), "_NET_WM_ICON_NAME", NULL);

    return net_wm_icon_name;
}

//_NET_WM_VISIBLE_ICON_NAME
gchar *get_window_visible_icon_name(Display *disp, Window win) {
    gchar *net_wm_visible_icon_name = NULL;
    net_wm_visible_icon_name = get_property(disp, win, 
        XInternAtom(disp, "UTF8_STRING", False), 
        "_NET_WM_VISIBLE_ICON_NAME", NULL);

    return net_wm_visible_icon_name;
}

struct geometry *get_window_geometry(Display *disp, Window win) {
    struct geometry *win_geometry = malloc(sizeof(struct geometry));
    if (!win_geometry) {
        return NULL;
    }

    unsigned int wwidth, wheight,  bw, depth;
    Window junkroot;
    int junkx,junky, x, y;

    XGetGeometry(disp, win, &junkroot, &junkx, &junky,
                        &wwidth, &wheight, &bw, &depth);
    XTranslateCoordinates (disp, win, junkroot, junkx, junky,
                            &x, &y, &junkroot);
    win_geometry->x = x;
    win_geometry->y = y;
    win_geometry->width = wwidth;
    win_geometry->height = wheight;
    return win_geometry;
}