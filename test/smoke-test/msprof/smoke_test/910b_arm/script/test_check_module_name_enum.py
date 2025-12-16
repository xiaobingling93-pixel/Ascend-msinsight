import os
import re
import sys

import test_base
import argparse
import unittest
import logging
from collections import Counter

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')

class CheckModuleNameTest(test_base.TestProfiling):
    def remove_comments(self, code):
        # 移除多行注释
        code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
        # 移除单行注释
        code = re.sub(r'//.*', '', code)
        return code.replace('\n', '')

    def extract_c_enum_values(self, filename):
        ans = []
        with open(filename, 'r') as file:
            content = file.read()
        # 移除注释
        content = self.remove_comments(content)
        # 正则表达式匹配枚举定义
        enum_pattern = r'enum\s+([a-zA-Z_][a-zA-Z0-9_]*)?\s*\{([^}]*)\};'
        match = re.findall(enum_pattern, content, re.DOTALL)
        if match:
            for enum_string in match:
                enum_name = enum_string[0]  # 枚举名称
                enum_values = {}
                enum_items = enum_string[1].split(',')
                for item in enum_items:
                    item = item.strip()
                    if '=' in item:
                        name, value = item.split('=')
                        enum_values[name.strip()] = int(value.strip())
                    else:
                        # 如果没有显式值，假设是连续的整数
                        if not enum_values:
                            enum_values[item.strip()] = 0
                        else:
                            last_value = list(enum_values.values())[-1] + 1
                            enum_values[item.strip()] = last_value
                ans.append(enum_values)
            return ans
        else:
            return ans

    def checkResDir(self, scene=None):
        slog_header_path = os.path.join(self.cfg_path.latest_path,
                                        'aarch64-linux/include/base/log_types.h')
        slog_enum = self.extract_c_enum_values(slog_header_path)[1]
        data_tag_py_path = os.path.join(self.cfg_path.latest_path, 'tools/profiler/profiler_tool/analysis/profiling_bean/prof_enum/')
        sys.path.append(data_tag_py_path)
        from data_tag import ModuleName
        python_enum_values = {item.name: item.value for item in ModuleName}
        # 这两项是约定好加在SLOG后面的，不能删
        slog_enum.pop("INVLID_MOUDLE_ID")
        python_enum_values.pop("MBUFF")
        python_enum_values.pop("CUSTOM")
        # 比较两个枚举
        assert len(Counter(slog_enum) - Counter(python_enum_values)) == 0, \
            (f"log_types.h is different from data_tag.py, differ element"
             f"is {Counter(slog_enum) - Counter(python_enum_values)}")

    def getTestCmd(self, scene=None):
        pass

if __name__ == "__main__":
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
        CheckModuleNameTest(args.id, args.scene, args.mode, args.params,
                            timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
