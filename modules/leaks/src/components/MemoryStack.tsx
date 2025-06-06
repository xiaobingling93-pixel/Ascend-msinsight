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
    const linkageHandle = (): void => {
        if (!funcIns || !barIns) {
            return;
        }
        [funcIns, barIns].forEach((ins) => {
            ins.off('restore');
            ins.off('dataZoom');
            ins.on('restore', () => {
                getFuncNewData(session);
                getBarNewData(session);
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
        <div>
            <div style={{ marginLeft: 24, marginTop: 24 }}>
                <Label name={t('ThreadID')} />
                <Select
                    id={'threadId'}
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
                    id={'deviceId'}
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
                    id={'type'}
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
            {session.memoryStamp ? <MemorySliceChart session={session} /> : <></>}
        </div>
    );
});

export default MemoryStack;
