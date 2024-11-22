/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

import { detail, linkData } from '../../entity/insight';
import type { DetailDescriptor, LinkDataDesc } from '../../entity/insight';
import { isEmpty } from 'lodash';
import styled from '@emotion/styled';
import React from 'react';
import { action, runInAction } from 'mobx';
import type { AscendMultiSliceList, ThreadMetaData, ThreadTrace } from '../../entity/data';
import type { Session } from '../../entity/session';
import { getSliceTimeDisplay, ThreadUnit } from './AscendUnit';
import { getTimestamp } from '../../utils/humanReadable';
import { colorPalette, getTimeOffset } from './utils';
import { hashToNumber } from '../../utils/colorUtils';
export const slicesListDetail = detail({
    name: 'Slice List',
    columns: [
        ['Name', (data): string => `${isEmpty(data.title) ? 'null' : data.title}`, 'max-content', 'scroll'],
        ['Wall Duration', (data): string => getSliceTimeDisplay(data.wallDuration as number), 180],
        ['Self Time', (data): string => getSliceTimeDisplay(data.selfTime as number), 180],
        ['Average Wall Duration', (data): string => getSliceTimeDisplay(data.avgWallDuration as number), 180],
        ['Occurrences', (data): string => `${(data.occurrences as string) ?? 0}`, 180],
    ],
    actions: [
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList): number => a.title?.localeCompare(b?.title ?? '') ?? 0, filterKey: 'title' },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList): number => (a.wallDuration ?? 0) - (b.wallDuration ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList): number => (a.selfTime ?? 0) - (b.selfTime ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList): number => (a.avgWallDuration ?? 0) - (b.avgWallDuration ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList): number => (a.occurrences ?? 0) - (b.occurrences ?? 0) },
    ],
    fetchData: async (session: Session, metadata: ThreadMetaData) => {
        let startTime = session.selectedRange?.[0] ?? 0;
        startTime = startTime < 0 ? 0 : startTime;
        let endTime = session.selectedRange?.[1] ?? 0;
        endTime = endTime < 0 ? 0 : endTime;
        const timestampOffset = getTimeOffset(session, metadata);
        const metadataList = session.selectedUnits.map(selectUnit => {
            const { threadId, processId, metaType } = selectUnit?.metadata as ThreadMetaData ?? {};
            return {
                tid: threadId,
                pid: processId,
                metaType,
            };
        });
        const params = {
            rankId: metadata.cardId,
            startTime: Math.floor(startTime + timestampOffset),
            endTime: Math.ceil(endTime + timestampOffset),
            metadataList,
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
            metaType: row.metaType,
            count: row.occurrences,
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
export const generateFlowParam = function(metadata: ThreadMetaData, data: any, metaType?: string): Record<string, unknown> {
    return {
        rankId: metadata.cardId ?? '',
        tid: data.tid ?? (metadata.threadId ?? ''),
        pid: data.pid ?? (metadata.processId ?? ''),
        id: data.id,
        metaType: metaType ?? metadata?.metaType ?? '',
        startTime: data.startTime ?? data.timestamp,
        endTime: (data.startTime ?? data.timestamp) + data.duration,
    };
};

const generateFlowData = function (data: any, timeOffset: number): any {
    return {
        startTime: data.timestamp - timeOffset,
        duration: data.duration,
        name: data.name,
        type: data.name,
        color: colorPalette[hashToNumber(data.name, colorPalette.length)],
        depth: data.depth,
        threadId: data.tid,
        id: data.id,
        metaType: data.metaType,
    };
};

/* eslint-disable */
export const generateLinkDetail = (field: string): LinkDataDesc<Record<string, unknown>> => {
    return linkData({
        renderFields: [],
        templateField: [field, (dataset: any, tempFieldSession):JSX.Element => <Link onClick={():void => {
            runInAction(() => {
                tempFieldSession.linkDetail = linkData({
                    renderFields: [
                        ['ID', (data: any):string => data.id],
                        ['Title', (data: any):string => data.title],
                        ['Category', (data: any):string => data.cat],
                        ['from', (data: any, session, metadata):JSX.Element => <Link onClick={action(() => {
                            const rankId = data.from.rankId && data.from.rankId !== '' ? data.from.rankId : (metadata as ThreadMetaData).cardId;
                            calculateDomainRange(session, data.from, data.to, rankId);
                            session.linkDetail = generateLinkDetail('Outgoing flow');
                            doJumpSlice(session, data.from, rankId);
                        })}>{`Slice ${data.from.name} at ${getTimestamp(data.from.timestamp ?? 0, { precision: 'ns' })}`}</Link>],
                        ['to', (data: any, session, metadata):JSX.Element => <Link onClick={action(() => {
                            const rankId = data.to.rankId && data.to.rankId !== '' ? data.to.rankId : (metadata as ThreadMetaData).cardId;
                            calculateDomainRange(session, data.from, data.to, rankId);
                            session.linkDetail = generateLinkDetail('Incoming flow');
                            doJumpSlice(session, data.to, rankId);
                        })}>{`Slice ${data.to.name} at ${getTimestamp(data.to.timestamp ?? 0, { precision: 'ns' })}`}</Link>],
                    ],
                    fetchData: async (session: Session, metadata) => {
                        return {};
                    },
                });
                tempFieldSession.linkFlow = dataset;
            });
        }}>{dataset.title}</Link>],
        fetchData: async (session: Session, metadata) => {
            return {};
        },
    });
};

export const calculateDomainRange = (session: Session, from: any, to: any, rankId: number): void => {
    const fromRankId = from.rankId && from.rankId !== '' ? from.rankId : rankId;
    const toRankId = to.rankId && to.rankId !== '' ? to.rankId : rankId;
    let rangeStart = from.timestamp - getTimeOffset(session, fromRankId);
    let rangeEnd = to.timestamp - getTimeOffset(session, toRankId);
    if ( rangeEnd < rangeStart) {
        [rangeStart, rangeEnd] = [rangeEnd, rangeStart];
    }
    rangeStart = rangeStart - (from.duration * 10); //  此出为计算范围，选10作为一个折中的值，无具体含义
    rangeStart = rangeStart > 0 ? rangeStart : 0;
    rangeEnd = Math.min(rangeEnd + (to.duration * 10), session.endTimeAll ?? Number.MAX_SAFE_INTEGER);
    session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
};

const doJumpSlice = (session: Session, data: any, rankId: string): void => {
    if (data === undefined) {
        return;
    }
    runInAction(() => {
        session.renderTrigger = !session.renderTrigger;
        session.locateUnit = {
            target: (unit): boolean => {
                return unit instanceof ThreadUnit && (Boolean(unit.metadata.cardId.includes(rankId))) &&
                    unit.metadata.processId === data.pid && unit.metadata.threadId === data.tid;
            },
            onSuccess: (unit): void => {
                session.selectedData = generateFlowData(data, getTimeOffset(session, unit.metadata as ThreadMetaData));
                session.linkFlow = generateFlowParam(unit.metadata as ThreadMetaData, data);
            },
        };
    });
};
