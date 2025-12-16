import os
import hashlib


class WorkspaceChange:
    def __init__(self, workspace):
        self.workspace = workspace
        self.ori_workspace = os.getcwd()

    def __enter__(self):
        self.ori_workspace = os.getcwd()
        os.chdir(self.workspace)

    def __exit__(self, exc_type, exc_val, exc_tb):
        os.chdir(self.ori_workspace)


def count_success_status(data_dict):
    """
    统计字典中success为True和False的数量

    参数:
        data_dict: 包含多个子字典的字典，每个子字典有'success'键

    返回:
        一个元组 (true_count, false_count, total_count)，分别表示success为True和False的数量和总数量
    """
    true_count = 0
    false_count = 0

    # 遍历字典中的每个子项
    for item in data_dict.values():
        # 检查子项是否包含'success'键
        if 'success' in item:
            if item['success']:
                true_count += 1
            else:
                false_count += 1

    return true_count, false_count, true_count + false_count


def generate_key(password, length):
    """根据密码生成指定长度的密钥"""
    hash_obj = hashlib.sha256(password.encode('utf-8'))
    hash_bytes = hash_obj.digest()

    key = []
    for i in range(length):
        key_val = hash_bytes[i % len(hash_bytes)] % 62  # 扩大范围以支持更多字符转换
        key.append(key_val)
    return key


def decrypt(text, password):
    """解密函数，将加密后的字符还原为原始数字"""
    if not text:
        return ""

    # 先进行混淆还原
    confusion_recovery = []
    for i, c in enumerate(text):
        recovered_char = chr(ord(c) ^ (i % 10 + 1))
        confusion_recovery.append(recovered_char)
    encrypted_str = ''.join(confusion_recovery)

    key = generate_key(password, len(encrypted_str))
    decrypted = []

    for i, char in enumerate(encrypted_str):
        if char.isalpha():
            # 判断是原始字母还是由数字转换来的字母
            key_val = key[i]
            is_from_number = False

            # 检查是否可能是由数字转换来的字母
            if key_val % 3 == 0 and 'a' <= char.lower() <= 'j':
                # 可能是数字映射的小写字母
                num = ord(char.lower()) - ord('a')
                decrypted.append(str(num))
                is_from_number = True
            elif key_val % 3 == 1 and 'A' <= char <= 'J':
                # 可能是数字映射的大写字母
                num = ord(char) - ord('A')
                decrypted.append(str(num))
                is_from_number = True

            # 如果不是数字转换来的，则按字母解密
            if not is_from_number:
                is_upper = char.isupper()
                shift = key_val % 26
                current_pos = ord(char.lower()) - ord('a')
                original_pos = (current_pos - shift) % 26
                original_char = chr(original_pos + ord('a'))
                if is_upper:
                    original_char = original_char.upper()
                decrypted.append(original_char)
        elif char in "!@#$%^&*()~":
            # 处理特殊字符，可能是由数字转换来的
            key_val = key[i]
            if key_val % 3 == 2:
                special_chars = "!@#$%^&*()~"
                num = special_chars.index(char)
                decrypted.append(str(num))
            else:
                # 不是数字转换来的特殊字符，直接保留
                decrypted.append(char)
        else:
            # 其他字符保持不变
            decrypted.append(char)

    return ''.join(decrypted)
