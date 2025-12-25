# 昇腾IDE

## 1. rust的环境

### rust安装

##### 1.1.1 官网安装(外部网络可访问)

采用rustup安装rust，先进入[rustup官网](https://www.rust-lang.org/tools/install)，根据电脑环境下载32位或者64位安装包，根据提示安装。

在安装过程的某个步骤，你会收到一个信息说明为什么需要安装 Visual Studio 2013 或更新版本的 C++ build tools。获取这些 build tools 最方便的方法是安装 [Build Tools for Visual Studio 2019](https://visualstudio.microsoft.com/visual-cpp-build-tools/)。

在rust开发环境中，所有工具都会安装到 ~/.cargo/bin 目录下，包括rustc，cargo和rustup，因此需要将此目录包含在path环境变量中。

安装完成之后，检查是否正确安装，打开shell并运行以下命令，成功获取最新稳定版的版本号说明安装成功。

```shell
rustc --version

cargo -V
```

### 1.2 配置

##### 1.2.1 华为镜像配置

因为网络问题，下载包可能很慢或者失败，所以可以配置华为rust镜像。
进入目录C:\Users\username\.cargo ,新建/修改config文件，增加以下代码，保存修改。

```shell
[source.crates-io]
replace-with = 'mirror'
[source.mirror]
registry = "https://mirrors.tools.huawei.com/rust/crates.io-index/"
```

##### 1.2.2 ssh-agent失败

针对ssh-agent失败的问题，同样在config文件增加下面配置，具体原因有兴趣可以看[issue](https://github.com/rust-lang/cargo/issues/2078)

```
[net]
git-fetch-with-cli = true
```

### 1.3 cargo-make的安装

cargo-make的安装，运行下面的命令语句，也会安装到~/.cargo/bin 目录下。

```shell
# Ubuntu Linux可能安装以下依赖包
sudo apt-get install pkg-config libssl-dev

cargo install --force cargo-make
```

## 2. 昇腾前后端的替换

如果涉及到手动替换前后端，则前后端替换的文件的位置：

```textile
前端：reourece/profiler/

后端：resources/profiler/server
```

### 3. 编译构建

#### 3.1 版本号的修改

    1.  安装包的版本号

        因为是通过widnwos的打包程序nsi来打包的，所以如果需要更改，可以更改其中的字段第`6`行`Outfile`的字段。

    2. ascend_ insight的版本号

    修改根目录下的`Cargo.toml` 文件中的version字段

#### 3.2 编译构建

    如果要使用流水线的构建，可以直接使用 ** cargo make bundle**    


