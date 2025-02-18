/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#![windows_subsystem = "windows"]
#![allow(non_snake_case)]

#[cfg(all(not(debug_assertions), feature = "default"))]
mod default;
#[cfg(all(not(debug_assertions), feature = "default"))]
mod webview;

#[cfg(all(debug_assertions, feature = "default"))]
mod dev;

#[cfg(not(feature = "default"))]
mod http;

fn main() {
    #[cfg(feature = "default")]
    {
        #[cfg(debug_assertions)]
        dev::main();
        #[cfg(not(debug_assertions))]
        default::main();
    }

    #[cfg(not(feature = "default"))]
    http::main();
}
