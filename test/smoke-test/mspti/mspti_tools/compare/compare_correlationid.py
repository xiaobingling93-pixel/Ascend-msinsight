import re
import sys
from pathlib import Path

# 正则表达式定义
api_pattern = re.compile(r'\[API\].*?correlationId:\s*(\d+)')
kernel_pattern = re.compile(r'\[KERNEL\].*?correlationId:\s*(\d+)')
comm_pattern = re.compile(r'\[COMMUNICATION\].*?correlationId:\s*(\d+)')

def extract_correlation_ids(text):
    api_ids = set(map(int, api_pattern.findall(text)))
    kernel_ids = set(map(int, kernel_pattern.findall(text)))
    comm_ids = set(map(int, comm_pattern.findall(text)))
    return api_ids, kernel_ids, comm_ids

def check_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    api_ids, kernel_ids, comm_ids = extract_correlation_ids(content)

    kernel_comm_ids = kernel_ids.union(comm_ids)

    api_missing = api_ids - kernel_comm_ids
    kernel_comm_missing = kernel_comm_ids - api_ids

    print(f"🔍 Checking file: {file_path}")
    print(f"  Total APIs: {len(api_ids)}")
    print(f"  Total KERNELs: {len(kernel_ids)}")
    print(f"  Total COMMUNICATIONs: {len(comm_ids)}")

    if api_missing:
        print(f"❌ API correlationId(s) with no matching KERNEL/COMM: {sorted(api_missing)}")
    else:
        print("✅ All API correlationId(s) matched with KERNEL or COMMUNICATION")

    if kernel_comm_missing:
        print(f"❌ KERNEL/COMM correlationId(s) with no matching API: {sorted(kernel_comm_missing)}")
    else:
        print("✅ All KERNEL/COMM correlationId(s) matched with API")

    print("-" * 60)

def main():
    if len(sys.argv) < 2:
        print("Usage: python check_correlation.py <log_file1> <log_file2> ...")
        return

    for file in sys.argv[1:]:
        if Path(file).is_file():
            check_file(file)
        else:
            print(f"⚠️  File not found or not a regular file: {file}")

if __name__ == "__main__":
    main()
