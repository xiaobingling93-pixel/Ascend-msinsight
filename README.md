# **MindStudio Insight**

## 最新消息

- [2025.12.30]：MindStudio Insight项目首次上线

## 简介

### 概述

MindStudio Insight是面向昇腾AI开发者的可视化调优工具，支持系统调优、算子调优、服务化调优和内存调优的能力，帮助开发者在训练、推理以及算子开发场景快速完成性能优化。

MindStudio Insight提供了丰富的调优分析手段，能够可视化呈现真实软硬件运行数据，多维度分析性能瓶颈点，支持百卡、千卡及以上规模的可视化集群性能分析，助力开发者在天级时间内完成性能调优。

### 优势

- MindStudio Insight支持在时间线（Timeline）查看集群场景下的Profiling数据，并以单卡为维度进行展示，且可以自动遍历输入路径下的db文件，或者所有的trace_view.json文件（PyTorch场景和MindSpore场景）和msprof*.json文件（TensorFlow场景和离线推理场景），无需手动合并文件，操作简单。

- MindStudio Insight借助于数据库支持超大性能数据处理，可以支持20GB的集群性能数据分析，并且能够支持大模型场景下的性能调优。

## 目录结构


```
├── build                              # 构建脚本 
├── docs                               # 项目文档介绍 
├── e2e                                # 测试用例
├── modules                            # 模块目录
│   ├── build                          # 构建脚本
│   ├── cluster                        # 概览、通信模块
│   ├── compute                        # 算子调优模块
│   ├── framework                      # 前端主框架模块
│   ├── leaks                          # 内存泄露模块
│   ├── lib                            # 公共目录
│   ├── memory                         # 内存模块
│   ├── operator                       # 算子模块
│   ├── reinforcement-learning         # 强化学习模块
│   ├── statistic                      # 服务化调优模块
│   ├── timeline                       # 时间线模块
├── platform                           # 底座目录 
├── plugins                            # 插件目录
├── scripts                            # 脚本目录 
├── server                             # 后端服务模块
│   ├── build						   # 构建脚本
│   ├── cmake						   # 开源软件构建脚本
│   ├── src								              
│   │   ├── channel					   # 网络通讯
│   │   ├── defs					   # 全局定义
│   │ 	├── entry					   # 编译模块
│   │	  │   ├── server					  
│   │	  │		    ├── bin			   # server模块
│   │	  ├── protocol				   # 消息定义
│   │	  ├── module						 
│   │	  │   ├── base				   # 模块共用基类 
│   │	  │   ├── global			   # 全局消息
│   │	  │   ├── timeline			   # timeline消息处理
│   │	  │   │	  ├── core			   # 核心处理逻辑
│   │	  │   │	  ├── handler		   # 消息处理
│   │	  │   │	  ├──protocol    	   # 消息格式转换
│   │	  │   ├── ...						
│   │	  ├── server				   # server服务
│   │	  ├── utils					   # 工具类
│   ├── third_party					   # 开源软件 
```

## 环境部署

MindStudio Insight工具支持在Windows、Linux，以及macOS系统中安装使用，并且支持通过插件方式安装，具体安装操作可参见[MindStudio Insight安装指南](./docs/zh/user_guide/mindstudio_insight_install_guide.md.md)。

## 工具限制与注意事项

MindStudio Insight工具支持导入并展示多种格式的性能数据文件，并对文件规格给出了指导性建议和限制要求。

| 文件类型 | 指导建议                                            | 规格限制               |
| ----------- | --------------------------------------------- | ---------------------- |
| json文件 | 建议单文件大小不超过5GB，多个文件总大小不超过20GB。          | 单文件大小不超过10GB。 |
| bin文件  | 建议单文件大小不超过5GB。                                    | 单文件大小不超过10GB。 |
| db文件   | 建议单文件大小不超过5GB。<br> 其中npumonitor数据的db单文件大小建议不超过2GB。leaks_dump_{*timestamp*}.db文件大小建议不超过2GB。 | 单文件大小不超过10GB。 |
| csv文件  | csv格式的文件存在于text数据中，建议单文件大小不超过1GB。     | 单文件大小不超过2GB。  |

## 基础操作

介绍MindStudio Insight工具的基础设置，数据导入，快捷键使用等基础操作。具体请参见[MindStudio Insight基础操作](https://www.hiascend.com/document/detail/zh/mindstudio/830/GUI_baseddevelopmenttool/msascendinsightug/Insight_userguide_0029.html)。

## 特性介绍

MindStudio Insight工具支持系统调优、算子调优、服务化调优和内存调优，可视化呈现数据情况，助力开发者快速完成性能调优。

- 系统调优

  MindStudio Insight提供时间线视图、内存、算子耗时、通信瓶颈分析等功能，帮助开发者快速定位模型性能瓶颈，进行深度调优。具体内容请参见[系统调优](https://www.hiascend.com/document/detail/zh/mindstudio/830/GUI_baseddevelopmenttool/msascendinsightug/Insight_userguide_0032.html)。

  | 功能界面              | 介绍                                                         | 场景说明                         |
  | --------------------- | ------------------------------------------------------------ | -------------------------------- |
  | 时间线（Timeline）    | 以时间线视图方式为用户提供全流程在线推理/训练过程中的运行情况，并按照调度流程来呈现整体的运行状况，支持集群时间线（Timeline）展示、系统视图详情查看等功能。 | -                                |
  | 内存（Memory）        | 提供采集过程中内存信息的可视化呈现。通过算子内存折线图直观清晰了解算子内存趋势。 | -                                |
  | 算子（Operator）      | 提供算子耗时统计和分析。                                     | -                                |
  | 概览（Summary）       | 展示计算算子和通信算子的耗时分析，并以柱状图、折线图以及数据窗格等呈现方式显示分析结果。 | 支持PyTorch集群场景。 |
  | 通信（Communication） | 展示集群中全网链路性能以及所有节点的通信性能，通过集群通信与计算重叠时间的分析可以找出集群训练中的慢主机或慢节点。 | 支持PyTorch集群场景。 |
  | 强化学习（RL）        | 基于采集数据进行高层次抽象，可视化展示控制流的时序关系，帮助快速定位耗时任务与空泡，并支持进一步的性能分析。 | -                                |


- 算子调优

  MindStudio Insight提供指令流水视图、算子源码视图以及算子运行负载分析视图，直观地将运行在昇腾AI处理器上的算子的关键性能指标进行可视化呈现，帮助用户快速定位算子的软硬件性能瓶颈，提升算子性能分析的效率。具体内容请参见[算子调优](https://www.hiascend.com/document/detail/zh/mindstudio/830/GUI_baseddevelopmenttool/msascendinsightug/Insight_userguide_0059.html)。

  | 功能界面           | 介绍                                                         | 备注                                     |
    | ------------------ | ------------------------------------------------------------ | ---------------------------------------- |
    | 时间线（Timeline） | 以时间线视图方式为用户提供指令在昇腾处理器上的运行情况，并按照调度流程来呈现整体的运行状况，支持查看指令详情、搜索指令等功能。 | -                                        |
    | 源码（Source）     | 展示算子指令热点图，支持查看算子源码与指令集的映射关系和耗时情况。 | 支持msprof采集的算子Profiling的bin文件。 |
    | 详情（Details）    | 展示算子基础信息、计算负载分析和内存负载分析，并以图形和数据窗格呈现方式展示分析结果。 | 支持msprof采集的算子Profiling的bin文件。 |
    | 缓存（Cache）      | 展示用户程序Kernel函数内的L2 Cache访问情况，以便用户优化Cache命中率。 | 支持msprof采集的算子Profiling的bin文件。 |


- 服务化调优

  MindStudio Insight工具以时间线（Timeline）的呈现方式，将请求端到端的执行情况平铺在时间轴上，直观体现请求在各个关键阶段的耗时情况以及当下请求的状态信息，可帮助用户快速识别服务化性能瓶颈，并根据问题现象，调整调优策略。具体内容请参见[服务化调优](https://www.hiascend.com/document/detail/zh/mindstudio/830/GUI_baseddevelopmenttool/msascendinsightug/Insight_userguide_0112.html)。

  | 功能界面           | 介绍                                                         | 场景说明                                |
  | ------------------ | ------------------------------------------------------------ | --------------------------------------- |
  | 时间线（Timeline） | 以时间线视图方式为用户提供请求端到端的执行情况，直观地查看请求在各个关键阶段的耗时情况以及当下请求的状态信息。 | 支持推理服务化请求trace数据的json文件。 |
  | 折线图（Curve）    | 以折线图和数据详情表的形式展示推理服务化进程中端到端的性能情况。 | 支持profiler.db文件。                   |


- 内存调优

  MindStudio Insight工具以图形化形式呈现device侧内存详细分配情况，并结合Python调用栈及自定义打点标签标记各种内存申请使用详情，进行内存问题的详细定位及调优。具体内容请参见[内存调优](https://www.hiascend.com/document/detail/zh/mindstudio/830/GUI_baseddevelopmenttool/msascendinsightug/Insight_userguide_0120.html)。

  | 功能界面          | 介绍                                                         | 场景说明                                      |
  | ----------------- | ------------------------------------------------------------ | --------------------------------------------- |
  | 内存详情（Leaks） | 通过调用栈图、折线块图和内存拆解图，将内存情况直观地呈现出来，便于开发者分析定位内存问题，有效缩短定位时间。 | 支持msLeaks工具采集到的db格式的内存结果文件。 |


## FAQ

FAQ汇总了在使用MindStudio Insight工具过程中可能遇到的典型问题及其解决方案，具体内容请参见[FAQ](./docs/zh/user_guide/FAQ.md)

## 安全声明

介绍了MindStudio Insight工具安全相关信息和通信矩阵信息等，具体内容请参见[MindStudio Insight安全声明](./docs/zh/user_guide/security_statement.md)。


## 免责声明

### 致MindStudio Insight使用者

1. MindStudio Insight工具提供的所有内容仅供您用于非商业目的。

2. 对于MindStudio Insight测试用例以及示例文件中所涉及的数据，平台仅用于功能测试，华为不提供任何数据。

3. 如您在使用MindStudio Insight工具过程中，发现任何问题（包括但不限于功能问题、合规问题），请在Gitcode提交issues，我们将及时审视并解决。

4. MindStudio Insight工具依赖的第三方开源软件，均由第三方社区提供和维护，因第三方开源软件导致的问题需依赖相关社区的贡献和反馈进行修复。您应理解，MindStudio Insight仓库不保证对第三方开源软件本身的问题进行修复，也不保证会测试、纠正所有第三方开源软件的漏洞和错误。

### 致数据所有者

如果您不希望您的数据集在MindStudio Insight中被提及，或希望更新MindStudio Insight中有关的描述，请在Gitcode提交issues，我们将根据您的issues要求删除或更新您相关描述。衷心感谢您对MindStudio Insight的理解和贡献。

## 贡献声明

1. **提交错误报告**：如果您在MindStudio Insight中发现了一个不存在安全问题的漏洞，请在MindStudio Insight仓库中的issues中搜索，以防该漏洞已被提交，如果找不到漏洞可以创建一个新的issues。如果发现了一个安全问题请不要将其公开，请参阅安全问题处理方式。提交错误报告时应该包含完整信息。
2. **安全问题处理**：本项目中对安全问题处理的形式，请通过邮箱通知项目核心人员确认编辑。 
3. **解决现有问题**：通过查看仓库的issues列表可以发现需要处理的问题信息, 可以尝试解决其中的某个问题。
4. **如何提出新功能**：请使用issues的Feature标签进行标记，我们会定期处理和确认开发。
5. **开始贡献：**
   1. Fork本项目的仓库。
   2. Clone到本地。
   3. 创建开发分支。
   4. 本地自测，提交前请通过所有的已经单元测试，以及为您要解决的问题新增单元测试。
   5. 提交代码。
   6. 新建Pull Request。
   7. 代码检视，您需要根据评审意见修改代码，并再次推送更新。此过程可能会有多轮。
   8. 当您的PR获得足够数量的检视者批准后，Committer会进行最终审核。
   9. 审核和测试通过后，CI会将您的PR合并入项目的主干分支。

## License

MindStudio Insight工具的使用许可证，具体情参见[License](./License)。

MindStudio Insight工具docs目录下的文档适用CC-BY 4.0许可证，具体请参见[License](./docs/LICENSE)。

## 建议与交流

欢迎大家为社区做贡献。如果有任何疑问或建议，请提交issues，我们会尽快回复。感谢您的支持。

## 致谢

MindStudio Insight由华为公司的下列部门联合贡献：

- 计算产品线

感谢来自社区的每一个PR，欢迎贡献MindStudio Insight！

