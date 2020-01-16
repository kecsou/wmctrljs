#include "../src/wmctrl.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void test_list_window(Display *disp) {
    struct window_list *wl = list_windows(disp, NULL);
    printf("Window number %ld\n", wl->client_list_size);
    for (int i = 0; i < wl->client_list_size; i++) {
        print_window_info(wl->client_list + i);
    }
    free_window_list(wl);
}

void debugend(char *test_name) {
    printf("\n------- %s ok\n\n", test_name);
}

struct codeInfos {
    clock_t begin;
    char *id;
};

struct codeInfos **times = NULL;
size_t size_times = 512;

void timer(char *str) {
    if (!times)
        times = calloc(sizeof(struct codeInfos *), size_times);

    //CREATE IF NOT EXIST
    for (size_t i = 0; i < size_times; i++) {
        if (!times[i]) {
            times[i] = malloc(sizeof(struct codeInfos));
            if (!times[i])
                return;

            times[i]->id = strdup(str);
            times[i]->begin = clock();
            break;
        }
    }
}

void timerEnd(char *str) {
    clock_t call_time = clock();
    for (size_t i = 0; i < size_times; i++) {
        if (times[i] && strcmp(times[i]->id, str) == 0) {
            clock_t t = call_time - times[i]->begin;
            double time_taken = ((double)t) / 1000;
            printf("%s: %fms\n", times[i]->id, time_taken);
            free(times[i]->id);
            free(times[i]);
            times[i] = NULL;
            break;
        }
    }
}

int main(int argc, char **argv) {
    char *winProcess = "xclock";
    char *class_name = "wmctrlTest.out.XClock";//"xclock.XClock";
    enum STATES st;
    struct window_list *wl = NULL;
    Screen *sc = NULL;

    timer("closeWindowById");
    close_window_by_id(NULL, 4);
    timerEnd("closeWindowById");

    timer("getScreen");
    sc = get_screen(NULL, &st);
    timerEnd("getScreen");
    if (sc) {
        free_screen(sc);
    }

    timer("getWindowList");
    wl = list_windows(NULL, &st);
    timerEnd("getWindowList");
    free_window_list(wl);

    timer("closeWindowsByClassName");
    st = close_windows_by_class_name(NULL, class_name);
    timerEnd("closeWindowsByClassName");

    if (st != WINDOWS_CLOSED) {
        char *error = get_error_message(st);
        if (error) {
            printf("Voluntary error [Must failed] close of windows %s failed error:%s\n", class_name, error);
            free(error);
            if (st != NO_WINDOW_FOUND)
                return 1;
        }
        else
            printf("close of windows %s failed error:Unexpected\n", class_name);
    }

    for (int i = 0; i < 150; i++) {
        if (fork() == 0) {
            execvp(winProcess, argv);
            return 1;
        }
    }
    sleep(10);

    timer("getWindowsByClassName");
    wl = get_windows_by_class_name(NULL, class_name, &st);
    timerEnd("getWindowsByClassName");
    if (!wl) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s\n", error);
            free(error);
        }
        return 1;
    }

    printf("%ld windows are open with the class_name %s\n", wl->client_list_size, class_name);

    timer("activeWindowsByClassName");
    st = active_windows_by_class_name(NULL, class_name);
    timerEnd("activeWindowsByClassName");
    if (st != WINDOWS_ACTIVATED) {
        char *error = get_error_message(st);
        if (error) {
            printf("Voluntary error [Must failed][activeWindowsByClassName] %s\n", error);
            free(error);
            for (size_t i = 0; i < wl->client_list_size; i++) {
                active_window_by_id(NULL, wl->client_list[i].win_id);
            }
        }else
            printf("[activeWindowsByClassName] Unexpected error\n");
    }

    for (size_t i = 0; i < wl->client_list_size; i++) {
        struct window_info *wi = wl->client_list + i;

        clock_t begin = clock();
        st = close_windows_by_pid(NULL, wi->win_pid);
        clock_t duration = clock() - begin;

        if (st != WINDOWS_CLOSED) {
            timer("closeWindowById");
            close_window_by_id(NULL, wi->win_id);
            timerEnd("closeWindowById");
        }else {
            double time_taken = ((double)duration) / 1000;
            printf("closeWindowsByPid: %fms\n", time_taken);
        }
        usleep(100 * 1000);
    }

    timer("closeWindowsByClassName");
    st = close_windows_by_class_name(NULL, class_name);
    timerEnd("closeWindowsByClassName");
    if (st != WINDOWS_CLOSED) {
        char *error = get_error_message(st);
        if (error) {
            printf("Voluntary error [Must failed] %s\n", error);
            free(error);
        }
    }
    free_window_list(wl);
/*
    timer("getActiveWindow");
    struct window_info *wi = get_active_window(NULL, &st);
    timerEnd("getActiveWindow");
    free_window_info(wi);
    if (fork() == 0) {
        execvp("xeyes", argv);
        return 1;
    }
    sleep(2);

    timer("activeWindowsByClassName");
    active_windows_by_class_name(NULL, "wmctrlTest.out.XEyes");
    timerEnd("activeWindowsByClassName");

    timer("getActiveWindow");
    wi = get_active_window(NULL, &st);
    timerEnd("getActiveWindow");
    if (st != CLIENT_LIST_GET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
    }
    free_window_info(wi);

    timer("getWindowsByClassName");
    wl = get_windows_by_class_name(NULL, "wmctrlTest.out.XEyes", &st);
    timerEnd("getWindowsByClassName");
    if (!wl || st != CLIENT_LIST_GET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
 
    wi = wl->client_list;
    timer("windowMoveResize");
    st = window_move_resize(NULL, wi->win_id, 0, 0, 0, 500, 500);
    timerEnd("windowMoveResize");
    if (st != WINDOW_MOVED_RESIZED) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }

    sleep(2);

    timer("windowMoveResize");
    st = window_move_resize(NULL, wi->win_id, 0, 0, 0, 1000, 1000);
    timerEnd("windowMoveResize");
    if (st != WINDOW_MOVED_RESIZED) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
    sleep(5);

    st = window_state(NULL, wi->win_id, _NET_WM_STATE_ADD, "maximized_vert", "maximized_horz");
    if (st != WINDOW_STATE_SET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
    sleep(5);

    st = window_state(NULL, wi->win_id, _NET_WM_STATE_REMOVE, "maximized_vert", "maximized_horz");
    if (st != WINDOW_STATE_SET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
    sleep(5);

    st = window_state(NULL, wi->win_id, _NET_WM_STATE_ADD, "fullscreen", NULL);
    if (st != WINDOW_STATE_SET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
    sleep(5);

    st = window_state(NULL, wi->win_id, _NET_WM_STATE_REMOVE, "fullscreen", NULL);
    if (st != WINDOW_STATE_SET) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
    sleep(5);

    free_window_list(wl);

    timer("closeWindowsByClassName");
    st = close_windows_by_class_name(NULL, "wmctrlTest.out.XEyes");
    timerEnd("closeWindowsByClassName");
    if (st != WINDOWS_CLOSED) {
        char *error = get_error_message(st);
        if (error) {
            printf("%s", error);
            free(error);
        }
        return 1;
    }
*/
    return 0;
}