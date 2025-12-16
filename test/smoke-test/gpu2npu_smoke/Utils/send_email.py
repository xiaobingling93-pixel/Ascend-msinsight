#!/usr/bin/env python
# -*- coding: utf-8 -*-

from email.header import Header
import smtplib
import base64

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import datetime
import os
import sys
import csv

ReceiversAtt1 = ["duanmingliang@huawei.com", "caishangqiu@huawei.com", "hekunkun@huawei.com", "pengxiaopeng1@huawei.com", "maochen7@huawei.com",
            "jiangchangting@huawei.com", "litian68@huawei.com", "louyujing@huawei.com", "lvkaimeng@huawei.com", "huxiaobo11@huawei.com", "jiwei34@huawei.com",
            "sunboquan@huawei.com", "sunyiming14@huawei.com", "wangchao285@huawei.com", "xiepeng20@huawei.com", "xieting21@huawei.com", "lichangwei4@huawei.com",
            "yangminghai2@huawei.com", "yangtao279@huawei.com", "zhouxianqi@huawei.com"]

ReceiversAtt2 = ["qiukaida1@huawei.com","sunboquan@huawei.com", "xiepeng20@huawei.com", "yangminghai2@huawei.com", "zhouxianqi@huawei.com"]

class SendEmail():
    def __init__(self, report_file, receivers):
        self.report_file = report_file
        date_now = datetime.datetime.now() + datetime.timedelta(days=0)
        self.date_day = date_now.strftime("%Y-%m-%d")
        self.sender = "yangminghai2@huawei.com"
        self.Cc = ['yangminghai2@huawei.com']
        self.receivers = receivers
        self.username = 'y30023421'
        self.password = bytes.decode(base64.b64decode(b'SFcyM0BZYW5n'))
    
    def exec(self, smoke_code="att1"):
        subject, html = self._parse_output()
        message = MIMEMultipart()
        message['Subject'] = Header(subject, 'utf-8')
        message['From'] = self.sender  # 发件人
        message['To'] = ', '.join(self.receivers)  # 收件人
        message['Cc'] = ', '.join(self.Cc) # 抄送者
        message_context = MIMEText(html, _subtype='html', _charset='utf-8')
        message.attach(message_context)
        att1 = MIMEText(open(self.report_file, 'rb').read(), 'base64', 'utf-8')
        att1["Content-Type"] = 'application/octet-stream'
        att1["Content-Disposition"] = 'attachment; filename={}'.format(self.report_file.split('/')[-1])
        message.attach(att1)
        smtpObj = smtplib.SMTP('smtp.huawei.com')
        smtpObj.login(self.username, self.password)
        smtpObj.sendmail(self.sender, self.receivers + self.Cc, message.as_string())
        smtpObj.quit()
    
    def _parse_output(self):
        html_head = "<tr><td><strong>Case No</strong></td><td><strong>Case Name</strong></td><td><strong><font>Test Case Result</font></strong></td><td><strong>Duration(s)</strong></td><td><strong>Torch Version</strong></td></tr>"
        html_end = '</table></p> </div></div></body> '
        success_num = 0
        failed_num = 0
        html_body = ""
        with open(self.report_file, "r") as f:
            lines = f.readlines()
            index = 0
            for line in lines:
                data = str(line)
                if data.find(" ") > -1 and len(data.split(" ")) == 4:
                    if data.find("pass") > -1:
                        index  = index + 1
                        success_num += 1
                        html_body += "<tr><td>" + str(index) +"</td><td>" + data.split(" ")[0] + "</td><td class = 'pass'><strong>" + data.split(" ")[1] + "</strong></td><td>" + data.split(" ")[2] + "</td><td>" + data.split(" ")[3] + "</td></tr>"
                    elif data.find("fail") > -1:
                        index  = index + 1
                        failed_num += 1
                        html_body += "<tr><td>" + str(index) +"</td><td>" + data.split(" ")[0] + "</td><td class = 'fail'><strong>" + data.split(" ")[1] + "</strong></td><td>" + data.split(" ")[2] + "</td><td>" + data.split(" ")[3] + "</td></tr>"
        subject = u"冒烟测试报告_%s (Fail: %s Pass: %s Total: %s)" %(self.date_day, failed_num, success_num, success_num + failed_num)
        html = "<title>" + subject + "</title><style> .fail { color:#CD0000; } .pass{ color:#458B00; } font{ size:'5'; face:'arial'; }td{ text-align:center;vertical-align:middle;}</style></head><body><div id='container'><div id='content'><p>" + subject + "<table width='800' border='2' bordercolor='black' cellspacing='2'>"
        html += html_head
        html += html_body
        html += html_end
        return (subject, html)



if __name__ == "__main__":
    SendEmail(sys.argv[1], ReceiversAtt1).exec()
    SendEmail(sys.argv[2], ReceiversAtt2).exec()
    print("[INFO] amtt_smoke daily result send success !")