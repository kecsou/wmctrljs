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
    win_geometry:any;

    wm_normal_hints_supplied:number;
    WM_NORMAL_HINTS:any;
    WM_HINTS:any;
    desktop_number:number;
    showing_desktop:number;
}

export function getWindowList():Window[]|null {
    try {
        return wmctrl.getWindowList();
    }catch (e) {
        return null;
    }
}

export function getActiveWindow():Window|null {
    try {
        return wmctrl.getActiveWindow();
    } catch(e) {
        return null;
    }
}

export function getWindowsByPid(pid:number):Window[]|null {
    if (typeof pid !== "number")
        throw new Error("pid must be a number");
    try {
        return wmctrl.getWindowsByPid(pid);
    } catch(e) {
        return null;
    }
    
}

export function getWindowsByClassName(class_name:string):Window[]|null {
    if (typeof class_name !== "string")
        throw new Error("class_name must be a string");
    try {
        return wmctrl.getWindowsByClassName(class_name);
    }catch(e) {
        return null;
    }
}

export function activeWindowById(id:number):boolean {
    if (typeof id !== "number")
        throw new Error("id must be a number");

    try {
        return Boolean(wmctrl.activeWindowById(id));
    } catch(e) {
        return false;
    }
}

export function activeWindowsByClassName(class_name:string):boolean {
    if (typeof class_name !== "string")
        throw new Error("class_name must be a string");

    try {
        return Boolean(wmctrl.activeWindowsByClassName(class_name));
    } catch(e) {
        return false;
    }
}

export function activeWindowsByPid(pid:number):boolean {
    if (typeof pid !== "number")
        throw new Error("pid must be a number");

    try {
        return Boolean(wmctrl.activeWindowsByPid(pid));
    } catch(e) {
        return false;
    }
}

export function closeWindowById(id:number):boolean {
    if (typeof id !== "number")
        throw new Error("id must be a number");

    try {
        return Boolean(wmctrl.closeWindowById(id));
    } catch(e) {
        return false;
    }
}

export function closeWindowsByClassName(class_name:string):boolean {
    if (typeof class_name !== "string")
        throw new Error("class_name must be a string");

    try {
        return Boolean(wmctrl.closeWindowsByClassName(class_name));
    } catch(e) {
        return false;
    }
}

export function closeWindowsByPid(pid:number):boolean {
    if (typeof pid !== "number")
        throw new Error("pid must be a number");

    try {
        return Boolean(wmctrl.closeWindowsByPid(pid));
    } catch(e) {
        return false;
    }
}