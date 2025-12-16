import os
import glob
import pandas as pd
import test_base
import argparse
import unittest
import logging

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MstxProfiling(test_base.TestProfiling):
    expect_csv_header = "Device_id, pid, tid, category, event_type, payload_type, payload_value, Start_time(us), " \
                        "End_time(us), message_type, message, domain, DeviceStart_time(us), DeviceEnd_time(us)"
    ecpect_mstx_data_num = 3200

    def view_csv_content(self, csv_list):
        for _, csv in enumerate(sorted(csv_list)):
            self.logger.info("start view {} content ...".format(csv))
            df = pd.read_csv(csv)
            # check csv header
            header_list = df.columns.tolist()
            header_list = [header.replace(' ', '') for header in header_list]
            res_headers = ', '.join(header_list)
            if res_headers != self.expect_csv_header:
                self.logger.error('current header: {}'.format(res_headers))
                self.logger.info('expect header: {}'.format(self.expect_csv_header))
                self.res += 1
            # check csv data num
            num_of_data = len(df)
            if num_of_data != self.ecpect_mstx_data_num:
                self.logger.error('current data num: {}'.format(num_of_data))
                self.logger.info('expect data num: {}'.format(self.ecpect_mstx_data_num))
                self.res += 1
            # check Device_id column data
            filtered_df = df[df.iloc[:, 10].isin(['mark_without_stream', 'range_start_without_stream',
                                                  'mark_without_stream_with_domain_mark',
                                                  'range_start_without_stream_with_domain_range'])]
            device_id_column_list = filtered_df.iloc[:, 0].tolist()
            if any(str(x).isdigit() for x in device_id_column_list):
                self.logger.error('there is num value in column Device_id for tx data without stream, '
                                  'which should be NA')
                self.res += 1
            # check event_type column data
            event_type_column_list = df.iloc[:, 4].tolist()
            if any(x not in ('marker', 'start/end') for x in event_type_column_list):
                self.logger.error('there is no invalid value in column event_type')
                self.res += 1
            # check domain column data
            mark_domain_filter_df = df[df.iloc[:, 10].isin(['mark_without_stream_with_domain_mark',
                                                            'mark_with_stream_with_domain_mark'])]
            mark_domain_list = mark_domain_filter_df.iloc[:, 11].tolist()
            if any(domain != 'mark' for domain in mark_domain_list):
                self.logger.error('there is no mark domain value in column domain for tx data with mark domain')
                self.res += 1
            range_domain_filter_df = df[df.iloc[:, 10].isin(['range_start_without_stream_with_domain_range',
                                                             'range_start_with_stream_with_domain_range'])]
            range_domain_list = range_domain_filter_df.iloc[:, 11].tolist()
            if any(domain != 'range' for domain in range_domain_list):
                self.logger.error('there is no range domain value in column domain for tx data with range domain')
                self.res += 1
            # check message column data
            message_column_list = df.iloc[:, 10].tolist()
            if any(x not in ('mark_without_stream', 'range_start_without_stream',
                             'mark_with_stream', 'range_start_with_stream',
                             'mark_without_stream_with_domain_mark',
                             'range_start_without_stream_with_domain_range',
                             'mark_with_stream_with_domain_mark',
                             'range_start_with_stream_with_domain_range') for x in message_column_list):
                self.logger.error('there is no invalid value in column message')
                self.res += 1

    def getTestCmd(self, scene=None):
        scene = self.scene if scene is None else scene
        mstx_scene_to_script = {"acljson": self.cfg_path.mstx_profiling_acljson_path,
                                "aclapi": self.cfg_path.mstx_profiling_aclapi_path,
                                "msprof": self.cfg_path.mstx_profiling_msprof_path,
                                "pytorch_domain_include": self.cfg_path.mstx_profiling_pytorch_domain_include_path,
                                "pytorch_domain_exclude": self.cfg_path.mstx_profiling_pytorch_domain_exclude_path}
        script_path = mstx_scene_to_script.get(scene, "")
        self.msprofbin_cmd += ("cd {}; source activate smoke_test_env_pta; source {}/set_env.sh; "
                               "bash run_api.sh {} > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.res_dir, self.slog_stdout)

    def check_output_file(self, output_dir: str):
        mstx_csv = glob.glob(os.path.join(output_dir, 'msprof_tx_*.csv'))
        mstx_json = glob.glob(os.path.join(output_dir, 'msprof_tx_*.json'))
        if not mstx_csv:
            self.logger.error("msprof_tx.csv does not exist")
            self.res += 1
        else:
            self.view_csv_content(mstx_csv)
        if not mstx_json:
            self.logger.error("msprof_tx.json does not exist")
            self.res += 1

    def checkResDir(self, scend=None):
        prof_lst = []
        for prof_ in os.listdir(self.res_dir):
            res_path = os.path.join(self.res_dir, prof_)
            if os.path.isfile(res_path):
                continue
            prof_lst.append(res_path)
        for prof_ in prof_lst:
            if self.scene == "pytorch":
                for dir in os.listdir(prof_):
                    if dir.startswith("PROF"):
                        prof_ = os.path.join(prof_, dir)
                        break
            for dir_ in os.listdir(prof_):
                if "logs" in dir_:
                    self.view_error_msg(os.path.join(prof_, dir_), "collection")
                if "output" in dir_:
                    self.check_output_file(os.path.join(prof_, dir_))


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
        MstxProfiling(args.id, args.scene, args.mode, args.params,
                      timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
