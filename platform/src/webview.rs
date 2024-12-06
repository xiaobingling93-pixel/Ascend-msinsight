/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

mod cleanup;
#[cfg(windows)]
pub mod webview2err;

use std::fs::read;
use std::path::PathBuf;
use std::process::Child;
use std::sync::{Arc, Mutex};
#[cfg(target_os = "macos")]
use wry::application::menu::{MenuBar, MenuItem};
use wry::{
    application::{
        event::{Event, WindowEvent},
        event_loop::{ControlFlow, EventLoop, EventLoopProxy},
        window::{Window, WindowBuilder},
    },
    http::{header::CONTENT_TYPE, Response},
    webview::{FileDropEvent, WebView, WebViewBuilder},
};
pub use wry::webview::webview_version;
const MIMETYPE_HTML: &str = "text/html";

fn create_webview(
    window: Window,
    resource_path: Arc<PathBuf>,
    port: &str,
    sid: &String,
    proxy: Arc<EventLoopProxy<PathBuf>>,
) -> wry::Result<WebView> {
    WebViewBuilder::new(window)?
        .with_custom_protocol("wry".into(), move |request| {
            let path = request.uri().path();
            let content = match read(resource_path.join(&path[1..]).as_path()) {
                Ok(a) => a.into(),
                Err(e) => return Err(wry::Error::Io(e)),
            };

            let mimetype = extract_mimetype(path);

            Response::builder()
                .header(CONTENT_TYPE, mimetype)
                .body(content)
                .map_err(Into::into)
        })
        .with_url(format!("wry://localhost/resources/profiler/frontend/index.html?port={}&sid={}", port, sid).as_str())?
        .with_file_drop_handler(move |_, ev| {
            match ev {
                FileDropEvent::Dropped(paths) => {
                    if let Err(e) = proxy.send_event(paths[0].to_owned()) {
                        eprintln!("app closed unexpectedly: {:#?}", e);
                    }
                }
                _ => {}
            }

            true
        })
        .build()
}

fn handle_user_event(webview: &WebView, path: PathBuf) {
    if let Err(e) =
        webview.evaluate_script(&format!("window.handleDrop({:#?})", path))
    {
        eprintln!("drop-file ipc failed: {:#?}", e);
    }
}

fn run_event_loop(
    event_loop: EventLoop<PathBuf>,
    server_process: Arc<Mutex<Child>>,
    webview: WebView,
) {
    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Wait;
        match event {
            Event::WindowEvent {
                event: WindowEvent::CloseRequested, ..
            } => {
                cleanup::handle_close_requested(server_process.clone());
                *control_flow = ControlFlow::Exit;
            }
            Event::UserEvent(path) => handle_user_event(&webview, path),
            _ => (),
        }
    });
}

#[cfg(windows)]
fn set_windows_icon(window: &Window, root_path: &PathBuf) {
    use wry::application::{platform::windows::IconExtWindows, window::Icon};

    window.set_window_icon(
        Icon::from_path(
            root_path
                .to_path_buf()
                .join("resources/images/icons/mindstudio.ico"),
            None,
        )
            .ok(),
    );
}

#[cfg(target_os = "macos")]
fn macos_menu() -> MenuBar {
    let mut menu = MenuBar::new();

    let mut window_menu = MenuBar::new();
    window_menu.add_native_item(MenuItem::Minimize);
    window_menu.add_native_item(MenuItem::Hide);
    window_menu.add_native_item(MenuItem::HideOthers);
    window_menu.add_native_item(MenuItem::Separator);
    window_menu.add_native_item(MenuItem::Services);
    window_menu.add_native_item(MenuItem::Separator);
    window_menu.add_native_item(MenuItem::CloseWindow);
    window_menu.add_native_item(MenuItem::Quit);

    menu.add_submenu("Window", true, window_menu);

    let mut edit_menu = MenuBar::new();
    edit_menu.add_native_item(MenuItem::Cut);
    edit_menu.add_native_item(MenuItem::Copy);
    edit_menu.add_native_item(MenuItem::Paste);
    edit_menu.add_native_item(MenuItem::SelectAll);

    menu.add_submenu("Edit", true, edit_menu);

    menu
}

// run script
pub fn run_script(
    server_process: Arc<Mutex<Child>>,
    root_path: &PathBuf,
    port: &str,
    sid: &String,
) -> wry::Result<()> {
    let event_loop = EventLoop::with_user_event();

    let proxy = Arc::new(event_loop.create_proxy());

    let window_builder: WindowBuilder = WindowBuilder::new()
        .with_title("MindStudio Insight")
        .with_maximized(true);

    #[cfg(target_os = "macos")]
    {
        let menu = macos_menu();
        window_builder = window_builder.with_menu(menu);
    }

    let window = window_builder
        .build(&event_loop)
        .expect("Error occurred when create App window");

    let resource_path = Arc::new(root_path.to_path_buf());

    #[cfg(windows)]
    set_windows_icon(&window, root_path);

    let webview = create_webview(window, resource_path, port, sid, proxy)?;

    Ok(run_event_loop(event_loop, server_process, webview))
}

fn extract_mimetype(path: &str) -> &str {
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

    mimetype
}