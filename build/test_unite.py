#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2023 Huawei Technologies Co., Ltd
# ============================================================================

"""jacoco unite for Insight"""
import os
import shutil
import stat
import csv
import json
import logging
from bs4 import BeautifulSoup


class Constant:
    MODULES_LIST = [
        r"modules/memory",
        r"modules/timeline",
        r"modules/cluster",
        r"modules/compute"
    ]
    SCRIPT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    FINAL_REPORT_PATH = os.path.join(SCRIPT_PATH, "llt_cov")


def copy_exist_directory(src, dst):
    for file in os.listdir(src):
        shutil.copy(os.path.join(src, file), dst)


def gen_result():
    total_info = {}
    module_report_path = ""
    for module in Constant.MODULES_LIST:
        module_report_path = os.path.join(Constant.SCRIPT_PATH,
                                          module, "coverage", "lcov-report")
        if not os.path.exists(module_report_path):
            logging.info("[Warning] No such file or directory:%s", module_report_path)
            continue
        logging.info("[Info] Deal directory:%s", module_report_path)

        with open(os.path.join(module_report_path, "index.html")) as report:
            report_content = report.read()
            soup = BeautifulSoup(report_content, 'html.parser')
            percentage = soup.find_all(attrs={'class': 'strong'})[0:4]
            fraction = soup.find_all(attrs={'class': 'fraction'})[0:4]
            total_info[module] = {"percentage": percentage, "fraction": fraction}

    return total_info


def clear_old_result():
    if os.path.isdir(Constant.FINAL_REPORT_PATH):
        shutil.rmtree(Constant.FINAL_REPORT_PATH)


def write_junit_json(total_line_cov, total_method_cov, total_branch_cov):
    json_text = {'lineCoverage': {}, 'methodCoverage': {}, 'branchCoverage': {}}
    total_line_cov_per = str(round(total_line_cov, 2))
    total_method_cov_per = str(round(total_method_cov, 2))
    total_branch_cov_per = str(round(total_branch_cov, 2))
    json_text.get('lineCoverage').\
        update({'percentage': total_line_cov_per})
    json_text.get('methodCoverage').\
        update({'percentage': total_method_cov_per})
    json_text.get('branchCoverage').\
        update({'percentage': total_branch_cov_per})
    jsondata = json.dumps(json_text, indent=4, separators=(',', ': '))

    if not os.path.exists("junit"):
        os.mkdir("junit")
    junit_json = os.fdopen(
        os.open('./junit/junit.json', os.O_CREAT | os.O_RDWR, 0o640),
        'w')
    junit_json.write(jsondata)
    junit_json.close()


def write_total_coverage(results):
    cov_line_cnt = 0
    total_line_cnt = 0
    cov_method_cnt = 0
    total_method_cnt = 0
    cov_branch_cnt = 0
    total_branch_cnt = 0

    result_path = os.path.join(Constant.FINAL_REPORT_PATH, "total_cov.csv")
    os.mkdir(Constant.FINAL_REPORT_PATH)
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IRGRP
    with os.fdopen(os.open(result_path, flags, modes), 'w') as file_csv:
        field = ["module", "total_line", "line_cov", "total_method", "method_cov", "total_branch", "branch_cov"]
        writer = csv.DictWriter(file_csv, field)
        writer.writeheader()

        for module, total in results.items():
            fraction = total.get("fraction")
            lines = str(fraction[3]).removeprefix("<span class=\"fraction\">").removesuffix("</span>")
            branchs = str(fraction[1]).removeprefix("<span class=\"fraction\">").removesuffix("</span>")
            methods = str(fraction[2]).removeprefix("<span class=\"fraction\">").removesuffix("</span>")
            cov_branch = int(branchs.split("/")[0])
            total_branch = int(branchs.split("/")[1])
            cov_line = int(lines.split("/")[0])
            total_line = int(lines.split("/")[1])
            cov_method = int(methods.split("/")[0])
            total_method = int(methods.split("/")[1])
            line = {
                "module": module,
                "total_line": total_line,
                "line_cov": cal_coverage(cov_line, total_line),
                "total_method": total_method,
                "method_cov": cal_coverage(cov_method, total_method),
                "total_branch": total_branch,
                "branch_cov": cal_coverage(cov_branch, total_branch)
            }
            writer.writerow(line)
            logging.info("[module] %-28s  [line cov] %.3f  [method cov] %.3f  [branch cov] %.3f",
                module, line.get("line_cov"), line.get("method_cov"), line.get("branch_cov"))
            cov_line_cnt += cov_line
            total_line_cnt += total_line
            cov_method_cnt += cov_method
            total_method_cnt += total_method
            cov_branch_cnt += cov_branch
            total_branch_cnt += total_branch
    total_line_cov = cal_coverage(cov_line_cnt, total_line_cnt) * 100
    total_method_cov = cal_coverage(cov_method_cnt, total_method_cnt) * 100
    total_branch_cov = cal_coverage(cov_branch_cnt, total_branch_cnt) * 100
    write_junit_json(total_line_cov, total_method_cov, total_branch_cov)
    logging.info("Total line cov: %.2f", total_line_cov)
    logging.info("Total method cov: %.2f", total_method_cov)
    logging.info("Total branch cov: %.2f", total_branch_cov)


def cal_coverage(cov, total):
    if total == 0:
        return 0
    return cov * 1.0 / total


def main():
    logging.basicConfig(level=logging.INFO)
    clear_old_result()
    results = gen_result()
    write_total_coverage(results)


if __name__ == "__main__":
    main()
