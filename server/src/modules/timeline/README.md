# Timeline模块文件目录结构逻辑
core // 待进一步理清、拆分<br>
handler // 前端请求处理handler<br>
protocol // 前后端请求、响应、事件转换实现，JSON与数据结构间相互转换

## core/database
Timeline模块作为Insight的核心功能，其实现代码非常多，特别是数据库查询相关操作，因此需要实现代码拆分，非数据库直接操作，而是调用数据库操作进一步完成任务，可以放在包外面。
```
core/database                           // 父目录
-- TraceDatabaseDef.h                   // 数据结构定义
-- TraceDatabaseConst.h                 // 常量定义，如表名等复用字段

-- VirtualTraceDatabase.h               // Trace相关DB的接口头文件

-- DBTraceDatabase.h                    // DB场景下TraceDatabase的头文件
-- DBTraceDatabaseOperation.cpp         // DB场景下打开/关闭DB、增删表、创建表/索引等操作对应的实现
-- DBTraceDatabaseSliceChart.cpp        // DB场景下Timeline图表区域相关的查询接口，也包括Slice Detail/List和Find的实现
-- DBTraceDatabaseBarChart.cpp          // DB场景下Timeline图表区域相关的查询接口，也包括Slice Detail/List和Find的实现
-- DBTraceDatabaseAdvice.cpp            // DB场景下专家建议相关实现，也包括Summary、Communication页面分析相关的接口实现
-- DBTraceDatabaseStatistics.cpp        // DB场景下System View下统计分析的相关实现，包括Stat System View和Event System View

-- TextTraceDatabase.h                  // Text场景下TraceDatabase的头文件
-- TextTraceDatabaseOperation.cpp       // Text场景下打开/关闭DB、增删表、创建表/索引等操作对应的实现
-- TextTraceDatabaseChart.cpp           // Text场景下Timeline图表区域相关的查询接口，也包括Slice Detail/List和Find的实现
-- TextTraceDatabaseAdvice.cpp          // Text场景下专家建议相关实现，也包括Summary、Communication页面分析相关的接口实现
-- TextTraceDatabaseStatistics.cpp      // Text场景下System View下统计分析的相关实现，包括Stat System View和Event System View

-- TraceDatabaseHelper.h/cpp            // TraceDatabase下Text和DB下通用方法的实现
```

