# prof2memscope

# 1. 简介
prof2memscope工具用于将`Ascend Pytorch Profiler`采集的调优数据, 解析转化为memscope(原名msleaks)输出件格式db.
导入insight后可在"内存详情“(英文名"MemScope")进行展示, 可在同一视图中分析PTA内存申请分配生命周期与调用栈关联, 增强内存问题定位与内存调优易用性.

# 2. 使用说明
## 2.1 Ascend Pytorch Profiler采集设置
- profiler采集时需包含参数`torch_npu.profiler.ProfilerActivity.CPU,
        torch_npu.profiler.ProfilerActivity.NPU`
- 如需查看内存块图, profiler采集时需配置参数`profile_memory=True`
- 如需查看内存调用栈图, profiler采集时需配置参数`with_stack=True`

示例如下:
```python
torch_npu.profiler.profile(
    activities=[
        torch_npu.profiler.ProfilerActivity.CPU,
        torch_npu.profiler.ProfilerActivity.NPU
        ],
    profile_memory=True,
    with_stack=True)
```

## 2.2 Profiling数据解析为memscope数据
```shell
# 在memscope_adaptors项目根目录下执行
python3 prof2memscope/dump.py [-h] [-s START] [-d DURATION] [-o OUTPUT_PATH] profiler_path
```

参数说明如下
- profiler_path (必需)
  - 类型: 路径
  - 说明: 指定要解析的 PyTorch Profiler数据目录路径。 
  - 示例: `/path/to/profiler_data/xxx_ascend_pt`
- -s, --start (可选)
  - 类型: int, 必须为正整数。
  - 说明: 指定数据解析和裁剪的开始(unix)时间戳，单位为纳秒 (ns)。如果未提供，将从 Profiler 数据的开始时间裁剪。
  - 缺省值: 最早Profiler数据开始位置
  - 示例: `--start 1752808501531484300` 或 `-s 1752808501531484300`
- -d, --duration (可选)
  - 类型: int, 必须为正整数。
  - 说明: 指定从开始时间起要裁剪的数据持续时间，单位为纳秒 (ns)。如果未提供，数据将从开始时间裁剪到 Profiler 数据的结束位置。 
  - 缺省值: 从开始时间到数据结束
  - 示例: 5s持续时间 `--duration 5000000000` 或 `-d 5000000000`

- -o, --output_path (可选)
  - 类型: 路径 
  - 描述: 指定解析结果的输出文件路径。默认情况下，输出文件将保存在 Profiler 路径下的 dump_data 子目录中，文件名为 leaks_dump_<timestamp>.db。
  - 缺省值：解析指定profiler目录路径下的 dump_data(不存在会自行创建) 子目录中，文件名为 leaks_dump_<timestamp>.db。 
  - 示例: --output_path /path/to/output/dump_data.db
# 3. 使用约束
- 解析数据源来自`[profiler_data_dir]/FRAMEWORK`目录下`torch.xx`二进制, 解析前请检查确保数据存在。
- 解析完成的数据仅具备查看Python调用栈、内存块图/折线图、内存块/事件表，**内存拆解功能无法使用**。
- 解析环境依赖`torch_npu`，请确保解析与采集环境一致或解析环境`torch_npu`版本高于采集环境。
- 解析时需要在指定`output_path`(或缺省时的`[profiler_data_dir]/dump_data`)下创建db文件，请确保使用该脚本时的用户具备对应目录的写权限。