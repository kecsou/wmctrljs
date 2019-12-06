#include "./wmctrl.h"

void free_window_list(struct window_list *wl) {
    free(wl->client_list);
    free(wl);
}

void free_wm_info(struct wm_info *wm) {
    free(wm->wm_name);
    free(wm->wm_class);
    free(wm);
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

static int change_viewport (Display *disp, unsigned long x, unsigned long y) {
    return client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_VIEWPORT", 
        x, y, 0, 0, 0);
}

static int change_geometry (Display *disp, unsigned long x, unsigned long y) {
    return client_msg(disp, DefaultRootWindow(disp), "_NET_DESKTOP_GEOMETRY", 
        x, y, 0, 0, 0);
}