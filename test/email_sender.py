#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
"""
-------------------------------------------------------------------------
This file is part of the MindStudio project.
Copyright (c) 2025 Huawei Technologies Co.,Ltd.

MindStudio is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:

         http://license.coscl.org.cn/MulanPSL2

THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details.
-------------------------------------------------------------------------
"""
# # email send
import configparser
import logging
import os
import smtplib
from email import encoders
from email.header import Header
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText


class ReadEmailConfigException(Exception):
    def __init__(self, msg=''):
        self.msg = msg
        super().__init__(self.msg)


def read_email_config(config_file):
    config = configparser.ConfigParser()
    try:
        with open(config_file, 'r') as f:
            config.read_file(f)
    except FileNotFoundError as e:
        logging.error('The email config file does not exist.')
        raise ReadEmailConfigException from e
    except Exception as e:
        logging.error(f'Failed to read email config file: %s', e)
        raise ReadEmailConfigException from e

    email_config = 'email_config'
    try:
        username = config.get(email_config, 'username')
        passwd = config.get(email_config, 'passwd')
        sender = config.get(email_config, 'sender')
        receiver = config.get(email_config, 'receiver')
        receivers = receiver.split(',')
    except Exception as e:
        raise ReadEmailConfigException from e

    return sender, username, passwd, receivers


class EmailSender:
    def __init__(self, sender, username, passwd, receivers):
        self.sender = sender
        self.username = username
        self.passwd = passwd
        self.receivers = receivers

    def send(self, subject, html_content, attachment_path):
        message = MIMEMultipart()
        message['Subject'] = Header(subject, 'utf-8')
        message['From'] = self.sender
        message['To'] = ', '.join(self.receivers)
        message['Cc'] = ''

        content = MIMEText(html_content, _subtype='html', _charset='utf-8')
        message.attach(content)

        if os.path.exists(attachment_path) and os.path.isfile(attachment_path):
            with open(attachment_path, 'rb') as file:
                part = MIMEBase('application', 'octet-stream')
                part.set_payload(file.read())
                encoders.encode_base64(part)
                part.add_header('Content-Disposition', f'attachment; filename={os.path.basename(attachment_path)}')
                message.attach(part)

        smtp_obj = smtplib.SMTP('smtp.huawei.com')
        smtp_obj.starttls()
        smtp_obj.login(self.username, self.passwd)
        smtp_obj.sendmail(self.sender, self.receivers, message.as_string())
        smtp_obj.quit()
