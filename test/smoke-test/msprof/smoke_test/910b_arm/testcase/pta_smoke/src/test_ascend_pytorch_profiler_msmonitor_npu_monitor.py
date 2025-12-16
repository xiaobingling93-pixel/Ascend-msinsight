import glob
import sys

import logging

from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


def test_profiler_with_msmonitor_npu_monitor(npu_monitor_path):
    # 校验db类型的npu_monitor数据结果
    _check_monitor_files(npu_monitor_path)


def _check_monitor_files(npu_monitor_path):
    db_path = glob.glob(
        f"{npu_monitor_path}/*.db"
    )
    assert db_path, f"No db file found in {npu_monitor_path}"
    expect_tables = ["CANN_API", "COMMUNICATION_OP", "COMPUTE_TASK_INFO", "TASK"]
    for db in db_path:
        for table_name in expect_tables:
            # 1. Check table exist
            FileChecker.check_db_table_exist(db, table_name)
            # 2. Check table fields
            res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
            assert res_table_fields == TableFields.get_fields(table_name), \
                f"Fields for table '{table_name}' do not match. Expected: {TableFields.get_fields(table_name)}, Actual: {res_table_fields}"


if __name__ == '__main__':
    prof_path = sys.argv[1]
    test_profiler_with_msmonitor_npu_monitor(prof_path)