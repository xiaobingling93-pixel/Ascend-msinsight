import { detail, DetailDescriptor, linkData, LinkDataDesc } from '../../entity/insight';
import { isEmpty } from 'lodash';
import styled from '@emotion/styled';
import React from 'react';
import { action, runInAction } from 'mobx';
import { AscendMultiSliceList, ThreadMetaData, ThreadTrace } from '../../entity/data';
import { Session } from '../../entity/session';
import { getSliceTimeDisplay } from './AscendUnit';
import { getTimestamp } from '../../utils/humanReadable';
import { colorPalette } from './utils';
import { hashToNumber } from '../../utils/colorUtils';
export const slicesListDetail = detail({
    name: 'Slices List',
    columns: [
        ['Name', data => `${isEmpty(data.title) ? 'null' : data.title}`, 'max-content', 'scroll'],
        ['Wall Duration', data => getSliceTimeDisplay(data.wallDuration), 180],
        ['Self Time', data => getSliceTimeDisplay(data.selfTime), 180],
        ['Average Wall Duration', data => getSliceTimeDisplay(data.avgWallDuration), 180],
        ['Occurrences', data => `${data.occurrences ?? 0}`, 180],
    ],
    actions: [
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => a.title?.localeCompare(b?.title ?? '') ?? 0 },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => (a.wallDuration ?? 0) - (b.wallDuration ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => (a.selfTime ?? 0) - (b.selfTime ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => (a.avgWallDuration ?? 0) - (b.avgWallDuration ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => (a.occurrences ?? 0) - (b.occurrences ?? 0) },
    ],
    fetchData: async (session: Session, metadata: ThreadMetaData) => {
        let startTime = session.selectedRange?.[0] ?? 0;
        startTime = startTime < 0 ? 0 : startTime;
        let endTime = session.selectedRange?.[1] ?? 0;
        endTime = endTime < 0 ? 0 : endTime;
        const timestampOffset = metadata.cardId !== undefined
            ? (session?.unitsConfig.offsetConfig.timestampOffset as Record<string, number>)?.[metadata.cardId] ?? 0
            : 0;
        const params = {
            rankId: metadata.cardId,
            tid: metadata.threadId,
            pid: metadata.processId,
            startTime: Math.floor(startTime + timestampOffset),
            endTime: Math.ceil(endTime + timestampOffset),
        };
        const raw = await window.request(metadata.dataSource, { command: 'unit/threads', params });
        const res = raw.data;

        let totalWallDuration = 0;
        let totalSelfTime = 0;
        let totalOccurrences = 0;

        res.forEach((element: AscendMultiSliceList) => {
            totalWallDuration += element.wallDuration ?? 0;
            totalSelfTime += element.selfTime ?? 0;
            totalOccurrences += element.occurrences ?? 0;
            element.rankId = metadata.cardId;
            element.tid = metadata.threadId;
            element.pid = metadata.processId;
            element.startTime = Math.floor(startTime + timestampOffset);
            element.endTime = Math.ceil(endTime + timestampOffset);
        });

        res.push({
            title: 'Totals',
            wallDuration: totalWallDuration,
            selfTime: totalSelfTime,
            avgWallDuration: totalOccurrences === 0 ? totalWallDuration : totalWallDuration / totalOccurrences,
            occurrences: totalOccurrences,
        });

        return res;
    },
    mouseEnterCallback: ({ session, row }) => {
        const selectedRangeData = session.selectedRangeData;
        const name = row.title;
        const hoveredData = selectedRangeData?.find((item) => item.name === name);
        runInAction(() => {
            session.sharedState.threadTrace = hoveredData;
        });
    },
    mouseLeaveCallback: ({ session, row }) => {
        const name = row.title;
        const hoveredData = session.sharedState.threadTrace as ThreadTrace | undefined;
        if (hoveredData && hoveredData.name === name) {
            runInAction(() => {
                session.sharedState.threadTrace = undefined;
            });
        }
    },
    clickCallback: async ({ row, session }): Promise<void> => {
        const data = {
            rankId: row.rankId,
            tid: row.tid,
            pid: row.pid,
            startTime: row.startTime,
            endTime: row.endTime,
            name: row.title,
            wallDuration: row.wallDuration,
        };
        runInAction(() => {
            session.selectedMultiSlice = JSON.stringify(data);
        });
    },
}) as DetailDescriptor<unknown>;

const Link = styled.span`
    &:hover {
        border-bottom: 1px solid white;
    }
    border-bottom: 2px solid rgb(140, 140, 140);
`;

export const generateFlowParam = function(metadata: ThreadMetaData, startTime: number, pid?: string, tid?: string):
{ rankId: string; tid: string; pid: string; startTime: number } {
    return {
        rankId: metadata.cardId ?? '',
        tid: tid ?? (metadata.threadId ?? ''),
        pid: pid ?? (metadata.processId ?? ''),
        startTime,
    };
};

const generateFlowData = function (data: any): any {
    return {
        startTime: data.timestamp,
        duration: data.duration,
        name: data.name,
        type: data.name,
        color: colorPalette[hashToNumber(data.name, colorPalette.length)],
        depth: data.depth,
        threadId: data.tid,
    };
};

/* eslint-disable */
export const generateLinkDetail = (field: string): LinkDataDesc<Record<string, unknown>> => {
    return linkData({
        renderFields: [],
        templateField: [ field, (data: any, session) => <Link onClick={() => {
            runInAction(() => {
                session.linkDetail = linkData({
                    renderFields: [
                        [ 'ID', (data: any) => data.id ],
                        [ 'Title', (data: any) => data.title ],
                        [ 'Category', (data: any) => data.cat ],
                        [ 'from', (data: any, session, metadata) => <Link onClick={action(() => {
                            session.selectedData= generateFlowData(data.from);
                            session.linkDetail = generateLinkDetail('Outgoing flow');
                            session.linkFlow = generateFlowParam(metadata as ThreadMetaData, data.from.timestamp, data.from.pid, data.from.tid);
                        })}>{`Slice ${data.from.name} at ${getTimestamp(data.from.timestamp ?? 0, { precision: 'ns' })}`}</Link>],
                        [ 'to', (data: any, session, metadata) => <Link onClick={action(() => {
                            session.selectedData= generateFlowData(data.to);
                            session.linkDetail = generateLinkDetail('Incoming flow');
                            session.linkFlow = generateFlowParam(metadata as ThreadMetaData, data.to.timestamp, data.to.pid, data.to.tid);
                        })}>{`Slice ${data.to.name} at ${getTimestamp(data.to.timestamp ?? 0, { precision: 'ns' })}`}</Link>],
                    ],
                    fetchData: async (session: Session, metadata) => {
                        const startTime = session.selectedData?.startTime;
                        const flowId = session.linkFlow?.flowId as string;
                        const type = session.linkFlow?.type as string;
                        const rankId = (metadata as Record<string, unknown>)?.cardId;
                        const raw = await window.request(metadata.dataSource, { command: 'unit/flow', params: { flowId, type, startTime, rankId } } ) as any;
                        const linkLine = { [raw.cat]: [{ category: raw.cat, ...raw, cardId: rankId }] }
                        runInAction(() => {
                            session.linkLines = linkLine;
                            session.singleLinkLine = linkLine;
                            session.renderTrigger = !session.renderTrigger;
                        });
                        return raw;
                    },
                });
                session.linkFlow = data;
            });
        }}>{data.title}</Link> ],
        fetchData: async (session: Session, metadata) => {
            const raw = await window.request(metadata.dataSource, { command: 'unit/flowName', params: session.linkFlow as Record<string, unknown> });
            return raw.flowDetail;
        },
    });
};
