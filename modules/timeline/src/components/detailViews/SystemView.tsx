/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

import { observer } from 'mobx-react';
import React, { useEffect, useMemo, useState } from 'react';
import { Divider, Space } from 'antd/lib/index';
import styled from '@emotion/styled';
import { Button, Select } from 'antd';
import {
    getColumnSearchProps,
    getDefaultColumData,
    GetPageData,
    kernelDetails,
    Label,
    Loading,
    pythonApiSummaryColumns,
    queryKernelDetails,
    queryOneKernel,
    querySystemViewDetails,
    systemViewItems,
} from './Common';
import ResizeTable from '../resize/ResizeTable';
import { CardMetaData } from '../../entity/data';
import { runInAction } from 'mobx';
import { ChartErrorBoundary } from '../error/ChartErrorBoundary';

const Container = styled.div`
    width: 100%;
    height: 100%;
    display: flex;
    flex-flow: nowrap;
    user-select:text;
    overflow: scroll;
    .ant-tree {
        width: 280px;
        height: 100%;
        background-color: ${p => p.theme.contentBackgroundColor};
        overflow: auto;
    }
    .ant-tree-node-selected {
        background-color: var(--grey50) !important;
    }
    .ant-divider-vertical {
        height: 100%;
        border-color: var(--grey80);
    }
`;

const SelectContainer = styled.div`
    width: calc(100% - 290px);
    height: 100%;
    .ant-table-wrapper {
        height: 100%;
    }
`;

export const SystemView = observer((props: any) => {
    const [key, setKey] = useState(0);
    const SelectContent = useMemo(() => ContentList[key], [key]);
    const [conditions, setConditions] = useState<{ rankId: string }>({ rankId: '' });
    const handleChange = (rankId: string): void => {
        setConditions({ rankId });
    };
    return (<Container className={'theme-view'}>
        <Space direction="vertical" size="middle" style={{ display: 'flex' }}>
            <ViewSelect />
            <RankFilter session={props.session} handleChange={handleChange}></RankFilter>
            <SelectList setKey={setKey}></SelectList>
        </Space>
        <Divider type="vertical" />
        <ChartErrorBoundary><SelectContainer><SelectContent className={'SelectContent'} key={key} rankId={conditions.rankId} session={props.session}></SelectContent></SelectContainer></ChartErrorBoundary>
    </Container>);
});

const ViewSelect = observer(() => {
    const options = [{ label: 'Stats System View', value: 0 }];
    return (
        <div className={'systemViewSelect'}>
            <Select style={{ width: 180 }} defaultValue={0} options={options}/>
        </div>
    );
});

const RankFilter = observer((props: any): JSX.Element => {
    const [rankId, setRankId] = useState<string | undefined>(undefined);
    const [rankIdList, setRankIdList] = useState<string[]>([]);
    useEffect(() => {
        const rankList: any[] = [];
        for (const unit of props.session.units) {
            rankList.push((unit.metadata as CardMetaData).cardId);
        }
        setRankIdList(rankList.sort((a: any, b: any) => Number(a) - Number(b)));
        setRankId(rankList[0]);
    }, [props.session.units.length]);

    useEffect(() => {
        props.handleChange(rankId);
    }, [rankId]);
    const onRankIdChanged = (value: string): void => {
        setRankId(value);
    };
    return (<div className={'systemViewRank'} >
        <Label name="RankId" />
        <Select
            value={rankId}
            style={{ width: 120 }}
            onChange={onRankIdChanged}
            options={rankIdList.map((rankId) => {
                return {
                    value: rankId,
                    label: rankId,
                };
            })}
            showSearch={true}
        />
    </div>);
});

const SelectList = observer((props: any) => {
    const [selectedKey, setSelectedKey] = useState('0');
    const handleClick = (key: string): void => {
        props.setKey(key);
        setSelectedKey(key);
    };
    return (<div className={'selectLayer'}>
        {
            systemViewItems.map((item, index) =>
                (<div
                    className={selectedKey === item.key ? 'selected' : ''}
                    key={index}
                    onClick={() => { handleClick(item.key); }}
                >
                    {item.title}
                </div>
                ))
        }
    </div>
    );
});

// eslint-disable-next-line max-lines-per-function
const BaseSummary = observer((props: any) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'totalTime', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [isLoading, setLoading] = useState(false);
    const [searchText, setSearchText] = useState('');
    const [searchedColumn, setSearchedColumn] = useState('');
    const status = props.session.units.find((unit: any) => (unit.metadata as CardMetaData).cardId === props.rankId)?.phase;
    const updateData = async(searchName: string, pages: any, sorters: {field: string;order: string}, prop: any): Promise<void> => {
        if (props.rankId === undefined || props.rankId === '') {
            setDataSource([]);
            setPage(defaultPage);
            return;
        }
        setLoading(true);
        const res = await querySystemViewDetails({
            isQueryTotal: true,
            rankId: prop.rankId,
            pageSize: pages.pageSize,
            current: pages.current,
            orderBy: sorters.field ?? defaultSorter.field,
            order: sorters.order ?? defaultSorter.order,
            layer: prop.layerType,
            searchName,
        });
        setLoading(false);
        setDataSource(res.systemViewDetails);
        setPage({ ...page, total: res.count });
    };
    useEffect(() => {
        updateData(searchText, page, sorter, props);
    }, [sorter, props.rankId]);
    useEffect(() => {
        if (status === 'download') {
            updateData(searchText, page, sorter, props);
        }
    }, [status]);

    const colums = [
        {
            title: 'Name',
            dataIndex: 'name',
            ...getDefaultColumData('name'),
            ...getColumnSearchProps({ dataIndex: 'name', setSearchText, searchText, setSearchedColumn, searchedColumn }),
        },
        ...pythonApiSummaryColumns,
    ];
    return (
        (status === 'download' || props.rankId === undefined)
            ? <ResizeTable
                onChange={(pagination: any, filters: any, sorter: any) => {
                    setSorter(sorter);
                }}
                pagination={GetPageData(page, setPage)}
                dataSource={dataSource}
                columns={colums}
                size="small"
                loading = {isLoading}/>
            : <Loading style={{ marginTop: '10px' }}/>
    );
});

const PythonApiSummary = observer((props: any) => {
    return <BaseSummary layerType={'Python'} {...props}/>;
});

const CannApiSummary = observer((props: any) => {
    return <BaseSummary layerType={'CANN'} {...props}/>;
});

const AscendHardWareTask = observer((props: any) => {
    return <BaseSummary layerType={'Ascend Hardware'} {...props}/>;
});

const OverlapAnalysis = observer((props: any) => {
    return <BaseSummary layerType={'Overlap Analysis'} {...props}/>;
});

const HCCLSummary = observer((props: any) => {
    return <BaseSummary layerType={'HCCL'} {...props}/>;
});

// eslint-disable-next-line max-lines-per-function
const KernelDetails = observer((props: any) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'duration', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [rowData, setRowData] = useState<any>({});
    const [searchText, setSearchText] = useState('');
    const [searchedColumn, setSearchedColumn] = useState('');

    const status = props.session.units.find((unit: any) => (unit.metadata as CardMetaData).cardId === props.rankId)?.phase;
    useEffect(() => {
        updateData(searchText, page, sorter);
    }, [sorter, props.rankId]);
    useEffect(() => {
        if (status === 'download') {
            updateData(searchText, page, sorter);
        }
    }, [status]);
    const updateData = async(searchName: string, pages: any, sorters: {field: string;order: string}): Promise<void> => {
        if (props.rankId === undefined) {
            setDataSource([]);
            setPage(defaultPage);
            return;
        }
        const res = await queryKernelDetails({
            rankId: props.rankId,
            pageSize: pages.pageSize,
            current: pages.current,
            orderBy: sorters.field ?? defaultSorter.field,
            order: sorters.order ?? defaultSorter.order,
            coreType: '',
            searchName,
        });
        setDataSource(res.kernelDetails);
        setPage({ ...page, total: res.count });
    };
    const handleSelected = async(): Promise<void> => {
        const res = await queryOneKernel({
            rankId: props.rankId,
            name: rowData.name,
            timestamp: rowData.startTime,
            duration: Number((rowData.duration * 1000).toFixed(0)),
        });
        runInAction(() => {
            props.session.locateUnit = {
                target: (unit: any) => {
                    return unit.metadata.threadId === res.threadId && unit.metadata.processId === res.pid;
                },
                onSuccess: () => {
                    props.session.selectedData = {
                        startTime: rowData.startTime,
                        name: rowData.name,
                        duration: Number((rowData.duration * 1000).toFixed(0)),
                        depth: res.depth,
                        threadId: res.threadId,
                        startRecordTime: props.session.startRecordTime,
                    };
                },
            };
        });
    };

    const colums = [
        {
            title: 'Name',
            dataIndex: 'name',
            ...getDefaultColumData('name'),
            ...getColumnSearchProps({ dataIndex: 'name', setSearchText, searchText, setSearchedColumn, searchedColumn }),
        },
        ...kernelDetails,
        {
            title: 'Click To Timeline',
            dataIndex: 'click',
            key: 'click',
            ellipsis: true,
            render: (_: any, record: any) => (<Button type="link"
                onClick={() => {
                    setRowData(record as any);
                }}>click</Button>),
        },
    ];

    useEffect(() => {
        if (rowData.name === null || rowData.name === undefined) {
            return;
        }
        handleSelected();
    }, [rowData]);
    return (
        (status === 'download' || props.rankId === undefined)
            ? <ResizeTable
                onChange={(pagination: any, filters: any, sorter: any) => {
                    setSorter(sorter);
                }}
                pagination={GetPageData(page, setPage)}
                dataSource={dataSource}
                columns={colums}
                size="small"/>
            : <Loading style={{ marginTop: '10px' }}/>
    );
});

const ContentList = [PythonApiSummary, CannApiSummary,
    AscendHardWareTask, HCCLSummary, OverlapAnalysis, KernelDetails];
