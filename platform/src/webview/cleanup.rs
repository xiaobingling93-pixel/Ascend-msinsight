/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#[cfg(any(target_os = "macos", target_os = "linux"))]
use std::collections::{HashMap, VecDeque};
#[cfg(windows)]
use std::os::windows::process::CommandExt;
#[cfg(any(target_os = "macos", target_os = "linux"))]
use std::process::ChildStdout;
use std::{
    io::{BufRead, BufReader, Result},
    process::{Child, Command, Stdio},
    sync::{Arc, Mutex},
};

#[cfg(windows)]
fn query_child_pids(parent_pid: u32) -> Result<Vec<String>> {
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
fn process_child_stdout_line(
    line: String,
    pid_map: &mut HashMap<String, Vec<String>>,
) {
    let parts: Vec<&str> = line.split_whitespace().collect();
    if let [key, value] = parts.as_slice() {
        pid_map
            .entry(key.to_string())
            .and_modify(|v| v.push(value.to_string()))
            .or_insert(vec![value.to_string()]);
    }
}

#[cfg(any(target_os = "macos", target_os = "linux"))]
fn process_child_stdout(
    stdout: ChildStdout,
    pid_map: &mut HashMap<String, Vec<String>>,
) {
    let reader = BufReader::new(stdout);
    for line in reader.lines().skip(1) {
        if let Ok(line) = line {
            process_child_stdout_line(line, pid_map)
        }
    }
}

#[cfg(any(target_os = "macos", target_os = "linux"))]
fn query_child_pids(parent_pid: u32) -> Result<Vec<String>> {
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
                process_child_stdout(stdout, &mut pid_map)
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

fn kill_process_tree(parent_pid: String) -> Result<()> {
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

fn kill_child_pids(child_pids: Vec<String>) {
    for child_pid in child_pids {
        if let Err(e) = kill_process_tree(child_pid) {
            eprintln!("Err when kill process: {e}");
        }
    }
}

pub(crate) fn handle_close_requested(server_process: Arc<Mutex<Child>>) {
    let mut server_process_guard =
        server_process.lock().expect("Failed to lock server-process mutex");
    let pid = server_process_guard.id();

    match query_child_pids(pid) {
        Ok(child_pids) => kill_child_pids(child_pids),
        Err(e) => eprintln!("Err when query child pids {e}"),
    }

    if let Err(e) = server_process_guard.kill() {
        eprintln!("server process could not be killed: {e}");
    }
}
