import time
import torch
import torch_npu
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader


class TinyTransformer(nn.Module):
    """ Tiny Transformer for light profiling"""

    def __init__(self,
                 d_model=32,
                 nhead=2,
                 num_encoder_layers=1,
                 num_decoder_layers=1,
                 dim_feedforward=64
                 ):
        super(TinyTransformer, self).__init__()
        self.model = nn.Transformer(
            d_model=d_model,
            nhead=nhead,
            num_encoder_layers=num_encoder_layers,
            num_decoder_layers=num_decoder_layers,
            dim_feedforward=dim_feedforward
        )

    def forward(self, src, tgt):
        return self.model(src, tgt)


def create_fake_nlp_dataset(
        seq_len: int = 10,
        batch_size: int = 32,
        d_model: int = 32,
        tgt_len: int = 20,
        num_samples: int = 100,
        num_parallel_workers=1
):
    """
    Create a fake dataset for NLP models.

    Args:
        seq_len(int): Sequence length.
        batch_size(int): Size of each batch.
        d_model(int): Dimensionality of the model.
        tgt_len(int): Length of the target sequence.
        num_samples(int): Number of samples in the dataset.
        num_parallel_workers(int): Number of workers to read the data in parallel.

    Returns:
        torch.utils.data.Dataset: A randomly generated dataset according to the specified parameters.
    """
    src_data = torch.randn(num_samples, seq_len, batch_size, d_model)
    tgt_data = torch.randn(num_samples, tgt_len, batch_size, d_model)
    return [(src, tgt) for src, tgt in zip(src_data, tgt_data)]


def run_pt_model(config, open_profiler = True):
    # device = 'cuda' if torch.cuda.is_available() else 'cpu'
    # device = torch_npu.device("npu:7" if torch_npu.is_available() else "cpu")
    # net = TinyTransformer().to(device)
    # criterion = nn.CrossEntropyLoss()

    device = torch.device("npu:7")
    torch.npu.set_device(device)

    # 初始化模型、损失函数和优化器
    model = TinyTransformer(d_model=256, nhead=8, num_encoder_layers=20, num_decoder_layers=10, dim_feedforward=512).to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.SGD(model.parameters(), lr=1, momentum=0.9)
    model.train()
    # 生成数据集
    num_samples = config.get("skip_first", 0) + (config.get("wait", 0)+config.get("warmup", 0)+config.get("active", 1))*config.get("repeat", 1)
    dataset = create_fake_nlp_dataset(seq_len=10, batch_size=10, d_model=256, tgt_len=20, num_samples=num_samples)

    def train_one_step(src, tgt):
        src, tgt = src.to(device), tgt.to(device)
        # 前向传播
        output = model(src, tgt)
        loss = criterion(output, tgt)
        # 反向传播和优化
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

    if config.get("with_cpu", True):
        activities = [torch_npu.profiler.ProfilerActivity.NPU, torch_npu.profiler.ProfilerActivity.CPU]
    else:
        activities = [torch_npu.profiler.ProfilerActivity.NPU]

    level_str = config.get("level")
    profiler_level = torch_npu.profiler.ProfilerLevel.Level0
    if level_str == "Level0":
        profiler_level = torch_npu.profiler.ProfilerLevel.Level0
    elif level_str == "Level1":
        profiler_level = torch_npu.profiler.ProfilerLevel.Level1
    elif level_str == "Level2":
        profiler_level = torch_npu.profiler.ProfilerLevel.Level2

    aic_metrics_str = config.get("aicore_metrics")
    aicore_metrics = torch_npu.profiler.AiCMetrics.AiCoreNone
    if aic_metrics_str == "ACL_AICORE_NONE":
        aicore_metrics = torch_npu.profiler.AiCMetrics.AiCoreNone
    elif aic_metrics_str == "ACL_AICORE_PIPE_UTILIZATION":
        aicore_metrics = torch_npu.profiler.AiCMetrics.PipeUtilization
    elif aic_metrics_str == "ACL_AICORE_ARITHMETIC_UTILIZATION":
        aicore_metrics = torch_npu.profiler.AiCMetrics.ArithmeticUtilization
    elif aic_metrics_str == "ACL_AICORE_MEMORY_BANDWIDTH":
        aicore_metrics = torch_npu.profiler.AiCMetrics.Memory
    elif aic_metrics_str == "ACL_AICORE_L0B_AND_WIDTH":
        aicore_metrics = torch_npu.profiler.AiCMetrics.MemoryL0
    elif aic_metrics_str == "ACL_AICORE_MEMORY_UB":
        aicore_metrics = torch_npu.profiler.AiCMetrics.MemoryUB
    elif aic_metrics_str == "ACL_AICORE_RESOURCE_CONFLICT_RATIO":
        aicore_metrics = torch_npu.profiler.AiCMetrics.ResourceConflictRatio
    elif aic_metrics_str == "ACL_AICORE_L2_CACHE":
        aicore_metrics = torch_npu.profiler.AiCMetrics.L2Cache

    experimental_config = torch_npu.profiler._ExperimentalConfig(
        export_type=torch_npu.profiler.ExportType.Text,
        profiler_level=profiler_level,
        msprof_tx=False,
        aic_metrics=aicore_metrics,
        l2_cache=config.get("l2_cache", False),
        op_attr=False,
        data_simplification=False,
        record_op_args=False
    )

    if open_profiler:
        with torch_npu.profiler.profile(
            activities=activities,
            schedule=torch_npu.profiler.schedule(
                wait=int(config.get("wait", 0)),
                warmup=config.get("warmup", 0),
                active=config.get("active", 1),
                repeat=config.get("repeat", 1),
                skip_first=config.get("skip_first", 0)
            ),
            on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(
                dir_name=config.get('result_path'),
                analyse_flag=False
            ),
            record_shapes=False,
            profile_memory=config.get("with_memory", False),
            with_stack=config.get("with_stack", False),
            with_modules=False,
            with_flops=False,
            experimental_config=experimental_config
        ) as profiler:
            step_times = []
            for src, tgt in dataset:
                start_time = time.time()
                train_one_step(src, tgt)
                end_time = time.time()
                step_time = end_time - start_time
                step_times.append(step_time)
                profiler.step()
        return step_times

    step_times = []
    for src, tgt in dataset:
        start_time = time.time()
        train_one_step(src, tgt)
        end_time = time.time()
        step_time = end_time - start_time
        step_times.append(step_time)

    return step_times

def main():
    run_pt_model(None, None)

if __name__ == "__main__":
    main()


    # net.train()
    # optimizer.zero_grad()
    # output = net(src, tgt)
    # loss = criterion(output.view(-1), tgt.view(-1))
    # loss.backward()
    # optimizer.step()