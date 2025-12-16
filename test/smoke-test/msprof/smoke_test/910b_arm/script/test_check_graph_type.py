#  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
import os
import re
import sys

import test_base
import argparse
import unittest
import logging

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')

ge_data_type_exclude_key = ['NUMBER_TYPE_BEGIN_', 'BOOL_', 'INT_', 'INT8_', 'INT16_', 'INT32_', 'INT64_', 'UINT_',
                            'UINT8_', 'UINT16_', 'UINT32_', 'UINT64_', 'FLOAT_', 'FLOAT16_', 'FLOAT32_', 'FLOAT64_',
                            'COMPLEX_', 'NUMBER_TYPE_END_', 'UNDEFINED']

ge_data_format_exclude_key = ['UNKNOWN_', 'DEFAULT_', 'NC1KHKWHWC0_', 'ND_', 'NCHW_', 'NHWC_', 'HWCN_', 'NC1HWC0_',
                              'FRAC_Z_', 'C1HWNCOC0_', 'FRAC_NZ_', 'NC1HWC0_C04_', 'FRACTAL_Z_C04_', 'NDHWC_',
                              'FRACTAL_ZN_LSTM_', 'FRACTAL_ZN_RNN_', 'ND_RNN_BIAS_', 'NDC1HWC0_', 'NCDHW_',
                              'FRACTAL_Z_3D_', 'DHWNC_', 'DHWCN_', 'UNDEFINED']


class CheckGraphTypeTest(test_base.TestProfiling):
    def getEnumDataTypes(self, filename):
        with open(filename, 'r') as file:
            content = file.read()
        # 正则表达式匹配枚举定义
        enum_pattern = r'enum\s+([a-zA-Z_][a-zA-Z0-9_]*)?\s*\{([^}]*)\};'
        pattern = r'enum DataType \{(.+?)\}'  # 使用非贪婪匹配(.+?)来匹配大括号内的内容
        match_data_type = re.findall(pattern, content, re.DOTALL)  # re.DOTALL使得.可以匹配换行
        data_type = match_data_type[0].split("\n")
        pattern = r'enum Format \{(.+?)\}'
        match_format = re.findall(pattern, content, re.DOTALL)
        format_type = match_format[0].split("\n")
        format_list = []
        data_type_list = []
        for i in format_type:
            match_type = re.search(r'\bFORMAT_[A-Za-z0-9_]*\b', i)
            if match_type:
                value = match_type.group().removeprefix("FORMAT_")
                if value not in format_list:
                    format_list.append(value)
        for i in data_type:
            match_type = re.search(r'\b[A-Z][A-Za-z0-9_]*\b', i)
            if match_type:
                value = match_type.group()
                value = match_type.group().removeprefix("DT_")
                if value not in data_type_list:
                    data_type_list.append(value)

        return format_list, data_type_list

    def checkResDir(self, scene=None):
        graph_type_path = os.path.join(self.cfg_path.latest_path,
                                       'aarch64-linux/include/graph/types.h')
        cann_ge_format, cann_ge_data_type = self.getEnumDataTypes(graph_type_path)
        data_tag_py_path = os.path.join(self.cfg_path.latest_path,
                                        'tools/profiler/profiler_tool/analysis/common_func/ms_constant/')
        sys.path.append(data_tag_py_path)
        from ge_enum_constant import GeDataType
        from ge_enum_constant import GeDataFormat
        # CANN包和Python侧比较时均统一去掉 DT_ 和 FORMAT_ 的前缀
        python_ge_data_type = [item.name.removeprefix("DT_") for item in GeDataType if
                               item.name not in ge_data_type_exclude_key]
        python_ge_data_format = [item.name for item in GeDataFormat if item.name not in ge_data_format_exclude_key]

        assert python_ge_data_type == cann_ge_data_type, (f"graph/type.h is different from ge_enum_constant.py,"
                                                          f" differ element is GeDataType:"
                                                          f" {[key for key in cann_ge_data_type if key not in python_ge_data_type]}")

        assert python_ge_data_format == cann_ge_format, (f"graph/type.h is different from ge_enum_constant.py,"
                                                         f" differ element is GeDataFormat:"
                                                         f"{[key for key in cann_ge_format if key not in python_ge_data_format]}")

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
        CheckGraphTypeTest(args.id, args.scene, args.mode, args.params,
                           timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
