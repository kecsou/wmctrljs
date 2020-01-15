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

export function getWindowList():Window[] {
    const windows = wmctrl.getWindowList();
    return !windows ? [] : windows;
}

export function getActiveWindow():Window|null {
    return wmctrl.getActiveWindow();
}

export function getWindowsByPid(pid:number):Window[] {
    if (typeof pid !== "number")
        throw new Error("pid must be a number");
    const windows = wmctrl.getWindowsByPid(pid);
    return !windows ? [] : windows;
}

export function getWindowsByClassName(class_name:string):Window[] {
    if (typeof class_name !== "string")
        throw new Error("class_name must be a string");
    const windows = wmctrl.getWindowsByClassName(class_name);
    return !windows ? [] : windows;
}

export function activeWindowById(id:number):boolean {
    if (typeof id !== "number")
        throw new Error("id must be a number");
    return Boolean(wmctrl.activeWindowById(id));
}

export function activeWindowsByClassName(class_name:string):boolean {
    if (typeof class_name !== "string")
        throw new Error("class_name must be a string");
    return Boolean(wmctrl.activeWindowsByClassName(class_name));
}

export function activeWindowsByPid(pid:number):boolean {
    if (typeof pid !== "number")
        throw new Error("pid must be a number");
    return Boolean(wmctrl.activeWindowsByPid(pid));
}

export function closeWindowById(id:number):boolean {
    if (typeof id !== "number")
        throw new Error("id must be a number");
    return Boolean(wmctrl.closeWindowById(id));
}

export function closeWindowsByClassName(class_name:string):boolean {
    if (typeof class_name !== "string")
        throw new Error("class_name must be a string");
    return Boolean(wmctrl.closeWindowsByClassName(class_name));
}

export function closeWindowsByPid(pid:number):boolean {
    if (typeof pid !== "number")
        throw new Error("pid must be a number");
    return Boolean(wmctrl.closeWindowsByPid(pid));
}