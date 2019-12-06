#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>
#include <glib.h>

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

#define MAX_PROPERTY_VALUE_LEN 4096
#define SELECT_WINDOW_MAGIC ":SELECT:"
#define ACTIVE_WINDOW_MAGIC ":ACTIVE:"

struct window_list{
    Window *client_list;
    int client_list_size;
};

struct wm_info {
    gchar *wm_name;
    gchar *wm_class;
    unsigned long wm_pid;
    unsigned long showing_desktop;
};

//UTILS
void free_window_list(struct window_list *wl);
void free_wm_info(struct wm_info *wm);
gchar *get_output_str (gchar *str);
int client_msg(Display *disp, Window win, char *msg,
        unsigned long data0, unsigned long data1, 
        unsigned long data2, unsigned long data3,
        unsigned long data4);
Window *get_client_list (Display *disp, unsigned long *size);

//WINDOW-MANAGER
struct wm_info *get_wm_info(Display *disp);
gboolean wm_supports(Display *disp, const gchar *prop);

//DESKTOP
int switch_desktop (Display *disp, unsigned long target);

//WINDOW
gchar *get_window_title (Display *disp, Window win);
gchar *get_window_class (Display *disp, Window win);
struct window_list *list_windows(Display *disp);
Window get_active_window(Display *disp);
gchar *get_property (Display *disp, Window win,
        Atom xa_prop_type, const char *prop_name, unsigned long *size);

//ACTION-WINDOW
extern void active_window_by_id(Display *disp, unsigned long wid);
extern void close_window_by_id(Display *disp, unsigned long wid);
int window_to_desktop (Display *disp, Window win, int desktop);
int window_move_resize (Display *disp, Window win, unsigned long grav, 
    unsigned long x, unsigned long y, 
    unsigned long w, unsigned long h);