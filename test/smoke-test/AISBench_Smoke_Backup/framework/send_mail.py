import os.path
import sys
import smtplib
import time
import base64
from pathlib import Path
from cryptography.fernet import InvalidToken, Fernet
from email.mime.text import MIMEText
from email.header import Header
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email import encoders

from run_log import log
from email_list import receivers, sender, cc
from utils import print_json, get_max_deep, find_value_in_same_level


def get_credentials_paths() -> tuple[Path, Path]:
    """获取凭据文件的安全路径，包含异常处理"""
    try:
        # 获取当前脚本目录的父目录（2级目录）
        project_path = Path(os.getenv('PROJECT_RESOURCE_PATH') or Path(__file__).resolve().parent.parent).resolve()
        
        # 构建凭据目录路径
        credentials_dir = project_path / "credentials"
        
        # 检查目录是否存在
        if not credentials_dir.is_dir():
            raise FileNotFoundError(f"凭据目录不存在: {credentials_dir}")
        
        # 构建文件路径
        enc_file = credentials_dir / "credentials.enc"
        key_file = credentials_dir / "secret.key"
        
        # 检查文件是否存在
        if not enc_file.is_file():
            raise FileNotFoundError(f"加密凭据文件不存在: {enc_file}")
        if not key_file.is_file():
            raise FileNotFoundError(f"密钥文件不存在: {key_file}")
        
        return enc_file, key_file
        
    except Exception as e:
        # 将错误信息输出到标准错误流
        log(f"❌ 凭据路径获取失败: {str(e)}")
        log("\n👉 解决方案:")
        log("     请直接使用以下命令生成加密凭据:")
        log("     python scripts/generate_secret_key.py")
        # 重新抛出异常, 在send_mail函数中捕获
        raise e

def decrypt_credentials() -> tuple[str, str]:
    """解密凭据文件并返回账号密码"""
    try:
        # 获取文件路径
        enc_path, key_path = get_credentials_paths()
        
        # 1. 读取密钥
        with open(key_path, 'rb') as f:
            key = f.read()
        
        # 2. 检查密钥有效性
        if len(key) != 44:  # Fernet密钥固定长度
            raise ValueError("无效的密钥格式: 长度应为44字节")
        
        # 3. 读取加密凭据
        with open(enc_path, 'rb') as f:
            encrypted_data = f.read()
        
        # 4. 解密凭据
        cipher = Fernet(key)
        decrypted_bytes = cipher.decrypt(encrypted_data)
        credentials = decrypted_bytes.decode('utf-8')
        
        # 5. 验证格式
        if ':' not in credentials:
            raise ValueError("解密内容格式错误，缺少冒号分隔符")
        
        # 6. 解析凭据
        username, password = credentials.split(':', 1)
        
        return username.strip(), password.strip()
    
    except InvalidToken as e:
        # 处理解密失败
        log("❌ 凭据解密失败: 可能是密钥不匹配或文件损坏")
        log("🔑 请尝试重新生成凭据文件并替换现有文件")
        raise e
        
    except ValueError as e:
        # 处理值格式错误
        log(f"❌ 凭据格式错误: {str(e)}")
        log("请确保使用最新版本的凭据生成工具创建凭据")
        raise e
        
    except Exception as e:
        # 处理其他未知异常
        log(f"❌ 凭据处理过程中发生未知错误: {str(e)}")
        raise e


def send_mail(case_result, workspace_path, case_type, case_names, excel_name = None):
    try:
        body_html = create_table(case_result, case_type, case_names)
        if excel_name:
            messages = message_init(body_html, workspace_path, excel_name=excel_name)
        else:
            messages = message_init(body_html, workspace_path)

        account, password = decrypt_credentials()
        log(f"🔐 成功获取SMTP凭据，账号: {account}")
        smtp_obj = smtplib.SMTP('smtp.huawei.com') # 修改为smtp.huawei.com（发邮件）或pop.huawei.com（收邮件），或者其对应的新IP：7.221.188.53(2025.6.17版本, 更新请联系陶蓓 00825433)
        smtp_obj.login(account, password)  # 打开电脑的账号密码
        smtp_obj.sendmail(sender, receivers + cc, messages.as_string())
        smtp_obj.quit()
        log('send mail success')
        log(f"from \033[1;33m{sender}\033[0m send to : \033[1;33m{', '.join(receivers + cc)}\033[0m")
    except smtplib.SMTPException as e:
        log("send mail fail: {}".format(e))


def create_table(case_result, case_type, case_names):
    body_html = ""
    if case_type == "benchmark":
        body_html = aisbench_benchmark_table(body_html, case_names, case_result)
    return body_html

def message_init(html, workspace_path, excel_name:str = None):
    # 创建一个带附件的实例
    subject = "【冒烟测试】【AISBench】 AISBench功能自动化冒烟测试报告"  # 邮件标题
    message = MIMEMultipart()  # 发送消息定义
    message['Subject'] = Header(subject, 'utf-8')
    message['From'] = sender  # 发件人
    message['To'] = ';'.join(receivers)  # 收件人
    message['Cc'] = ';'.join(cc)  # 抄送者
    # 解决乱码,html是html格式的str
    message_context = MIMEText(html, _subtype='html', _charset='utf-8')
    # 邮件正文内容
    message.attach(message_context)

    if excel_name:
        acc_fps_file_path = os.path.join(workspace_path, excel_name)
        acc_fps_file = MIMEBase('application', "octet-stream")
        acc_fps_file.set_payload(open(acc_fps_file_path, 'rb').read())
        encoders.encode_base64(acc_fps_file)
        acc_fps_file.add_header('Content-Disposition', 'attachment; filename={}'.format(excel_name))

    log_file_path = os.path.join(workspace_path, "run.log")
    log_file = MIMEBase('application', "octet-stream")
    log_file.set_payload(open(log_file_path, 'rb').read())
    encoders.encode_base64(log_file)
    log_file.add_header('Content-Disposition', 'attachment; filename={}'.format("run.log"))

    run_steps_log_file_path = os.path.join(workspace_path, "run_steps.log")
    run_steps_log_file = MIMEBase('application', "octet-stream")
    run_steps_log_file.set_payload(open(run_steps_log_file_path, 'rb').read())
    encoders.encode_base64(run_steps_log_file)
    run_steps_log_file.add_header('Content-Disposition', 'attachment; filename={}'.format("run_steps.log"))

    message.attach(acc_fps_file)  # 添加邮件附件
    message.attach(log_file)      # 添加日志记录
    message.attach(run_steps_log_file)
    return message

def aisbench_benchmark_table(body_html, case_names, case_result):
    #表头
    body_html += '<body><p align="center" style="color:#1F497D;font-weight:bold;font-size:150%">' \
                            'AISBench Benchmark自动化冒烟测试报告_{}</p></br>'.format(time.strftime("%Y-%m-%d %H:%M:%S"))
    
    body_html += '</br><p align="center" style="color:#1F497D;font-weight:bold;font-size:120%">冒烟总览</p>'
    success_count, failed_cases = get_suc_case_count_and_failed_case_name(case_names, case_result)
    body_html += '<table width="100%" border="1" cellspacing="0" cellpadding="10" height="100%">'
    body_html += '<thead><tr>'
    body_html += '<th bgcolor="#CCFFFF" rowspan="1">成功用例数量</th>'
    body_html += '<th bgcolor="#CCFFFF" rowspan="1">失败用例名称</th>'
    body_html += "</tr></thead>"
    body_html += '<tbody><tr>'
    body_html += "<td align='center' bgcolor='#92D050'>{}</td>".format(success_count)
    failed_bg_color = '#FF0000' if failed_cases else ''
    body_html += "<td align='center' bgcolor='{}'>{}</td>".format(failed_bg_color, ', '.join(map(str, failed_cases)))
    body_html += '</tr></tbody></table>'
    
    body_html += '</br><p align="center" style="color:#1F497D;font-weight:bold;font-size:120%">用例详情</p>'

    body_html += '<table width="100%" border="1" cellspacing="0" cellpadding="10" height="100%">'
    body_html += '<th bgcolor="#CCFFFF" rowspan="1">用例名称</th>'
    body_html += '<th bgcolor="#CCFFFF" rowspan="1">结果</th>'
    max_deep = get_max_deep(case_result, case_names, match_key="case_name", target_key="metric_key")
    for i in range(max_deep):
        body_html += '<th bgcolor="#CCFFFF" rowspan="1">用例场景第 {} 层</th>'.format(i + 1)
    if failed_cases:
        body_html += '<th bgcolor="#CCFFFF" rowspan="1">执行失败详情</th>'
    body_html += "</td>"
    body_html += "</tr>"
    body_html += '<body>'
    # 表内容
    for case_name in case_names:
        result = find_value_in_same_level(("case_name", case_name), case_result)
        body_html += "<td align='center' >{}</td>".format(case_name)
        if not result.get("success"): # 失败用例下，颜色记载为红色
            body_html += "<td align='center' bgcolor='#FF0000'>{}</td>" \
                .format("Failed")
        else:
            body_html += "<td align='center' bgcolor='#92D050'>{}</td>" \
                .format("Success")
        metric_path = result.get("metric_key")
        for i in range(max_deep):
            body_html += "<td align='center' >{}</td>".format(metric_path[i] if i < len(metric_path) else None) # 用例场景
        if failed_cases:
            body_html += "<td align='left'>{}</td>".format(result.get("message").replace("\n", "</br>"))
        body_html += "</tr>"
    body_html += "</table>"
    body_html += "</body>"
    return body_html

def get_suc_case_count_and_failed_case_name(case_names, case_result):
    success_count = 0
    failed_cases = []
    for case_name in case_names:
        result = find_value_in_same_level(("case_name", case_name), case_result, target_key="success")
        if result:
            success_count += 1
        else:
            failed_cases.append(case_name)
    return success_count, failed_cases