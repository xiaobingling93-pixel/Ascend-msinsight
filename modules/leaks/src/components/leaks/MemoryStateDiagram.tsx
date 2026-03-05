/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

import React, { useEffect, useMemo, useRef, useState } from 'react';
import { runInAction } from 'mobx';
import {
    workerInitCanvas,
    workerResizeCanvas,
    workerTransform,
    workerHoverItem,
    workerClickItem,
    workerSetMemoryStateData,
} from '@/leaksWorker/stateWorker/worker';
import { Session } from '@/entity/session';
import { Input, ResizeTable, ResizeTableRef, SearchIcon } from '@insight/lib';
import { LeftOutlined, RightOutlined } from '@ant-design/icons';
import { type Theme, useTheme } from '@emotion/react';
import { formatBytes } from '@/utils/utils';
import { type EvenItem, getMemoryStateData } from '@/utils/RequestUtils';
import { getAllEventListData } from '../dataHandler';
import { observer } from 'mobx-react';

export const MemoryStateDiagram = ({ session }: { session: Session }): JSX.Element => {
    return <div style={{ display: 'flex', height: 800 }}>
        <div style={{ width: 350 }}>
            <EventList session={session} />
        </div>
        <div style={{ flex: 1, padding: '0 30px' }}>
            <StateDiagramCanvas session={session} />
        </div>
    </div>;
};

const EventItemRender = ({ record }: { record: EvenItem }): JSX.Element => {
    const commonStyle = { width: 100, whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis' };

    const computeSize = formatBytes(record.size);

    return <div style={{ display: 'flex', width: 320 }}>
        <div style={commonStyle} title={record.action}>{record.action}</div>
        <div style={{ width: 10 }}></div>
        <div>
            <div style={commonStyle} title={record.address}>{record.address}</div>
            <div style={commonStyle} title={`stream ${record.stream}`}>(stream {record.stream})</div>
        </div>
        <div style={{ width: 10 }}></div>
        <div>
            <div style={commonStyle} title={computeSize}>{computeSize}</div>
            <div style={commonStyle} title={`${record.size} B`}>({record.size} B)</div>
        </div>
    </div >;
};

const EventList = observer(({ session }: { session: Session }): JSX.Element => {
    const [searchValue, setSearchValue] = useState<string>('');
    const [searchIndexList, setSearchIndexList] = useState<number[]>([]);
    const [dataSource, setDataSource] = useState<EvenItem[]>([]);
    const [currentShowRow, setCurrentShowRow] = useState<number>(-1);
    const [currentSelectRow, setCurrentSelectRow] = useState<number>(-1);
    const tableRef = useRef<ResizeTableRef>(null);
    const theme: Theme = useTheme();

    const columns = [{
        key: 'id',
        render: (_value: string, record: EvenItem, _index: number) => <EventItemRender record={record} />,
    }];

    const handleChange = (e: React.ChangeEvent<HTMLInputElement>): void => {
        const inputContent = e.target.value;
        if (/^\d*$/.test(inputContent)) {
            setSearchValue(inputContent);
        }
    };

    const handleSearch = (): void => {
        const result: number[] = [];
        if (searchValue.length < 1) {
            setSearchIndexList(result);
            setCurrentShowRow(-1);
            return;
        }
        for (let i = 0; i < dataSource.length; i++) {
            if (String(dataSource[i].address).includes(searchValue)) {
                result.push(i);
            }
        }
        setSearchIndexList(result);
        if (result.length > 0) {
            tableRef.current?.getVirtualBoxDom()?.scrollTo({ top: 32 * result[0] });
            setCurrentShowRow(0);
        } else {
            setCurrentShowRow(-1);
        }
    };

    const [canUpRow, canDownRow] = useMemo(() => {
        return [
            currentShowRow > 0 && currentShowRow < searchIndexList.length,
            currentShowRow > -1 && currentShowRow < searchIndexList.length - 1,
        ];
    }, [currentShowRow, searchIndexList]);

    const changeScroll = (type: string = 'up'): void => {
        if (type === 'up' && !canUpRow) {
            return;
        }
        if (type === 'down' && !canDownRow) {
            return;
        }
        const offset = type === 'up' ? -1 : 1;
        tableRef.current?.getVirtualBoxDom()?.scrollTo({ top: 32 * searchIndexList[currentShowRow + offset] });
        setCurrentShowRow(oVal => (oVal + offset));
    };

    useEffect(() => {
        if (session.deviceId === '') return;
        getAllEventListData(session).then((data) => {
            setDataSource(data.map((item, index) => ({
                ...item,
                index,
            })));
            setCurrentSelectRow(0);
        });
    }, [session.deviceId]);

    useEffect(() => {
        if (session.leaksWorkerInfo.clickItem === null) {
            return;
        }
        const index = dataSource.findIndex(item => item.id === session.leaksWorkerInfo.clickItem?.id);
        setCurrentSelectRow(index);
        tableRef.current?.getVirtualBoxDom()?.scrollTo({ top: 32 * index });
    }, [session.leaksWorkerInfo.clickItem]);

    useEffect(() => {
        if (session.deviceId === '') return;
        const currentRow = dataSource[currentSelectRow];
        if (currentRow === undefined) {
            return;
        }
        getMemoryStateData({ eventId: currentRow.id }).then(data => {
            workerSetMemoryStateData({ data: data.segments });
        });
    }, [currentSelectRow, dataSource]);

    return <div>
        <div style={{ display: 'flex', paddingBottom: 8 }}>
            <Input allowClear style={{ flex: 1 }} value={searchValue} onChange={handleChange} onPressEnter={handleSearch} />
            <div style={{ padding: '0 10px', display: 'flex', alignItems: 'center', cursor: 'pointer' }} onClick={handleSearch}>
                <SearchIcon />
            </div>
            <LeftOutlined style={{ color: theme.textColor, cursor: canUpRow ? 'pointer' : 'not-allowed' }} onClick={() => changeScroll()} />
            <div style={{ paddingLeft: 10 }} />
            <RightOutlined style={{ color: theme.textColor, cursor: canDownRow ? 'pointer' : 'not-allowed' }} onClick={() => changeScroll('down')} />
            <div style={{ paddingLeft: 10 }} />
        </div>
        <ResizeTable className="table-slice-list" ref={tableRef} virtual scroll={{ y: 760 }} dataSource={dataSource} columns={columns} showHeader={false}
            rowClassName={(row: any): string => {
                if (currentSelectRow === row.index) {
                    return 'click-select';
                }
                return searchIndexList.includes(row.index) ? 'selected-row' : 'click-able';
            }}
            onRow={(row): React.HTMLAttributes<any> => ({
                onClick: (): void => {
                    setCurrentSelectRow(row.index);
                    runInAction(() => {
                        session.clickEventItem = row;
                    });
                },
            })}
        />
    </div>;
});

const StateDiagramCanvas = ({ session }: { session: Session }): JSX.Element => {
    const containerRef = useRef<HTMLDivElement>(null);
    const ref = useRef<HTMLCanvasElement>(null);
    const isDragging = useRef(false);
    const isClick = useRef(false);
    const dragStartPoint = useRef({ x: 0, y: 0 });

    const handleResize = (): void => {
        if (containerRef.current === null) {
            return;
        }
        const containerRect = containerRef.current.getBoundingClientRect();
        const width = containerRect.width;
        const height = containerRect.height;
        runInAction(() => {
            session.stateWorkerInfo.renderOptions.viewport = { width, height };
        });
        workerResizeCanvas({ width, height });
    };

    const handleWheel = (ev: WheelEvent): void => {
        ev.preventDefault();

        if (ref.current === null) {
            return;
        }

        const rect = ref.current.getBoundingClientRect();

        // 计算鼠标相对于画布的坐标
        const mouseX = ev.clientX - rect.left;
        const mouseY = ev.clientY - rect.top;

        // 获取当前变换参数
        const currentTransform = session.stateWorkerInfo.renderOptions.transform;

        // 计算缩放前鼠标在实际内容中的相对位置（相对于画布原点）
        const originalContentMouseX = (mouseX - currentTransform.x) / currentTransform.scale;
        const originalContentMouseY = (mouseY - currentTransform.y) / currentTransform.scale;

        // 计算新的缩放值
        const deltaScale = ev.deltaY > 0 ? -0.1 : 0.1;
        const newScale = Math.max(0.1, currentTransform.scale + deltaScale);

        const maxRangeX = rect.width;
        const minRangeX = -rect.width * newScale;
        const maxRangeY = rect.height;
        const minRangeY = -rect.height * newScale;
        // 计算缩放后的新偏移，使鼠标下的内容位置不变
        // 原始偏移距离 + (内容相对位置 * (新缩放 - 旧缩放))
        const newX = Math.min(Math.max(mouseX - originalContentMouseX * newScale, minRangeX), maxRangeX);
        const newY = Math.min(Math.max(mouseY - originalContentMouseY * newScale, minRangeY), maxRangeY);

        // 更新变换参数
        const transform = { x: newX, y: newY, scale: newScale };
        runInAction(() => {
            session.stateWorkerInfo.renderOptions.transform = transform;
        });

        workerTransform({ transform });
    };

    const handleMouseDown = (ev: MouseEvent): void => {
        if (ev.button !== 0 || ref.current === null) {
            return;
        }
        isClick.current = true;
        const rect = ref.current.getBoundingClientRect();
        dragStartPoint.current = {
            x: ev.clientX - rect.left,
            y: ev.clientY - rect.top,
        };
    };

    const handleMouseUp = (): void => {
        isDragging.current = false;
    };

    const handleMouseLeave = (): void => {
        isDragging.current = false;
        isClick.current = false;
    };

    const handleMouseMove = (ev: MouseEvent): void => {
        if (ref.current === null) {
            return;
        }
        if (isClick.current) {
            isClick.current = false;
            isDragging.current = true;
        }
        const rect = ref.current.getBoundingClientRect();
        const currentX = ev.clientX - rect.left;
        const currentY = ev.clientY - rect.top;
        if (!isDragging.current) {
            workerHoverItem({ clientX: currentX, clientY: currentY });
            return;
        }

        const currentTransform = session.stateWorkerInfo.renderOptions.transform;

        const deltaX = currentX - dragStartPoint.current.x;
        const deltaY = currentY - dragStartPoint.current.y;
        const maxRangeX = rect.width;
        const minRangeX = -rect.width * currentTransform.scale;
        const maxRangeY = rect.height;
        const minRangeY = -rect.height * currentTransform.scale;

        const transform = {
            ...currentTransform,
            x: Math.min(Math.max(currentTransform.x + deltaX, minRangeX), maxRangeX),
            y: Math.min(Math.max(currentTransform.y + deltaY, minRangeY), maxRangeY),
        };
        runInAction(() => {
            session.stateWorkerInfo.renderOptions.transform = transform;
        });

        workerTransform({ transform });

        dragStartPoint.current = { x: currentX, y: currentY };
    };

    const handleClick = (ev: MouseEvent): void => {
        if (ref.current === null) {
            return;
        }
        if (isClick.current) {
            isClick.current = false;
            const rect = ref.current.getBoundingClientRect();
            workerClickItem({ clientX: ev.clientX - rect.left, clientY: ev.clientY - rect.top });
        }
    };

    useEffect(() => {
        if (ref.current === null || containerRef.current === null) {
            return;
        }
        const canvas = ref.current;
        try {
            const containerRect = containerRef.current.getBoundingClientRect();
            const width = containerRect.width;
            const height = containerRect.height;

            runInAction(() => {
                session.stateWorkerInfo.renderOptions.viewport = { width, height };
            });
            workerInitCanvas({ canvas, width, height });
        } catch (_e) {
            // 进入这里，说明画布已经离屏代理，不需要做额外处理
        }
        handleResize();
    }, []);

    useEffect(() => {
        if (ref.current === null || containerRef.current === null) {
            return;
        }
        const canvas = ref.current;

        window.addEventListener('resize', handleResize);

        canvas.addEventListener('wheel', handleWheel, { passive: false, capture: true });
        canvas.addEventListener('mousedown', handleMouseDown);
        canvas.addEventListener('mousemove', handleMouseMove);
        canvas.addEventListener('mouseup', handleMouseUp);
        canvas.addEventListener('mouseleave', handleMouseLeave);
        canvas.addEventListener('click', handleClick);

        return () => {
            window.removeEventListener('resize', handleResize);

            canvas.removeEventListener('wheel', handleWheel, { capture: true });
            canvas.removeEventListener('mousedown', handleMouseDown);
            canvas.removeEventListener('mousemove', handleMouseMove);
            canvas.removeEventListener('mouseup', handleMouseUp);
            canvas.removeEventListener('mouseleave', handleMouseLeave);
            canvas.removeEventListener('click', handleClick);
        };
    }, []);

    return <div ref={containerRef} style={{ position: 'relative', width: '100%', height: '100%', overflow: 'hidden' }}>
        <canvas
            ref={ref}
            style={{ position: 'absolute', top: 0, imageRendering: 'pixelated', touchAction: 'none' }}
        />
    </div>;
};
