import sqlite3
import glob

# 数据库文件列表
db_files = glob.glob(f"/home/h00812463/vllm_benchmark/activity_log_*.db")

# 要对比的表名列表
tables = ["api", "kernel", "communication", "api"]

results = {}

for db_file in db_files:
    conn = sqlite3.connect(db_file)
    cur = conn.cursor()
    results[db_file] = {}
    for table in tables:
        cur.execute(f"SELECT COUNT(*) FROM `{table}`")
        cnt = cur.fetchone()[0]
        results[db_file][table] = cnt
    cur.close()
    conn.close()

# 打印对比结果
print("表行数对比：")
header = ["Table"] + [db_file for db_file in db_files]
print("\t".join(header))

for table in tables:
    row = [table] + [str(results[db_file][table]) for db_file in db_files]
    print("\t".join(row))
