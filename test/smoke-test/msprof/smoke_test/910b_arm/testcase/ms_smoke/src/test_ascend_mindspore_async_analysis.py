import os
import glob
import shutil
import tempfile
from mark_utils import arg_mark


def cleanup():
    data_path = os.path.join(os.getcwd(), "data")
    kernel_meta_path = os.path.join(os.getcwd(), "kernel_data")
    cache_path = os.path.join(os.getcwd(), "__pycache__")
    if os.path.exists(data_path):
        shutil.rmtree(data_path)
    if os.path.exists(kernel_meta_path):
        shutil.rmtree(kernel_meta_path)
    if os.path.exists(cache_path):
        shutil.rmtree(cache_path)


class CheckProfilerFiles:
    def __init__(self, profiler_path, device_target, profile_framework='all'):
        """Args init."""
        self.profiler_path = profiler_path
        self.device_target = device_target
        if device_target == "Ascend":
            self._check_d_profiling_file()
            self._check_host_profiling_file(profile_framework=profile_framework)

    def _check_d_profiling_file(self):
        """Check Ascend profiling file."""
        kernel_details_file = f'kernel_details.csv'
        trace_viewer_file = f'trace_view.json'

        d_profiler_files = (kernel_details_file, trace_viewer_file)
        for _file in d_profiler_files:
            result_file = os.path.join(self.profiler_path, _file)
            assert os.path.isfile(result_file)

    def _check_host_profiling_file(self, profile_framework='all'):
        dataset_csv = os.path.join(self.profiler_path, f'dataset.csv')
        if profile_framework in ['all', 'time']:
            assert os.path.isfile(dataset_csv)
        else:
            assert not os.path.exists(dataset_csv)


@arg_mark(plat_marks=['platform_ascend'], level_mark='level1', card_mark='onecard', essential_mark='essential')
def test_ascend_profiler():
    """
    Feature: Ascend Profiler
    Description: Test Ascend Profiler with step profiler.
    Expectation: The profiler successfully collects data and generates the expected files.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        status = os.system(
            """python ./run_net_with_profiler.py --target=Ascend --mode=0 --output_path=%s""" % tmpdir
        )
        ascend_profiler_output_path = glob.glob(f"{tmpdir}/*_ascend_ms/ASCEND_PROFILER_OUTPUT")[0]
        CheckProfilerFiles(ascend_profiler_output_path, "Ascend")
        assert status == 0


if __name__ == '__main__':
    test_ascend_profiler()
