use std::{env, io};

fn main() -> io::Result<()> {
    if env::var("CARGO_CFG_TARGET_ARCH").is_ok() {
        #[cfg(windows)]
        {
            use winres::WindowsResource;

            WindowsResource::new()
                .set_icon_with_id("./resources/images/icons/main.ico", "main_icon")
                .set("ProductVersion", env!("CARGO_PKG_VERSION"))
                .set(
                    "LegalCopyright",
                    "Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.",
                )
                .compile()?;
        }
    }
    Ok(())
}
