import gzip
import sqlite3
import re
import os
from multiprocessing import Pool, cpu_count

# 匹配各类字段的正则
api_pattern = re.compile(
    r"\[API\]\s+name:\s*(?P<name>.*?),\s*start:\s*(?P<start>\d+),\s*end:\s*(?P<end>\d+),\s*processId:\s*(?P<processId>\d+),\s*threadId:\s*(?P<threadId>\d+),\s*correlationId:\s*(?P<correlationId>\d+)"
)
kernel_pattern = re.compile(
    r"\[KERNEL\].*?type: (?P<type>.*?), name: (?P<name>.*?), start: (?P<start>\d+), end: (?P<end>\d+), deviceId: (?P<deviceId>\d+), streamId: (?P<streamId>\d+), correlationId: (?P<correlationId>\d+)"
)
comm_pattern = re.compile(
    r"\[COMMUNICATION\].*?dataType: (?P<dataType>.*?), count: (?P<count>\d+), start: (?P<start>\d+), end: (?P<end>\d+), deviceId: (?P<deviceId>\d+), streamId: (?P<streamId>\d+), correlationId: (?P<correlationId>\d+), algType (?P<algType>.*?), name: (?P<name>.*?), commName: (?P<commName>.*)"
)

def is_gzip(file_path):
    with open(file_path, "rb") as f:
        return f.read(2) == b'\x1f\x8b'

def parse_log_file(file_path, db_path):
    conn = sqlite3.connect(db_path)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS correlation_check (
            file TEXT,
            category TEXT,
            name TEXT,
            correlationId INTEGER,
            start INTEGER,
            end INTEGER,
            processId INTEGER,
            threadId INTEGER,
            deviceId INTEGER,
            streamId INTEGER,
            count INTEGER,
            algType TEXT,
            commName TEXT,
            missing INTEGER
        )
    """)
    cursor = conn.cursor()

    api_map, kernel_map, comm_map = {}, {}, {}

    open_func = gzip.open if is_gzip(file_path) else open

    with open_func(file_path, "rt", errors="ignore") as f:
        for line in f:
            if "[API]" in line:
                m = api_pattern.search(line)
                if m:
                    d = m.groupdict()
                    cid = int(d["correlationId"])
                    api_map[cid] = d
            elif "[KERNEL]" in line:
                m = kernel_pattern.search(line)
                if m:
                    d = m.groupdict()
                    cid = int(d["correlationId"])
                    kernel_map[cid] = d
            elif "[COMMUNICATION]" in line:
                m = comm_pattern.search(line)
                if m:
                    d = m.groupdict()
                    cid = int(d["correlationId"])
                    comm_map[cid] = d

    # 生成插入列表，标记 missing
    insert_list = []

    for cid, d in api_map.items():
        missing = 1 if cid not in kernel_map and cid not in comm_map else 0
        insert_list.append((
            file_path,
            "API",
            d.get("name"),
            int(d.get("correlationId")),
            int(d.get("start")),
            int(d.get("end")),
            int(d.get("processId")),
            int(d.get("threadId")),
            None, None, None, None, None,
            missing
        ))

    for cid, d in kernel_map.items():
        missing = 1 if cid not in api_map else 0
        insert_list.append((
            file_path,
            "KERNEL",
            d.get("name"),
            int(d.get("correlationId")),
            int(d.get("start")),
            int(d.get("end")),
            None, None,
            int(d.get("deviceId")),
            int(d.get("streamId")),
            None, d.get("type"), None,
            missing
        ))

    for cid, d in comm_map.items():
        missing = 1 if cid not in api_map else 0
        insert_list.append((
            file_path,
            "COMMUNICATION",
            d.get("name"),
            int(d.get("correlationId")),
            int(d.get("start")),
            int(d.get("end")),
            None, None,
            int(d.get("deviceId")),
            int(d.get("streamId")),
            int(d.get("count")),
            d.get("algType"),
            d.get("commName"),
            missing
        ))

    cursor.executemany(
        "INSERT INTO correlation_check VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
        insert_list
    )
    conn.commit()
    conn.close()

def worker(args):
    file_path, idx = args
    db_path = f"correlation_check_{idx}.db"
    parse_log_file(file_path, db_path)
    return db_path

def main(files):
    with Pool(cpu_count()) as pool:
        db_files = pool.map(worker, [(f, i) for i, f in enumerate(files)])
    print("生成的数据库文件：", db_files)
    print("你可以后续用 ATTACH 或导出 CSV 合并分析")

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print(f"用法: {sys.argv[0]} file1 [file2 ...]")
        sys.exit(1)
    main(sys.argv[1:])
