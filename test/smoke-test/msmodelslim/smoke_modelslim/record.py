import json
import os
from typing import Dict


def record(**kwargs):
    record_to_file(get_default_record_path(), **kwargs)


def record_to_file(record_path, **kwargs):
    record_dict: Dict = dict()
    if os.path.exists(record_path):
        with open(record_path, "r") as f:
            record_dict = json.load(f)

    with open(record_path, "w+") as f:
        record_dict.update(kwargs)
        json.dump(record_dict, f, indent=4)


def get_default_record_path():
    record_path = os.getenv("CASE_WORKSPACE_PATH", os.getenv("WORKSPACE_PATH", ""))
    record_file_path = os.path.join(record_path, "metric.json")
    return record_file_path


def get_default_log_path():
    log_path = os.getenv("CASE_WORKSPACE_PATH", os.getenv("WORKSPACE_PATH", ""))
    log_file_path = os.path.join(log_path, "run.log")
    return log_file_path


def get_default_log_detail_path():
    log_path = "/path/log_path"
    log_detail_name = os.getenv("LOG_DETAIL_FILE_NAME", "")
    log_detail_file_path = ""
    if log_detail_name:
        log_detail_file_path = os.path.join(log_path, log_detail_name)
    return log_detail_file_path
