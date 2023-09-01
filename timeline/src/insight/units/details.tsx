import { detail, DetailDescriptor, linkData, LinkDataDesc } from '../../entity/insight';
import { isEmpty } from 'lodash';
import styled from '@emotion/styled';
import React from 'react';
import { action, runInAction } from 'mobx';
import { AscendMultiSliceList, ThreadMetaData } from '../../entity/data';
import { Session } from '../../entity/session';
import { getSliceTimeDisplay } from './AscendUnit';
import { getTimestamp } from '../../utils/humanReadable';

export const slicesListDetail = detail({
    name: 'Slices List',
    columns: [
        [ 'Name', data => `${isEmpty(data.title) ? 'null' : data.title}`, 'max-content', 'scroll' ],
        [ 'Wall Duration', data => getSliceTimeDisplay(data.wallDuration), 180 ],
        [ 'Self Time', data => getSliceTimeDisplay(data.selfTime), 180 ],
        [ 'Average Wall Duration', data => getSliceTimeDisplay(data.avgWallDuration), 180 ],
        [ 'Occurrences', data => `${data.occurrences ?? 0}`, 180 ],
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
            startTime: startTime + timestampOffset,
            endTime: endTime + timestampOffset,
        };
        const raw = await window.request(metadata.remote as string, { command: 'unit/threads', params });
        return raw.data;
    },
}) as DetailDescriptor<unknown>;

const Link = styled.span`
    &:hover {
        border-bottom: 1px solid white;
    }
    border-bottom: 2px solid rgb(140, 140, 140);
`;

export const generateFlowParam = function(metadata: ThreadMetaData, startTime: number): { rankId: string; tid: number; pid: string; startTime: number } {
    return {
        rankId: metadata.cardId ?? '',
        tid: metadata.threadId,
        pid: metadata.processId ?? '',
        startTime,
    };
};

/* eslint-disable */
export const generateLinkDetail = (field: string): LinkDataDesc<Record<string, unknown>, unknown> => {
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
                            session.linkDetail = generateLinkDetail('Outgoing flow');
                            session.linkFlow = generateFlowParam(metadata as ThreadMetaData, data.from.timestamp);
                        })}>{`Slice at ${getTimestamp(data.to.timestamp ?? 0, { precision: 'ns' })}`}</Link> ],
                        [ 'to', (data: any, session, metadata) => <Link onClick={action(() => {
                            session.linkDetail = generateLinkDetail('Incoming flow');
                            session.linkFlow = generateFlowParam(metadata as ThreadMetaData, data.to.timestamp);
                        })}>{`Slice at ${getTimestamp(data.to.timestamp ?? 0, { precision: 'ns' })}`}</Link> ],
                    ],
                    fetchData: async (session: Session, metadata) => {
                        const flowId = session.linkFlow?.flowId as string;
                        const rankId = (metadata as Record<string, unknown>)?.cardId;
                        const raw = await window.request((metadata as Record<string, unknown>)?.remote as string, { command: 'unit/flow', params: { flowId, rankId } } ) as any;
                        const from = raw.from;
                        const to = raw.to;
                        session.linkData = {
                            sources: [{
                                data: { ...from, startTime: from.timestamp, duration: from?.duration ?? 0 },
                                matcher: (unit) => {
                                    const typedMetadata = unit?.metadata as any;
                                    return typedMetadata?.processId === from.pid && typedMetadata?.threadId === from.tid;
                                },
                            }],
                            target: {
                                data: { ...to, startTime: to.timestamp, duration: to?.duration ?? 0 },
                                matcher: (unit) => {
                                    const typedMetadata = unit?.metadata as any;
                                    return typedMetadata?.processId === to.pid && typedMetadata?.threadId === to.tid;
                                },
                            },
                        };
                        return raw;
                    },
                });
                session.linkFlow = data;
            });
        }}>{data.title}</Link> ],
        fetchData: async (session: Session, metadata) => {
            const raw = await window.request((metadata as Record<string, unknown>)?.remote as string, { command: 'unit/flowName', params: session.linkFlow as Record<string, unknown> });
            return raw.flowDetail;
        },
    });
};
