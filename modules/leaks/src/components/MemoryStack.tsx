/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import React, { useEffect, useState } from 'react';
import { Select, Checkbox, CollapsiblePanel } from '@insight/lib/components';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import type { CheckboxChangeEvent } from 'antd/lib/checkbox';
import MemoryBarChart from './MemoryBarChart';
import MemorySliceChart from './MemorySliceChart';
import MemoryFunctionCall from './MemoryFunctionCall';
import MemoryTable from './MemoryTable';
import { Label } from './Common';
import { getFuncNewData, getBarNewData } from './dataHandler';
import { Line, initLine, cancelLine } from './LineHandler';
import { chartResize, convertNanoseconds } from '../utils/utils';

const MemoryStack = observer(({ session }: { session: any }): React.ReactElement => {
    const { t } = useTranslation('leaks');
    const [funcIns, setFuncIns] = useState<echarts.ECharts | null>();
    const [barIns, setBarIns] = useState<echarts.ECharts | null>();
    const [lineShow, setLineshow] = useState('none');
    const [offset, setOffset] = useState(0);
    const mouseEnter = (): void => {
        setLineshow('block');
    };
    const mouseMove = (e: MouseEvent): void => {
        requestAnimationFrame(() => {
            setOffset(e.clientX - 16);
        });
    };
    const mouseLeave = (): void => {
        setLineshow('none');
        setOffset(0);
    };
    const restoreSelect = (): void => {
        runInAction(() => {
            Object.keys(session.legendSelect).forEach(key => { session.legendSelect[key] = true; });
        });
    };
    const linkageHandle = (): void => {
        if (!funcIns || !barIns) {
            return;
        }
        funcIns.off('dblclick');
        barIns.off('legendselectchanged');
        funcIns.on('dblclick', (params: any) => {
            const data = params.value;
            const start: number = data[1];
            const end: number = data[2];
            getFuncNewData(session, start, end);
            getBarNewData(session, start, end);
        });
        barIns.on('legendselectchanged', (params: any) => {
            chartResize(barIns);
            runInAction(() => {
                session.legendSelect = params.selected;
            });
        });
        [funcIns, barIns].forEach((ins) => {
            ins.off('restore');
            ins.off('dataZoom');
            ins.on('restore', () => {
                getFuncNewData(session);
                getBarNewData(session);
                restoreSelect();
            });
            ins.on('dataZoom', (params: any) => {
                const { startValue, endValue } = params.batch[0];
                getFuncNewData(session, Math.floor(startValue), Math.ceil(endValue));
                getBarNewData(session, Math.floor(startValue), Math.ceil(endValue));
            });
        });
    };
    useEffect(() => {
        const newIdOpts = Object.keys(session.deviceIds).map((id: string) => ({ label: id, value: id }));
        if (newIdOpts.length) {
            const newTypeOpts = session.deviceIds[newIdOpts[0].value].map((type: string) => ({ label: type, value: type }));
            const newThreadOpts = session.threadIds.map((thread: number) => ({ label: thread, value: thread }));
            initLine(mouseEnter, mouseMove, mouseLeave);
            runInAction(() => {
                session.deviceIdOpts = newIdOpts;
                session.typeOpts = newTypeOpts;
                session.threadOps = newThreadOpts;
                session.deviceId = newIdOpts[0].value;
                session.eventType = newTypeOpts[0].value;
                session.threadId = newThreadOpts[0].value;
            });
        }
        return () => {
            cancelLine(mouseEnter, mouseMove, mouseLeave);
        };
    }, [session.deviceIds, session.threadIds]);
    useEffect(() => {
        linkageHandle();
    }, [funcIns, barIns]);
    return (
        <>
            <CollapsiblePanel title={t('FlameGraph')} style={{ minWidth: 1000, display: session.threadOps.length > 0 && session.threadId !== '' ? 'block' : 'none' }}>
                <Label name={t('ThreadID')} />
                <Select
                    id={'select-threadId'}
                    value={session.threadId}
                    size="middle"
                    onChange={(value): void => {
                        runInAction(() => {
                            session.threadId = value;
                            session.threadFlag = false;
                            session.searchFunc = [];
                        });
                    }}
                    options={session.threadOps}
                />
                <Label name={t('Search')} style={{ marginLeft: 24 }} />
                <Select
                    id={'select-funcName'}
                    mode="multiple"
                    value={session.searchFunc}
                    style={{ width: 550, marginRight: 20 }}
                    onChange={(val: string[]): void => {
                        runInAction(() => { session.searchFunc = val; });
                    }}
                    options={session.funcOptions}
                    showSearch={true}
                    maxTagTextLength={10}
                    maxTagCount={4}
                />
                <Checkbox
                    checked={session.allowTrim}
                    onChange={(event: CheckboxChangeEvent): void => {
                        runInAction(() => { session.allowTrim = event.target.checked; getFuncNewData(session, session.minTime, session.maxTime); });
                    }}
                >{t('allowTrim')}</Checkbox>
                <div id="funcContent" style={{ overflow: 'auto', padding: 0, position: 'relative' }}>
                    <Line id="funcLine" lineShow={lineShow} offset={offset} color="#999" />
                    <MemoryFunctionCall session={session} setFuncIns={setFuncIns} />
                </div>
            </CollapsiblePanel >
            <CollapsiblePanel title={t('LineBlockGraph')} style={{ minWidth: 1000 }}>
                <Label name={t('DeviceID')} />
                <Select
                    id={'select-deviceId'}
                    style={{ marginRight: 20 }}
                    value={session.deviceId}
                    size="middle"
                    onChange={(value): void => {
                        runInAction(() => {
                            session.threadFlag = false;
                            session.typeOpts = session.deviceIds[value].map((type: string) => ({ label: type, value: type }));
                            session.deviceId = value;
                            session.eventType = session.deviceIds[value][0];
                        });
                    }}
                    options={session.deviceIdOpts}
                />
                <Label name={t('Type')} />
                <Select
                    id={'select-type'}
                    value={session.eventType}
                    size="middle"
                    onChange={(value): void => {
                        runInAction(() => {
                            session.threadFlag = false;
                            session.eventType = value;
                        });
                    }}
                    options={session.typeOpts}
                />
                <div id="barContent" style={{ overflow: 'auto', padding: 0, position: 'relative' }}>
                    <Line id="barLine" lineShow={lineShow} offset={offset} color="#999" />
                    <MemoryBarChart session={session} setBarIns={setBarIns} />
                </div>
            </CollapsiblePanel>
            {session.memoryStamp
                ? (
                    <CollapsiblePanel title={t('DetailsDiagram')} collapsible style={{ minWidth: 1000 }}>
                        <div id="detailsContent" style={{ position: 'relative' }}>
                            <div style={{ position: 'absolute', left: '42%' }}>{`${t('Current Time')}: ${convertNanoseconds(session.memoryStamp)}`}</div>
                            <MemorySliceChart session={session} />
                        </div>
                    </CollapsiblePanel>
                )
                : (
                    <></>
                )
            }
            <CollapsiblePanel title={t('DetailsTable')} collapsible style={{ minWidth: 1000 }} destroy={false}>
                <MemoryTable session={session} />
            </CollapsiblePanel>

        </>
    );
});

export default MemoryStack;
