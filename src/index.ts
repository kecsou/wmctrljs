import * as path from "path";
const root = path.join(__dirname, "../");
const wmctrl = require('node-gyp-build')(root);

wmctrl.initialiseClient();
process.addListener("exit", () => {
    wmctrl.freeDispClient();
});

interface TypeDesc {
    flags:string
    number:number
}

interface Geometry {
    x:number,
    y:number,
    width:number,
    height:number
}

interface Aspect {
    x:number /* numerator */
    y:number /* denominator */
}

interface XSizeHints { //Based on Xutils.h
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

interface Window {
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
    win_geometry:Geometry;

    wm_normal_hints_supplied:number;
    WM_NORMAL_HINTS:XSizeHints;
    WM_HINTS:XSizeHints;
    desktop_number:number;
    showing_desktop:number;
}

function expectParam(fnName:string, argName:string, arg:any, type:string) {
    if (typeof arg !== type)
        throw new Error(`[${fnName}] expect a type [${type}] for param [${argName}]`);
}

export function getWindowList():Window[] {
    const windows = wmctrl.getWindowList();
    return !windows ? [] : windows;
}

export function getActiveWindow():Window|null {
    return wmctrl.getActiveWindow();
}

export function getWindowsByPid(win_pid:number):Window[] {
    expectParam("getWindowsByPid", "win_pid", win_pid, "number");
    const windows = wmctrl.getWindowsByPid(win_pid);
    return !windows ? [] : windows;
}

export function getWindowsByClassName(win_class_name:string):Window[] {
    expectParam("getWindowsByClassName", "win_class_name", win_class_name, "string");
    const windows = wmctrl.getWindowsByClassName(win_class_name);
    return !windows ? [] : windows;
}

export function activeWindowById(win_id:number):boolean {
    expectParam("activeWindowById", "win_id", win_id, "number");
    return Boolean(wmctrl.activeWindowById(win_id));
}

export function activeWindowsByClassName(win_class_name:string):boolean {
    expectParam("activeWindowsByClassName", "win_class_name", win_class_name, "string");
    return Boolean(wmctrl.activeWindowsByClassName(win_class_name));
}

export function activeWindowsByPid(win_pid:number):boolean {
    expectParam("activeWindowsByPid", "win_pid", win_pid, "number");
    return Boolean(wmctrl.activeWindowsByPid(win_pid));
}

export function closeWindowById(win_id:number):boolean {
    expectParam("closeWindowById", "win_id", win_id, "number");
    return Boolean(wmctrl.closeWindowById(win_id));
}

export function closeWindowsByClassName(win_class_name:string):boolean {
    expectParam("closeWindowsByClassName", "win_class_name", win_class_name, "string");
    return Boolean(wmctrl.closeWindowsByClassName(win_class_name));
}

export function closeWindowsByPid(win_pid:number):boolean {
    expectParam("closeWindowsByPid", "win_pid", win_pid, "number");
    return Boolean(wmctrl.closeWindowsByPid(win_pid));
}

export function windowMoveResize(win_id:number, win_gravity:number, win_x:number, 
        win_y:number, win_width:number, win_height:number) {
    expectParam("windowMoveResize", "win_id", win_id, "number");
    expectParam("windowMoveResize", "win_gravity", win_gravity, "number");
    expectParam("windowMoveResize", "win_x", win_x, "number");
    expectParam("windowMoveResize", "win_y", win_y, "number");
    expectParam("windowMoveResize", "win_width", win_width, "number");
    expectParam("windowMoveResize", "win_height", win_height, "number");
    return Boolean(wmctrl.windowMoveResize(win_id, win_gravity, win_x, win_y, win_width, win_height));
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
export function windowState(win_id:number, action:string, prop1:string, prop2?:string) {
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