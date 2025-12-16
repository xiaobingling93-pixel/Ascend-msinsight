import os
import logging
from datetime import datetime
from pathlib import Path
from colorama import init, Fore, Style

# 初始化colorama，支持Windows
init()


class ColorFormatter(logging.Formatter):
    """带颜色的日志格式化器"""
    
    COLORS = {
        'INFO': Fore.GREEN,
        'WARNING': Fore.YELLOW,
        'ERROR': Fore.RED,
        'DEBUG': Fore.BLUE,
        'CRITICAL': Fore.RED + Style.BRIGHT,
    }

    def format(self, record):
        original_levelname = record.levelname
        if record.levelname in self.COLORS:
            record.levelname = (f"{self.COLORS[record.levelname]}{record.levelname}"
                              f"{Style.RESET_ALL}")
            record.msg = f"{self.COLORS[original_levelname]}{record.msg}{Style.RESET_ALL}"
        return super().format(record)


class TestLogger:
    """测试日志记录器"""
    
    def __init__(self, log_dir: str = "output/logs"):
        self.log_dir = log_dir
        self._setup_logger()
    
    def _setup_logger(self):
        """设置日志记录器"""
        # 创建日志目录
        Path(self.log_dir).mkdir(parents=True, exist_ok=True)
        
        # 生成日志文件名
        timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        log_file = os.path.join(self.log_dir, f"{timestamp}.log")
        
        # 普通格式（用于文件）
        file_formatter = logging.Formatter(
            '%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        
        # 带颜色的格式（用于控制台）
        console_formatter = ColorFormatter(
            '%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        
        # 配置文件处理器
        file_handler = logging.FileHandler(log_file, encoding='utf-8')
        file_handler.setFormatter(file_formatter)
        
        # 配置控制台处理器
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(console_formatter)
        
        # 配置根日志记录器
        self.logger = logging.getLogger('profiler_test')
        self.logger.setLevel(logging.INFO)
        self.logger.addHandler(file_handler)
        self.logger.addHandler(console_handler)
    
    def info(self, msg: str):
        """记录信息级别的日志"""
        self.logger.info(msg)
    
    def error(self, msg: str):
        """记录错误级别的日志"""
        self.logger.error(msg)
    
    def warning(self, msg: str):
        """记录警告级别的日志"""
        self.logger.warning(msg)
    

# 创建全局日志记录器实例
logger = TestLogger() 
