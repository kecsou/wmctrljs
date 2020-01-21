import * as path from "path";
const root = path.join(__dirname, "../");
const wmctrl = require('node-gyp-build')(root);

let wmctrl_lib_initialised;
do {
    wmctrl_lib_initialised = Boolean(wmctrl.initialise_wmctrl_lib());
}
while (!wmctrl_lib_initialised);

export interface TypeDesc {
    flags:string
    number:number
}

export interface Geometry {
    x:number
    y:number
    abs_x:number
    abs_y:number
    width:number
    height:number
}

export interface Aspect {
    x:number /* numerator */
    y:number /* denominator */
}

export interface XSizeHints { //Based on Xutils.h
    flags:number	/* marks which fields in this structure are defined */
    x:number
    y:number		/* obsolete for new window mgrs, but clients */
    width:number
    height:number	/* should set so old wm's don't mess up */
    min_width:number
    min_height:number
    max_width:number
    max_height:number
    width_inc:number
    height_inc:number
    min_aspect:Aspect 
    max_aspect:Aspect
    base_width:number
    base_height:number
    win_gravity:number
}

export interface FrameExtents {
    left:number,
    right:number,
    top:number,
    bottom:number
}

export interface Window {
    win_id:number;
    win_pid:number;
    win_client_machine:string;
    win_class:string;
    win_types:TypeDesc[];
    win_actions:TypeDesc[];
    win_states:TypeDesc[];
    win_name:string;
    win_visible_name:string;
    win_icon_name:string;
    win_visible_icon_name:string;
    win_geometry:Geometry|undefined;

    wm_normal_hints_supplied:number;
    WM_NORMAL_HINTS:XSizeHints|undefined;
    WM_HINTS:XSizeHints|undefined;
    desktop_number:number;
    showing_desktop:number;
    frame_extents:FrameExtents|undefined
}

export interface Screen {
    backing_store:number;
    black_pixel:number;
    cmap:number;
    height:number;
    max_maps:number;
    mheight:number;
    min_maps:number;
    mwidth:number;
    ndepths:number;
    root_depth:number;
    root_input_mask:number;
    save_unders:number;
    white_pixel:number;
    width:number
}

function expectParam(fnName:string, argName:string, arg:any, type:string) {
    if (typeof arg !== type)
        throw new Error(`[${fnName}] expect a type [${type}] for param [${argName}]`);
}

export function getScreenSync():Screen[] {
    return wmctrl.getScreen();
}

export function getScreen():Promise<Screen[]> {
    return new Promise((resolve) => {
        resolve(wmctrl.getScreen());
    });
}

export function getWindowListSync():Window[] {
    const windows = wmctrl.getWindowList();
    return !windows ? [] : windows;
}

export function getWindowList():Promise<Window[]> {
    return new Promise((resolve) => {
        const windows = wmctrl.getWindowList();
        resolve(!windows ? [] : windows);
    });
}

export function getActiveWindowSync():Window|null {
    return wmctrl.getActiveWindow();
}

export function getActiveWindow():Promise<Window|null> {
    return new Promise(resolve => {
        resolve(wmctrl.getActiveWindow());
    });
}

export function getWindowsByPidSync(win_pid:number):Window[] {
    expectParam("getWindowsByPid", "win_pid", win_pid, "number");
    const windows = wmctrl.getWindowsByPid(win_pid);
    return !windows ? [] : windows;
}

export function getWindowsByPid(win_pid:number):Promise<Window[]> {
    return new Promise(resolve => {
        expectParam("getWindowsByPid", "win_pid", win_pid, "number");
        const windows = wmctrl.getWindowsByPid(win_pid);
        resolve(!windows ? [] : windows);
    });
}

export function getWindowsByClassNameSync(win_class_name:string):Window[] {
    expectParam("getWindowsByClassName", "win_class_name", win_class_name, "string");
    const windows = wmctrl.getWindowsByClassName(win_class_name);
    return !windows ? [] : windows;
}

export function getWindowsByClassName(win_class_name:string):Promise<Window[]> {
    return new Promise(resolve => {
        expectParam("getWindowsByClassName", "win_class_name", win_class_name, "string");
        const windows = wmctrl.getWindowsByClassName(win_class_name);
        resolve(!windows ? [] : windows);
    });
}

export function activeWindowByIdSync(win_id:number):boolean {
    expectParam("activeWindowById", "win_id", win_id, "number");
    return Boolean(wmctrl.activeWindowById(win_id));
}

export function activeWindowById(win_id:number):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("activeWindowById", "win_id", win_id, "number");
        resolve(Boolean(wmctrl.activeWindowById(win_id)));
    });
}

export function activeWindowsByClassNameSync(win_class_name:string):boolean {
    expectParam("activeWindowsByClassName", "win_class_name", win_class_name, "string");
    return Boolean(wmctrl.activeWindowsByClassName(win_class_name));
}

export function activeWindowsByClassName(win_class_name:string):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("activeWindowsByClassName", "win_class_name", win_class_name, "string");
        resolve(Boolean(wmctrl.activeWindowsByClassName(win_class_name)));
    });
}

export function activeWindowsByPidSync(win_pid:number):boolean {
    expectParam("activeWindowsByPid", "win_pid", win_pid, "number");
    return Boolean(wmctrl.activeWindowsByPid(win_pid));
}

export function activeWindowsByPid(win_pid:number):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("activeWindowsByPid", "win_pid", win_pid, "number");
        resolve(Boolean(wmctrl.activeWindowsByPid(win_pid)));
    });
}

export function closeWindowByIdSync(win_id:number):boolean {
    expectParam("closeWindowById", "win_id", win_id, "number");
    return Boolean(wmctrl.closeWindowById(win_id));
}

export function closeWindowById(win_id:number):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("closeWindowById", "win_id", win_id, "number");
        resolve(Boolean(wmctrl.closeWindowById(win_id)));
    });
}

export function closeWindowsByClassNameSync(win_class_name:string):boolean {
    expectParam("closeWindowsByClassName", "win_class_name", win_class_name, "string");
    return Boolean(wmctrl.closeWindowsByClassName(win_class_name));
}

export function closeWindowsByClassName(win_class_name:string):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("closeWindowsByClassName", "win_class_name", win_class_name, "string");
        resolve(Boolean(wmctrl.closeWindowsByClassName(win_class_name)));
    });
}

export function closeWindowsByPidSync(win_pid:number):boolean {
    expectParam("closeWindowsByPid", "win_pid", win_pid, "number");
    return Boolean(wmctrl.closeWindowsByPid(win_pid));
}

export function closeWindowsByPid(win_pid:number):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("closeWindowsByPid", "win_pid", win_pid, "number");
        resolve(Boolean(wmctrl.closeWindowsByPid(win_pid)));
    });
}

export function windowMoveResizeSync(win_id:number, win_gravity:number, win_x:number, 
    win_y:number, win_width:number, win_height:number):boolean {
    expectParam("windowMoveResize", "win_id", win_id, "number");
    expectParam("windowMoveResize", "win_gravity", win_gravity, "number");
    expectParam("windowMoveResize", "win_x", win_x, "number");
    expectParam("windowMoveResize", "win_y", win_y, "number");
    expectParam("windowMoveResize", "win_width", win_width, "number");
    expectParam("windowMoveResize", "win_height", win_height, "number");
    return Boolean(wmctrl.windowMoveResize(win_id, win_gravity, win_x, 
            win_y, win_width, win_height));
}

export function windowMoveResize(win_id:number, win_gravity:number, win_x:number, 
        win_y:number, win_width:number, win_height:number):Promise<boolean> {
    expectParam("windowMoveResize", "win_id", win_id, "number");
    expectParam("windowMoveResize", "win_gravity", win_gravity, "number");
    expectParam("windowMoveResize", "win_x", win_x, "number");
    expectParam("windowMoveResize", "win_y", win_y, "number");
    expectParam("windowMoveResize", "win_width", win_width, "number");
    expectParam("windowMoveResize", "win_height", win_height, "number");
    return new Promise(resolve => {
        resolve(Boolean(wmctrl.windowMoveResize(win_id, win_gravity, win_x, 
            win_y, win_width, win_height)));
    });
}

const enabledActions = ["REMOVE", "ADD", "TOGGLE"];
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
export function windowStateSync(win_id:number, action:string, prop1:string, prop2?:string) {
    expectParam("windowState", "win_id", win_id, "number");
    expectParam("windowState", "action", action, "string");
    expectParam("windowState", "prop1", prop1, "string");
    action = action.toUpperCase();
    if (!enabledActions.includes(action))
        throw new Error(`[windowState] only actions [${enabledActions.join(',')}] are enabled note (${action})`);

    prop1 = prop1.toLowerCase();
    if (!enabledProperties.includes(prop1))
        throw new Error(`[windowState] property prop1 (${prop1}) is not an enabled property ${enabledActions.join(',')}`);

    if (prop2) {
        expectParam("windowState", "prop2", prop2, "string");
        prop2 = prop2.toLowerCase();
        if (!enabledProperties.includes(prop2))
            throw new Error(`[windowState] property prop2 (${prop2}) is not an enabled property ${enabledActions.join(',')}`);
    }
    else
        prop2 = "";

    return Boolean(wmctrl.windowState(win_id, action, prop1, prop2));
}

export function windowState(win_id:number, action:string, prop1:string, prop2?:string):Promise<boolean> {
    return new Promise((resolve) => {
        expectParam("windowState", "win_id", win_id, "number");
        expectParam("windowState", "action", action, "string");
        expectParam("windowState", "prop1", prop1, "string");
        action = action.toUpperCase();
        if (!enabledActions.includes(action))
            throw new Error(`[windowState] only actions [${enabledActions.join(',')}] are enabled note (${action})`);

        prop1 = prop1.toLowerCase();
        if (!enabledProperties.includes(prop1))
            throw new Error(`[windowState] property prop1 (${prop1}) is not an enabled property ${enabledActions.join(',')}`);

        if (prop2) {
            expectParam("windowState", "prop2", prop2, "string");
            prop2 = prop2.toLowerCase();
            if (!enabledProperties.includes(prop2))
                throw new Error(`[windowState] property prop2 (${prop2}) is not an enabled property ${enabledActions.join(',')}`);
        }
        else
            prop2 = "";
        resolve(Boolean(wmctrl.windowState(win_id, action, prop1, prop2)));
    });
}

export function windowMinimizeSync(win_id:number):boolean {
    expectParam("windowMinimize", "win_id", win_id, "number");
    return Boolean(wmctrl.windowMinimize(win_id));
}

export function windowMinimize(win_id:number):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("windowMinimize", "win_id", win_id, "number");
        resolve(Boolean(wmctrl.windowMinimize(win_id)));
    });
}

export function windowAllowAllSizesSync(win_id:number):boolean {
    expectParam("windowAllowAllSizes", "win_id", win_id, "number");
    return Boolean(wmctrl.windowAllowAllSizes(win_id));
}

export function windowAllowAllSizes(win_id:number):Promise<boolean> {
    return new Promise(resolve => {
        expectParam("windowAllowAllSizes", "win_id", win_id, "number");
        resolve(Boolean(wmctrl.windowAllowAllSizes(win_id)));
    });
}