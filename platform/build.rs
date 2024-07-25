use std::{env, io, path::PathBuf, process};

fn main() -> io::Result<()> {
    if env::var("CARGO_CFG_TARGET_ARCH").is_ok() {
        #[cfg(windows)]
        {
            let output = PathBuf::from("./bundle/main.lib");
            let input = PathBuf::from("./bundle/main.rc");
            let mut command = process::Command::new("windres");

            let status = command
                .arg("-i")
                .arg(format!("{}", input.display()))
                .arg("-o")
                .arg(format!("{}", output.display()))
                .output()?;

            if !status.status.success() {
                return Err(io::Error::new(
                    io::ErrorKind::Other,
                    "Could not compile resource file",
                ));
            }

            println!("cargo:rustc-link-search=native={}", "./bundle/");
            println!("cargo:rustc-link-lib=dylib=main");
        }
    }
    Ok(())
}
