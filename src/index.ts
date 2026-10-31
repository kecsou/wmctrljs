/**
 * Window management for X11 through EWMH, like `wmctrl` and more, without spawning a process.
 *
 * Every operation comes in two flavours: `xxxSync` runs on the calling thread and throws,
 * `xxx` runs on the libuv thread pool and returns a promise that rejects.
 *
 * Errors are `Error` objects whose message names the function and an error code, such as
 * `[getActiveWindow] NO_WINDOW_ACTIVE_FOR_NOW (28)`. Every call can also fail with:
 * - `CAN_NOT_OPEN_DISPLAY (3)`: `DISPLAY` is unset, or the X server cannot be reached.
 * - `INTERNAL_ERROR (43)`: a bug in the native code, reported instead of crashing the process.
 * - `[fn] expect a type [number] for param [win_id]`: an argument has the wrong type, or is `NaN`.
 *
 * Actions resolve once the X server has the request. The window manager then applies it on its
 * own schedule: read the window back if you need to wait for the result.
 *
 * @example
 * ```ts
 * import { getActiveWindowSync, windowStateSync } from "wmctrljs";
 *
 * const win = getActiveWindowSync();
 * windowStateSync(win.win_id, "add", "maximized_vert", "maximized_horz");
 * ```
 *
 * @packageDocumentation
 */

// glibc and musl (Alpine) Linux need different binaries.
const musl = process.platform === "linux" && !(process.report.getReport() as any).header?.glibcVersionRuntime;
const target = `${process.platform}-${process.arch}${musl ? "-musl" : ""}`;
/** The native addon prebuilt for this platform: `prebuilds/<platform>-<arch>[-musl]/wmctrljs.node`. */
const wmctrl = (() => {
    try {
        return require(`../prebuilds/${target}/wmctrljs.node`);
    } catch (e) {
        throw new Error(`wmctrljs: cannot load the binary for ${target}: ${(e as Error).message}`, { cause: e });
    }
})();

/** An EWMH atom found in a window's type, allowed actions or state list. */
export interface TypeDesc {
    /** Atom name, such as `_NET_WM_STATE_HIDDEN`. */
    flag:string
    /** Atom id on this X server. */
    number:number
}

/** Position and size of a window, in pixels. */
export interface Geometry {
    /** Left edge, relative to the parent window (the frame, under a reparenting window manager). */
    x:number
    /** Top edge, relative to the parent window. */
    y:number
    /** Left edge, relative to the root window of the window's screen. */
    abs_x:number
    /** Top edge, relative to the root window of the window's screen. */
    abs_y:number
    /** Width, without the window manager's frame. */
    width:number
    /** Height, without the window manager's frame. */
    height:number
}

/** An aspect ratio: `numerator / denominator`. */
export interface Aspect {
    numerator:number
    denominator:number
}

/**
 * ICCCM size hints (`WM_NORMAL_HINTS`), laid out like Xlib's `XSizeHints` (Xutil.h).
 * Fields the client did not supply are 0.
 */
export interface XSizeHints {
    /**
     * Which fields the client set: USPosition 1, USSize 2, PPosition 4, PSize 8, PMinSize 16,
     * PMaxSize 32, PResizeInc 64, PAspect 128, PBaseSize 256, PWinGravity 512.
     */
    flags:number
    /** Obsolete: window managers ignore `x`, `y`, `width` and `height`, but clients still set them for old ones. */
    x:number
    /** Obsolete, see `x`. */
    y:number
    /** Obsolete, see `x`. */
    width:number
    /** Obsolete, see `x`. */
    height:number
    /** Smallest width the window accepts. */
    min_width:number
    /** Smallest height the window accepts. */
    min_height:number
    /** Largest width the window accepts. */
    max_width:number
    /** Largest height the window accepts. */
    max_height:number
    /** Width step: the width is `base_width + i * width_inc`. */
    width_inc:number
    /** Height step: the height is `base_height + i * height_inc`. */
    height_inc:number
    /** Smallest width / height ratio the window accepts. */
    min_aspect:Aspect 
    /** Largest width / height ratio the window accepts. */
    max_aspect:Aspect
    /** Width the width steps count from. */
    base_width:number
    /** Height the height steps count from. */
    base_height:number
    /** Gravity the window manager places the window with: 1 (NorthWest) to 9 (SouthEast), 10 Static. */
    win_gravity:number
}

/** Border widths the window manager adds around a window (`_NET_FRAME_EXTENTS`), in pixels. */
export interface FrameExtents {
    left:number,
    right:number,
    top:number,
    bottom:number
}

/** A window and its EWMH/ICCCM properties. Properties the client did not set are 0, `""` or `[]`. */
export interface Window {
    /** X window id. */
    win_id:number;
    /** Id of the process that owns the window (`_NET_WM_PID`). */
    win_pid:number;
    /** Host the client runs on (`WM_CLIENT_MACHINE`). */
    win_client_machine:string;
    /** `WM_CLASS` as `instance.Class`, such as `xeyes.XEyes`. */
    win_class:string;
    /** Window types (`_NET_WM_WINDOW_TYPE`), such as `_NET_WM_WINDOW_TYPE_NORMAL`. */
    win_types:TypeDesc[];
    /** Actions the window manager allows (`_NET_WM_ALLOWED_ACTIONS`), such as `_NET_WM_ACTION_CLOSE`. */
    win_actions:TypeDesc[];
    /** Current states (`_NET_WM_STATE`), such as `_NET_WM_STATE_HIDDEN`. */
    win_states:TypeDesc[];
    /** Title: `_NET_WM_NAME`, or `WM_NAME` when it is unset. */
    win_name:string;
    /** Title as the window manager shows it: `_NET_WM_VISIBLE_NAME`, or `WM_VISIBLE_NAME` when it is unset. */
    win_visible_name:string;
    /** Title when minimized (`_NET_WM_ICON_NAME`). */
    win_icon_name:string;
    /** Title when minimized, as the window manager shows it (`_NET_WM_VISIBLE_ICON_NAME`). */
    win_visible_icon_name:string;
    /** Position and size. `undefined` when the window does not exist. */
    win_geometry:Geometry|undefined;

    /** Which `WM_NORMAL_HINTS` fields the client supplied, like `XGetWMNormalHints`: 0 none, 255 all but base size and gravity, 1023 all. */
    wm_normal_hints_supplied:number;
    /** Always 0, kept for compatibility with 1.x. */
    wm_hints_supplied:number;
    /** Size hints (`WM_NORMAL_HINTS`), all 0 when the client set none. */
    WM_NORMAL_HINTS:XSizeHints|undefined;
    /** Always all 0, kept for compatibility with 1.x. */
    WM_HINTS:XSizeHints|undefined;
    /** Desktop the window is on (`_NET_WM_DESKTOP`), 0xFFFFFFFF for all desktops. */
    desktop_number:number;
    /** `_NET_SHOWING_DESKTOP` read on the window itself. Window managers set it on the root window, so this is usually 0. */
    showing_desktop:number;
    /** Frame the window manager draws around the window. `undefined` until it sets one. */
    frame_extents:FrameExtents|undefined
}

/** The default screen of the display (the screen number in `DISPLAY`), as the X server describes it on connection. */
export interface Screen {
    /** Backing store support: 0 Never, 1 WhenMapped, 2 Always. */
    backing_store:number;
    /** Pixel value of black in the default colormap. */
    black_pixel:number;
    /** Id of the default colormap. */
    cmap:number;
    /** Height in pixels. */
    height:number;
    /** Most colormaps that can be installed at once. */
    max_maps:number;
    /** Height in millimeters. */
    mheight:number;
    /** Fewest colormaps that can be installed at once. */
    min_maps:number;
    /** Width in millimeters. */
    mwidth:number;
    /** Number of depths the screen supports. */
    ndepths:number;
    /** Depth of the root window, in bits per pixel. */
    root_depth:number;
    /** Events all clients currently select on the root window. */
    root_input_mask:number;
    /** 1 when the screen supports save-unders. */
    save_unders:number;
    /** Pixel value of white in the default colormap. */
    white_pixel:number;
    /** Width in pixels. */
    width:number
}

/** Throws `[fnName] expect a type [type] for param [argName]` unless `arg` has `type` (and is not `NaN` for numbers). */
function expectParam(fnName:string, argName:string, arg:any, type:string) {
    if (typeof arg !== type)
        throw new Error(`[${fnName}] expect a type [${type}] for param [${argName}]`);

    if (type === "number" && isNaN(arg))
        throw new Error(`[${fnName}] expect a type [${type}] for param [${argName}] received NaN value`);
}

/**
 * Describes the default screen of the display.
 * Blocks the calling thread: {@link getScreen} does the same on the thread pool.
 *
 * @returns The screen.
 */
export function getScreenSync():Screen {
    return wmctrl.getScreenSync();
}

/**
 * Describes the default screen of the display.
 *
 * @returns A promise of the screen.
 */
export function getScreen():Promise<Screen> {
    return wmctrl.getScreenAsync();
}

/**
 * Lists the client windows of the default screen, in mapping order (`_NET_CLIENT_LIST`).
 * Blocks the calling thread: {@link getWindowList} does the same on the thread pool.
 *
 * @returns The windows.
 * @throws {Error} `CAN_NOT_GET_CLIENT_LIST (1)` when the window manager publishes no client list.
 */
export function getWindowListSync():Window[] {
    return wmctrl.getWindowListSync();
}

/**
 * Lists the client windows of the default screen, in mapping order (`_NET_CLIENT_LIST`).
 *
 * @returns A promise of the windows.
 * @throws {Error} Rejects with `CAN_NOT_GET_CLIENT_LIST (1)` when the window manager publishes no client list.
 */
export function getWindowList():Promise<Window[]> {
    return wmctrl.getWindowListAsync();
}

/**
 * Reads a window.
 * Blocks the calling thread: {@link getWindowById} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns The window. A window that does not exist reads as empty: no {@link Window.win_geometry}, every other property 0, `""` or `[]`.
 */
export function getWindowByIdSync(win_id:number):Window {
    expectParam("getWindowById", "win_id", win_id, "number");
    return wmctrl.getWindowByIdSync(win_id);
}

/**
 * Reads a window.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns A promise of the window. A window that does not exist reads as empty: no {@link Window.win_geometry}, every other property 0, `""` or `[]`.
 */
export async function getWindowById(win_id:number):Promise<Window> {
    expectParam("getWindowById", "win_id", win_id, "number");
    return await wmctrl.getWindowByIdAsync(win_id);
}

/**
 * Reads the window that has the focus (`_NET_ACTIVE_WINDOW`).
 * Blocks the calling thread: {@link getActiveWindow} does the same on the thread pool.
 *
 * @returns The active window.
 * @throws {Error} `NO_WINDOW_ACTIVE_FOR_NOW (28)` when no window has the focus.
 * @throws {Error} `CAN_NOT_ACTIVATE_WINDOW (10)` when the window manager does not publish the active window.
 */
export function getActiveWindowSync():Window {
    return wmctrl.getActiveWindowSync();
}

/**
 * Reads the window that has the focus (`_NET_ACTIVE_WINDOW`).
 *
 * @returns A promise of the active window.
 * @throws {Error} Rejects with `NO_WINDOW_ACTIVE_FOR_NOW (28)` when no window has the focus.
 * @throws {Error} Rejects with `CAN_NOT_ACTIVATE_WINDOW (10)` when the window manager does not publish the active window.
 */
export function getActiveWindow():Promise<Window> {
    return wmctrl.getActiveWindowAsync();
}

/**
 * Lists the client windows that belong to a process.
 * Blocks the calling thread: {@link getWindowsByPid} does the same on the thread pool.
 *
 * @param win_pid Process id to match against {@link Window.win_pid}. 0 matches no window.
 * @returns The matching windows, `[]` when none match.
 * @throws {Error} `CAN_NOT_GET_CLIENT_LIST (1)` when the window manager publishes no client list.
 */
export function getWindowsByPidSync(win_pid:number):Window[] {
    expectParam("getWindowsByPid", "win_pid", win_pid, "number");
    return wmctrl.getWindowsByPidSync(win_pid);
}

/**
 * Lists the client windows that belong to a process.
 *
 * @param win_pid Process id to match against {@link Window.win_pid}. 0 matches no window.
 * @returns A promise of the matching windows, `[]` when none match.
 * @throws {Error} Rejects with `CAN_NOT_GET_CLIENT_LIST (1)` when the window manager publishes no client list.
 */
export async function getWindowsByPid(win_pid:number):Promise<Window[]> {
    expectParam("getWindowsByPid", "win_pid", win_pid, "number");
    return await wmctrl.getWindowsByPidAsync(win_pid);
}

/**
 * Lists the client windows of a class.
 * Blocks the calling thread: {@link getWindowsByClassName} does the same on the thread pool.
 *
 * @param win_class_name `WM_CLASS` to match exactly against {@link Window.win_class}, as `instance.Class`.
 * @returns The matching windows, `[]` when none match.
 * @throws {Error} `CAN_NOT_GET_CLIENT_LIST (1)` when the window manager publishes no client list.
 */
export function getWindowsByClassNameSync(win_class_name:string):Window[] {
    expectParam("getWindowsByClassName", "win_class_name", win_class_name, "string");
    return wmctrl.getWindowsByClassNameSync(win_class_name);
}

/**
 * Lists the client windows of a class.
 *
 * @param win_class_name `WM_CLASS` to match exactly against {@link Window.win_class}, as `instance.Class`.
 * @returns A promise of the matching windows, `[]` when none match.
 * @throws {Error} Rejects with `CAN_NOT_GET_CLIENT_LIST (1)` when the window manager publishes no client list.
 */
export async function getWindowsByClassName(win_class_name:string):Promise<Window[]> {
    expectParam("getWindowsByClassName", "win_class_name", win_class_name, "string");
    return wmctrl.getWindowsByClassNameAsync(win_class_name);
}

/**
 * Activates a window: raises it, maps it (which restores it when minimized) and asks the window manager to focus it.
 * Blocks the calling thread: {@link activeWindowById} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns `true`.
 * @throws {Error} `CAN_NOT_ACTIVATE_WINDOW (10)` when the window is not managed: it has no desktop, or does not exist.
 */
export function activeWindowByIdSync(win_id:number):boolean {
    expectParam("activeWindowById", "win_id", win_id, "number");
    return wmctrl.activeWindowByIdSync(win_id);
}

/**
 * Activates a window: raises it, maps it (which restores it when minimized) and asks the window manager to focus it.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns A promise of `true`.
 * @throws {Error} Rejects with `CAN_NOT_ACTIVATE_WINDOW (10)` when the window is not managed: it has no desktop, or does not exist.
 */
export async function activeWindowById(win_id:number):Promise<boolean> {
    expectParam("activeWindowById", "win_id", win_id, "number");
    return await wmctrl.activeWindowByIdAsync(win_id);
}

/**
 * Activates every client window of a class, one after the other in mapping order. See {@link activeWindowById}.
 * Blocks the calling thread: {@link activeWindowsByClassName} does the same on the thread pool.
 *
 * @param win_class_name `WM_CLASS` to match exactly against {@link Window.win_class}, as `instance.Class`.
 * @returns `true`.
 * @throws {Error} `NO_WINDOW_FOUND (24)` when no window matches.
 * @throws {Error} `CAN_NOT_ACTIVATE_WINDOW (10)` when a matching window is not managed.
 */
export function activeWindowsByClassNameSync(win_class_name:string):boolean {
    expectParam("activeWindowsByClassName", "win_class_name", win_class_name, "string");
    return wmctrl.activeWindowsByClassNameSync(win_class_name);
}

/**
 * Activates every client window of a class, one after the other in mapping order. See {@link activeWindowById}.
 *
 * @param win_class_name `WM_CLASS` to match exactly against {@link Window.win_class}, as `instance.Class`.
 * @returns A promise of `true`.
 * @throws {Error} Rejects with `NO_WINDOW_FOUND (24)` when no window matches.
 * @throws {Error} Rejects with `CAN_NOT_ACTIVATE_WINDOW (10)` when a matching window is not managed.
 */
export async function activeWindowsByClassName(win_class_name:string):Promise<boolean> {
    expectParam("activeWindowsByClassName", "win_class_name", win_class_name, "string");
    return await wmctrl.activeWindowsByClassNameAsync(win_class_name);
}

/**
 * Activates every client window of a process, one after the other in mapping order. See {@link activeWindowById}.
 * Blocks the calling thread: {@link activeWindowsByPid} does the same on the thread pool.
 *
 * @param win_pid Process id to match against {@link Window.win_pid}. 0 matches no window.
 * @returns `true`.
 * @throws {Error} `NO_WINDOW_FOUND (24)` when no window matches.
 * @throws {Error} `CAN_NOT_ACTIVATE_WINDOW (10)` when a matching window is not managed.
 */
export function activeWindowsByPidSync(win_pid:number):boolean {
    expectParam("activeWindowsByPid", "win_pid", win_pid, "number");
    return wmctrl.activeWindowsByPidSync(win_pid);
}

/**
 * Activates every client window of a process, one after the other in mapping order. See {@link activeWindowById}.
 *
 * @param win_pid Process id to match against {@link Window.win_pid}. 0 matches no window.
 * @returns A promise of `true`.
 * @throws {Error} Rejects with `NO_WINDOW_FOUND (24)` when no window matches.
 * @throws {Error} Rejects with `CAN_NOT_ACTIVATE_WINDOW (10)` when a matching window is not managed.
 */
export async function activeWindowsByPid(win_pid:number):Promise<boolean> {
    expectParam("activeWindowsByPid", "win_pid", win_pid, "number");
    return await wmctrl.activeWindowsByPidAsync(win_pid);
}

/**
 * Asks the window manager to close a window (`_NET_CLOSE_WINDOW`), like its close button: the application may ask for confirmation, or refuse.
 * Blocks the calling thread: {@link closeWindowById} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns `true`.
 */
export function closeWindowByIdSync(win_id:number):boolean {
    expectParam("closeWindowById", "win_id", win_id, "number");
    return wmctrl.closeWindowByIdSync(win_id);
}

/**
 * Asks the window manager to close a window (`_NET_CLOSE_WINDOW`), like its close button: the application may ask for confirmation, or refuse.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns A promise of `true`.
 */
export async function closeWindowById(win_id:number):Promise<boolean> {
    expectParam("closeWindowById", "win_id", win_id, "number");
    return await wmctrl.closeWindowByIdAsync(win_id);
}

/**
 * Asks the window manager to close every client window of a class. See {@link closeWindowById}.
 * Blocks the calling thread: {@link closeWindowsByClassName} does the same on the thread pool.
 *
 * @param win_class_name `WM_CLASS` to match exactly against {@link Window.win_class}, as `instance.Class`.
 * @returns `true`.
 * @throws {Error} `NO_WINDOW_FOUND (24)` when no window matches.
 */
export function closeWindowsByClassNameSync(win_class_name:string):boolean {
    expectParam("closeWindowsByClassName", "win_class_name", win_class_name, "string");
    return wmctrl.closeWindowsByClassNameSync(win_class_name);
}

/**
 * Asks the window manager to close every client window of a class. See {@link closeWindowById}.
 *
 * @param win_class_name `WM_CLASS` to match exactly against {@link Window.win_class}, as `instance.Class`.
 * @returns A promise of `true`.
 * @throws {Error} Rejects with `NO_WINDOW_FOUND (24)` when no window matches.
 */
export async function closeWindowsByClassName(win_class_name:string):Promise<boolean> {
    expectParam("closeWindowsByClassName", "win_class_name", win_class_name, "string");
    return await wmctrl.closeWindowsByClassNameAsync(win_class_name);
}

/**
 * Asks the window manager to close every client window of a process. See {@link closeWindowById}.
 * Blocks the calling thread: {@link closeWindowsByPid} does the same on the thread pool.
 *
 * @param win_pid Process id to match against {@link Window.win_pid}. 0 matches no window.
 * @returns `true`.
 * @throws {Error} `NO_WINDOW_FOUND (24)` when no window matches.
 */
export function closeWindowsByPidSync(win_pid:number):boolean {
    expectParam("closeWindowsByPid", "win_pid", win_pid, "number");
    return wmctrl.closeWindowsByPidSync(win_pid);
}

/**
 * Asks the window manager to close every client window of a process. See {@link closeWindowById}.
 *
 * @param win_pid Process id to match against {@link Window.win_pid}. 0 matches no window.
 * @returns A promise of `true`.
 * @throws {Error} Rejects with `NO_WINDOW_FOUND (24)` when no window matches.
 */
export async function closeWindowsByPid(win_pid:number):Promise<boolean> {
    expectParam("closeWindowsByPid", "win_pid", win_pid, "number");
    return await wmctrl.closeWindowsByPidAsync(win_pid);
}

/**
 * Moves and/or resizes a window, through the window manager (`_NET_MOVERESIZE_WINDOW`).
 * When the window manager does not support it, the window is configured directly: the position then
 * only changes when `win_x` and `win_y` are both 0 or more, and the size when `win_width` and `win_height` are both 1 or more.
 * Blocks the calling thread: {@link windowMoveResize} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @param win_gravity Point of the window that `win_x`/`win_y` place: 0 the window's own gravity, 1 (NorthWest) to 9 (SouthEast), 10 Static.
 * @param win_x New left edge of the window's frame on the screen, -1 to keep it.
 * @param win_y New top edge of the window's frame on the screen, -1 to keep it.
 * @param win_width New width, without the frame, -1 to keep it.
 * @param win_height New height, without the frame, -1 to keep it.
 * @returns `true`.
 * @throws {Error} `CAN_NOT_MOVE_RESIZE_WINDOW (27)` when `win_gravity` is negative.
 */
export function windowMoveResizeSync(win_id:number, win_gravity:number, win_x:number, 
    win_y:number, win_width:number, win_height:number):boolean {
    expectParam("windowMoveResize", "win_id", win_id, "number");
    expectParam("windowMoveResize", "win_gravity", win_gravity, "number");
    expectParam("windowMoveResize", "win_x", win_x, "number");
    expectParam("windowMoveResize", "win_y", win_y, "number");
    expectParam("windowMoveResize", "win_width", win_width, "number");
    expectParam("windowMoveResize", "win_height", win_height, "number");
    return wmctrl.windowMoveResizeSync(win_id, win_gravity, win_x, 
            win_y, win_width, win_height);
}

/**
 * Moves and/or resizes a window, through the window manager (`_NET_MOVERESIZE_WINDOW`).
 * When the window manager does not support it, the window is configured directly: the position then
 * only changes when `win_x` and `win_y` are both 0 or more, and the size when `win_width` and `win_height` are both 1 or more.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @param win_gravity Point of the window that `win_x`/`win_y` place: 0 the window's own gravity, 1 (NorthWest) to 9 (SouthEast), 10 Static.
 * @param win_x New left edge of the window's frame on the screen, -1 to keep it.
 * @param win_y New top edge of the window's frame on the screen, -1 to keep it.
 * @param win_width New width, without the frame, -1 to keep it.
 * @param win_height New height, without the frame, -1 to keep it.
 * @returns A promise of `true`.
 * @throws {Error} Rejects with `CAN_NOT_MOVE_RESIZE_WINDOW (27)` when `win_gravity` is negative.
 */
export async function windowMoveResize(win_id:number, win_gravity:number, win_x:number, 
        win_y:number, win_width:number, win_height:number):Promise<boolean> {
    expectParam("windowMoveResize", "win_id", win_id, "number");
    expectParam("windowMoveResize", "win_gravity", win_gravity, "number");
    expectParam("windowMoveResize", "win_x", win_x, "number");
    expectParam("windowMoveResize", "win_y", win_y, "number");
    expectParam("windowMoveResize", "win_width", win_width, "number");
    expectParam("windowMoveResize", "win_height", win_height, "number");
    return await wmctrl.windowMoveResizeAsync(win_id, 
                win_gravity, 
                win_x, win_y, 
                win_width, 
                win_height);
}

/**
 * Animates a move/resize: steps x, y, width and height from the window's current position (relative to its root)
 * and size towards the target, `pad` pixels at a time, pausing `await_time` ms after each step.
 * Each step goes through {@link windowMoveResize}. Promise only: the animation would block the thread.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @param win_gravity Point of the window that `win_x`/`win_y` place: 0 the window's own gravity, 1 (NorthWest) to 9 (SouthEast), 10 Static.
 * @param win_x New left edge of the window's frame on the screen, -1 to keep it.
 * @param win_y New top edge of the window's frame on the screen, -1 to keep it.
 * @param win_width New width, without the frame, -1 to keep it.
 * @param win_height New height, without the frame, -1 to keep it.
 * @param await_time Pause after each step, in milliseconds (negative counts as 0). Defaults to 1.
 * @param pad Pixels per step (below 1 counts as 1). Defaults to 1.
 * @returns A promise of `true`, once the last step is sent.
 * @throws {Error} Rejects with `CAN_NOT_MOVE_RESIZE_WINDOW (27)` when `win_gravity` is negative, or `await_time` is too long for a timer (`Infinity`).
 * @throws {Error} Rejects with `CAN_NOT_OPEN_DISPLAY (3)` when the window does not exist.
 */
export async function windowMoveResizeTransition(win_id:number, win_gravity:number, win_x:number, 
    win_y:number, win_width:number, win_height:number, await_time:number=1, pad:number=1):Promise<boolean> {
    expectParam("windowMoveResizeTransition", "win_id", win_id, "number");
    expectParam("windowMoveResizeTransition", "win_gravity", win_gravity, "number");
    expectParam("windowMoveResizeTransition", "win_x", win_x, "number");
    expectParam("windowMoveResizeTransition", "win_y", win_y, "number");
    expectParam("windowMoveResizeTransition", "win_width", win_width, "number");
    expectParam("windowMoveResizeTransition", "win_height", win_height, "number");
    expectParam("windowMoveResizeTransition", "await_time", await_time, "number");
    expectParam("windowMoveResizeTransition", "pad", pad, "number");

    return await wmctrl.windowMoveResizeTransitionAsync(win_id, 
        win_gravity, 
        win_x, win_y, 
        win_width, 
        win_height, 
        await_time, 
        pad);
}

/** `_NET_WM_STATE` actions {@link windowState} accepts, in any case. */
const enabledActions = ["REMOVE", "ADD", "TOGGLE"];
/** States {@link windowState} accepts, in any case: `_NET_WM_STATE_<NAME>` in lower case. */
const enabledProperties = [
    "modal", 
    "sticky", 
    "maximized_vert", 
    "maximized_horz", 
    "shaded", 
    "skip_taskbar", 
    "skip_pager", 
    "hidden", 
    "fullscreen", 
    "above",
    "below"
];
/**
 * Adds, removes or toggles one or two states of a window (`_NET_WM_STATE`), through the window manager.
 * Two states change together: `maximized_vert` with `maximized_horz` maximizes.
 * Blocks the calling thread: {@link windowState} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @param action `"add"`, `"remove"` or `"toggle"`, in any case.
 * @param prop1 `modal`, `sticky`, `maximized_vert`, `maximized_horz`, `shaded`, `skip_taskbar`, `skip_pager`, `hidden`, `fullscreen`, `above` or `below`, in any case.
 * @param prop2 A second property from the same list, changed together with `prop1`.
 * @returns `true`.
 * @throws {Error} When `action` or a property is not in the lists above.
 */
export function windowStateSync(win_id:number, action:string, prop1:string, prop2?:string):boolean {
    expectParam("windowState", "win_id", win_id, "number");
    expectParam("windowState", "action", action, "string");
    expectParam("windowState", "prop1", prop1, "string");
    action = action.toUpperCase();
    if (!enabledActions.includes(action))
        throw new Error(`[windowState] only actions [${enabledActions.join(',')}] are enabled note (${action})`);

    prop1 = prop1.toLowerCase();
    if (!enabledProperties.includes(prop1))
        throw new Error(`[windowState] property prop1 (${prop1}) is not an enabled property ${enabledProperties.join(',')}`);

    if (prop2) {
        expectParam("windowState", "prop2", prop2, "string");
        prop2 = prop2.toLowerCase();
        if (!enabledProperties.includes(prop2))
            throw new Error(`[windowState] property prop2 (${prop2}) is not an enabled property ${enabledProperties.join(',')}`);
    }
    else
        prop2 = "";

    return wmctrl.windowStateSync(win_id, action, prop1, prop2);
}

/**
 * Adds, removes or toggles one or two states of a window (`_NET_WM_STATE`), through the window manager.
 * Two states change together: `maximized_vert` with `maximized_horz` maximizes.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @param action `"add"`, `"remove"` or `"toggle"`, in any case.
 * @param prop1 `modal`, `sticky`, `maximized_vert`, `maximized_horz`, `shaded`, `skip_taskbar`, `skip_pager`, `hidden`, `fullscreen`, `above` or `below`, in any case.
 * @param prop2 A second property from the same list, changed together with `prop1`.
 * @returns A promise of `true`.
 * @throws {Error} Rejects when `action` or a property is not in the lists above.
 */
export async function windowState(win_id:number, action:string, prop1:string, prop2?:string):Promise<boolean> {
    expectParam("windowState", "win_id", win_id, "number");
    expectParam("windowState", "action", action, "string");
    expectParam("windowState", "prop1", prop1, "string");
    action = action.toUpperCase();
    if (!enabledActions.includes(action))
        throw new Error(`[windowState] only actions [${enabledActions.join(',')}] are enabled note (${action})`);

    prop1 = prop1.toLowerCase();
    if (!enabledProperties.includes(prop1))
        throw new Error(`[windowState] property prop1 (${prop1}) is not an enabled property ${enabledProperties.join(',')}`);

    if (prop2) {
        expectParam("windowState", "prop2", prop2, "string");
        prop2 = prop2.toLowerCase();
        if (!enabledProperties.includes(prop2))
            throw new Error(`[windowState] property prop2 (${prop2}) is not an enabled property ${enabledProperties.join(',')}`);
    }
    else
        prop2 = "";

    return await wmctrl.windowStateAsync(win_id, action, prop1, prop2);
}

/**
 * Minimizes a window: asks the window manager of its screen to iconify it, like `XIconifyWindow`.
 * Blocks the calling thread: {@link windowMinimize} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns `true`.
 * @throws {Error} `CAN_NOT_GET_WINDOW_ATTRIBUTES (36)` when the window does not exist.
 */
export function windowMinimizeSync(win_id:number):boolean {
    expectParam("windowMinimize", "win_id", win_id, "number");
    return wmctrl.windowMinimizeSync(win_id);
}

/**
 * Minimizes a window: asks the window manager of its screen to iconify it, like `XIconifyWindow`.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns A promise of `true`.
 * @throws {Error} Rejects with `CAN_NOT_GET_WINDOW_ATTRIBUTES (36)` when the window does not exist.
 */
export async function windowMinimize(win_id:number):Promise<boolean> {
    expectParam("windowMinimize", "win_id", win_id, "number");
    return await wmctrl.windowMinimizeAsync(win_id);
}

/**
 * Lifts the size constraints a client put on its window by replacing its `WM_NORMAL_HINTS` with empty hints, so it can be resized freely.
 * Blocks the calling thread: {@link windowAllowAllSizes} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns `true`.
 */
export function windowAllowAllSizesSync(win_id:number):boolean {
    expectParam("windowAllowAllSizes", "win_id", win_id, "number");
    return wmctrl.windowAllowAllSizesSync(win_id);
}

/**
 * Lifts the size constraints a client put on its window by replacing its `WM_NORMAL_HINTS` with empty hints, so it can be resized freely.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns A promise of `true`.
 */
export async function windowAllowAllSizes(win_id:number):Promise<boolean> {
    expectParam("windowAllowAllSizes", "win_id", win_id, "number");
    return await wmctrl.windowAllowAllSizesAsync(win_id);
}

/**
 * Puts a window above its siblings. It does not focus it: see {@link activeWindowById}.
 * Blocks the calling thread: {@link windowRaise} does the same on the thread pool.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns `true`.
 */
export function windowRaiseSync(win_id:number):boolean {
    expectParam("windowRaise", "win_id", win_id, "number");
    return wmctrl.windowRaiseSync(win_id);
}

/**
 * Puts a window above its siblings. It does not focus it: see {@link activeWindowById}.
 *
 * @param win_id X window id, as in {@link Window.win_id}.
 * @returns A promise of `true`.
 */
export async function windowRaise(win_id:number):Promise<boolean> {
    expectParam("windowRaise", "win_id", win_id, "number");
    return await wmctrl.windowRaiseAsync(win_id);
}