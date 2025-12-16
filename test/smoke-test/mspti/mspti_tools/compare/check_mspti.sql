-- kernel中有api中没有的
SELECT kernel.*, api.*
FROM kernel
         LEFT JOIN api ON kernel.correlationId = api.correlationId
WHERE api.correlationId IS NULL;

-- api中有,kernel中没有的
SELECT kernel.*, api.*
FROM api
         LEFT JOIN kernel ON kernel.correlationId = api.correlationId
WHERE kernel.correlationId IS NULL;

-- aicpu问题
SELECT kernel.*, api.*
FROM api
         LEFT JOIN kernel ON kernel.correlationId = api.correlationId
WHERE kernel.type = 'KERNEL_AICPU';

-- communication有, api没有
SELECT communication.name, api.name
FROM communication
         LEFT JOIN api ON communication.correlationId = api.correlationId
where communication.name != api.name;

-- api中有, op_summary中没有的
SELECT DISTINCT(api.name)
FROM api
WHERE NOT EXISTS (
    SELECT 1
    FROM op_summary_slice_0_20250815013902
    WHERE op_summary_slice_0_20250815013902."Op Name" = api.name
);

-- op_summary中有, api中没有的
SELECT DISTINCT(op_summary_slice_0_20250815013902."Op Name")
FROM op_summary_slice_0_20250815013902
WHERE NOT EXISTS (
    SELECT 1
    FROM api
    WHERE api.name = op_summary_slice_0_20250815013902."Op Name"
) and op_summary_slice_0_20250815013902."Op Name" not like 'hcom_%';

-- 对比数据量
SELECT count(name) FROM api;
SELECT count("Op Name") FROM op_summary_slice_0_20250815013902 WHERE op_summary_slice_0_20250815013902."Op Name" not like '%hcom_%';

-- 对比各个api数据数量
SELECT
    COALESCE(a.name, b.`Op Name`) AS name,
    COALESCE(a.cnt, 0) AS cnt_api,
    COALESCE(b.cnt, 0) AS cnt_op_summary,
    COALESCE(a.cnt, 0) - COALESCE(b.cnt, 0) AS diff
FROM (
         SELECT name, COUNT(*) AS cnt
         FROM api
         GROUP BY name
     ) a
         LEFT JOIN (
    SELECT `Op Name`, COUNT(*) AS cnt
    FROM op_summary_slice_0_20250815013902
    WHERE op_summary_slice_0_20250815013902."Op Name" not like '%hcom_%'
    GROUP BY `Op Name`
) b ON a.name = b.`Op Name`
UNION ALL
SELECT
    COALESCE(a.name, b.`Op Name`) AS name,
    COALESCE(a.cnt, 0) AS cnt_api,
    COALESCE(b.cnt, 0) AS cnt_op_summary,
    COALESCE(a.cnt, 0) - COALESCE(b.cnt, 0) AS diff
FROM (
         SELECT `Op Name`, COUNT(*) AS cnt
         FROM op_summary_slice_0_20250815013902
         WHERE op_summary_slice_0_20250815013902."Op Name" not like '%hcom_%'
         GROUP BY `Op Name`
     ) b
         LEFT JOIN (
    SELECT name, COUNT(*) AS cnt
    FROM api
    GROUP BY name
) a ON a.name = b.`Op Name`
WHERE a.name IS NULL;