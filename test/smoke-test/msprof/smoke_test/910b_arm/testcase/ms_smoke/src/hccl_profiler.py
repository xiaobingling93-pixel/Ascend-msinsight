"""test hccl allreduce with 2p profiling"""

import mindspore.context as context
from mindspore import Profiler
from mindspore.communication.management import init, get_rank, get_group_size
from model_zoo import AllReduceNet

context.set_context(mode=context.GRAPH_MODE, device_target='Ascend')
context.set_context(jit_level='O0')

init()
rank = get_rank()
size = get_group_size()

def test_AllReduce():
    """
    Feature: hccl operator test.
    Description: msrun hccl all_reduce 2P case.
    Expectation: success
    """
    profiler = Profiler(output_path="profiler_hccl_data_" + str(rank), profile_communication=True,
                        data_simplification=False)
    all_reduce = AllReduceNet()
    all_reduce()
    profiler.analyse()
