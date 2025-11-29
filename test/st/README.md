# ST数据采集方法
采集ST性能数据使用LLM。考虑到模型获取的方便性，使用MindSpeed-LLM支持的模型。同时考虑到ST使用的数据不需要很大，使用Qwen3-0.6B作为采集ST数据使用的模型。为了采集通信数据，模型使用多卡进行训练。数据集使用alpaca。

ST数据每个季度使用最新的profiling工具进行采集。
# 采集配置
## 2025Q4
CANN 8.3.RC1

Python 3.10

PyTorch 2.7.1

MindSpeed 2.2.0

4卡

```shell
TP=1
 
PP=2
 
MBS=1
 
GBS=4
 
PROFILING_ARGS="
    --profile \
    --profile-export-type text \
    --profile-level level2 \
    --profile-data-simplification \
    --profile-with-cpu \
    --profile-step-start 15 \
    --profile-step-end 16 \
    --profile-save-path ./profile_dir \
    --profile-record-shapes \
    --profile-with-memory \
    --profile-ranks -1 \
"
```

采集数据删除operator_details.csv，因为MindStudio Insight未用到。

为了测试集群相关功能，1卡和3卡仅保留ascend_pytorch_profiler.db analysis.db communication_matrix.json communication.json step_trace_time.csv,其它文件都删除。

裁剪后文件大小共68M左右。