/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Select } from 'ascend-components';
import MemoryBarChart from './MemoryBarChart';
import MemorySliceChart from './MemorySliceChart';
import MemoryFunctionCall from './MemoryFunctionCall';
import { Label } from './Common';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import { getFuncNewData, getBarNewData } from './dataHandler';
import { Line, initLine, cancelLine } from './LineHandler';
import { chartResize } from '../utils/utils';

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
        funcIns.off('click');
        barIns.off('legendselectchanged');
        funcIns.on('click', (params: any) => {
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
        <div>
            <div style={{ marginLeft: 24, marginTop: 24 }}>
                <Label name={t('ThreadID')} />
                <Select
                    id={'select-threadId'}
                    value={session.threadId}
                    size="middle"
                    onChange={(value): void => {
                        runInAction(() => {
                            session.threadId = value;
                        });
                    }}
                    options={session.threadOps}
                />
            </div>
            <div id='funcContent' style={{ overflow: 'auto', padding: 0, position: 'relative' }}>
                <Line id='funcLine' lineShow={lineShow} offset={offset} />
                {<MemoryFunctionCall session={session} setFuncIns={setFuncIns} />}
            </div>
            <div style={{ marginLeft: 24 }}>
                <Label name={t('DeviceID')} />
                <Select
                    id={'select-deviceId'}
                    style={{ marginRight: 20 }}
                    value={session.deviceId}
                    size="middle"
                    onChange={(value): void => {
                        runInAction(() => {
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
                            session.eventType = value;
                        });
                    }}
                    options={session.typeOpts}
                />
            </div>
            <div id='barContent' style={{ overflow: 'auto', padding: 0, position: 'relative' }}>
                <Line id='barLine' lineShow={lineShow} offset={offset} />
                <MemoryBarChart session={session} setBarIns={setBarIns} />
            </div>
            {session.memoryStamp
                ? (
                    <div id='detailsContent' style={{ position: 'relative' }}>
                        <div style={{ position: 'absolute', left: '45%' }}>{`${t('Current Time')}: ${session.memoryStamp}ns`}</div>
                        <MemorySliceChart session={session} />
                    </div>
                )
                : (
                    <></>
                )}
        </div>
    );
});

export default MemoryStack;
