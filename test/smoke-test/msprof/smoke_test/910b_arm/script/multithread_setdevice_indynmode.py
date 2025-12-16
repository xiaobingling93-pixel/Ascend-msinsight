import logging
import subprocess
from datetime import datetime
import argparse
import unittest
import test_base

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MulThreadSetDevInDynMode(test_base.TestProfiling):
    def getTestCmd(self):
        script_path = self.cfg_path.multi_thread_setdevice_in_dynmode
        self.msprofbin_cmd = ("cd {}; source {}/set_env.sh; "
                              "bash run.sh > {}").format(script_path,
                                                         self.cfg_path.ascend_toolkit_path,
                                                         self.slog_stdout)

    def checkResDir(self, scene=None):
        pass


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--id', help='test case')
    parser.add_argument('-s', '--scene', required=True,
                        help='run App or Sys or All')
    parser.add_argument('-m', '--mode', required=True,
                        help='mode in slogConfig')
    parser.add_argument('-p', '--params',
                        help='params of runtime test script')
    parser.add_argument('-t', '--timeout',
                        help='timeout of test case')
    args = parser.parse_args()
    suite = unittest.TestSuite()
    suite.addTest(
        MulThreadSetDevInDynMode(args.id, args.scene, args.mode, args.params,
                                 timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
