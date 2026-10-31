/**
 * @file Builds the addon into prebuilds/<dir>/wmctrljs.node and the X11 test fixture into fixtures/<dir>/.
 * Usage: node scripts/build-native.js [--target <rust triple>[.<glibc version>]] [--dir <platform>-<arch>[-musl]] [--zig]
 * --zig cross-compiles with cargo-zigbuild, which also accepts a minimum glibc suffix (x86_64-unknown-linux-gnu.2.17).
 */
const { parseArgs } = require("node:util");
const { execFileSync } = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");

const { values: opts } = parseArgs({ options: { target: { type: "string" }, dir: { type: "string" }, zig: { type: "boolean" } } });
const native = path.join(__dirname, "../native");
const triple = opts.target?.replace(/(\.\d+)+$/, "");
const musl = process.platform === "linux" && !process.report.getReport().header.glibcVersionRuntime;
const dir = opts.dir ?? `${process.platform}-${process.arch}${musl ? "-musl" : ""}`;
const platform = dir.split("-")[0];
/**
 * Runs a command in native/, showing its output, and throws if it fails.
 * @param {string} cmd
 * @param {string[]} args
 */
const run = (cmd, args) => execFileSync(cmd, args, { cwd: native, stdio: "inherit" });

if (triple)
    run("rustup", ["target", "add", triple]);
// cwd is native/ so cargo picks up native/.cargo/config.toml.
run("cargo", [opts.zig ? "zigbuild" : "build", "--release", "--lib", "--example", "fixture", ...(opts.target ? ["--target", opts.target] : [])]);

const out = path.join(native, "target", triple ?? "", "release");
const lib = { win32: "wmctrljs.dll", darwin: "libwmctrljs.dylib" }[platform] ?? "libwmctrljs.so";
const exe = platform === "win32" ? "fixture.exe" : "fixture";
for (const d of ["prebuilds", "fixtures"])
    fs.mkdirSync(path.join(__dirname, "..", d, dir), { recursive: true });
fs.copyFileSync(path.join(out, lib), path.join(__dirname, "../prebuilds", dir, "wmctrljs.node"));
fs.copyFileSync(path.join(out, "examples", exe), path.join(__dirname, "../fixtures", dir, exe));
