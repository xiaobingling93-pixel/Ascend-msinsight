#![windows_subsystem = "windows"]

use std::{env, fs::read, process::{Command, Child}, sync::Mutex};
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
            Response::builder()
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
                let _ = server_process.lock().unwrap().kill();
                ControlFlow::Exit
            },
            
            _ => (),
        }
    });

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

fn main() {
    let cache_path = home_dir().unwrap().join(".ascend_insight"); //cache folder generated for each user.
    std::fs::create_dir_all(cache_path.as_path()).expect("no permission to create cache_path");
    #[cfg(windows)] {
        let mut webview_path = cache_path.clone();
        if is_admin() {
            webview_path.push("admin");
        }
        env::set_var("WEBVIEW2_USER_DATA_FOLDER", webview_path.as_path());
    }
    let root_path =  std::env::current_exe().unwrap().parent().unwrap().to_path_buf();

    if wry::webview::webview_version().is_err() {
        return;
    }
    let mut port : String = String::new();
    // start the server
    if let Some(child) = run_server(&root_path, &cache_path, &mut port) {
        if run_script(child, &root_path, &port.as_str()).is_ok() {
        }
    }
}
