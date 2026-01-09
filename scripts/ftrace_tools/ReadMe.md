# 使用MindStudio Insight加载Linux Kernel数据以联合分析Host Bound问题
## 问题背景
在大模型中，CPU主要负责任务的下发，NPU负责计算任务的执行。无论训练还是推理领域，Host Bound都是现网高发问题。分析Host Bound问题通常需要采集Linux Kernel ftrace数据，分析CPU上的进程调度情况。

目前缺乏一种工具，能够统合profiling数据和Linux Kernel ftrace数据，做到联合分析。MindStudio Insight在本仓库中，提出一些工具脚本，帮助开发者实现两种数据的联合分析，提高Host Bound问题定位的效率。

## 特性

+ 支持**命令行**和**API接口**两种方式的ftrace数据采集
+ 支持将ftrace格式数据转换为Chrome Trace Json格式，联合Profiling数据导入MindStudio Insight，进行可视化展示
+ 支持**容器内**运行模型，宿主机采集Linux Kernel ftrace数据，并进行无缝PID映射

## Host Bound问题定位思路

1. 尝试通用的调度优化手段，包括绑核、流水优化、内存分配库替换三板斧。PyTorch框架调度优化可参考：[调度优化-Ascend Extension for PyTorch-昇腾社区](https://www.hiascend.com/document/detail/zh/Pytorch/720/ptmoddevg/trainingmigrguide/performance_tuning_0059.html)
2. 若通用优化手段效果不及预期，可采集数据，进一步深入分析。建议同时采集ftrace和profiling数据。
3. 将ftrace数据转换为MindStudio Insight可识别的数据格式。
4. 同时导入ftrace数据与profiling数据，分析进程调度情况。

## 模型Profiling采集

参考昇腾社区有关profiling采集相关的文档：[简介-CANN商用版8.3.RC1-昇腾社区](https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/devaids/Profiling/atlasprofiling_16_0001.html)

## Linux Kernel ftrace数据采集

### 1. Linux Kernel ftrace数据简介

Linux内核内置了多种跟踪（trace）工具，其中ftrace作为从2.6.27版本开始引入主流内核的跟踪框架，可用于监控和调试内核中发生的各类事件，帮助开发人员深入分析系统运行时的内部行为。ftrace支持多种跟踪器，例如函数调用跟踪、上下文切换跟踪、中断延迟分析等，能够有效辅助定位内核态性能问题与调度异常。在本仓库中，仅开启了与CPU进程调度相关的事件（sched）。

各种事件代表了不同的含义，这里我们主要关注以下几类事件：

+ sched_switch: 记录了CPU上的进程切换
+ sched_wakeup: 进程被唤醒
+ sched_wakeup_new: 新创建的进程首次被唤醒
+ shced_process_fork/shced_process_exec/sched_process_exit: 进程创建销毁

trace-cmd是一个命令行工具，它封装了trace采集的过程，提供了更加简易的命令接口。

### 2. 数据采集前置准备

+ 安装trace-cmd命令

  Ubuntu安装命令：`sudo apt-get install trace-cmd`

  CentOs安装命令：`sudo yum install trace-cmd`

+ 获取仓库中提供的采集、转换脚本`trace_record.py`，`trace_convert.py`，推荐profiling和ftrace数据同步采集。

### 3. ftrace数据采集

支持**命令行**和**API接口**两种方式的ftrace数据采集。

#### 方法一：命令行采集

这种方式不需要修改现有代码，将`trace_record.py`脚本作为整体使用，用户无需修改现有代码即可快速启动采集任务。该方法适合快速部署和一次性数据收集场景，但自定义程度相对有限。

**参数说明**

| 参数 | 说明 | 示例 | 默认值 |
|-----|-----|-----|-----|
| `--cpu` | cpu_mask列表，多个核心使用逗号分隔<br>缺省时默认采集所有CPU核 | `--cpu=0,1,4` 采集CPU 0、1、4的数据 | None，采集**全部**CPU核心 |
| `--output` | 输出文件路径与文件名 | `--output=my_trace_data.txt` | `ftrace.txt` |
| `--record_time` | 采集持续时间（单位：秒）<br>• 正值：采集指定秒数后自动停止<br>• ≤0：持续采集，需`Ctrl+C`手动终止，长时间采集，请注意磁盘空间占用情况。 | `--record_time=30` | 60 |
|`--NSpid`| 容器场景下，可开启该开关，获取容器与宿主机PID映射关系，详见[采集容器中的Linux Kernel ftrace数据](#采集容器中的linux-kernel-ftrace数据) | `--NSpid` | 关闭 |


**使用示例:**

场景：采集CPU核心`0-4`的30秒训练数据
1. 启动数据采集（需root权限）：
```bash
sudo python trace_record.py --record_time=30 --cpu=0,1,2,3,4
```
2. 执行训练任务（在新终端中）：
```bash
python train.py
```
3. 采集结果：

脚本运行30秒后自动停止，默认生成 ftrace.txt 文件（或通过 --output 参数指定文件名）。

**注意：**
> 采集数据量过大时，`trace-cmd show`步骤耗时会较长，建议使用`--cpu`参数，推荐CPU采集核心数不超过16。

#### 方法二：API接口采集模式

`trace_record.py`中提供两个接口，分别控制ftrace采集的启停，允许开发者将数据采集逻辑精细地嵌入到应用程序中。这种方式适合需要动态控制采集时机、条件触发采集或与业务逻辑深度集成的场景。

**1. 开始采集接口**

```python
def ftrace_record_start(cpu_list: List[int] = None)
```
**参数**：
+ `cpu_list`（可选）：指定要采集的CPU核心编号列表。默认为 `None`，表示采集所有可用CPU核心。

**注意：**
> 采集数据量过大时，`trace-cmd show`步骤耗时会较长，建议使用`cpu_list`参数，推荐CPU采集核心数不超过16。

**2. 停止采集并保存数据接口**

```python
def ftrace_record_stop(output: str)
```

**参数**：

+ `output`（可选）： 输出文件的路径和名称。

**使用示例**:

```python
import ftrace_record
def train()
    ftrace_record_start(cpu=[0,1,4]) //开始采集ftrace
    profiling_start() //开始采集profiling

    // 模型运行...

    profiling_stop() // 停止采集profiling
    ftrace_record_stop(output="ftrace.txt") // 停止采集ftrace
```

### 4. 数据采集后处理

`trace_convert.py`脚本用于将原始 ftrace 数据转换为 Chrome Trace JSON 格式，并与所采集的profiling数据时间轴对齐，以便导入 MindStudio Insight 可视化工具联合展示与分析。

**使用方法**
```bash
python trace_convert.py [-h] [--input INPUT] [--output OUTPUT] [--cpu_list CPU_LIST] [--profiling_data PROFILING_DATA]
```
**参数详解**

| 参数 | 说明 | 示例值 | 默认值 |
|------|------|--------|--------|
| `--input` | 输入的原始 trace 文件路径，通过 `trace_record.py` 生成 | `/path/to/trace_data.txt` | `ftrace.txt` |
| `--output` | 输出的 JSON 文件路径 | `trace.json` | `output.json` |
| `--profiling_data` | 同步采集的Profiling数据文件路径，用于时间轴对齐，以便导入MindStudio Insight联合分析 | `/profiling/xxxx_ascend_pt` | - |
| `--pid_mapping` | 容器场景下，可传入pid映射文件路径，进行容器与宿主机PID转换，详见[采集容器中的Linux Kernel ftrace数据](#采集容器中的linux-kernel-ftrace数据) | `pid_mapping.json`| - |

**使用示例**

假设第一步采集的profiling数据在目录`result_dir/xxxx_ascend_pt`下，对应的ftrace文件保存在`result_dir/frace.txt`中。

执行命令：
```bash
python trace_convert.py --input=result_dir/frace.txt --profiling_data=result_dir/xxxx_ascend_pt
```
多卡场景下，`--profiling_data`可指定为包含多卡的上级目录，或任意单卡数据目录。

## 采集容器中的Linux Kernel ftrace数据

### 问题背景

默认情况下（即容器启动时，未设置`--pid=host`参数），Docker容器有自己独立的PID命名空间。在容器中运行模型，profiling采集的进程号，为容器内部进程号。而ftrace采集的时Linux Kernel内核数据，**与容器内进程号存在命名空间隔离，无法直接对齐**。

针对该问题，`trace_record.py`脚本中提供了接口，用于获取容器内PID与宿主机PID的映射关系（基于遍历`/proc/$PID/status`文件的方式）。

### 方式一：命令行模式
1. 使用`trace_record.py`脚本采集ftrace数据时，打开`--NSpid`开关，得到`pid_mapping.json`交付件，记录了PID映射关系。
2. 使用`trace_convert.py`脚本进行数据后处理时，通过`--pid_mapping`参数，传入`pid_mapping.json`路径。脚本执行时，会自动将采集到的进程号转换为容器内进程号，以便于和profiling所采集进程号对应。

### 方式二：API接口模式
可单独使用`trace_convert.py`脚本中的`ContainerPidMapper`类，提供以下对外接口：

**类构造函数**

  ```python
  def __init__(self, output_file: str = "pid_mapping.json")
  ```

参数说明：`output_file`：输出文件路径

**PID映射关系dump功能**
```python
// 开启接口
def start(self, duration=None)
// 停止接口
def stop(self)
```
**说明**：参数`duration`代表映射关系dump采集的开启周期，当传入`None`时，仅dump一次，且无需调用stop接口；传入其它有效值时，每`duration`秒dump一次，并通过stop接口停止。



## 基于vllm-ascend场景的ftrace数据与profiling数据联合分析样例

待补充