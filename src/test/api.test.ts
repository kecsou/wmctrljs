// API surface tests: run anywhere, no X server needed.
import { test, describe } from "node:test";
import * as assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import * as path from "node:path";
import * as wm from "../index";

// Every call must fail to connect: this file runs in its own process.
process.env.DISPLAY = ":4242";

const api = wm as unknown as Record<string, (...args:unknown[]) => unknown>;
const INDEX = path.join(__dirname, "../index.js");

const NATIVE_NAMES = [
    "activeWindowByIdAsync", "activeWindowByIdSync", "activeWindowsByClassNameAsync", "activeWindowsByClassNameSync",
    "activeWindowsByPidAsync", "activeWindowsByPidSync", "closeWindowByIdAsync", "closeWindowByIdSync",
    "closeWindowsByClassNameAsync", "closeWindowsByClassNameSync", "closeWindowsByPidAsync", "closeWindowsByPidSync",
    "getActiveWindowAsync", "getActiveWindowSync", "getScreenAsync", "getScreenSync", "getWindowByIdAsync",
    "getWindowByIdSync", "getWindowListAsync", "getWindowListSync", "getWindowsByClassNameAsync",
    "getWindowsByClassNameSync", "getWindowsByPidAsync", "getWindowsByPidSync", "windowAllowAllSizesAsync",
    "windowAllowAllSizesSync", "windowMinimizeAsync", "windowMinimizeSync", "windowMoveResizeAsync",
    "windowMoveResizeSync", "windowMoveResizeTransitionAsync", "windowRaiseAsync", "windowRaiseSync",
    "windowStateAsync", "windowStateSync"
];

/** A parameter: its name, the type `expectParam` checks, and a valid value. */
type Param = [name:string, type:"number"|"string", valid:unknown];
const id:Param = ["win_id", "number", 1];
const pid:Param = ["win_pid", "number", 1];
const cls:Param = ["win_class_name", "string", "a.A"];

/** Every public function: [name, params, has a Sync variant]. */
const FUNCTIONS:[string, Param[], boolean][] = [
    ["getScreen", [], true],
    ["getWindowList", [], true],
    ["getActiveWindow", [], true],
    ["getWindowById", [id], true],
    ["getWindowsByPid", [pid], true],
    ["getWindowsByClassName", [cls], true],
    ["activeWindowById", [id], true],
    ["activeWindowsByPid", [pid], true],
    ["activeWindowsByClassName", [cls], true],
    ["closeWindowById", [id], true],
    ["closeWindowsByPid", [pid], true],
    ["closeWindowsByClassName", [cls], true],
    ["windowMoveResize", [id, ["win_gravity", "number", 0], ["win_x", "number", 0], ["win_y", "number", 0],
        ["win_width", "number", 10], ["win_height", "number", 10]], true],
    ["windowMoveResizeTransition", [id, ["win_gravity", "number", 0], ["win_x", "number", 0], ["win_y", "number", 0],
        ["win_width", "number", 10], ["win_height", "number", 10], ["await_time", "number", 0], ["pad", "number", 1]], false],
    ["windowState", [id, ["action", "string", "add"], ["prop1", "string", "above"], ["prop2", "string", "below"]], true],
    ["windowMinimize", [id], true],
    ["windowAllowAllSizes", [id], true],
    ["windowRaise", [id], true],
];

/** Valid arguments for `params`. */
const valid = (params:Param[]) => params.map(p => p[2]);
/** The error every call gets without an X server. */
const noDisplay = (name:string) => ({ name: "Error", message: `[${name}] CAN_NOT_OPEN_DISPLAY (3)` });

test("the module exports exactly the documented functions", () => {
    const names = FUNCTIONS.flatMap(([n, , sync]) => sync ? [n, `${n}Sync`] : [n]).sort();
    assert.deepEqual(Object.keys(wm).filter(k => typeof api[k] === "function").sort(), names);
});

test("the native binary exports exactly the expected functions", () => {
    const musl = process.platform === "linux" && !(process.report.getReport() as any).header?.glibcVersionRuntime;
    const native = require(`../../prebuilds/${process.platform}-${process.arch}${musl ? "-musl" : ""}/wmctrljs.node`);
    assert.deepEqual(Object.keys(native).sort(), NATIVE_NAMES);
});

for (const [name, params, hasSync] of FUNCTIONS) {
    describe(name, () => {
        test("async variant rejects with an Error when no display can be opened", async () => {
            const p = api[name](...valid(params));
            assert.ok(p instanceof Promise);
            await assert.rejects(p as Promise<unknown>, noDisplay(name));
        });

        if (hasSync)
            test("sync variant throws an Error when no display can be opened", () => {
                assert.throws(() => api[`${name}Sync`](...valid(params)), noDisplay(`${name}Sync`));
            });

        params.forEach(([param, type], i) => {
            const wrong = type === "number" ? "1" : 1;
            const args = (bad:unknown) => valid(params).map((v, j) => j === i ? bad : v);
            const message = `[${name}] expect a type [${type}] for param [${param}]`;

            test(`rejects a ${typeof wrong} ${param}`, async () => {
                await assert.rejects(async () => api[name](...args(wrong)), { message });
                if (hasSync)
                    assert.throws(() => api[`${name}Sync`](...args(wrong)), { message });
            });

            if (type === "number")
                test(`rejects a NaN ${param}`, async () => {
                    const nan = { message: `${message} received NaN value` };
                    await assert.rejects(async () => api[name](...args(NaN)), nan);
                    if (hasSync)
                        assert.throws(() => api[`${name}Sync`](...args(NaN)), nan);
                });
        });
    });
}

describe("windowState validation", () => {
    for (const action of ["bogus", "", "added"])
        test(`refuses action "${action}"`, async () => {
            const message = `[windowState] only actions [REMOVE,ADD,TOGGLE] are enabled note (${action.toUpperCase()})`;
            assert.throws(() => wm.windowStateSync(1, action, "above"), { message });
            await assert.rejects(wm.windowState(1, action, "above"), { message });
        });

    const props = "modal,sticky,maximized_vert,maximized_horz,shaded,skip_taskbar,skip_pager,hidden,fullscreen,above,below";
    for (const prop of ["bogus", "demands_attention", "maximized"])
        test(`refuses property "${prop}"`, async () => {
            assert.throws(() => wm.windowStateSync(1, "add", prop), { message: `[windowState] property prop1 (${prop}) is not an enabled property ${props}` });
            assert.throws(() => wm.windowStateSync(1, "add", "above", prop), { message: `[windowState] property prop2 (${prop}) is not an enabled property ${props}` });
            await assert.rejects(wm.windowState(1, "add", prop), /is not an enabled property/);
        });

    for (const prop of props.split(","))
        test(`accepts property "${prop}" in any case`, () => {
            // Validation passes, then the native call fails to reach the display.
            assert.throws(() => wm.windowStateSync(1, "Toggle", prop.toUpperCase()), noDisplay("windowStateSync"));
            assert.throws(() => wm.windowStateSync(1, "remove", "above", prop), noDisplay("windowStateSync"));
        });

    test("prop2 is optional and may be empty", () => {
        assert.throws(() => wm.windowStateSync(1, "add", "above"), noDisplay("windowStateSync"));
        assert.throws(() => wm.windowStateSync(1, "add", "above", ""), noDisplay("windowStateSync"));
    });
});

test("windowMoveResizeTransition refuses a pause no timer can hold", async () => {
    // Checked before connecting, so this needs no display.
    for (const ms of [Infinity, 1e300])
        await assert.rejects(wm.windowMoveResizeTransition(1, 0, 0, 0, 10, 10, ms, 1), { message: "[windowMoveResizeTransition] CAN_NOT_MOVE_RESIZE_WINDOW (27)" });
});

describe("binary loader", () => {
    // Runs index.js in a child process after `setup`, printing the native path it requires.
    const load = (setup:string) => execFileSync(process.execPath, ["-e", `
        const Module = require("module");
        const req = Module.prototype.require;
        Module.prototype.require = function (id) {
            if (id.endsWith(".node")) { console.log(id); process.exit(0); }
            return req.apply(this, arguments);
        };
        ${setup}
        require(${JSON.stringify(INDEX)});`]).toString().trim();

    test("loads prebuilds/<platform>-<arch>", () => {
        assert.match(load(""), new RegExp(`^\\.\\./prebuilds/${process.platform}-${process.arch}(-musl)?/wmctrljs\\.node$`));
    });

    test("picks the musl build when the runtime has no glibc", { skip: process.platform !== "linux" }, () => {
        assert.equal(load(`process.report.getReport = () => ({ header: {} });`), `../prebuilds/linux-${process.arch}-musl/wmctrljs.node`);
        assert.equal(load(`process.report.getReport = () => ({ header: { glibcVersionRuntime: "2.17" } });`), `../prebuilds/linux-${process.arch}/wmctrljs.node`);
    });

    test("explains which binary is missing", () => {
        const out = execFileSync(process.execPath, ["-e", `
            Object.defineProperty(process, "arch", { value: "mips" });
            try { require(${JSON.stringify(INDEX)}); } catch (e) { console.log(e.message); console.log(e.cause instanceof Error); }`]).toString();
        assert.match(out, /^wmctrljs: cannot load the binary for \w+-mips(-musl)?: Cannot find module/);
        assert.match(out, /\ntrue\n$/);
    });
});
