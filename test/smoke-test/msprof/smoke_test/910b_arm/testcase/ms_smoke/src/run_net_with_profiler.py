from argparse import ArgumentParser
from mindspore.profiler import ProfilerLevel

from mindspore import Profiler
from model_zoo import TinyTransformer
from fake_dataset import FakeDataset


def train_with_profiler():
    """Train Net with profiling."""
    output_path = args.output_path
    profiler = Profiler(
        profiler_level=ProfilerLevel.Level0,
        output_path=output_path,
        op_time=True,
        profile_communication=True,
        profile_memory=True,
        parallel_strategy=True,
        start_profile=True,
        aicore_metrics=1,
        l2_cache=True,
        hbm_ddr=True,
        pcie=True,
        sync_enable=True,
        data_process=True,
        profile_framework='all',
        with_stack=False,
        data_simplification=False
    )
    net = TinyTransformer(d_model=2, nhead=1, num_encoder_layers=1, num_decoder_layers=1, dim_feedforward=4)
    nlp_dataset = FakeDataset.create_fake_nlp_dataset(seq_len=1, batch_size=1, d_model=2, tgt_len=1, num_samples=1)
    for src, tgt in nlp_dataset:
        net(src, tgt)
    profiler.analyse()


parser = ArgumentParser(description='test async analysis profiler')
parser.add_argument('--target', type=str)
parser.add_argument('--mode', type=int)
parser.add_argument('--output_path', type=str)
args = parser.parse_args()
train_with_profiler()
