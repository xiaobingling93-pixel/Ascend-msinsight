import os.path
import smtplib
import time
import base64
from email.mime.text import MIMEText
from email.header import Header
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email import encoders
from create_excel import test_info
from utils import count_success_status, decrypt

subject = "【ModelSlim】 自动化冒烟测试报告"  # 邮件标题
sender = 'example—mail'  # 发件人
receivers = ["example—mail"]
cc = ["example—mail"]  # 抄送人
cc_test = [
    "example—mail",
]
cc_ms_all = [
    "example—mail"
]


def check_email(email):
    check_list = [
        "example—mail"
    ]
    if email in check_list:
        return True
    return False


def send_mail(case_result, excel_name, workspace_path, case_type, version=None, cc_email=None):
    try:
        global cc, subject
        if cc_email and isinstance(cc_email, str) and check_email(cc_email):
            cc.append(cc_email)

        # 定时任务执行的存在报错抄送组内
        success_count, failed_count, total_count = count_success_status(case_result)
        subject += f"（pass: {success_count}, failed: {failed_count}）"
        if version == "rc1" and not cc_email and total_count >= 30 and failed_count > 0:
            cc.extend(cc_ms_all)

        body_html = create_table(case_result, case_type, version)
        messages = message_init(body_html, excel_name, workspace_path)
        # 设置 smtp.huawei.com 或者直接设置当前服务的ip地址
        smtp_obj = smtplib.SMTP('smtp.huawei.com')
        smtp_obj.login('username', decrypt("text", "password"))
        smtp_obj.sendmail(sender, receivers + cc, messages.as_string())
        smtp_obj.quit()
        print('send mail success')
    except smtplib.SMTPException as e:
        print("send mail fail: {}".format(e))


def create_table(case_result, case_type, version):
    # 结合原索引的稳定排序
    case_names = list(case_result.keys())  # 转为列表保留顺序
    sorted_case_names = sorted(
        case_names,
        key=lambda k: (case_result[k]['success'], case_names.index(k))
    )
    body_html = ""
    if case_type == "modelslim":
        body_html = modelslim_table(body_html, sorted_case_names, case_result, version)
    return body_html


def message_init(html, excel_name, workspace_path):
    # 创建一个带附件的实例
    message = MIMEMultipart()  # 发送消息定义
    message['Subject'] = Header(subject, 'utf-8')
    message['From'] = sender  # 发件人
    message['To'] = ';'.join(receivers)  # 收件人
    message['Cc'] = ';'.join(cc)  # 抄送者
    # 解决乱码,html是html格式的str
    message_context = MIMEText(html, _subtype='html', _charset='utf-8')
    # 邮件正文内容
    message.attach(message_context)

    metric_file_path = os.path.join(workspace_path, excel_name)
    metric_file = MIMEBase('application', "octet-stream")
    metric_file.set_payload(open(metric_file_path, 'rb').read())
    encoders.encode_base64(metric_file)
    metric_file.add_header('Content-Disposition', 'attachment; filename={}'.format(excel_name))

    log_file_path = os.path.join(workspace_path, "run.log")
    log_file = MIMEBase('application', "octet-stream")
    log_file.set_payload(open(log_file_path, 'rb').read())
    encoders.encode_base64(log_file)
    log_file.add_header('Content-Disposition', 'attachment; filename={}'.format("run.log"))

    message.attach(metric_file)  # 添加附件
    message.attach(log_file)  # 添加日志记录
    return message


def modelslim_table(body_html, case_names, case_result, version=None):
    version_name = ""
    if version == "master":
        version_name = "主线版"
    else:
        version_name = "测试版"

    # 表头
    body_html = body_html + '<body><p style="color:#1F497D;font-weight:bold;font-size:120%">' \
                + version_name + ' CANN包 --- ' \
                                 'modelslim 自动化冒烟测试报告_{}</p>' \
                                 '<p style="color:#1F497D;font-weight:bold;font-size:100%">' \
                                 'auto package time: {}</p><br/>' \
                    .format(time.strftime("%Y-%m-%d %H:%M:%S"), time.strftime("%Y-%m-%d"))
    body_html = body_html + '<table width="100%" border="1" cellspacing="0" cellpadding="10" height="100%">'
    body_html = body_html + '<tr>'
    body_html = body_html + '<th bgcolor="#CCFFFF" rowspan="1">用例名称</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">结果</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">用例场景</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">用例场景细分</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">用例细分1</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">用例细分2</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">用例描述</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">用例重要性</th>' \
                            '<th bgcolor="#CCFFFF" rowspan="1">粒度</th>'
    body_html = body_html + "</td>"
    body_html = body_html + "</tr>"
    # 表内容
    for case_name in case_names:
        result = case_result.get(case_name)

        body_html = body_html + '<tr>'
        body_html = body_html + "<td align='center' >{}</td>".format(case_name)
        if not result.get("success"):
            body_html = body_html + "<td align='center' bgcolor='#FF0000'>{}</td>" \
                .format(result.get("success"))
        else:
            body_html = body_html + "<td align='center' bgcolor='#92D050'>{}</td>" \
                .format(result.get("success"))

        # 处理test_info
        case_data = test_info.get(case_name)
        if case_data is None:
            # 填充默认值或跳过
            print(f"Warning: test_info missing for case {case_name}")
            case_data = ["N/A"] * 7  # 假设每个用例有7个字段

        # 填充各个字段
        body_html += "<td align='center'>{}</td>".format(case_data[0])
        body_html += "<td align='center'>{}</td>".format(case_data[1])
        body_html += "<td align='center'>{}</td>".format(case_data[2])
        body_html += "<td align='center'>{}</td>".format(case_data[3])
        body_html += "<td align='center'>{}</td>".format(case_data[4])
        body_html += "<td align='center'>{}</td>".format(case_data[5])
        body_html += "<td align='center'>{}</td>".format(case_data[6])
        body_html += "</tr>"
    body_html = body_html + "</table>"
    body_html = body_html + "</body>"
    return body_html
