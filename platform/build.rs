/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
fn main() {
    #[cfg(windows)]
    if let Ok(arch) = std::env::var("CARGO_CFG_TARGET_ARCH") {
        gen_rc();
        static_vcruntime::build(&arch);
    }
}

#[cfg(windows)]
fn gen_rc() {
    use std::{path::PathBuf, process::Command};

    let output = PathBuf::from("./bundle/main.lib");
    let input = PathBuf::from("./bundle/main.rc");

    /// TODO: 为什么本地机器上{:?}配合{.\\}可行，构建机上不行
    Command::new("windres")
        .arg("-i")
        .arg(format!("{}", input.display()))
        .arg("-o")
        .arg(format!("{}", output.display()))
        .output()
        .unwrap();

    println!("cargo:rustc-link-search=native={}", "./bundle/");
    println!("cargo:rustc-link-lib=dylib=main");
}

/// By default, Rust requires programs to deploy vcruntime140.dll (or equivalent)
///
/// Statically links the library instead
#[cfg(windows)]
mod static_vcruntime {
    use std::{env, fs::OpenOptions, io::Write, path::Path};

    pub fn build(arch: &str) {
        if env::var("CARGO_CFG_TARGET_ENV").as_deref() != Ok("msvc") {
            return;
        }

        override_msvcrt_lib(arch);
       
        /// 去除这些是为了避免符号冲突
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:libvcruntimed.lib");
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:vcruntime.lib");
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:vcruntimed.lib");
        
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:msvcrt.lib");
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:msvcrtd.lib");
        
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:libucrt.lib");
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:libucrtd.lib");
        
        println!("cargo:rustc-link-arg=/NODEFAULTLIB:libcmtd.lib");
       
        /// Link `libvcruntime` and `univeral-c-runtime`
        /// 
        /// 这些devel-lib会被硬编码到可执行文件
        println!("cargo:rustc-link-arg=/DEFAULTLIB:libvcruntime.lib");
        println!("cargo:rustc-link-arg=/DEFAULTLIB:ucrt.lib");
        println!("cargo:rustc-link-arg=/DEFAULTLIB:libcmt.lib");
    }
    
    fn override_msvcrt_lib(arch: &str) {
        let machine: &[u8] = match arch {
            "x86_d6" => &[0x64, 0x86],
            "x86" => &[0x4C, 0x01],
            _ => return,
        };

        /// 这些bytes主要是file header和section header，是为了linker能将文件识别为有效的`COFF (obj file)`
        let bytes: &[u8] = &[
            1, 0, 94, 3, 96, 98, 60, 0, 0, 0, 1, 0, 0, 0, 0, 0, 132, 1, 46,
            100, 114, 101, 99, 116, 118, 101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 16, 0,
            46, 100, 114, 101, 99, 116, 118, 101, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0,
            4, 0, 0, 0,
        ];
        
        /// `target/$profile/build/MindStudio_Insight-hash`
        let out_dir = env::var("OUT_DIR").unwrap();
        let path = Path::new(&out_dir).join("msvcrt.lib");
        let f = OpenOptions::new().write(true).create_new(true).open(&path);
        if let Ok(mut f) = f {
            f.write_all(machine).unwrap();
            f.write_all(bytes).unwrap();
        }

        println!("cargo:rustc-link-search=native={}", out_dir);
    }
}
