/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

use std::env;
use std::env::current_exe;
use std::process::Command;

pub fn main() {
    use std::process::exit;

    let args: Vec<String> = env::args().collect();
    let profiler_path = current_exe()
        .expect("Failed to get current exe path")
        .parent()
        .expect("Failed to get parent path of  current exe")
        .join("resources")
        .join("profiler")
        .join("start_script.py")
        .to_path_buf();
    let Some(path) = profiler_path.to_str() else {
        return;
    };
    let mut server_command = Command::new("python3");
    server_command.arg(path);
    for arg in args {
        server_command.arg(arg);
    }
    match server_command.output() {
        Ok(output) => {
            let formatted = String::from_utf8(output.stdout).unwrap();
            if !formatted.is_empty() {
                eprintln!("{}", formatted);
            }
            exit(output.status.code().unwrap_or(0))
        }
        Err(e) => {
            eprintln!("Failed to start server.Error:{e:?}")
        }
    };
}