import datetime
from functools import cached_property


class FormatDate:
    """日期格式化工具"""
    def __init__(self, date: datetime.date = None):
        self.date = date or datetime.date.today()

    @cached_property
    def today(self):
        return "".join(str(self.date).split("-"))

    @cached_property
    def month(self):
        return self.today[:6]

    @staticmethod
    def get_recent_days(days: int = 3):
        """获取最近几天的日期"""
        today = datetime.date.today()
        return [today - datetime.timedelta(days=i) for i in range(days)]
