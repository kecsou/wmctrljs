//! N-API bindings: every operation is exported as `<name>Sync` and `<name>Async`.

use napi::bindgen_prelude::*;
use napi_derive::napi;
use serde_json::Value;

mod wm;
use wm::{By, Fail, Res};

pub struct Json(Value);

impl TypeName for Json {
    fn type_name() -> &'static str {
        "Object"
    }

    fn value_type() -> ValueType {
        ValueType::Object
    }
}

impl ToNapiValue for Json {
    unsafe fn to_napi_value(env: sys::napi_env, val: Self) -> Result<sys::napi_value> {
        unsafe { Value::to_napi_value(env, val.0) }
    }
}

fn error(name: &str, fail: Fail) -> Error {
    Error::from_reason(format!("[{name}] {fail}"))
}

fn now(name: &str, res: Res) -> Result<Json> {
    res.map(Json).map_err(|f| error(name, f))
}

type Op = Box<dyn FnOnce() -> Res + Send>;

/// Runs an operation on the libuv thread pool and settles a promise with it.
pub struct Job(&'static str, Option<Op>);

impl Task for Job {
    type Output = Value;
    type JsValue = Json;

    fn compute(&mut self) -> Result<Value> {
        let op = self.1.take().expect("a job runs once");
        op().map_err(|f| error(self.0, f))
    }

    fn resolve(&mut self, _env: Env, out: Value) -> Result<Json> {
        Ok(Json(out))
    }
}

fn later(name: &'static str, op: impl FnOnce() -> Res + Send + 'static) -> AsyncTask<Job> {
    AsyncTask::new(Job(name, Some(Box::new(op))))
}

#[napi]
pub fn get_screen_sync() -> Result<Json> {
    now("getScreenSync", wm::screen())
}

#[napi]
pub fn get_screen_async() -> AsyncTask<Job> {
    later("getScreen", wm::screen)
}

#[napi]
pub fn get_window_by_id_sync(win_id: u32) -> Result<Json> {
    now("getWindowByIdSync", wm::window(win_id))
}

#[napi]
pub fn get_window_by_id_async(win_id: u32) -> AsyncTask<Job> {
    later("getWindowById", move || wm::window(win_id))
}

#[napi]
pub fn get_active_window_sync() -> Result<Json> {
    now("getActiveWindowSync", wm::active_window())
}

#[napi]
pub fn get_active_window_async() -> AsyncTask<Job> {
    later("getActiveWindow", wm::active_window)
}

#[napi]
pub fn get_window_list_sync() -> Result<Json> {
    now("getWindowListSync", wm::list())
}

#[napi]
pub fn get_window_list_async() -> AsyncTask<Job> {
    later("getWindowList", wm::list)
}

#[napi]
pub fn get_windows_by_pid_sync(win_pid: u32) -> Result<Json> {
    now("getWindowsByPidSync", wm::windows_by(&By::Pid(win_pid)))
}

#[napi]
pub fn get_windows_by_pid_async(win_pid: u32) -> AsyncTask<Job> {
    later("getWindowsByPid", move || wm::windows_by(&By::Pid(win_pid)))
}

#[napi]
pub fn get_windows_by_class_name_sync(win_class_name: String) -> Result<Json> {
    now("getWindowsByClassNameSync", wm::windows_by(&By::Class(win_class_name)))
}

#[napi]
pub fn get_windows_by_class_name_async(win_class_name: String) -> AsyncTask<Job> {
    later("getWindowsByClassName", move || wm::windows_by(&By::Class(win_class_name)))
}

#[napi]
pub fn active_window_by_id_sync(win_id: u32) -> Result<Json> {
    now("activeWindowByIdSync", wm::activate(win_id))
}

#[napi]
pub fn active_window_by_id_async(win_id: u32) -> AsyncTask<Job> {
    later("activeWindowById", move || wm::activate(win_id))
}

#[napi]
pub fn active_windows_by_pid_sync(win_pid: u32) -> Result<Json> {
    now("activeWindowsByPidSync", wm::activate_by(&By::Pid(win_pid)))
}

#[napi]
pub fn active_windows_by_pid_async(win_pid: u32) -> AsyncTask<Job> {
    later("activeWindowsByPid", move || wm::activate_by(&By::Pid(win_pid)))
}

#[napi]
pub fn active_windows_by_class_name_sync(win_class_name: String) -> Result<Json> {
    now("activeWindowsByClassNameSync", wm::activate_by(&By::Class(win_class_name)))
}

#[napi]
pub fn active_windows_by_class_name_async(win_class_name: String) -> AsyncTask<Job> {
    later("activeWindowsByClassName", move || wm::activate_by(&By::Class(win_class_name)))
}

#[napi]
pub fn close_window_by_id_sync(win_id: u32) -> Result<Json> {
    now("closeWindowByIdSync", wm::close(win_id))
}

#[napi]
pub fn close_window_by_id_async(win_id: u32) -> AsyncTask<Job> {
    later("closeWindowById", move || wm::close(win_id))
}

#[napi]
pub fn close_windows_by_pid_sync(win_pid: u32) -> Result<Json> {
    now("closeWindowsByPidSync", wm::close_by(&By::Pid(win_pid)))
}

#[napi]
pub fn close_windows_by_pid_async(win_pid: u32) -> AsyncTask<Job> {
    later("closeWindowsByPid", move || wm::close_by(&By::Pid(win_pid)))
}

#[napi]
pub fn close_windows_by_class_name_sync(win_class_name: String) -> Result<Json> {
    now("closeWindowsByClassNameSync", wm::close_by(&By::Class(win_class_name)))
}

#[napi]
pub fn close_windows_by_class_name_async(win_class_name: String) -> AsyncTask<Job> {
    later("closeWindowsByClassName", move || wm::close_by(&By::Class(win_class_name)))
}

#[napi]
pub fn window_move_resize_sync(win_id: u32, gravity: i32, x: i32, y: i32, width: i32, height: i32) -> Result<Json> {
    now("windowMoveResizeSync", wm::move_resize(win_id, gravity, x, y, width, height))
}

#[napi]
pub fn window_move_resize_async(win_id: u32, gravity: i32, x: i32, y: i32, width: i32, height: i32) -> AsyncTask<Job> {
    later("windowMoveResize", move || wm::move_resize(win_id, gravity, x, y, width, height))
}

#[napi]
#[allow(clippy::too_many_arguments)]
pub fn window_move_resize_transition_async(
    win_id: u32,
    gravity: i32,
    x: i32,
    y: i32,
    width: i32,
    height: i32,
    await_time: f64,
    pad: i32,
) -> AsyncTask<Job> {
    later("windowMoveResizeTransition", move || {
        wm::move_resize_transition(win_id, gravity, x, y, width, height, await_time, pad)
    })
}

#[napi]
pub fn window_state_sync(win_id: u32, action: String, prop1: String, prop2: String) -> Result<Json> {
    now("windowStateSync", wm::state(win_id, &action, &prop1, &prop2))
}

#[napi]
pub fn window_state_async(win_id: u32, action: String, prop1: String, prop2: String) -> AsyncTask<Job> {
    later("windowState", move || wm::state(win_id, &action, &prop1, &prop2))
}

#[napi]
pub fn window_minimize_sync(win_id: u32) -> Result<Json> {
    now("windowMinimizeSync", wm::minimize(win_id))
}

#[napi]
pub fn window_minimize_async(win_id: u32) -> AsyncTask<Job> {
    later("windowMinimize", move || wm::minimize(win_id))
}

#[napi]
pub fn window_allow_all_sizes_sync(win_id: u32) -> Result<Json> {
    now("windowAllowAllSizesSync", wm::allow_all_sizes(win_id))
}

#[napi]
pub fn window_allow_all_sizes_async(win_id: u32) -> AsyncTask<Job> {
    later("windowAllowAllSizes", move || wm::allow_all_sizes(win_id))
}

#[napi]
pub fn window_raise_sync(win_id: u32) -> Result<Json> {
    now("windowRaiseSync", wm::raise(win_id))
}

#[napi]
pub fn window_raise_async(win_id: u32) -> AsyncTask<Job> {
    later("windowRaise", move || wm::raise(win_id))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn errors_are_prefixed_with_the_js_name() {
        assert_eq!(error("getScreenSync", Fail::CanNotOpenDisplay).reason, "[getScreenSync] CAN_NOT_OPEN_DISPLAY (3)");
        assert_eq!(now("closeWindowsByPidSync", Err(Fail::NoWindowFound)).err().unwrap().reason, "[closeWindowsByPidSync] NO_WINDOW_FOUND (24)");
        assert_eq!(now("x", Ok(Value::Bool(true))).ok().unwrap().0, Value::Bool(true));
    }

    #[test]
    fn jobs_compute_their_operation_once() {
        let mut ok = Job("getWindowList", Some(Box::new(|| Ok(Value::Array(vec![])))));
        assert_eq!(ok.compute().unwrap(), Value::Array(vec![]));
        let mut fail = Job("getWindowList", Some(Box::new(|| Err(Fail::CanNotGetClientList))));
        assert_eq!(fail.compute().unwrap_err().reason, "[getWindowList] CAN_NOT_GET_CLIENT_LIST (1)");
        assert!(std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| ok.compute())).is_err());
    }

    #[test]
    fn no_display_fails_every_operation() {
        // SAFETY: tests touching DISPLAY run in this single test.
        unsafe { std::env::set_var("DISPLAY", ":4242") };
        let ops: Vec<(&str, Res)> = vec![
            ("screen", wm::screen()),
            ("window", wm::window(1)),
            ("active_window", wm::active_window()),
            ("list", wm::list()),
            ("windows_by", wm::windows_by(&By::Pid(1))),
            ("activate", wm::activate(1)),
            ("activate_by", wm::activate_by(&By::Class("a.A".into()))),
            ("close", wm::close(1)),
            ("close_by", wm::close_by(&By::Pid(1))),
            ("move_resize", wm::move_resize(1, 0, 0, 0, 1, 1)),
            ("move_resize_transition", wm::move_resize_transition(1, 0, 0, 0, 1, 1, 0.0, 1)),
            ("state", wm::state(1, "ADD", "above", "")),
            ("minimize", wm::minimize(1)),
            ("allow_all_sizes", wm::allow_all_sizes(1)),
            ("raise", wm::raise(1)),
        ];
        for (name, res) in ops {
            assert_eq!(res, Err(Fail::CanNotOpenDisplay), "{name}");
        }
        // Validated before connecting.
        assert_eq!(wm::state(1, "add", "above", ""), Err(Fail::CanNotSetWindowState));
    }
}
