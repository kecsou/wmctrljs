#include "./wmctrl.h"

gchar *get_property (Display *disp, Window win,
        Atom xa_prop_type, const char *prop_name, unsigned long *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    gchar *ret_prop;
    gchar *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    /* MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):
     *
     * long_length = Specifies the length in 32-bit multiples of the
     *               data to be retrieved.
     */
    if (XGetWindowProperty(disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
            xa_prop_type, &xa_ret_type, &ret_format,     
            &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        g_free(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / 8) * ret_nitems;
    ret = g_malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

gchar *get_window_title (Display *disp, Window win) {
    gchar *title_utf8;
    gchar *wm_name;
    gchar *net_wm_name;

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

gchar *get_window_class (Display *disp, Window win) {
    gchar *class_utf8;
    gchar *wm_class;
    unsigned long size;

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

struct window_list *list_windows(Display *disp) {
    Window *client_list;
    unsigned long client_list_size;

    if ((client_list = get_client_list(disp, &client_list_size)) == NULL) {
        return NULL; 
    }
    struct window_list *wl = malloc(sizeof(struct window_list));
    wl->client_list = client_list;
    wl->client_list_size = client_list_size/sizeof(unsigned long);
    return wl;
}

Window get_active_window(Display *disp) {
    char *prop;
    unsigned long size;
    Window ret = (Window)0;

    prop = get_property(disp, DefaultRootWindow(disp), XA_WINDOW, 
                        "_NET_ACTIVE_WINDOW", &size);
    if (prop) {
        ret = *((Window*)prop);
        g_free(prop);
    }

    return(ret);
}

static void window_set_title (Display *disp, Window win,
    char *title, char mode) {
    gchar *title_utf8;
    gchar *title_local;

    title_utf8 = g_strdup(title);
    title_local = NULL;

    if (mode == 'T' || mode == 'N') {
        /* set name */
        if (title_local) {
            XChangeProperty(disp, win, XA_WM_NAME, XA_STRING, 8, PropModeReplace,
                    title_local, strlen(title_local));
        }
        else {
            XDeleteProperty(disp, win, XA_WM_NAME);
        }
        XChangeProperty(disp, win, XInternAtom(disp, "_NET_WM_NAME", False), 
                XInternAtom(disp, "UTF8_STRING", False), 8, PropModeReplace,
                title_utf8, strlen(title_utf8));
    }

    if (mode == 'T' || mode == 'I') {
        /* set icon name */
        if (title_local) {
            XChangeProperty(disp, win, XA_WM_ICON_NAME, XA_STRING, 8, PropModeReplace,
                    title_local, strlen(title_local));
        }
        else {
            XDeleteProperty(disp, win, XA_WM_ICON_NAME);
        }
        XChangeProperty(disp, win, XInternAtom(disp, "_NET_WM_ICON_NAME", False), 
                XInternAtom(disp, "UTF8_STRING", False), 8, PropModeReplace,
                title_utf8, strlen(title_utf8));
    }
    g_free(title_utf8);
    g_free(title_local);
}

static Window Select_Window(Display *dpy) {
    /*
     * Routine to let user select a window using the mouse
     * Taken from xfree86.
     */
    int status;
    Cursor cursor;
    XEvent event;
    Window target_win = None, root = DefaultRootWindow(dpy);
    int buttons = 0;
    int dummyi;
    unsigned int dummy;

    /* Make the target cursor */
    cursor = XCreateFontCursor(dpy, XC_crosshair);

    /* Grab the pointer using target cursor, letting it room all over */
    status = XGrabPointer(dpy, root, False,
            ButtonPressMask|ButtonReleaseMask, GrabModeSync,
            GrabModeAsync, root, cursor, CurrentTime);
    if (status != GrabSuccess) {
        fputs("ERROR: Cannot grab mouse.\n", stderr);
        return 0;
    }

    /* Let the user select a window... */
    while ((target_win == None) || (buttons != 0)) {
        /* allow one more event */
        XAllowEvents(dpy, SyncPointer, CurrentTime);
        XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
        switch (event.type) {
            case ButtonPress:
                if (target_win == None) {
                    target_win = event.xbutton.subwindow; /* window selected */
                    if (target_win == None) target_win = root;
                }
                buttons++;
                break;
            case ButtonRelease:
                if (buttons > 0) /* there may have been some down before we started */
                    buttons--;
                break;
        }
    }

    XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */

    if (XGetGeometry (dpy, target_win, &root, &dummyi, &dummyi,
                &dummy, &dummy, &dummy, &dummy) && target_win != root) {
        target_win = XmuClientWindow (dpy, target_win);
    }

    return(target_win);
}

static int window_state (Display *disp, Window win, char *arg) {
    unsigned long action;
    Atom prop1 = 0;
    Atom prop2 = 0;
    char *p1, *p2;
    const char *argerr = "The -b option expects a list of comma separated parameters: \"(remove|add|toggle),<PROP1>[,<PROP2>]\"\n";

    if (!arg || strlen(arg) == 0) {
        fputs(argerr, stderr);
        return EXIT_FAILURE;
    }

    if ((p1 = strchr(arg, ','))) {
        gchar *tmp_prop1, *tmp1;

        *p1 = '\0';

        /* action */
        if (strcmp(arg, "remove") == 0) {
            action = _NET_WM_STATE_REMOVE;
        }
        else if (strcmp(arg, "add") == 0) {
            action = _NET_WM_STATE_ADD;
        }
        else if (strcmp(arg, "toggle") == 0) {
            action = _NET_WM_STATE_TOGGLE;
        }
        else {
            fputs("Invalid action. Use either remove, add or toggle.\n", stderr);
            return EXIT_FAILURE;
        }
        p1++;

        /* the second property */
        if ((p2 = strchr(p1, ','))) {
            gchar *tmp_prop2, *tmp2;
            *p2 = '\0';
            p2++;
            if (strlen(p2) == 0) {
                fputs("Invalid zero length property.\n", stderr);
                return EXIT_FAILURE;
            }
            tmp_prop2 = g_strdup_printf("_NET_WM_STATE_%s", tmp2 = g_ascii_strup(p2, -1));
            printf("State 2: %s\n", tmp_prop2); 
            prop2 = XInternAtom(disp, tmp_prop2, False);
            g_free(tmp2);
            g_free(tmp_prop2);
        }

        /* the first property */
        if (strlen(p1) == 0) {
            fputs("Invalid zero length property.\n", stderr);
            return EXIT_FAILURE;
        }
        tmp_prop1 = g_strdup_printf("_NET_WM_STATE_%s", tmp1 = g_ascii_strup(p1, -1));
        printf("State 1: %s\n", tmp_prop1); 
        prop1 = XInternAtom(disp, tmp_prop1, False);
        g_free(tmp1);
        g_free(tmp_prop1);

        return client_msg(disp, win, "_NET_WM_STATE", 
            action, (unsigned long)prop1, (unsigned long)prop2, 0, 0);
    }
    else {
        fputs(argerr, stderr);
        return EXIT_FAILURE;
    }
}