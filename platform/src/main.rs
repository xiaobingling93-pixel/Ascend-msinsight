/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#![windows_subsystem = "windows"]
#![allow(non_snake_case)]

#[cfg(feature = "default")]
mod webview;
#[cfg(feature = "default")]
mod default;

#[cfg(not(feature = "default"))]
mod http;

fn main() {
    #[cfg(feature = "default")]
    default::main();
    #[cfg(not(feature = "default"))]
    http::main();
}
