## 检测脚本
### compare_correlationid.py

用于检测pti打印的log中api，kernel，communication之间的数据关系，主要校验correlationId

### compare_profiler.py

用于检测profiler中的数据量与pti中的关系，主要校验kernel和communication

### check_mspti.sql

检测mspti中api和communication，api与kernel中的数据准确性，以及对比api与op_summary中的数据准确性