/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#![windows_subsystem = "windows"]

use std::{env, fs::read, io, process::{Command, Child, Stdio}, sync::Mutex};
#[cfg(any(target_os = "macos", target_os = "linux"))]
use std::collections::{HashMap, VecDeque};

#[cfg(windows)]
use std::os::windows::process::CommandExt;
use std::path::PathBuf;
use wry::{
    application::{
        event::{Event, WindowEvent},
        event_loop::{ControlFlow, EventLoop},
        window::{Icon, WindowBuilder}
    },
    http::{header::CONTENT_TYPE, Response},
    webview::WebViewBuilder,
};
#[cfg(windows)]
mod webview2err;
#[cfg(windows)]
use webview2err::show_webview_err_message;
use std::io::{BufRead, BufReader};
use std::path::MAIN_SEPARATOR;

const SERVER_RELATIVE_LIST: [&str; 4] = ["resources", "profiler", "server", "profiler_server"];
const NO_WINDOW_FLAG: u32 = 0x08000000;

fn run_server(root_path: &PathBuf, cache_path: &PathBuf, port: &mut String) -> Option<Mutex<Child>> {
    let mut server_path = root_path.to_path_buf();
    if cfg!(target_os = "macos") {
        env::set_var("DYLD_LIBRARY_PATH", server_path.join("resources/profiler/server/:$DYLD_LIBRARY_PATH").as_path());
    }

    if cfg!(target_os = "linux") {
        env::set_var("LD_LIBRARY_PATH", server_path.join("resources/profiler/server/:$LD_LIBRARY_PATH").as_path());
    }
    for tmp in SERVER_RELATIVE_LIST {
        server_path.push(tmp);
    }
    if cfg!(target_os = "windows") {
        server_path.set_extension("exe");
    }
    if !server_path.exists() {
        return None;
    }
    let Some(path) = server_path.to_str() else { return None };

    let output = Command::new(path).arg("--scan=9000")
        .arg(format!("--logPath={}", cache_path.display())).output().unwrap();

    let scan_info = String::from_utf8_lossy(&output.stdout).to_string();

    if scan_info.len() > 0 && scan_info.starts_with("Available port: "){
        port.push_str(scan_info.replace("Available port: ", "").trim());
    } else {
        return None;
    };

    let mut server_command = Command::new(path);

    #[cfg(windows)]
    if cfg!(target_os = "windows") {
        server_command.creation_flags(NO_WINDOW_FLAG);
    }

    match server_command.arg(format!("--wsPort={}", port))
        .arg(format!("--logPath={}", cache_path.display()))
        .spawn() {
            Ok(child) => {
                Some(Mutex::new(child))
            }
            _ => {
                None
            }
        }
}

// run script
fn run_script(server_process:Mutex<Child>, root_path: &PathBuf, port: &str) -> wry::Result<()> {
    let event_loop = EventLoop::new();

    let resource_path = root_path.to_path_buf();
    let window: wry::application::window::Window = WindowBuilder::new()
        .with_title("Ascend Insight (powered by MindStudio)")
        .with_maximized(true)
        .build(&event_loop)
        .unwrap();

    #[cfg(windows)]
    {
        use wry::application::platform::windows::IconExtWindows;
        window.set_window_icon(Icon::from_path(resource_path.join("resources/images/icons/mindstudio.ico"), None).ok());
    }

    let _webview = WebViewBuilder::new(window)
        .unwrap()
        .with_custom_protocol("wry".into(), move |request| {
            let _a = request.uri().to_string();
            let path = request.uri().path();
            let content = match read(resource_path.join(&path[1..]).as_path()) {
                Ok(a) => a.into(),
                Err(_e) => {
                    return Err(wry::Error::DuplicateCustomProtocol("".to_string()));
                }
            };

            // Return asset contents and mime types based on file extensions
            // If you don't want to do this manually, there are some crates for you.
            // Such as `infer` and `mime_guess`.
            let mimetype = if path.ends_with(".html") || path == "/" {
                "text/html"
            } else if path.ends_with(".js") {
                "text/javascript"
            } else if path.ends_with(".png") {
                "image/png"
            } else if path.ends_with(".wasm") {
                "application/wasm"
            } else if path.ends_with(".css") {
                "text/css"
            } else if path.ends_with(".svg") {
                "image/svg+xml"
            } else {
                unimplemented!();
            };

            let mut res_builder = Response::builder();
            for (k, v) in request.headers() {
                res_builder = res_builder.header(k, v)
            }
            res_builder
                .header(CONTENT_TYPE, mimetype)
                .body(content)
                .map_err(Into::into)
        })
        .with_url(format!("wry://localhost/resources/profiler/frontend/index.html?port={}", port).as_str())?
        .build()?;
    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Wait;
        match event {
            Event::WindowEvent {
                event: WindowEvent::CloseRequested,
                ..
            } => *control_flow = {
                let mut server_process_process = server_process.lock().unwrap();
                let pid = server_process_process.id();
                match query_child_pids(pid) {
                    Ok(child_pids) => {
                        for child_pid in child_pids {
                            match kill_process_tree(child_pid) {
                                Ok(()) => {}
                                Err(_err) => {}
                            }
                        }
                    }
                    Err(_err) => {}
                }
                let _ = server_process_process.kill();
                ControlFlow::Exit
            },

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
                    let reader = BufReader::new(stdout);
                    for line in reader.lines().skip(1) {
                        match line {
                            Ok(line) => {
                                let line_trim = line.trim();
                                if !line_trim.is_empty() {
                                    child_pids.push(line_trim.to_string());
                                }
                            },
                            Err(_err) => {},
                        }
                    }
                }
            }
            Err(_err) => {}
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
            Ok(pair) => {
                if let Some(stdout) = pair.stdout {
                    let reader = BufReader::new(stdout);
                    for line in reader.lines().skip(1) {
                        let line_str = line.unwrap();
                        let parts: Vec<&str> = line_str.split_whitespace().collect();
                        if parts.len() == 2 {
                            let key = parts.get(0).unwrap().to_string();
                            let value = parts.get(1).unwrap().to_string();
                            if let Some(vec) = pid_map.get_mut(&key) {
                                vec.push(value);
                            } else {
                                let mut new_vec = Vec::new();
                                new_vec.push(value);
                                pid_map.insert(key, new_vec);
                            }
                        }
                    }
                }
            },
            Err(_err) => {}
        }

        let mut deque = VecDeque::new();
        deque.push_back(parent_pid.to_string());
        while !deque.is_empty() {
            let cur = deque.pop_back();
            if let Some(vec) = pid_map.get_mut(&cur.unwrap().to_string()) {
                for item in vec {
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
            .output()?;
    }
    #[cfg(any(target_os = "macos", target_os = "linux"))]
    {
        Command::new("kill")
            .arg("-9")
            .arg(parent_pid.to_string())
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status()?;
    }

    Ok(())
}

fn home_dir() -> Option<PathBuf> {
    let home: PathBuf;
    if cfg!(target_os = "windows") && env::var("USERPROFILE").is_ok() {
        home = PathBuf::from(env::var("USERPROFILE").unwrap());
    } else {
        home = PathBuf::from(env::var("HOME").unwrap());
    }
    Some(home)
}

#[cfg(windows)]
#[link(name = "shell32")]
extern "system" {
    pub fn IsUserAnAdmin() -> bool;
}

#[cfg(windows)]
fn is_admin() -> bool {
    unsafe {
        IsUserAnAdmin()
    }
}

#[cfg(windows)]
fn compare_first_segment(s1: &str, s2: &str) -> bool {
    let s1_segments: Vec<&str> = s1.split(MAIN_SEPARATOR).collect();
    let s2_segments: Vec<&str> = s2.split(MAIN_SEPARATOR).collect();

    if s1_segments.is_empty() || s2_segments.is_empty() {
        return false;
    }

    s1_segments[0] == s2_segments[0]
}

fn main() {
    let mut cache_path = home_dir().unwrap().join(".ascend_insight"); //cache folder generated for each user.
    let root_path =  std::env::current_exe().unwrap().parent().unwrap().to_path_buf();
    std::fs::create_dir_all(cache_path.as_path()).expect("no permission to create cache_path");
    #[cfg(windows)] {
        //当用户安装在C盘时，使用user目录
        let mut webview_path = cache_path.clone();
        //当用户安装在C盘外时，使用安装目录
        if !compare_first_segment(cache_path.to_str().unwrap(), root_path.to_str().unwrap()) {
            cache_path = root_path.join(".ascend_insight");
            webview_path = cache_path.clone();
        }
        if is_admin() {
            webview_path.push("admin");
        }
        env::set_var("WEBVIEW2_USER_DATA_FOLDER", webview_path.as_path());
    }

    if wry::webview::webview_version().is_err() {
        #[cfg(windows)] {
            show_webview_err_message();
        }
        return;
    }
    let mut port : String = String::new();
    // start the server
    if let Some(child) = run_server(&root_path, &cache_path, &mut port) {
        let _ = run_script(child, &root_path, &port.as_str());
    }
}
