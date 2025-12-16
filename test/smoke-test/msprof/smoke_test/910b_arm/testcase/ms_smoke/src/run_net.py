from argparse import ArgumentParser

from model_zoo import TinyTransformer
from fake_dataset import FakeDataset


def train_with_profiler():
    """Train Net with profiling."""
    net = TinyTransformer(d_model=2, nhead=1, num_encoder_layers=1, num_decoder_layers=1, dim_feedforward=4)
    nlp_dataset = FakeDataset.create_fake_nlp_dataset(seq_len=1, batch_size=1, d_model=2, tgt_len=1, num_samples=1)
    for src, tgt in nlp_dataset:
        net(src, tgt)


parser = ArgumentParser(description='test env enable profiler')
parser.add_argument('--target', type=str)
parser.add_argument('--mode', type=int)
args = parser.parse_args()
train_with_profiler()
