#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# -------------------------------------------------------------------------
# This file is part of the MindStudio project.
# Copyright (c) 2025 Huawei Technologies Co.,Ltd.
#
# MindStudio is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# -------------------------------------------------------------------------

import smtplib
from datetime import datetime, timezone
from email.header import Header
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText


class PerformanceResult:
    def __init__(self, num, module, name, result, baseline, real_time):
        self.num = num
        self.module = module
        self.name = name
        self.result = result
        self.baseline = baseline
        self.real_time = real_time


class EmailSender:
    def __init__(self, sender, username, passwd, receivers):
        self.sender = sender
        self.username = username
        self.passwd = passwd
        self.receivers = receivers

    @staticmethod
    def generate_email_content(test_results):
        count_pass = 0
        for tmp in test_results:
            if tmp.result == 'pass':
                count_pass += 1
        count_all = len(test_results)
        count_fail = count_all - count_pass
        pass_rate = 100 if count_all == 0 else round(count_pass * 100 / count_all, 2)
        test_time = datetime.now(tz=timezone.utc).strftime("%Y-%m-%d")
        subject = f'[MindStudio-Insight][master] 每日性能用例测试报告_{test_time} ' \
                  f'(Total: {count_all} Fail: {count_fail} Pass: {count_pass})'
        email = {}
        email.update({'subject': subject})
        if not test_results:
            email.update({'html_content': '<div>Test failed, see the log for details.</div>'})
            return email

        color = 'red' if pass_rate < 90 else 'black'
        html_content = (
            "<head>"
            f"<title>{subject}</title>"
            "<style>"
            ".fail { color:#CD0000; } .pass { color:#458B00; } "
            "font { size:'5'; face:'arial'; }"
            "td { text-align:center;vertical-align:middle; }"
            "</style>"
            "</head>"
            "<body>"
            f"<p><font size='4' color='black'><b>用例通过率为：</span><font size='4' color='{color}'>{pass_rate}%</span></p>"
            "<table width='800' border='2' bordercolor='black' cellspacing='2'>"
            "   <tr>"
            "       <td><strong>Case No</strong></td>"
            "       <td><strong>Model Name</strong></td>"
            "       <td><strong>Case Name</strong></td>"
            "       <td><strong><font>Test Result</font></strong></td>"
            "       <td><strong>Baseline Time(ms)</strong></td>"
            "       <td><strong>Real Time(ms)</strong></td>"
            "   </tr>"
        )

        for result in test_results:
            html_content += (
                "<tr>"
                f"   <td>{result.num}</td>"
                f"   <td>{result.module}</td>"
                f"   <td>{result.name}</td>"
                f"   <td class = {result.result}><strong>{result.result}<strong></td>"
                f"   <td>{result.baseline}</td>"
                f"   <td>{result.real_time}</td>"
                "</tr>"
            )
        email.update({'html_content': html_content})

        return email

    def send(self, subject, html_content, attachment_path, receivers, cc_list):
        message = MIMEMultipart()
        message['Subject'] = Header(subject, 'utf-8')
        message['From'] = self.sender
        message['To'] = ', '.join(receivers)
        message['Cc'] = ', '.join(cc_list)

        content = MIMEText(html_content, _subtype='html', _charset='utf-8')
        message.attach(content)

        smtp_obj = smtplib.SMTP('smtp.huawei.com')
        smtp_obj.login(self.username, self.passwd)
        smtp_obj.sendmail(self.sender, receivers + cc_list, message.as_string())
        smtp_obj.quit()
