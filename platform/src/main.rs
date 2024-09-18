/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#![windows_subsystem = "windows"]

use std::{
    env,
    env::current_exe,
    fs::{create_dir_all, read},
    io,
    io::{BufRead, BufReader},
    path::PathBuf,
    process::{Child, Command, Stdio},
    sync::Mutex,
};
#[cfg(any(target_os = "macos", target_os = "linux"))]
use std::collections::{HashMap, VecDeque};
#[cfg(windows)]
use std::os::windows::process::CommandExt;

use wry::{
    application::{
        event::{Event, WindowEvent},
        event_loop::{ControlFlow, EventLoop},
        window::{Window, WindowBuilder},
    },
    http::{header::CONTENT_TYPE, Response},
    webview::{webview_version, WebViewBuilder, FileDropEvent},
};
#[cfg(windows)]
use webview2err::show_webview_err_message;

#[cfg(windows)]
mod webview2err;

const SERVER_RELATIVE_LIST: [&str; 4] =
    ["resources", "profiler", "server", "profiler_server"];
const NO_WINDOW_FLAG: u32 = 0x08000000;
const MIMETYPE_HTML: &str = "text/html";

fn run_server(
    root_path: &PathBuf,
    cache_path: &PathBuf,
    port: &mut String,
    sid: &String,
) -> Option<Mutex<Child>> {
    let mut server_path = root_path.to_path_buf();

    #[cfg(target_os = "macos")]
    {
        env::set_var(
            "DYLD_LIBRARY_PATH",
            server_path
                .join("resources/profiler/server/:$DYLD_LIBRARY_PATH")
                .as_path(),
        );
    }

    #[cfg(target_os = "linux")]
    {
        let ld_library_path = env::var_os("LD_LIBRARY_PATH").unwrap_or_default();
        let new_ld_library_path = format!(
            "{}:{}",
             server_path.join("resources/profiler/server/").display(),
             ld_library_path.to_string_lossy()
        );
        env::set_var("LD_LIBRARY_PATH", &new_ld_library_path);
    }

    for tmp in SERVER_RELATIVE_LIST {
        server_path.push(tmp);
    }

    #[cfg(windows)]
    {
        server_path.set_extension("exe");
    }

    if !server_path.exists() {
        return None;
    }
    let Some(path) = server_path.to_str() else {
        return None;
    };

    let output = Command::new(path)
        .arg("--scan=9000")
        .arg(format!("--logPath={}", cache_path.display()))
        .output()
        .expect("Failed to start MindStudio Insight server");

    let scan_info = String::from_utf8_lossy(&output.stdout).to_string();

    if scan_info.len() > 0 && scan_info.starts_with("Available port: ") {
        port.push_str(scan_info.replace("Available port: ", "").trim());
    } else {
        return None;
    };

    let mut server_command = Command::new(path);

    #[cfg(windows)]
    {
        server_command.creation_flags(NO_WINDOW_FLAG);
    }

    match server_command
        .arg(format!("--wsPort={}", port))
        .arg(format!("--logPath={}", cache_path.display()))
        .arg(format!("--sid={}", sid))
        .spawn()
    {
        Ok(child) => Some(Mutex::new(child)),
        _ => None,
    }
}

// run script
fn run_script(
    server_process: Mutex<Child>,
    root_path: &PathBuf,
    port: &str,
    sid: &String,
) -> wry::Result<()> {
    let event_loop = EventLoop::with_user_event();
    
    let proxy = event_loop.create_proxy();

    let resource_path = root_path.to_path_buf();
    let window: Window = WindowBuilder::new()
        .with_title("MindStudio Insight")
        .with_maximized(true)
        .build(&event_loop)
        .expect("Error occurred when create App window");

    #[cfg(windows)]
    {
        use wry::application::window::Icon;
        use wry::application::platform::windows::IconExtWindows;

        window.set_window_icon(
            Icon::from_path(
                resource_path.join("resources/images/icons/mindstudio.ico"),
                None,
            )
            .ok(),
        );
    }

    let webview = WebViewBuilder::new(window)
        .expect("Failed to create webview builder")
        .with_custom_protocol("wry".into(), move |request| {
            let path = request.uri().path();
            let content = match read(resource_path.join(&path[1..]).as_path()) {
                Ok(a) => a.into(),
                Err(e) => return Err(wry::Error::Io(e)),
            };

            let mut mimetype = MIMETYPE_HTML;
            if let Some((_, ext)) = path.rsplit_once('.') {
                mimetype = match ext {
                    "html" => "text/html",
                    "js" => "text/javascript",
                    "css" => "text/css",
                    "svg" => "image/svg+xml",
                    "png" => "image/png",
                    "wasm" => "application/wasm",
                    _ => MIMETYPE_HTML,
                }
            }

            Response::builder().header(CONTENT_TYPE, mimetype).body(content).map_err(Into::into)
        })
        .with_url(
            format!("wry://localhost/resources/profiler/frontend/index.html?port={}&sid={}", port, sid)
                .as_str(),
        )?
        .with_file_drop_handler(move |window, ev| {
            match ev {
                FileDropEvent::Dropped(paths) => {
                    if let Err(e) = proxy.send_event(paths[0].to_owned()) {
                        eprintln!("app closed unexpectly: {:#?}", e);
                    }
                }
                _ => {}
            }

            true
        })
        .build()
        .expect("Failed to create webview");

    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Wait;
        match event {
            Event::WindowEvent {
                event: WindowEvent::CloseRequested, ..
            } => {
                *control_flow = {
                    let mut server_process_guard = server_process
                        .lock()
                        .expect("Failed to lock server-process mutex");
                    let pid = server_process_guard.id();
                    match query_child_pids(pid) {
                        Ok(child_pids) => {
                            for child_pid in child_pids {
                                if let Err(e) = kill_process_tree(child_pid) {
                                    eprintln!("Err when kill process: {e}")
                                }
                            }
                        }
                        Err(e) => eprintln!("Err when query child pids {e}"),
                    }
                    server_process_guard
                        .kill()
                        .expect("server process could not be killed");
                    ControlFlow::Exit
                }
            }
            Event::UserEvent(path) => {
                if let Err(e) = webview.evaluate_script(&format!("window.handleDrop({:#?})",path)) {
                    eprintln!("drop-file ipc failed: {:#?}", e);
                }
            }
            _ => (),
        }
    });
}

fn query_child_pids(parent_pid: u32) -> io::Result<Vec<String>> {
    #[cfg(windows)]
    {
        let wmic_output = Command::new("wmic")
            .arg("process")
            .arg("where")
            .arg(format!("ParentProcessId={}", parent_pid))
            .arg("get")
            .arg("ProcessId")
            .creation_flags(0x08000000)
            .stdout(Stdio::piped())
            .spawn();

        let mut child_pids = Vec::new();
        match wmic_output {
            Ok(child) => {
                if let Some(stdout) = child.stdout {
                    BufReader::new(stdout)
                        .lines()
                        .skip(1)
                        .filter_map(|line| line.ok())
                        .map(|line| line.trim().to_string())
                        .filter(|s| !s.is_empty())
                        .for_each(|pid| child_pids.push(pid));
                }
            }
            Err(e) => eprintln!("wmic command failed: {e}"),
        }
        Ok(child_pids)
    }

    #[cfg(any(target_os = "macos", target_os = "linux"))]
    {
        let ps_output = Command::new("ps")
            .arg("-o")
            .arg("ppid,pid")
            .stdout(Stdio::piped())
            .spawn();
        let mut child_pids = Vec::new();
        let mut pid_map: HashMap<String, Vec<String>> = HashMap::new();
        match ps_output {
            Ok(pairs) => {
                if let Some(stdout) = pairs.stdout {
                    let reader = BufReader::new(stdout);
                    for line in reader.lines().skip(1) {
                        if let Ok(line) = line {
                            let parts: Vec<&str> =
                                line.split_whitespace().collect();
                            if let [key, value] = parts.as_slice() {
                                pid_map
                                    .entry(key.to_string())
                                    .and_modify(|v| v.push(value.to_string()))
                                    .or_insert(vec![value.to_string()]);
                            }
                        }
                    }
                }
            }
            Err(e) => eprintln!("ps command failed: {e}"),
        }

        let mut deque = VecDeque::new();
        deque.push_back(parent_pid.to_string());
        while let Some(cur) = deque.pop_back() {
            if let Some(children) = pid_map.get(&cur) {
                for item in children {
                    child_pids.push(item.to_string());
                    deque.push_back(item.to_string());
                }
            }
        }

        Ok(child_pids)
    }
}

fn kill_process_tree(parent_pid: String) -> io::Result<()> {
    #[cfg(windows)]
    {
        // Windows 下使用 taskkill 命令
        Command::new("taskkill")
            .arg("/f")
            .arg("/t") // 终止包括子进程在内的所有进程
            .arg("/im")
            .arg(parent_pid.to_string())
            .creation_flags(0x08000000)
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .output()
            .expect("Failed to execute taskkill command");
    }
    #[cfg(any(target_os = "macos", target_os = "linux"))]
    {
        Command::new("kill")
            .arg("-9")
            .arg(parent_pid.to_string())
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status()
            .expect("Failed to execute kill command");
    }

    Ok(())
}

fn home_dir() -> Option<PathBuf> {
    #[cfg(target_os = "windows")]
    {
        env::var("USERPROFILE").ok().map(PathBuf::from)
    }

    #[cfg(not(target_os = "windows"))]
    {
        env::var("HOME").ok().map(PathBuf::from)
    }
}

#[cfg(windows)]
#[link(name = "shell32")]
extern "system" {
    pub fn IsUserAnAdmin() -> bool;
}

#[cfg(windows)]
fn is_admin() -> bool {
    unsafe { IsUserAnAdmin() }
}

#[cfg(windows)]
fn eq_prefix(lhs: &PathBuf, rhs: &PathBuf) -> bool {
    match (lhs.components().next(), rhs.components().next()) {
        (Some(l), Some(r)) => l == r,
        _ => false,
    }
}

fn main() {
    let mut cache_path = home_dir()
        .expect("Home dir not exists, check your env variable")
        .join(".mindstudio_insight"); //cache folder generated for each user.
    let root_path = current_exe()
        .expect("Failed to get current exe path")
        .parent()
        .expect("Failed to get parent path of  current exe")
        .to_path_buf();

    #[cfg(windows)]
    {
        // 当用户安装在C盘时，使用user目录
        let mut webview_path = cache_path.clone();
        // 当用户安装在C盘外时，使用安装目录
        if !eq_prefix(&cache_path, &root_path) {
            cache_path = root_path.join(".mindstudio_insight");
            webview_path = cache_path.clone();
        }
        if is_admin() {
            webview_path.push("admin");
        }
        env::set_var("WEBVIEW2_USER_DATA_FOLDER", webview_path.as_path());
    }

    if !cache_path.exists() {
        #[cfg(windows)]
        create_dir_all(cache_path.as_path())
            .expect("no permission to create cache_path");

        #[cfg(unix)]
        {
            use std::{fs::DirBuilder, os::unix::fs::DirBuilderExt};

            DirBuilder::new()
                .recursive(true)
                .mode(0o750)
                .create(cache_path.as_path())
                .expect("no permission to create cache_path");
        }
    }

    if webview_version().is_err() {
        #[cfg(windows)]
        {
            show_webview_err_message();
        }
        return;
    }

    let mut port: String = String::new();
    let sid = uuid::Uuid::new_v4().to_string();
    // start the server
    if let Some(child) = run_server(&root_path, &cache_path, &mut port, &sid) {
        let _ = run_script(child, &root_path, &port.as_str(), &sid);
    }
}
