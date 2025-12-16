样例参考
功能描述
本样例为一个Graph的构造和运行样例，该示例中的Graph主要包括：初始化图，checkpoint图和卷积图，最终使该Graph在昇腾AI处理器上运行起来。

该示例主要包括两部分：

第一部分：利用TensorDesc构建初始化子图，checkpoint和卷积图，关键函数分别为GenInitGraph，GenCheckpointGraph， GenConvGraph；

第二部分：创建Session类对象，添加图，运行图，关键函数为session类的AddGraph和RunGraph。

目录结构
├── src                   //源码文件
│   ├──main.cpp
│   ├──graph_utils.cpp
│   ├──graph_utils.h
├── Makefile              //编译脚本 
├── data         
│   ├──data_generate.py  //数据生成脚本，用于生成.bin格式的数据               
├── scripts
│   ├──host_version.conf
│   ├──testcase_800.sh
环境要求
操作系统及架构：Euleros x86系统、Euleros aarch64系统
编译器：g++
芯片：Ascend910系列、Ascend310P系列
python及依赖的库：python3.7.5
已完成昇腾AI软件栈在运行环境上的部署
样例使用
生成权重数据。

在data目录执行数据生成脚本。

python3.7.5 data_generate.py

在data目录下生成.bin格式的数据，后续构建Graph时会从文件中读取权重数据。

程序编译。

执行编译脚本。

a. 修改Makefile文件的安装包路径。

b. 分别执行make clean和make graph_run进行编译。

编译结束后，在out目录下生成可执行文件graph_run。

程序运行。

在运行环境配置环境变量。

若运行环境上安装的“Ascend-cann-toolkit”包，环境变量设置如下：

. ${HOME}/Ascend/ascend-toolkit/set_env.sh
若运行环境上安装的“Ascend-cann-nnrt”包，环境变量设置如下：

. ${HOME}/Ascend/nnrt/set_env.sh
若运行环境上安装的“Ascend-cann-nnae”包，环境变量设置如下：

. ${HOME}/Ascend/nnae/set_env.sh
“$HOME/Ascend”请替换相关软件包的实际安装路径。

在运行环境执行可执行文件。

cd out

./graph_run

检查执行结果。

执行成功提示（部分log）：

Session run Init graph success.
Session run checkpoint graph success.
Session run convolution graph success.
在out目录下生成图中算子的kernel_meta文件夹。

如果用户先保存变量的值，可利用checkpoint子图把数据保存到指定路径。

每张图的计算结果保存在RunGraph出参Tensor里，供用户使用。
