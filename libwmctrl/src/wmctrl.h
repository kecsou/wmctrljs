#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>
#include <stdbool.h>
#include <ctype.h>

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

#define MAX_PROPERTY_VALUE_LEN 4096
#define SELECT_WINDOW_MAGIC ":SELECT:"
#define ACTIVE_WINDOW_MAGIC ":ACTIVE:"

struct geometry {
    unsigned long x;
    unsigned long y;
    unsigned long width;
    unsigned long height;
};

struct type_desc {
        char *flag;
        Atom number;
};

struct action_desc {
        char *flag;
        Atom number;
};

struct state_desc {
        char *flag;
        Atom number;
};

struct window_info {
    Window win_id;
    unsigned long win_pid;
    unsigned long desktop_number;
    unsigned long showing_desktop;
    char *win_client_machine;
    char *win_class;
    struct type_desc *win_types;
    size_t nbr_type;
    struct action_desc *win_actions;
    size_t nbr_action;
    struct state_desc *win_states;
    size_t nbr_state;
    char *win_name;
    char *win_visible_name;
    char *win_icon_name;
    char *win_visible_icon_name;
    struct geometry *win_geometry;
    XSizeHints *WM_NORMAL_HINTS;
    long wm_normal_hints_supplied;
    XSizeHints *WM_HINTS;
    long wm_hints_supplied;
};

struct window_list{
    struct window_info *client_list;
    size_t client_list_size;
};

//NAPI
struct window_list *list_windows_napi();

//UTILS
struct window_list *list_windows(Display *disp);

struct window_info *create_window_info(Display *disp, Window win);
void fill_window_info(Display *disp, struct window_info *wi, Window win);
void free_window_info_properties(struct window_info *wi);
void free_window_info(struct window_info *wi);
void free_window_list(struct window_list *wl);
void print_window_info(struct window_info *wi);
void copy_window_info(struct window_info *dest_wi, struct window_info *src_wi);

int change_geometry (Display *disp, unsigned long x, unsigned long y);
int change_viewport (Display *disp, unsigned long x, unsigned long y);

char *get_output_str (char *str);
int client_msg(Display *disp, Window win, char *msg,
        unsigned long data0, unsigned long data1, 
        unsigned long data2, unsigned long data3,
        unsigned long data4);
Window *get_client_list (Display *disp, unsigned long *size);

//WINDOW-MANAGER
struct window_info *get_wm_info(Display *disp);
bool wm_supports(Display *disp, const char *prop);

//DESKTOP
int switch_desktop (Display *disp, unsigned long target);
int change_number_of_desktops (Display *disp, unsigned long n);
//int list_desktops (Display *disp);
int showing_desktop (Display *disp, unsigned long state);

//WINDOW-PROPERTIES
char *get_property (Display *disp, Window win,
        Atom xa_prop_type, char *prop_name, unsigned long *size);
char *get_window_name (Display *disp, Window win);
char *get_window_visible_name (Display *disp, Window win);
char *get_window_icon_name (Display *disp, Window win);
char *get_window_visible_icon_name(Display *disp, Window win);
unsigned long get_window_desktop(Display *disp, Window win);
struct type_desc *get_window_types(Display *disp, Window win, size_t *size);
struct action_desc *get_window_allowed_actions(Display *disp, Window win, size_t *size);
struct state_desc *get_window_states(Display *disp, Window win, size_t *size);
char *get_window_client_machine(Display *disp, Window win);
struct geometry *get_window_geometry(Display *disp, Window win);
unsigned long get_window_shwing_desktop(Display *disp, Window win);
char *get_window_class (Display *disp, Window win);
unsigned long get_window_pid(Display *disp, Window win);

//WINDOW
struct window_info *get_active_window(Display *disp);
struct window_list *get_windows_by_pid(Display *disp, unsigned long pid);
struct window_list *get_windows_by_class_name(Display *disp, char *class_name);

//WINDOW-EDIT
int active_window_by_id(Display *disp, Window win);
int active_windows_by_pid(Display *disp, unsigned long pid);
int active_windows_by_class_name(Display *disp, char *class_name);
int close_window_by_id(Display *disp, Window win);
int close_windows_by_pid(Display *disp, unsigned long pid);
int close_windows_by_class_name(Display *disp, char *class_name);
int window_state (Display *disp, Window win, unsigned long action, 
        char *prop1_str, char *prop2_str);
int window_set_title(Display *disp, Window win,
        const char *_net_wm_name);
int window_set_icon_name(Display *disp, Window win, 
        const char *_net_wm_icon_name);
Window Select_Window(Display *dpy);

//WINDOW-MOVE
int window_to_desktop (Display *disp, Window win, int desktop);
int window_to_current_desktop(Display *disp, Window win);
int window_move_resize (Display *disp, Window win, int32_t grav, 
    int32_t x, int32_t y, 
    int32_t w, int32_t h);