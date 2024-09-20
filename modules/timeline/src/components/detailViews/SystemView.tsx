/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
*/
import { observer } from 'mobx-react';
import React, { useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import styled from '@emotion/styled';
import { Button, Select, FormItem } from 'ascend-components';
import {
    getColumnSearchProps,
    getDefaultColumData,
    getPageData,
    useKernelDetails,
    pythonApiSummaryColumns,
    queryKernelDetails,
    queryOneKernel,
    querySystemViewDetails,
    statsSystemViewItems,
    expertSystemViewItems,
    layerTypes,
    type IQueryCondition,
    queryAffinityOptimizer,
    affinityOptimizerColumns,
    queryAICPUOperators,
    queryACLNNOperators,
    aicpuOperatorColumns,
    aclnnOperatorColumns,
    queryAffinityAPI,
    affinityAPIColumns,
    queryOperatorFusion,
    fusionOperatorColumns,
    eventViewData,
} from './Common';
import { ResizeTable } from 'ascend-resize';
import { limitInput, GroupRankIdsByHost, StyledEmpty } from 'ascend-utils';
import type { CardMetaData, ThreadMetaData } from '../../entity/data';
import { runInAction } from 'mobx';
import { ChartErrorBoundary } from '../error/ChartErrorBoundary';
import { calculateDomainRange } from '../CategorySearch';
import { colorPalette, getTimeOffset } from '../../insight/units/utils';
import type { InsightUnit } from '../../entity/insight';
import { hashToNumber } from '../../utils/colorUtils';
import { getDetailTimeDisplay, ThreadUnit } from '../../insight/units/AscendUnit';
import { EventDetail } from './EventsView';

export const DETAIL_HEADER_HEIGHT_ETC_PX = 146;
const Container = styled.div`
    width: 100%;
    height: 100%;
    display: flex;
    flex-flow: nowrap;
    user-select:text;
    background-color: ${(p): string => p.theme.bgColorDark};
    .ant-tree {
        width: 280px;
        height: 100%;
        background-color: ${(p): string => p.theme.contentBackgroundColor};
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

const AsideSelectContainer = styled.div`
    display: flex;
    flex-direction: column;
    flex: none;
    padding: 8px 16px;
    margin-right: 8px;
    border-radius: 4px;
    background-color: ${(p): string => p.theme.bgColor};

  .view-select{
    margin-bottom: 8px;
    flex: none;
  }
  .rank-filter{
    margin-bottom: 8px;
    flex: none;
    color: ${(p): string => p.theme.textColorSecondary};
  }
`;

const SelectContentContainer = styled.div`
    flex: 1;
    padding: 8px 16px;
    height: 100%;
    border-radius: 4px;
    overflow: hidden;
    background-color: ${(p): string => p.theme.bgColor};
    .ant-table-wrapper {
        height: 100%;
    }
`;

const AsideSelectList = styled.div`
    flex: 1;
    overflow: auto;
    & .aside-select-item {
        cursor: pointer;
        color: ${(props): string => props.theme.textColorSecondary};
        + .aside-select-item {
            margin-top: 8px;
        }
        &.selected{
            color: ${(props): string => props.theme.primaryColor};
        }
    }
`;

interface ConditionType {
    options: string[];
    value: string;
    ranks?: Map<string, string[]>;
}

export const SystemView = observer((props: any) => {
    const [viewOption, setViewOption] = useState(0);
    const [key, setKey] = useState(0);
    const SelectContent = useMemo(() => contentList[viewOption][key], [viewOption, key]);
    const [conditions, setConditions] = useState<{ rankId: string }>({ rankId: '' });
    const handleChange = (rankId: string): void => {
        setConditions({ rankId });
    };
    const handleViewChange = (value: number): void => {
        setViewOption(value);
        setKey(0);
    };
    useEffect(() => {
        if (props.session.showEvent as boolean) {
            setViewOption(2);
            setKey(0);
        }
    }, [props.session.showEvent]);
    return (<Container>
        <AsideSelectContainer>
            <ViewSelect viewOption={viewOption} handleViewChange={handleViewChange}/>
            {viewOption !== 2 && (<RankFilter session={props.session} viewOption={viewOption} handleChange={handleChange}></RankFilter>)}
            <SelectList viewOption={viewOption} selectKey={key} setKey={setKey}></SelectList>
        </AsideSelectContainer>
        <ChartErrorBoundary>
            <SelectContentContainer>
                <SelectContent key={key} rankId={conditions.rankId} session={props.session} bottomHeight={props.bottomHeight}></SelectContent>
            </SelectContentContainer>
        </ChartErrorBoundary>
    </Container>);
});

const ViewSelect = observer((props: any) => {
    const { viewOption, handleViewChange } = props;
    const { t } = useTranslation('timeline', { keyPrefix: 'systemView' });
    const options = [{ label: t('Stats System View'), value: 0 }, { label: t('Expert System View'), value: 1 }, { label: t('Events View'), value: 2 }];
    return (
        <div className={'view-select'}>
            <Select width={'100%'} value={viewOption} onChange={handleViewChange} options={options}/>
        </div>
    );
});

// eslint-disable-next-line max-lines-per-function
export const RankFilter = observer((props: any): JSX.Element => {
    const [rankIdCondition, setRankIdCondition] = useState<ConditionType>({ options: [], value: '' });
    const [hostCondition, setHostCondition] = useState<ConditionType>({ options: [], value: '' });
    const { t } = useTranslation('timeline');
    useEffect(() => {
        const rankList: any[] = [];
        for (const unit of props.session.units) {
            const cardId = (unit.metadata as CardMetaData).cardId;
            if (!cardId.endsWith('Host')) {
                rankList.push(cardId);
            }
        }
        const { hosts, ranks }: { hosts: string[]; ranks: Map<string, string[]> } = GroupRankIdsByHost(rankList);
        setHostCondition({ options: hosts, value: hosts[0] ?? '', ranks });
    }, [props.session.units]);

    useEffect(() => {
        const rankIdOptions = hostCondition.ranks?.get(hostCondition.value) ?? [];
        setRankIdCondition({ options: rankIdOptions, value: rankIdOptions[0] ?? '' });
    }, [hostCondition]);

    useEffect(() => {
        props.handleChange(rankIdCondition.value);
    }, [rankIdCondition]);
    useEffect(() => {
        limitInput();
    }, []);
    const onRankIdChanged = (value: string): void => {
        setRankIdCondition({ ...rankIdCondition, value });
    };
    return (<div className={'rank-filter'} >
        {hostCondition.options.length > 0
            ? <FormItem label={t('Host')} contentStyle={{ flex: 1 }}>
                <Select
                    value={hostCondition.value}
                    width={'100%'}
                    onChange={(value: string): void => setHostCondition({ ...hostCondition, value })}
                    options={hostCondition.options.map((host) => ({ value: host, label: host }))}
                />
            </FormItem>
            : <></>
        }
        <FormItem label={t('Rank ID')} contentStyle={{ flex: 1 }}>
            <Select
                value={rankIdCondition.value}
                width={'100%'}
                onChange={onRankIdChanged}
                options={rankIdCondition.options.map((rankId) => {
                    return {
                        value: rankId,
                        label: rankId.replace(`${hostCondition.value} `, ''),
                    };
                })}
                showSearch={true}
            />
        </FormItem>
    </div>);
});

const SelectList = observer((props: any) => {
    const [selectedKey, setSelectedKey] = useState(0);
    const { t } = useTranslation('timeline', { keyPrefix: 'systemView' });
    const handleClick = (key: number): void => {
        props.setKey(key);
        setSelectedKey(key);
    };
    useEffect(() => {
        setSelectedKey(props.selectKey);
    }, [props.selectKey]);
    let systemViewItems: string[] = [];
    switch (props.viewOption) {
        case 0:
            systemViewItems = statsSystemViewItems;
            break;
        case 1:
            systemViewItems = expertSystemViewItems;
            break;
        case 2:
            break;
        default:
            break;
    }
    return (<AsideSelectList>
        {
            systemViewItems.map((item, index) =>
                (<div
                    className={`aside-select-item ${selectedKey === index ? 'selected' : ''}`}
                    key={index}
                    onClick={(): void => handleClick(index)}
                >
                    {t(item)}
                </div>
                ))
        }
    </AsideSelectList>
    );
});

const handleAdvisorSelected = async(rowData: any, props: any): Promise<void> => {
    const queryName = rowData.name ?? rowData.originOptimizer;
    const nsDuration = Number((rowData.duration * 1000).toFixed(0));
    const res = await queryOneKernel({
        rankId: rowData.rankId,
        name: queryName,
        timestamp: rowData.startTime,
        duration: nsDuration,
    });
    const depth = rowData.depth > res.depth ? rowData.depth : res.depth;
    runInAction(() => {
        props.session.locateUnit = {
            target: (unit: any): boolean => {
                return unit instanceof ThreadUnit && unit.metadata.cardId === rowData.rankId &&
                    unit.metadata.threadId === rowData.tid && unit.metadata.processId === rowData.pid;
            },
            onSuccess: (unit: InsightUnit): void => {
                const startTime = rowData.startTime - getTimeOffset(props.session, (unit.metadata as ThreadMetaData).cardId);
                const [rangeStart, rangeEnd] = calculateDomainRange(props.session, startTime, nsDuration);
                props.session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                props.session.selectedData = {
                    id: res.id,
                    startTime,
                    name: queryName,
                    color: colorPalette[hashToNumber(queryName, colorPalette.length)],
                    duration: nsDuration,
                    metaType: rowData.pid,
                    threadId: rowData.tid,
                    startRecordTime: props.session.startRecordTime,
                    showDetail: false,
                    depth,
                };
            },
        };
    });
};

// eslint-disable-next-line max-lines-per-function
const BaseSummary = observer((props: any) => {
    const isStats = props.isStats as boolean;
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = isStats ? { field: 'totalTime', order: 'descend' } : { field: 'duration', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [isLoading, setLoading] = useState(false);
    const [searchText, setSearchText] = useState('');
    const [searchedColumn, setSearchedColumn] = useState('');
    const [rowData, setRowData] = useState<any>({});
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });
    const status = props.session.units.find((unit: any) => (unit.metadata as CardMetaData).cardId === props.rankId)?.phase;
    let columns = props.columns?.map((col: any) => ({
        ...col,
        title: t(col.title),
    }));

    if (isStats) {
        columns = [{
            title: t('Name'),
            dataIndex: 'name',
            ...getDefaultColumData('name'),
            ...getColumnSearchProps({ dataIndex: 'name', setSearchText, searchText, setSearchedColumn, searchedColumn }),
        }, ...columns];
    } else {
        columns = [...columns, {
            title: t('Click To Timeline'),
            dataIndex: 'click',
            key: 'click',
            ellipsis: true,
            render: (_: any, record: any): JSX.Element => (<Button type="link"
                onClick={(): void => {
                    setRowData({ name: record.name ?? record.originOptimizer, ...record });
                }}>{t('Click')}</Button>),
        }];
    }
    const updateData = async(searchName: string, pages: any, sorters: {field: string;order: string}, prop: any): Promise<void> => {
        const _isStats = prop.isStats as boolean;
        if (props.rankId === undefined || props.rankId === '') {
            setDataSource([]);
            setPage(defaultPage);
            return;
        }
        setLoading(true);
        let params: IQueryCondition = {
            rankId: prop.rankId,
            pageSize: pages.pageSize,
            current: pages.current,
            orderBy: sorters.field === 'startTimeLabel' ? 'startTime' : sorters.field ?? defaultSorter.field,
            order: sorters.order ?? defaultSorter.order,
        };
        if (_isStats) {
            params = { isQueryTotal: true, layer: prop.layerType, searchName, ...params };
        }
        const res = await props.request(params);
        setLoading(false);
        if (_isStats) {
            setDataSource(res.systemViewDetails);
        } else {
            const timestampoffset = getTimeOffset(props.session, props.rankId);
            const data = res.data.map((item: any) => {
                item.startTimeLabel = getDetailTimeDisplay(item.startTime - timestampoffset);
                return item;
            });
            setDataSource(data);
        }
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
    useEffect(() => {
        if (rowData.name === null || rowData.name === undefined) {
            return;
        }
        handleAdvisorSelected(rowData, props);
    }, [rowData]);

    return (
        (status === 'download' || props.rankId === undefined)
            ? <ResizeTable
                onChange={(pagination: any, filters: any, nwSorter: any): void => {
                    setSorter(nwSorter);
                }}
                rowClassName={(record: any): string => {
                    return record.id !== undefined && record.id === rowData.id ? 'selected-row' : 'click-able';
                }}
                pagination={getPageData(page, setPage)}
                dataSource={dataSource}
                columns={columns}
                size="small"
                scroll={{ y: props.bottomHeight - DETAIL_HEADER_HEIGHT_ETC_PX }}
                loading = {isLoading}/>
            : <div style={{ display: 'flex', height: '100%' }}>
                <StyledEmpty style={{ margin: 'auto' }}/>
            </div>
    );
});

const handleSelected = async(rowData: any, props: any): Promise<void> => {
    const res = await queryOneKernel({
        rankId: props.rankId,
        name: rowData.name,
        timestamp: rowData.startTime,
        duration: Number((rowData.duration * 1000).toFixed(0)),
    });
    runInAction(() => {
        props.session.locateUnit = {
            target: (unit: any): boolean => {
                return unit instanceof ThreadUnit && unit.metadata.cardId === props.rankId &&
                    unit.metadata.threadId === res.threadId && unit.metadata.processId === res.pid;
            },
            onSuccess: (unit: InsightUnit): void => {
                const startTime = rowData.startTime - getTimeOffset(props.session, (unit.metadata as ThreadMetaData).cardId);
                // 此处duration单位为us,计算和跳转时需要转换为ns
                const [rangeStart, rangeEnd] = calculateDomainRange(props.session, startTime, Number((rowData.duration * 1000).toFixed(0)));
                props.session.domainRange = { domainStart: rangeStart, domainEnd: rangeEnd };
                props.session.selectedData = {
                    id: res.id,
                    startTime,
                    name: rowData.name,
                    color: colorPalette[hashToNumber(rowData.name, colorPalette.length)],
                    duration: Number((rowData.duration * 1000).toFixed(0)),
                    depth: res.depth,
                    threadId: res.threadId,
                    startRecordTime: props.session.startRecordTime,
                    metaType: res.pid,
                    showDetail: false,
                };
            },
        };
    });
};

const defaultFilters = {
    name: [],
    type: [],
    accCore: [],
    inputShapes: [],
    inputDataTypes: [],
    inputFormats: [],
    outputShapes: [],
    outputDataTypes: [],
    outputFormats: [],
};

const filterColumn = [
    'name', 'type', 'acceleratorCore', 'inputShapes', 'inputDataTypes',
    'inputFormats', 'outputShapes', 'outputDataTypes', 'outputFormats',
];

// eslint-disable-next-line max-lines-per-function
const KernelDetails = observer((props: any) => {
    const defaultPage = { current: 1, pageSize: 10, total: 0 };
    const defaultSorter = { field: 'duration', order: 'descend' };
    const [dataSource, setDataSource] = useState<any[]>([]);
    const [page, setPage] = useState(defaultPage);
    const [sorter, setSorter] = useState(defaultSorter);
    const [filters, setFilters] = useState(defaultFilters);
    const [rowData, setRowData] = useState<any>({});
    const kernelDetails = useKernelDetails();
    const { t } = useTranslation('timeline', { keyPrefix: 'tableHead' });

    const status = props.session.units.find((unit: any) => (unit.metadata as CardMetaData).cardId === props.rankId)?.phase;
    useEffect(() => {
        updateData(page, sorter, filters);
    }, [sorter, filters, props.rankId]);
    useEffect(() => {
        if (status === 'download') {
            updateData(page, sorter, filters);
        }
    }, [status]);
    const updateData = async(pages: any, sorters: {field: string;order: string}, filtersConditions: any): Promise<void> => {
        if (props.rankId === undefined) {
            setDataSource([]);
            setPage(defaultPage);
            return;
        }
        const filterTypes: string[] = [];
        Object.keys(filtersConditions).forEach(key => {
            const filterValue = filtersConditions[key];
            if (filterColumn.includes(key) && filterValue != null) {
                if (Array.isArray((filterValue)) && filterValue.length > 0) {
                    filterTypes.push(JSON.stringify({ columnName: key, value: filterValue[0] }));
                }
            }
        });
        const res = await queryKernelDetails({
            rankId: props.rankId,
            pageSize: pages.pageSize,
            current: pages.current,
            orderBy: sorters.field === 'startTimeLabel' ? 'startTime' : sorters.field ?? defaultSorter.field,
            order: sorters.order ?? defaultSorter.order,
            coreType: '',
            filterCondition: filterTypes,
        });
        const timestampoffset = getTimeOffset(props.session, props.rankId);
        const data = res.kernelDetails.map((item: {
            startTimeLabel: string;
            startTime: number;}) => {
            item.startTimeLabel = getDetailTimeDisplay(item.startTime - timestampoffset);
            return item;
        });
        setDataSource(data);
        setPage({ ...page, total: res.count });
    };

    const colums = [
        ...kernelDetails,
        {
            title: t('Click To Timeline'),
            dataIndex: 'click',
            key: 'click',
            ellipsis: true,
            render: (_: any, record: any): JSX.Element => (<Button type="link"
                onClick={(): void => {
                    setRowData(record as any);
                }}>{t('Click')}</Button>),
        },
    ];

    useEffect(() => {
        if (rowData.name === null || rowData.name === undefined) {
            return;
        }
        handleSelected(rowData, props);
    }, [rowData]);
    return (
        (status === 'download' || props.rankId === undefined)
            ? <ResizeTable
                onChange={(pagination: any, newFilters: any, newSorter: any): void => {
                    setSorter(newSorter);
                    setFilters(newFilters);
                }}
                pagination={getPageData(page, setPage)}
                dataSource={dataSource}
                columns={colums}
                scroll={{ y: props.bottomHeight - DETAIL_HEADER_HEIGHT_ETC_PX }}
                rowClassName={(record: any): string => {
                    return record.id === rowData.id ? 'selected-row' : 'click-able';
                }}
                size="small"/>
            : <div style={{ display: 'flex', height: '100%' }}>
                <StyledEmpty style={{ margin: 'auto' }}/>
            </div>
    );
});

const AffinityAPI = observer((props: any) => {
    return <BaseSummary request={queryAffinityAPI} columns={affinityAPIColumns} {...props} />;
});

const AffinityOptimizer = observer((props: any) => {
    return <BaseSummary request={queryAffinityOptimizer} columns={affinityOptimizerColumns} {...props} />;
});

const AICPUOperator = observer((props: any) => {
    return <BaseSummary request={queryAICPUOperators} columns={aicpuOperatorColumns} {...props} />;
});

const ACLNNOperator = observer((props: any) => {
    return <BaseSummary request={queryACLNNOperators} columns={aclnnOperatorColumns} {...props} />;
});

const FusedOperator = observer((props: any) => {
    return <BaseSummary request={queryOperatorFusion} columns={fusionOperatorColumns} {...props} />;
});

const EventView = observer((props: any) => {
    return <EventDetail request={eventViewData} {...props} />;
});

const contentList: any[][] = [[...layerTypes.map((type) => {
    return observer((props: any) => {
        return (
            <BaseSummary
                layerType={type}
                request={querySystemViewDetails}
                isStats={true}
                columns={pythonApiSummaryColumns}
                {...props}
            />
        );
    });
}), KernelDetails], [AffinityAPI, AffinityOptimizer, AICPUOperator, ACLNNOperator, FusedOperator], [EventView]];
