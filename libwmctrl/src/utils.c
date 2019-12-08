#include "./wmctrl.h"

struct window_info *create_empty_window_info() {
    struct window_info *wi = g_malloc(sizeof(struct window_info));
    if (!wi)
        return NULL;
    wi->win_client_machine    = NULL;
    wi->win_class             = NULL;
    wi->win_types             = NULL;
    wi->win_actions           = NULL;
    wi->win_states            = NULL;
    wi->win_name              = NULL;
    wi->win_visible_name      = NULL;
    wi->win_icon_name         = NULL;
    wi->win_visible_icon_name = NULL;
    wi->win_geometry          = NULL;
    return wi;
}

struct window_info *create_window_info(Display *disp, Window win) {
    struct window_info *wi = create_empty_window_info();
    if (!wi)
        return NULL;
    fill_window_info(disp, wi, win);
    return wi;
}

void fill_window_info(Display *disp, struct window_info *wi, Window win) {
    wi->win_id = win;
    wi->win_pid = get_window_pid(disp, win);
    wi->desktop_number = get_window_desktop(disp, win);
    wi->showing_desktop = get_window_shwing_desktop(disp, win);
    wi->win_client_machine = get_window_client_machine(disp, win);
    wi->win_class = get_window_class(disp, win);
    wi->win_types = get_window_types(disp, win, &wi->nbr_type);
    wi->win_actions = get_window_allowed_actions(disp, win, &wi->nbr_action);
    wi->win_states = get_window_states(disp, win, &wi->nbr_state);
    wi->win_name = get_window_name(disp, win);
    wi->win_visible_name = get_window_visible_name(disp, win);
    wi->win_icon_name = get_window_icon_name(disp, win);
    wi->win_visible_icon_name = get_window_visible_icon_name(disp, win);
    wi->win_geometry = get_window_geometry(disp, win);
    wi->WM_NORMAL_HINTS = XAllocSizeHints();
    XGetWMNormalHints(disp, win, wi->WM_NORMAL_HINTS, &wi->supplied_return);
}

void copy_window_info(struct window_info *dest_wi, struct window_info *src_wi) {
    dest_wi->win_id = src_wi->win_id;
    dest_wi->win_pid = src_wi->win_pid;
    dest_wi->desktop_number = src_wi->desktop_number;
    dest_wi->showing_desktop = src_wi->showing_desktop;

    dest_wi->win_client_machine = src_wi->win_client_machine ? strdup(src_wi->win_client_machine) : NULL;
    dest_wi->win_class = src_wi->win_class ? strdup(src_wi->win_class) : NULL;

    if (src_wi->win_types) {
        size_t size = sizeof(struct type_desc) * src_wi->nbr_type;
        dest_wi->win_types = g_malloc(size);
        if (dest_wi->win_types) {
            for (size_t i = 0; i < src_wi->nbr_type; i++) {
                struct type_desc *win_types = dest_wi->win_types + i;
                win_types->flag = g_strdup((src_wi->win_types+i)->flag);
                win_types->number = (src_wi->win_types+i)->number;
            }
            dest_wi->nbr_type = src_wi->nbr_type;
        }
    }
    else
        dest_wi->win_types = NULL;

    if (src_wi->win_actions) {
        size_t size = sizeof(struct action_desc) * src_wi->nbr_action;
        dest_wi->win_actions = g_malloc(size);
        if (dest_wi->win_actions) {
            for (size_t i = 0; i < src_wi->nbr_action; i++) {
                struct action_desc *win_actions = dest_wi->win_actions + i;
                win_actions->flag = g_strdup((src_wi->win_actions+i)->flag);
                win_actions->number = (src_wi->win_actions+i)->number;
            }
            dest_wi->nbr_action = src_wi->nbr_action;
        }
    }
    else
        dest_wi->win_actions = NULL;

    if (src_wi->win_states) {
        size_t size = sizeof(struct state_desc) * src_wi->nbr_state;
        dest_wi->win_states = g_malloc(size);
        if (dest_wi->win_states) {
            for (size_t i = 0; i < src_wi->nbr_state; i++) {
                struct state_desc *win_states = dest_wi->win_states + i;
                win_states->flag = g_strdup((src_wi->win_states+i)->flag);
                win_states->number = (src_wi->win_states+i)->number;
            }
            dest_wi->nbr_state = src_wi->nbr_state;
        }
    }
    else
        dest_wi->win_states = NULL;

    dest_wi->win_name = src_wi->win_name ? strdup(src_wi->win_name) : NULL;
    dest_wi->win_visible_name = src_wi->win_visible_name ? strdup(src_wi->win_visible_name) : NULL;
    dest_wi->win_icon_name = src_wi->win_icon_name ? strdup(src_wi->win_icon_name) : NULL;
    dest_wi->win_visible_icon_name = src_wi->win_visible_icon_name ? strdup(src_wi->win_visible_icon_name) : NULL;

    if (src_wi->win_geometry) {
        dest_wi->win_geometry = g_malloc(sizeof(struct geometry));
        if (dest_wi->win_geometry)
            memcpy(dest_wi->win_geometry, src_wi->win_geometry, sizeof(struct geometry));
    }
    else
        dest_wi->win_geometry = NULL;
    if (src_wi->WM_NORMAL_HINTS) {
        XSizeHints *sh = XAllocSizeHints();
        memcpy(sh, src_wi->WM_NORMAL_HINTS, sizeof(XSizeHints));
        dest_wi->WM_NORMAL_HINTS = sh;
    }
    else
        dest_wi->WM_NORMAL_HINTS = NULL;
    dest_wi->supplied_return = src_wi->supplied_return;
}

void free_window_info_properties(struct window_info *wi) {
    if (!wi)
        return;

    if (wi->win_client_machine) {
        g_free(wi->win_client_machine);
        wi->win_client_machine = NULL;
    }

    if (wi->win_class) {
        g_free(wi->win_class);
        wi->win_class = NULL;
    }

    if (wi->win_types) {
        for (size_t i = 0; i < wi->nbr_type; i++) {
            char *flag = (wi->win_types + i)->flag;
            if (flag)
                g_free(flag);
        }
        g_free(wi->win_types);
        wi->win_types = NULL;
    }

    if (wi->win_actions) {
        for (size_t i = 0; i < wi->nbr_action; i++) {
            char *flag = (wi->win_actions + i)->flag;
            if (flag)
                g_free(flag);
        }
        g_free(wi->win_actions);
        wi->win_actions = NULL;
    }

    if (wi->win_states) {
        for (size_t i = 0; i < wi->nbr_state; i++) {
            char *flag = (wi->win_states + i)->flag;
            if (flag)
                g_free(flag);
        }
        g_free(wi->win_states);
        wi->win_states = NULL;
    }

    if (wi->win_name) {
        g_free(wi->win_name);
        wi->win_name = NULL;
    }

    if (wi->win_visible_name) {
        g_free(wi->win_visible_name);
        wi->win_visible_name = NULL;
    }

    if (wi->win_icon_name) {
        g_free(wi->win_icon_name);
        wi->win_icon_name = NULL;
    }

    if (wi->win_visible_icon_name) {
        g_free(wi->win_visible_icon_name);
        wi->win_visible_icon_name = NULL;
    }

    if (wi->win_geometry) {
        g_free(wi->win_geometry);
        wi->win_geometry = NULL;
    }

    if (wi->WM_NORMAL_HINTS) {
        XFree(wi->WM_NORMAL_HINTS);
        wi->WM_NORMAL_HINTS = NULL;
    }
}

void free_window_info(struct window_info *wi) {
    if (!wi)
        return;
    free_window_info_properties(wi);
    g_free(wi);
}

static struct window_list *create_window_list(Display *disp, Window *windows, size_t size) {
    struct window_list *wl = g_malloc(sizeof(struct window_list));
    if (!wl)
        return NULL;

    wl->client_list = g_malloc(sizeof(struct window_info) * size);
    if (!wl->client_list) {
        g_free(wl);
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        struct window_info *wi = wl->client_list + i;
        Window win = windows[i];
        fill_window_info(disp, wi, win);
    }
    wl->client_list_size = size;
    return wl;
}

struct window_list *list_windows(Display *disp) {
    Window *client_list;
    size_t client_list_size;

    if ((client_list = get_client_list(disp, &client_list_size)) == NULL) {
        return NULL; 
    }

    client_list_size = client_list_size/8;
    struct window_list *wl = create_window_list(disp, client_list, client_list_size);
    g_free(client_list);
    return wl;
}

void free_window_list(struct window_list *wl) {
    if (!wl)
        return;

    for (size_t i = 0;i < wl->client_list_size; i++)
        free_window_info_properties(wl->client_list + i);

    g_free(wl->client_list);
    g_free(wl);
}

void print_window_info(struct window_info *wi) {
    printf("\n");
    printf("| WIN_ID: %ld\n", wi->win_id);

    if (wi->win_pid)
        printf("| Pid: %ld\n", wi->win_pid);

    if (wi->win_class)
        printf("| Class: %s\n", wi->win_class);

    if (wi->win_name)
        printf("| Name: %s\n", wi->win_name);

    if (wi->win_visible_name)
        printf("| Visible_NAME: %s\n", wi->win_visible_name);

    if (wi->win_icon_name)
        printf("| ICON_NAME:%s\n", wi->win_icon_name);

    if (wi->win_visible_icon_name)
        printf("| VISIBLE_ICON_NAME:%s\n", wi->win_visible_icon_name);

    if (wi->desktop_number)
        printf("| Desktop: %ld\n", wi->desktop_number);

    if (wi->nbr_type > 0) {
        printf("| Types: %ld\n", wi->nbr_type);
        for (size_t i = 0; i < wi->nbr_type; i++) {
            char *flag = (wi->win_types+i)->flag;
            Atom number = (wi->win_types+i)->number;
            printf("| \t%s:%ld\n", flag, number);
        }
    }

    if (wi->nbr_action > 0) {
        printf("| Allowed actions: %ld\n", wi->nbr_action);
        for (size_t i = 0; i < wi->nbr_action; i++) {
            char *flag = (wi->win_actions+i)->flag;
            Atom number = (wi->win_actions+i)->number;
            printf("| \t%s:%ld\n", flag, number);
        }
    }

    if (wi->nbr_state > 0) {
        printf("| States: %ld\n", wi->nbr_state);
        for (size_t i = 0; i < wi->nbr_state; i++) {
            char *flag = (wi->win_states+i)->flag;
            Atom number = (wi->win_states+i)->number;
            printf("| \t%s:%ld\n", flag, number);
        }
    }

    if( wi->win_client_machine)
        printf("| CLIENT_MACHINE: %s\n", wi->win_client_machine);

    if (wi->win_geometry) {
        printf("| Geometry: \n");
        printf("| \twidth: %ld\n", wi->win_geometry->width);
        printf("| \theight: %ld\n", wi->win_geometry->height);
        printf("| \tx: %ld\n", wi->win_geometry->x);
        printf("| \ty: %ld\n", wi->win_geometry->y);
    }

    if (wi->WM_NORMAL_HINTS) {
        printf("| WM_NORMAL_HINTS: \n");
        printf("| \twin_gravity: %d\n", wi->WM_NORMAL_HINTS->win_gravity);
        printf("| \theight: %d\n", wi->WM_NORMAL_HINTS->height);
        printf("| \twidth: %d\n", wi->WM_NORMAL_HINTS->width);
        printf("| \tx: %d\n", wi->WM_NORMAL_HINTS->x);
        printf("| \ty: %d\n", wi->WM_NORMAL_HINTS->y);
        printf("| \tbase_width: %d\n", wi->WM_NORMAL_HINTS->base_width);
        printf("| \tbase_height: %d\n", wi->WM_NORMAL_HINTS->base_height);
        printf("| \tmin_width: %d\n", wi->WM_NORMAL_HINTS->min_width);
        printf("| \tmin_height: %d\n", wi->WM_NORMAL_HINTS->min_height);
        printf("| \tmin_aspect:\n");
        printf("| \t\tnumerator:%d\n", wi->WM_NORMAL_HINTS->min_aspect.x);
        printf("| \t\tdenominator:%d\n", wi->WM_NORMAL_HINTS->min_aspect.y);
        printf("| \tmax_width: %d\n", wi->WM_NORMAL_HINTS->max_width);
        printf("| \tmax_height: %d\n", wi->WM_NORMAL_HINTS->max_height);
        printf("| \tmax_aspect:\n");
        printf("| \t\tnumerator:%d\n", wi->WM_NORMAL_HINTS->max_aspect.x);
        printf("| \t\tdenominator:%d\n", wi->WM_NORMAL_HINTS->max_aspect.y);
        printf("| \twidth_inc: %d\n", wi->WM_NORMAL_HINTS->width_inc);
        printf("| \theight_inc: %d\n", wi->WM_NORMAL_HINTS->height_inc);
    }
    printf("\n");
}

gchar *get_output_str (gchar *str) {
    gchar *out;

    if (str == NULL) {
        return NULL;
    }
    out = g_strdup(str);
    return out;
}

Window *get_client_list (Display *disp, unsigned long *size) {
    Window *client_list;

    if ((client_list = (Window *)get_property(disp, DefaultRootWindow(disp), 
                    XA_WINDOW, "_NET_CLIENT_LIST", size)) == NULL) {
        if ((client_list = (Window *)get_property(disp, DefaultRootWindow(disp), 
                        XA_CARDINAL, "_WIN_CLIENT_LIST", size)) == NULL) {
            fputs("Cannot get client list properties. \n"
                  "(_NET_CLIENT_LIST or _WIN_CLIENT_LIST)"
                  "\n", stderr);
            return NULL;
        }
    }
    return client_list;
}

int client_msg(Display *disp, Window win, char *msg,
        unsigned long data0, unsigned long data1, 
        unsigned long data2, unsigned long data3,
        unsigned long data4) {
    XEvent event;
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(disp, msg, False);
    event.xclient.window = win;
    event.xclient.format = 32;
    event.xclient.data.l[0] = data0;
    event.xclient.data.l[1] = data1;
    event.xclient.data.l[2] = data2;
    event.xclient.data.l[3] = data3;
    event.xclient.data.l[4] = data4;
    
    if (XSendEvent(disp, DefaultRootWindow(disp), False, mask, &event)) {
        return EXIT_SUCCESS;
    }
    else {
        fprintf(stderr, "Cannot send %s event.\n", msg);
        return EXIT_FAILURE;
    }
}

int change_viewport (Display *disp, unsigned long x, unsigned long y) {
    return client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_VIEWPORT", 
        x, y, 0, 0, 0);
}

int change_geometry (Display *disp, unsigned long x, unsigned long y) {
    return client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_GEOMETRY", 
        x, y, 0, 0, 0);
}