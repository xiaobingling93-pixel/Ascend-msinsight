import re
import sqlite3
import gzip
import os
import sys

# 正则匹配模式
api_pattern = re.compile(r'name:\s*([^,]+).*?correlationId:\s*(\d+)')
kernel_pattern = re.compile(r'name:\s*([^,]+).*?correlationId:\s*(\d+)')
comm_pattern = re.compile(r'name:\s*([^,]+).*?correlationId:\s*(\d+)')

BATCH_SIZE = 5000  # 每次批量插入的记录数

def init_db(db_path="correlation_check.db"):
    conn = sqlite3.connect(db_path)
    # PRAGMA 性能优化（分析用）
    conn.execute("PRAGMA synchronous = OFF")
    conn.execute("PRAGMA journal_mode = MEMORY")
    conn.execute("PRAGMA temp_store = MEMORY")

    conn.execute('''
        CREATE TABLE IF NOT EXISTS correlation_check (
            log_file TEXT,
            correlation_id INTEGER,
            type TEXT,
            name TEXT,
            has_match INTEGER
        )
    ''')
    conn.commit()
    return conn

def detect_gzip(file_path):
    """判断是否为 gzip 文件"""
    with open(file_path, 'rb') as fb:
        return fb.read(2) == b'\x1f\x8b'

def extract_and_check(file_path):
    """流式读取日志并校验 correlationId"""
    is_gzip = detect_gzip(file_path)
    open_func = gzip.open if is_gzip else open

    api_list, kernel_list, comm_list = [], [], []
    api_ids, kernel_ids, comm_ids = set(), set(), set()

    with open_func(file_path, 'rt', encoding='utf-8', errors='ignore') as f:
        for line in f:
            if '[API]' in line:
                m = api_pattern.search(line)
                if m:
                    name, cid = m.groups()
                    cid = int(cid)
                    api_list.append((cid, name.strip()))
                    api_ids.add(cid)
            elif '[KERNEL]' in line:
                m = kernel_pattern.search(line)
                if m:
                    name, cid = m.groups()
                    cid = int(cid)
                    kernel_list.append((cid, name.strip()))
                    kernel_ids.add(cid)
            elif '[COMMUNICATION]' in line:
                m = comm_pattern.search(line)
                if m:
                    name, cid = m.groups()
                    cid = int(cid)
                    comm_list.append((cid, name.strip()))
                    comm_ids.add(cid)

    # 检查 API 是否都有对应的 Kernel 或 Communication
    api_missing = [cid for cid in api_ids if cid not in kernel_ids and cid not in comm_ids]
    # 检查 Kernel 是否都有对应的 API
    kernel_missing = [cid for cid in kernel_ids if cid not in api_ids]
    # 检查 Communication 是否都有对应的 API
    comm_missing = [cid for cid in comm_ids if cid not in api_ids]

    return api_list, kernel_list, comm_list, api_missing, kernel_missing, comm_missing

def save_to_db(conn, file_path, api_list, kernel_list, comm_list, api_missing, kernel_missing, comm_missing):
    """批量插入数据库"""
    cursor = conn.cursor()
    batch = []

    def add_records(records, rtype, missing_set):
        for cid, name in records:
            batch.append((file_path, cid, rtype, name, 0 if cid in missing_set else 1))
            if len(batch) >= BATCH_SIZE:
                cursor.executemany('''
                    INSERT INTO correlation_check (log_file, correlation_id, type, name, has_match)
                    VALUES (?, ?, ?, ?, ?)
                ''', batch)
                batch.clear()

    add_records(api_list, "API", set(api_missing))
    add_records(kernel_list, "KERNEL", set(kernel_missing))
    add_records(comm_list, "COMMUNICATION", set(comm_missing))

    # 插入剩余记录
    if batch:
        cursor.executemany('''
            INSERT INTO correlation_check (log_file, correlation_id, type, name, has_match)
            VALUES (?, ?, ?, ?, ?)
        ''', batch)

    conn.commit()

def check_file(conn, file_path):
    api_list, kernel_list, comm_list, api_missing, kernel_missing, comm_missing = extract_and_check(file_path)
    save_to_db(conn, file_path, api_list, kernel_list, comm_list, api_missing, kernel_missing, comm_missing)

def main():
    if len(sys.argv) < 2:
        print(f"用法: python {sys.argv[0]} log1 [log2 ...]")
        sys.exit(1)

    conn = init_db()

    for file in sys.argv[1:]:
        print(f"处理文件: {file}")
        check_file(conn, file)

    conn.close()
    print("处理完成")

if __name__ == "__main__":
    main()
