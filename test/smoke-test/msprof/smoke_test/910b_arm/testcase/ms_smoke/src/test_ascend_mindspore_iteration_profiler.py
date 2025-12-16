"""Test cluster analyse pytorch db"""
import glob
import json
import logging
import os
import re
import subprocess
from unittest import TestCase

from path_manager import PathManager

COMMAND_SUCCESS = 0
pattern = re.compile(r'Iteration \d+')


def load_json(json_file_path):
    with open(json_file_path, 'r', encoding='utf-8') as file:
        data = json.load(file)
        return data


def execute_cmd(cmd):
    logging.info('Execute command:%s' % " ".join(cmd))
    completed_process = subprocess.run(cmd, shell=False, stderr=subprocess.PIPE)
    if completed_process.returncode != COMMAND_SUCCESS:
        logging.error(completed_process.stderr.decode())
    return completed_process.returncode


class TestAscendMindsporeIterationProfiler(TestCase):
    """
       Test mindspore profiler iteration
    """
    ITERATION_MS_PROFILER = os.path.join(os.path.abspath(os.path.join(os.path.dirname(__file__))),
                                         "iteration_ms_profiler.py")
    OUTPUT_PATH = "./iteration"
    COMMAND_SUCCESS = 0

    def setup_class(self):
        # make output file
        PathManager.make_dir_safety(self.OUTPUT_PATH)
        # generate profiler data
        cmd = ["python3", self.ITERATION_MS_PROFILER, "--output_path", self.OUTPUT_PATH]
        if execute_cmd(cmd) != self.COMMAND_SUCCESS or not os.path.exists(self.OUTPUT_PATH):
            self.fail("iteration ms profiler task failed.")

    def teardown_class(self):
        # Delete profiler Data
        PathManager.remove_path_safety(self.OUTPUT_PATH)

    def test_iteration_profiler(self):
        """
        Test case to verify the time of the operator in model_id and iteration.
        """
        trace_view_json_path = glob.glob(f"{self.OUTPUT_PATH}/*_ascend_ms/"
                                         f"ASCEND_PROFILER_OUTPUT/trace_view.json")[0]
        trace_view_json = load_json(trace_view_json_path)
        hard_ware_pid, iteration, step_trace = self._extract_hardware_info_and_traces(trace_view_json)
        # check iteration number
        self.assertEqual(len(iteration), 1, "The number of iterations is incorrect.")
        # check iteration ID
        self.assertEqual(iteration[0].get('args').get('Iteration ID'), 1, "The ID of iterations is incorrect.")
        # check step trace number
        self.assertEqual(len(step_trace), 1, "The number of step trace is incorrect.")

        start_time, end_time = self._get_st_et_by_iteration(iteration)
        ops = self._extract_hardware_ops(trace_view_json, hard_ware_pid)
        for op in ops:
            ts = float(op.get('ts'))
            if not self._check_ts(ts, start_time, end_time):
                self.fail("name: %s is not in iteration", op.get('name'))

    def _extract_hardware_info_and_traces(self, data):
        hard_ware_pid = ''
        step_trace = []
        iteration = []
        for d in data:
            if d.get('args') and d.get('args').get('name') == 'Ascend Hardware':
                hard_ware_pid = d.get('pid')
            if pattern.match(d.get('name')):
                iteration.append(d)
            if d.get('args') and d.get('args').get('name'):
                if d.get('args').get('name').startswith('Step Trace') and d.get('pid') == hard_ware_pid:
                    step_trace.append(d)
        return hard_ware_pid, iteration, step_trace

    def _extract_hardware_ops(self, data, hard_ware_pid):
        ops = []
        for d in data:
            if d.get('pid') == hard_ware_pid and d.get('ts'):
                ops.append(d)
        return ops

    def _check_ts(self, ts, start_time, end_time):
        if start_time <= ts <= end_time:
            return True
        return False

    def _get_st_et_by_iteration(self, iterations):
        return float(iterations[0].get('ts')), float(iterations[0].get('args').get('Iteration End'))


if __name__ == '__main__':
    test = TestAscendMindsporeIterationProfiler()
    test.test_iteration_profiler()
