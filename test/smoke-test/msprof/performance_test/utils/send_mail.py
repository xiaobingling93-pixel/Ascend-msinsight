import os
import json
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.image import MIMEImage
from typing import Dict, Optional

class EmailSender:
    """邮件发送器"""
    
    def __init__(self, email_config:  Dict):
        """初始化邮件发送器。

        Args:
            email_config (Dict): 邮件配置，包含 SMTP 配置和收件人列表。
        """
        self.message_con = ""  # 邮件内容
        self.msg_obj = MIMEMultipart()  # 邮件对象
        self.smtp_config = email_config["smtp_config"]  # SMTP 配置
        self.recipients = email_config["recipients"]  # 收件人列表

    def _add_context(self, title: str, message: Optional[str] = None) -> "EmailSender":
        """添加邮件主题和内容。

        Args:
            title (str): 邮件主题。
            message (Optional[str]): 邮件内容，默认为 None。

        Returns:
            EmailSender: 返回实例对象以支持链式调用。
        """
        self.msg_obj["Subject"] = title
        if message:
            self.message_con += message
        return self

    def _add_file(self, file_path: str) -> "EmailSender":
        """添加附件（不支持中文路径）。

        Args:
            file_path (str): 附件文件路径。

        Returns:
            EmailSender: 返回实例对象以支持链式调用。
        """
        with open(file_path, "rb") as file:
            file_text = file.read()
        att_file = MIMEText(file_text, "base64", "utf-8")
        att_file["Content-Type"] = "application/octet-stream"
        att_file["Content-Disposition"] = f'attachment; filename="{os.path.basename(file_path)}"'
        self.msg_obj.attach(att_file)
        return self

    def _add_image(self, image_path: str, image_name: str) -> "EmailSender":
        """添加图片（不支持中文路径）。

        Args:
            image_path (str): 图片文件路径。
            image_name (str): 图片名称（用于 HTML 引用）。

        Returns:
            EmailSender: 返回实例对象以支持链式调用。
        """
        with open(image_path, "rb") as img_file:
            msg_image = MIMEImage(img_file.read())
        self.message_con += f'<p><img src="cid:{image_name}"></p>'
        msg_image.add_header("Content-ID", f"<{image_name}>")
        self.msg_obj.attach(msg_image)
        return self

    def _add_two_images(self, image_path1: str, image_name1: str, image_path2: str, image_name2: str) -> "EmailSender":
        """在一行中添加两张图片（不支持中文路径）。

        Args:
            image_path1 (str): 第一张图片路径。
            image_name1 (str): 第一张图片名称。
            image_path2 (str): 第二张图片路径。
            image_name2 (str): 第二张图片名称。

        Returns:
            EmailSender: 返回实例对象以支持链式调用。
        """
        with open(image_path1, "rb") as img_file1, open(image_path2, "rb") as img_file2:
            msg_image1 = MIMEImage(img_file1.read())
            msg_image2 = MIMEImage(img_file2.read())
        self.message_con += f"""
        <table>
            <tr>
                <td><img src="cid:{image_name1}"></td>
                <td><img src="cid:{image_name2}"></td>
            </tr>
        </table>"""
        msg_image1.add_header("Content-ID", f"<{image_name1}>")
        msg_image2.add_header("Content-ID", f"<{image_name2}>")
        self.msg_obj.attach(msg_image1)
        self.msg_obj.attach(msg_image2)
        return self

    def _add_html_msg(self, html_path: str) -> "EmailSender":
        """添加 HTML 格式内容（不支持中文路径）。

        Args:
            html_path (str): HTML 文件路径。

        Returns:
            EmailSender: 返回实例对象以支持链式调用。
        """
        with open(html_path, "r", encoding="utf-8") as html_file:
            self.message_con += html_file.read()
        return self

    def _send(self) -> bool:
        """发送邮件。

        Returns:
            bool: 发送结果，True 表示成功，False 表示失败。
        """
        try:
            txt = MIMEText(self.message_con, 'html', 'utf-8')
            self.msg_obj.attach(txt)
            self.msg_obj['from'] = self.smtp_config['sender']
            self.msg_obj['to'] = ', '.join(self.recipients)
            with smtplib.SMTP(self.smtp_config['server'], self.smtp_config['port']) as server:
                server.starttls()
                server.login(self.smtp_config['id'], self.smtp_config['password'])
                server.sendmail(self.msg_obj['from'], self.recipients, self.msg_obj.as_string())
                server.quit()
            return True
        except Exception as e:
            print(f"发送邮件失败: {str(e)}")
            return False

    def _generate_report_summarize(self, daily_state: dict, pkg_state: dict, result_dir):
        """生成报告摘要部分。

        Args:
            daily_state (Dict): 每日测试状态数据。
            pkg_state (Dict): 包版本信息。
        """
        summarize_html = f"""
        <html>
        <body>
        <h2>Profiler性能测试报告</h2>
        <p>测试时间: {daily_state['start_time']}</p>
        <p>minspore版本信息: {pkg_state.get('ms_package_date', 'None')}-{pkg_state.get('ms_package', 'None')}</p>
        <p>pta版本信息: {pkg_state.get('pta_package_date', 'None')}-{pkg_state.get('pta_package', 'None')}</p>

        <h3>测试汇总</h3>

        """
        # <table border="1">
        #     <tr>
        #         <th>指标</th>
        #         <th>值</th>
        #     </tr>
        #     <tr>
        #         <td><strong>总配置数</strong></td>
        #         <td>{daily_state['total_configs']}</td>
        #     </tr>
        #     <tr>
        #         <td><strong>成功数</strong></td>
        #         <td>{daily_state['success_configs']}</td>
        #     </tr>
        #     <tr>
        #         <td><strong>失败数</strong></td>
        #         <td>{daily_state['failed_configs']}</td>
        #     </tr>
        # </table>
        self.message_con += summarize_html

        json_file_name = "results.json"
        json_file = os.path.join(result_dir, json_file_name)

        if not os.path.isfile(json_file):
            raise RuntimeError("测试结果json文件不存在")
        
        with open(json_file, 'r', encoding='utf-8') as f:
            results = json.load(f)

        summarize_html = f"""
        <table border="1">
            <tr>
                <th>测试用例名</th>
                <th>用例类型</th>
                <th>是否成功</th>
                <th>PROF文件大小</th>
                <th>FRAMEWORK文件大小</th>
                <th>MsProf解析时间</th>
                <th>总解析时间</th>
                <th>平均每步时间</th>
                <th>开启Profiler平均每步时间</th>
                <th>膨胀率</th>
                <th>insight校验</th>
            </tr>
        """
        for key, result in results.items():
            config_title = key.split('.')[0]
            if "analysis" in config_title:
                summarize_html += self._generate_summary_analysis_config(config_title, result[-1])

        for key, result in results.items():
            config_title = key.split('.')[0]
            if "collection" in config_title:
                summarize_html += self._generate_summary_collection_config(config_title, result[-1])

        summarize_html += f"""
        </table>
        """
        self.message_con += summarize_html

    def _generate_summary_analysis_config(self, config_title, result: dict):
        if result['status'] == 'success':
            return f"""
                <tr>
                    <th>{config_title}</th>
                    <th>解析耗时</th>
                    <th>✅</th>
                    <th>{result['prof_size']:.2f} MB</th>
                    <th>{result['framework_size']:.2f} MB</th>
                    <th>{result['msprof_analysis_time']:.2f} 秒</th>
                    <th>{result['total_analysis_time']:.2f} 秒</th>
                    <th>-</th>
                    <th>-</th>
                    <th>-</th>
                    <th>{result.get('insight_result', "")}</th>
                </tr>
            """
        return f"""
            <tr>
                <th>{config_title}</th>
                <th>解析耗时</th>
                <th>❌</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
            </tr>
        """

    def _generate_summary_collection_config(self, config_title, result: dict):
        if result['status'] == 'success':
            return f"""
                <tr>
                    <th>{config_title}</th>
                    <th>采集膨胀</th>
                    <th>✅</th>
                    <th>-</th>
                    <th>-</th>
                    <th>-</th>
                    <th>-</th>
                    <th>{result['normal_avg_step_time']:.2f} 毫秒</th>
                    <th>{result['profiler_avg_step_time']:.2f} 毫秒</th>
                    <th>{result['inflation_ratio']:.2f} %</th>
                    <th>-</th>
                </tr>
            """
        return f"""
            <tr>
                <th>{config_title}</th>
                <th>采集膨胀</th>
                <th>❌</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
                <th>-</th>
            </tr>
        """

    def _generate_report_analysis_config(self, plot_dir, config_title, result: dict):
        """生成单个配置的解析测试结果。

       Args:
           plot_dir (str): 图表目录。
           config_title (str): 配置标题。
           result (Dict): 测试结果数据。
       """
        result_title = f"""<h3> {config_title}的结果：</h3>"""
        self.message_con += result_title
        # if result['status'] == 'success':
        #     result_table = f"""
        #     <p>配置文件: {result['config_file']}</p>
        #     <ul>
        #         <li>PROF文件的大小: {result['prof_size']:.2f} MB</li>
        #         <li>FRAMEWORK文件的大小: {result['framework_size']:.2f} MB</li>
        #         <li>MsProf解析时间: {result['msprof_analysis_time']:.2f} 秒</li>
        #         <li>总解析时间: {result['total_analysis_time']:.2f} 秒</li>
        #     </ul>
        #     """
        # else:
        #     result_table = f"""
        #     <p>配置文件: {result['config_file']}</p>
        #     <p style="color: red;">测试失败: {result['error']}</p>
        #     """
        # self.message_con += result_table
        config_pic_name = f"{config_title}.png"
        config_pic = os.path.join(plot_dir, config_pic_name)
        if os.path.exists(config_pic):
            self._add_image(config_pic, config_pic_name)

    def _generate_report_collection_config(self, plot_dir, config_title):
        """生成单个配置的性能膨胀结果。

        Args:
            plot_dir (str): 图表目录。
            config_title (str): 配置标题。
        """
        result_title = f"""<h3> {config_title}的结果：</h3>"""
        self.message_con += result_title
        config_time_pic_name = f"{config_title}_time.png"
        config_ratio_pic_name = f"{config_title}_ratio.png"
        config_time_pic = os.path.join(plot_dir, config_time_pic_name)
        config_ratio_pic = os.path.join(plot_dir, config_ratio_pic_name)
        
        if os.path.exists(config_time_pic) and os.path.exists(config_ratio_pic):
            self._add_two_images(config_time_pic, config_time_pic_name, config_ratio_pic, config_ratio_pic_name)

    def _generate_report_one_config(self, result_dir):
        """生成单个配置的报告内容。

        Args:
            result_dir (str): 测试结果目录。
        """
        json_file_name = "results.json"
        json_file = os.path.join(result_dir, json_file_name)

        if not os.path.isfile(json_file):
            raise RuntimeError("测试结果json文件不存在")
        
        with open(json_file, 'r', encoding='utf-8') as f:
            results = json.load(f)

        analysis_title = f"""<h3> 性能解析测试结果图表：</h3>"""
        self.message_con += analysis_title
        analysis_plot_dir = f"{result_dir}/plot/analysis"
        for key, result in results.items():
            config_title = key.split('.')[0]
            if "analysis" in config_title:
                self._generate_report_analysis_config(analysis_plot_dir, config_title, result[-1])

        collection_title = f"""<h3> 性能膨胀结果：</h3>"""
        self.message_con += collection_title
        collection_plot_dir = f"{result_dir}/plot/collection"
        for key, config in results.items():
            config_title = key.split('.')[0]
            if "collection" in config_title:
                self._generate_report_collection_config(collection_plot_dir, config_title)

    def _generate_report_context(self, daily_state: dict, result_dir, pkg_state: dict):
        """生成 HTML 报告内容。

        Args:
            daily_state (Dict): 每日测试状态数据。
            result_dir (str): 测试结果目录。
            pkg_state (Dict): 包版本信息。
        """

        if not os.path.exists(result_dir):
            raise RuntimeError("测试结果目录不存在")

        # 生成汇总报告
        self._generate_report_summarize(daily_state, pkg_state, result_dir)

        # 生成每一个config的图表内容
        self._generate_report_one_config(result_dir)

        print("message_con", self.message_con)


    def  send_performance_report(self, daily_state: dict, result_dir, pkg_state: dict):
        """发送性能测试报告邮件。

        Args:
            daily_state (Dict): 每日测试状态数据。
            result_dir (str): 测试结果目录。
            pkg_state (Dict): 包版本信息。
        """

        self._generate_report_context(daily_state, result_dir, pkg_state)
        title_content = "Profiler性能测试报告"
        context_tail = """
            </body>
            </html>
        """
        self._add_context(title_content, context_tail)
        self._send()
