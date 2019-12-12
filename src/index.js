const path = require("path");
const root = path.join(__dirname, "../");
const wmctrl = require('node-gyp-build')(root);

function getWindowList() {
    try {
        return wmctrl.getWindowList();
    }catch (e) {
        return null;
    }
}
const window_list = getWindowList();


function getActiveWindow() {
    try {
        return wmctrl.getActiveWindow();
    } catch(e) {
        return null;
    }
}
getActiveWindow();
const win = window_list[1];

function getWindowsByPid(pid) {
    if (typeof pid !== "number")
        throw new Error("pid must be a number");
    try {
        return wmctrl.getWindowsByPid(pid);
    } catch(e) {
        return null;
    }
    
}
const pid = win.win_pid;
console.log(getWindowsByPid(pid));

function getWindowsByClassName(class_name) {
    if (typeof class_name === "string")
        throw new Error("class_name must be a string");
    try {
        return wmctrl.getWindowsByClassName(class_name);
    }catch(e) {
        return null;
    }
}

//console.log(getWindowsByClassName("Navigator.Firefox"));

for (let i=0; i < 100; i++)
    getWindowsByClassName("libreoffice.libreoffice-calc");