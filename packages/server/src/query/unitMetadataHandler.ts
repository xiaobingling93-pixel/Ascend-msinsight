import { tableMap } from '../database/tableManager';
import { Table } from '../database/table';
import { ExtremumTimestamp, InsightMetaData, metadataDto, ProcessMetaData, ThreadMetaData } from './data';

export const unitMetadataHandler = async (req: { rankId: number }): Promise<Record<string, unknown>> => {
    if (req.rankId === undefined || !tableMap.has(req.rankId)) {
        console.error('rank id is invalid.');
        return {};
    }
    return queryUnitsMetadata(req.rankId);
};

/**
 * 获取某一card metadata
 *
 * @param rankId 即cardId
 */
export async function queryUnitsMetadata(rankId: number): Promise<any> {
    const table = tableMap.get(rankId) as Table;
    const rows = await table.selectUnitsMetadata() as metadataDto[];
    const insightMetaData: InsightMetaData<any> = { type: 'card', metadata: { cardId: rankId } };
    if (rows.length === 0) {
        console.error('not find metadata, rank id: ', rankId);
        return { insightMetaData, extremumTimestamp: { maxTimestamp: 0, minTimestamp: 0 } };
    }
    let tmpPid = '';
    insightMetaData.children = [];
    rows.forEach(row => {
        const threadMetaData: ThreadMetaData = {
            cardId: rankId,
            processId: row.pid,
            threadId: row.tid,
            threadName: row.threadName,
            maxDepth: row.maxDepth,
        };
        const threadInsightMetaData: InsightMetaData<any> = { type: 'thread', metadata: threadMetaData };
        if (tmpPid !== row.pid) {
            const processMetaData: ProcessMetaData = { processId: row.pid, processName: row.processName };
            const processInsightMetaData: InsightMetaData<any> = {
                type: 'process',
                metadata: processMetaData,
                children: [],
            };
            processInsightMetaData.children?.push(threadInsightMetaData);
            insightMetaData.children?.push(processInsightMetaData);
            tmpPid = row.pid;
        } else {
            (insightMetaData.children?.[insightMetaData.children?.length - 1] as InsightMetaData<any>).children?.push(threadInsightMetaData);
        }
    });
    const extremumTimestamp = await table.selectExtremumTimestamp() as ExtremumTimestamp;
    return { insightMetaData, extremumTimestamp };
}
