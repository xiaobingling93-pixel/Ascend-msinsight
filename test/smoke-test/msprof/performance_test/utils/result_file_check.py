import os

# 返回某个目录下所有文件的总大小（MB）
def directory_size(directory):
    total_size = 0
    for root, dirs, files in os.walk(directory):
        for f in files:
            file_path = os.path.join(root, f)
            total_size += os.path.getsize(file_path)
    return total_size / (1024 * 1024)
