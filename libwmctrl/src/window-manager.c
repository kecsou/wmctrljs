#include "./wmctrl.h"

gboolean wm_supports(Display *disp, const gchar *prop) {
    Atom xa_prop = XInternAtom(disp, prop, False);
    Atom *list;
    unsigned long size;
    int i;

    if (! (list = (Atom *)get_property(disp, DefaultRootWindow(disp),
            XA_ATOM, "_NET_SUPPORTED", &size))) {
        printf("Cannot get _NET_SUPPORTED property.\n");
        return FALSE;
    }

    for (i = 0; i < size / sizeof(Atom); i++) {
        if (list[i] == xa_prop) {
            g_free(list);
            return TRUE;
        }
    }

    g_free(list);
    return FALSE;
}

struct window_info *get_wm_info(Display *disp) {
    Window *sup_window = NULL;
    gchar *wm_name = NULL;
    gchar *wm_class = NULL;
    unsigned long *wm_pid = NULL;
    unsigned long *showing_desktop = NULL;
    gchar *name_out;
    gchar *class_out;

    if (! (sup_window = (Window *)get_property(disp, DefaultRootWindow(disp),
                    XA_WINDOW, "_NET_SUPPORTING_WM_CHECK", NULL))) {
        if (! (sup_window = (Window *)get_property(disp, DefaultRootWindow(disp),
                        XA_CARDINAL, "_WIN_SUPPORTING_WM_CHECK", NULL))) {
            fputs("Cannot get window manager info properties.\n"
                  "(_NET_SUPPORTING_WM_CHECK or _WIN_SUPPORTING_WM_CHECK)\n", stderr);
            return NULL;
        }
    }

    struct window_info *wi = malloc(sizeof(struct window_info));

    /* WM_NAME */
    if (! (wm_name = get_property(disp, *sup_window,
            XInternAtom(disp, "UTF8_STRING", False), "_NET_WM_NAME", NULL))) {
        if (! (wm_name = get_property(disp, *sup_window,
                XA_STRING, "_NET_WM_NAME", NULL))) {
            printf("Cannot get name of the window manager (_NET_WM_NAME).\n");
        }
    }
    name_out = get_output_str(wm_name);

    /* WM_CLASS */
    if (! (wm_class = get_property(disp, *sup_window,
            XInternAtom(disp, "UTF8_STRING", False), "WM_CLASS", NULL))) {
        if (! (wm_class = get_property(disp, *sup_window,
                XA_STRING, "WM_CLASS", NULL))) {
            printf("Cannot get class of the window manager (WM_CLASS).\n");
        }
    }
    class_out = get_output_str(wm_class);

    /* WM_PID */
    if (! (wm_pid = (unsigned long *)get_property(disp, *sup_window,
                    XA_CARDINAL, "_NET_WM_PID", NULL))) {
        printf("Cannot get pid of the window manager (_NET_WM_PID).\n");
    }

    /* _NET_SHOWING_DESKTOP */
    if (! (showing_desktop = (unsigned long *)get_property(disp, DefaultRootWindow(disp),
                    XA_CARDINAL, "_NET_SHOWING_DESKTOP", NULL))) {
        printf("Cannot get the _NET_SHOWING_DESKTOP property.\n");
    }

    /* match out the info */
    wi->win_name = strdup(name_out ? name_out : "N/A");
    wi->win_class = strdup(class_out ? class_out : "N/A");
    wi->win_pid = wm_pid ? *wm_pid : 0;
    wi->showing_desktop = showing_desktop ? *showing_desktop : 0;

    if (name_out)
        g_free(name_out);
    if (sup_window)
        g_free(sup_window);
    if (wm_name)
        g_free(wm_name);
    if (wm_class)
	    g_free(wm_class);
    if (wm_pid)
        g_free(wm_pid);
    if (showing_desktop)
        g_free(showing_desktop);
    return wi;
}