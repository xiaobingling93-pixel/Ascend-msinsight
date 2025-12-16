import getpass
import argparse
from cryptography.fernet import Fernet
import base64
import os
from pathlib import Path

# 获取当前脚本的绝对路径
BASE_PATH = Path(__file__)
# 拼接目标路径 - 推荐方式（使用parent属性）
CREDENTIALS_DIR = BASE_PATH.parent.parent / "resource" / "credentials"

CREDENTIALS_ENC_FILE_PATH = CREDENTIALS_DIR / "credentials.enc"

SECRET_KEY_FILE_PATH = CREDENTIALS_DIR / "secret.key"


def generate_encrypted_credentials(username, password):
    """
    生成加密的凭据文件和密钥文件
    """
    global CREDENTIALS_ENC_FILE_PATH, SECRET_KEY_FILE_PATH, CREDENTIALS_DIR
    # 生成加密密钥
    key = Fernet.generate_key()
    cipher = Fernet(key)
    
    # 创建凭据字符串
    credentials = f"{username}:{password}"
    
    try:
        if not CREDENTIALS_DIR.exists():
            CREDENTIALS_DIR.mkdir(parents=True, exist_ok=True)
            print(f"已创建目录: {CREDENTIALS_DIR}")
        # 加密凭据并写入文件
        with open(CREDENTIALS_ENC_FILE_PATH, "wb") as enc_file:
            encrypted_data = cipher.encrypt(credentials.encode())
            enc_file.write(encrypted_data)
        
        # 安全地保存密钥
        with open(SECRET_KEY_FILE_PATH, "wb") as key_file:
            key_file.write(key)
        
        print("="*50)
        print("✅ 凭据文件创建成功！")
        print(f"✅ 路径: {CREDENTIALS_DIR}")
        print(f"• 加密凭据文件: credentials.enc")
        print(f"• 解密密钥文件: secret.key")
        print("\n⚠️ 重要安全提示:")
        print("1. 将 'secret.key' 添加到 .gitignore 文件")
        print("2. 绝对不要将 'secret.key' 提交到代码仓库")
        print("3. 安全存储密钥文件（如密码管理器）")
        print("="*50)
        return True
    
    except Exception as e:
        print(f"❌ 文件创建失败: {e}")
        return False

def main():
    global CREDENTIALS_ENC_FILE_PATH, SECRET_KEY_FILE_PATH
    
    print("="*50)
    print('🔐 安全凭据加密工具 - 创建加密的SMTP凭据文件')
    print(">"*50)
    # 检查文件是否已存在
    if os.path.exists(CREDENTIALS_ENC_FILE_PATH) or os.path.exists(SECRET_KEY_FILE_PATH):
        print("="*50)
        response = input("⚠️ 凭据文件已存在! 覆盖? (y/N): ").lower()
        if response != 'y':
            print("操作已取消")
            exit(0)
    
    # 读取用户名（正常显示）
    username = str(input("输入SMTP用户名（即工号）: "))
    while not username.strip():
        username = str(input("SMTP用户名不能为空，请重新输入: "))
    # 读取密码（不显示）
    password = str(getpass.getpass("输入SMTP密码（不明文显示，请确保输入正确）: "))
    while not password.strip():
        password = str(getpass.getpass("SMTP密码不能为空，请重新输入"))

    # 生成加密文件
    generate_encrypted_credentials(username, password)

if __name__ == "__main__":
    main()