import time
import mindspore as ms
from mindspore import nn
from mindspore import dataset as ds
from mindspore.common import dtype as mstype
from mindspore import Profiler
from mindspore import context
from mindspore.profiler import ProfilerLevel, AicoreMetrics, ProfilerActivity, schedule


class TinyTransformer(nn.Cell):
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

    def construct(self, data):
        return self.model(data[0], data[1])


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
        mindspore.dataset.RandomDataset: A randomly generated dataset according to the specified parameters.
    """
    schema = ds.Schema()
    schema.add_column('src', de_type=mstype.float32, shape=[seq_len, batch_size, d_model])
    schema.add_column('tgt', de_type=mstype.float32, shape=[tgt_len, batch_size, d_model])

    # apply dataset operations
    ds.config.set_num_parallel_workers(num_parallel_workers)
    nlp_dataset = ds.RandomDataset(
        schema=schema,
        total_rows=num_samples,
        num_parallel_workers=num_parallel_workers
    )

    return nlp_dataset


def run_ms_model(config, open_profiler = True):
    # run mindspore profiler with tiny transformer
    ms.set_seed(42)
    ms.set_device(device_target="Ascend", device_id=7)
    if config.get("context_mode") == "pynative":
        ms.set_context(mode=context.PYNATIVE_MODE)
    elif config.get("context_mode") == "kbk":
        ms.set_context(mode=context.GRAPH_MODE)
        ms.set_context(jit_config={"jit_level": "O0"})
    elif config.get("context_mode") == "o2":
        ms.set_context(mode=context.GRAPH_MODE)
        ms.set_context(jit_config={"jit_level": "O2"})

    if config.get("with_cpu", True):
        activities = [ProfilerActivity.NPU, ProfilerActivity.CPU]
    else:
        activities = [ProfilerActivity.NPU]

    net = TinyTransformer(d_model=256, nhead=8, num_encoder_layers=20, num_decoder_layers=10, dim_feedforward=512)
    num_samples = config.get("skip_first", 0) + (config.get("wait", 0)+config.get("warmup", 0)+config.get("active", 1))*config.get("repeat", 1)
    data = create_fake_nlp_dataset(seq_len=10, batch_size=10, d_model=256, tgt_len=20, num_samples=num_samples)
    optimizer = nn.Momentum(net.trainable_params(), 1, 0.9)
    loss = nn.MSELoss()
    train_net = nn.WithLossCell(net, loss)
    train_net = nn.TrainOneStepCell(train_net, optimizer)

    if open_profiler:
        with Profiler(
            profiler_level=ProfilerLevel[config.get("level", "Level0")],
            output_path=config.get('result_path'),
            activities=activities,
            profile_memory=config.get("with_memory", False),
            aicore_metrics=AicoreMetrics[config.get("aicore_metrics", "None")],
            schedule=schedule(
                wait=int(config.get("wait", 0)),
                warmup=config.get("warmup", 0),
                active=config.get("active", 1),
                repeat=config.get("repeat", 1),
                skip_first=config.get("skip_first", 0)
            ),
            l2_cache=config.get("l2_cache", False),
            hbm_ddr=config.get("hbm_ddr", False),
            pcie=config.get("pcie", False),
            with_stack=config.get("with_stack", False),
            data_simplification=config.get("data_simplification", False)
        ) as profiler:
            step_times = []
            for src, tgt in data:
                start_time = time.time()
                loss = train_net(src, tgt)
                end_time = time.time()
                step_time = end_time - start_time
                step_times.append(step_time)
                profiler.step()
        return step_times

    step_times = []
    for src, tgt in data:
        start_time = time.time()
        loss = train_net(src, tgt)
        end_time = time.time()
        step_time = end_time - start_time
        step_times.append(step_time)

    return step_times
