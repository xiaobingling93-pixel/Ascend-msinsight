/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#[cfg(windows)]
use std::os::windows::process::CommandExt;
use std::{
    env,
    env::current_exe,
    path::PathBuf,
    process::{Child, Command},
    sync::{Arc, Mutex},
};

use crate::webview;

const SERVER_RELATIVE_LIST: [&str; 4] =
    ["resources", "profiler", "server", "profiler_server"];
#[cfg(windows)]
const NO_WINDOW_FLAG: u32 = 0x08000000;

fn server_path(root_path: &PathBuf) -> Option<PathBuf> {
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
        let ld_library_path =
            env::var_os("LD_LIBRARY_PATH").unwrap_or_default();
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
    server_path.set_extension("exe");

    if !server_path.exists() {
        return None;
    }

    Some(server_path)
}

fn run_server(
    root_path: &PathBuf,
    cache_path: &PathBuf,
    port: &mut String,
    sid: &String,
) -> Option<Mutex<Child>> {
    let binding = server_path(root_path)?;
    let Some(path) = binding.to_str() else { return None };

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
    server_command.creation_flags(NO_WINDOW_FLAG);

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

pub fn main() {
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
        {
            use std::fs::create_dir_all;

            create_dir_all(cache_path.as_path())
                .expect("no permission to create cache_path");
        }

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

    if webview::webview_version().is_err() {
        #[cfg(windows)]
        webview::webview2err::show_webview_err_message();

        return;
    }

    let mut port: String = String::new();
    let sid = uuid::Uuid::new_v4().to_string();

    if let Some(child) = run_server(&root_path, &cache_path, &mut port, &sid) {
        let _ = webview::run_script(Arc::new(child), &root_path, &port.as_str(), &sid);
    }
}