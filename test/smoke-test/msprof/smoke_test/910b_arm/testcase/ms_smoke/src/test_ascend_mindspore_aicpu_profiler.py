"""Test custom aicpu profiling."""
import os.path
import tempfile
import numpy as np
import glob

import mindspore.context as context
import mindspore.common.dtype as mstype
from mindspore import Profiler
from mindspore import Tensor
from mindspore.profiler import ProfilerLevel
from mark_utils import arg_mark
from model_zoo import CustomAICpuNet
from file_check import FileChecker


@arg_mark(plat_marks=['platform_ascend'], level_mark='level1', card_mark='onecard', essential_mark='unessential')
def test_collect_custom_aicpu():
    """
    Feature: Profiling can collect custom aicpu operators
    Description: Test profiling can collect custom aicpu operators on ascend
    Expectation: The file aicpu_intermediate_*.csv generated successfully and s1 == s2
    """
    context.set_context(mode=context.GRAPH_MODE, device_target="Ascend")
    context.set_context(jit_level="O2")
    with tempfile.TemporaryDirectory(suffix="profiler_ai_cpu") as tmpdir:
        profiler = Profiler(output_path=tmpdir, profiler_level=ProfilerLevel.Level1)
        net = CustomAICpuNet()
        net(Tensor(np.random.random((6,)), mstype.float16))
        profiler.analyse()
        op_dict = {"OP Type": ["Cast", "Select", "Xlogy"]}
        ascend_profiler_output_path = glob.glob(f"{tmpdir}/*_ascend_ms/ASCEND_PROFILER_OUTPUT")[0]
        FileChecker.check_csv_items(os.path.join(ascend_profiler_output_path, "op_statistic.csv"),
                                    op_dict, fuzzy_match=False)


if __name__ == '__main__':
    test_collect_custom_aicpu()
