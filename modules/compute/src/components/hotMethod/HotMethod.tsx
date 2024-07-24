/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useCallback, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { Checkbox, Tooltip } from 'antd';
import type { ColumnsType, ColumnType } from 'antd/es/table';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import './HotMethod.css';
import type { Session } from '../../entity/session';
import Filter from './Filter';
import CodeViewer from './codeViewer/CodeViewer';
import ResizeTable from 'lib/ResizeTable';
import Bar from './Bar';
import {
    HeaderFixedContainer,
    LeftRightContainer,
    syncScroller,
    limitInput,
    GetPageConfigWhithPageData,
} from '../Common';
import type { InstrsColumnType, Iline, Ilinetable, JsonInstructionType } from './defs';
import { queryApiInstr, queryApiLine, querySourceCode } from '../RequestUtils';
import { runInAction } from 'mobx';
import Layout from 'lib/Layout';

const BREAK_LINE_REGEXP = /\r\n|\r|\n/g;
const MAX_FILE_SIZE = 1000000; // 100,0000
const MAX_LINE_LENGTH = 10000; // 10000
const MAX_LINE = 10000;
const MAX_INSTRUCTION = 1000000; // 100,0000
const PAGE_LIMIT = 500000; // 50,0000
const ROW_HEIGHT = 29;

interface ConditionType {
    core: string;
    source: string;
    onlyRelated?: boolean;
};

const useCodeColumns = (): ColumnsType<Ilinetable> => {
    const { t } = useTranslation('source');
    return [{
        title: t('InstructionsExecuted'),
        dataIndex: 'Instructions Executed',
        ellipsis: true,
        width: 155,
    },
    {
        title: t('Cycles'),
        dataIndex: 'Cycles',
        ellipsis: true,
        width: 50,
    }];
};

const useInstrsColumns = (): ColumnsType<InstrsColumnType> => {
    const { t } = useTranslation('source');
    return [
        { title: '#', dataIndex: 'index', width: 40, align: 'right', ellipsis: true },
        {
            title: t('Address'),
            dataIndex: 'Address',
            width: 100,
            ellipsis: true,
        },
        {
            title: t('Pipe'),
            dataIndex: 'Pipe',
            width: 100,
            ellipsis: true,
        },
        {
            title: t('Source'),
            dataIndex: 'Source',
            ellipsis: { showTitle: false },
            render: source => (
                <Tooltip placement="topLeft" title={source} >
                    {source}
                </Tooltip>
            ),
        },
        {
            title: t('InstructionsExecuted'),
            dataIndex: 'Instructions Executed',
            ellipsis: true,
            width: 165,
        },
        {
            title: t('Cycles'),
            dataIndex: 'Cycles',
            width: 150,
            ellipsis: true,
            sorter: true,
            render: (cycles, record): string | React.ReactElement => {
                if (cycles === '') {
                    return '';
                }
                return <Bar value={cycles} max={record.maxCycles ?? cycles}/>;
            },
        },
    ];
};

// eslint-disable-next-line max-lines-per-function
const Index = observer(({ session }: { session: Session }) => {
    const domId = 'hotMethod';
    const [condition, setCondition] = useState<ConditionType>({ core: '', source: '', onlyRelated: false });
    const [code, setCode] = useState('');
    const [codeLines, setCodeLines] = useState<Ilinetable[]>([]);
    const [loggedCodeLines, setLoggedCodeLines] = useState<Ilinetable[]>([]);
    const [instrsData, setInstrsData] = useState<InstrsColumnType[]>([]);
    const [selectedline, setSelectedline] = useState<number>(-1);
    const [lineClickListener, setLineClickListener] = useState<number>(0);
    const [tableHeight, setTableHeight] = useState<number>(1000);
    const instrsColumns = useInstrsColumns();
    const [filterInstrsColumns, setFilterInstrsColumns] = useState<ColumnsType<InstrsColumnType>>(instrsColumns);
    const [doneQuery, setDoneQuery] = useState(false);
    const [instrLimit, setInstrLimit] = useState({ maxSize: MAX_INSTRUCTION, overlimit: false, current: 0 });
    const { t } = useTranslation('source');
    const reset = (): void => {
        // 重置选中行数，-1不选中任一行
        setSelectedline(-1);
    };
    const handleFilterChange = (newConditions: ConditionType): void => {
        setCondition({ ...condition, ...newConditions });
    };

    const handleInstrsClick = (instr: InstrsColumnType): void => {
        const data = loggedCodeLines.find((codeline: Ilinetable) => isRelated(codeline, instr));
        setSelectedline(data?.Line ?? -1);
    };

    const getRelatedInstrs = (): InstrsColumnType[] => {
        return instrsData.filter((record: InstrsColumnType) => isRelatedInstr(record));
    };

    const isRelatedInstr = useCallback((instr: InstrsColumnType): boolean => {
        if (selectedline > 0 && codeLines.length > 0) {
            return isRelated(codeLines[selectedline - 1], instr);
        }
        return false;
    }, [selectedline, codeLines]);
    const isRelated = (codeline: Ilinetable, instr: InstrsColumnType): boolean => {
        // 指令地址是否在代码行地址范围内
        return Boolean(codeline?.['Address Range']?.find(item => Number(item[0]) <= Number(instr.Address) && Number(item[1]) >= Number(instr.Address)));
    };

    const getCurInstrsData = (): InstrsColumnType[] => {
        return condition.onlyRelated ? getRelatedInstrs() : instrsData;
    };

    function resizeHeight(): void {
        const codeTable = document.getElementById('CodeTable');
        const height = codeTable?.clientHeight ?? 1000;
        if (height === 0) {
            return;
        }
        setTableHeight(height);
    }

    const updateInstrsColumns = (): void => {
        const newColumns: ColumnsType<InstrsColumnType> = [...instrsColumns];
        const fields = ['Address', 'Pipe', 'Source', 'Instructions Executed', 'Cycles'];
        newColumns.forEach((col: ColumnType<InstrsColumnType>) => {
            if (fields.includes(String(col.dataIndex))) {
                const items = [...new Set(getCurInstrsData().map(item => item[col.dataIndex as keyof InstrsColumnType]))];
                const filters = items.map(item => ({
                    text: item,
                    value: item,
                }));
                Object.assign(col, {
                    filters,
                    filterMode: 'menu',
                    filterSearch: true,
                    onFilterDropdownOpenChange: (open: boolean) => {
                        if (open) {
                            limitInput();
                        }
                    },
                });
            }
        });
        setFilterInstrsColumns(newColumns);
    };

    async function getCode(source: string): Promise<string> {
        if (source === '') {
            return '';
        }
        const res = await querySourceCode(source);
        let str: string = res?.fileContent ?? '';
        let linebreak;
        if (str.includes('\r\n')) {
            linebreak = '\r\n';
        } else if (str.includes('\r')) {
            linebreak = '\r';
        } else if (str.includes('\n')) {
            linebreak = '\n';
        } else {
            linebreak = '';
        }
        if (str.length > MAX_FILE_SIZE) {
            str = str.slice(0, MAX_FILE_SIZE);
            str += `${linebreak}----------【${t('CharactersExceed', { max: MAX_FILE_SIZE })}】----------`;
        }
        if (str.length > 0) {
            let splitlines: string[] = str.split(BREAK_LINE_REGEXP);
            if (splitlines.length > MAX_LINE) {
                splitlines = splitlines.slice(0, MAX_LINE);
                splitlines.push(`----------【${t('LineExceed', { max: MAX_LINE })}】----------`);
            }
            splitlines = splitlines.map(codeline => {
                if (codeline.length > MAX_LINE_LENGTH) {
                    return `${codeline.slice(0, MAX_LINE_LENGTH)} 【${t('Exceed', { max: MAX_LINE_LENGTH })}】`;
                } else {
                    return codeline;
                }
            });
            return splitlines.join(linebreak);
        }
        return str;
    }

    async function getInstrs(core: string): Promise<InstrsColumnType[]> {
        if (session.parseStatus && !doneQuery) {
            const res = await queryApiInstr();
            if (res === undefined || res === null || res.instructions === '') {
                return [];
            }
            let obj;
            try {
                obj = JSON.parse(res.instructions);
            } catch (err) {
                return [];
            }
            let list = obj.Instructions ?? [];
            setInstrLimit({ ...instrLimit, overlimit: list.length > instrLimit.maxSize });
            if (list.length > MAX_INSTRUCTION) {
                list = list.slice(0, MAX_INSTRUCTION);
            }
            runInAction(() => {
                session.instructions = list;
            });
            setDoneQuery(true);
        }
        const records = session.instructions;
        const coreIndex = session.coreList.findIndex(item => item === core);
        const list = records.map((item: JsonInstructionType, index: number) => ({
            ...item,
            Cycles: item.Cycles?.[coreIndex] ?? '',
            'Instructions Executed': item['Instructions Executed']?.[coreIndex] ?? '',
            index: index + 1,
            maxCycles: 0,
        }));
        let maxCycles = 1;
        list.forEach(item => {
            if (!isNaN(Number(item.Cycles))) {
                maxCycles = Math.max(maxCycles, Number(item.Cycles));
            }
        });
        list.forEach(item => {
            item.maxCycles = maxCycles;
        });
        return list;
    };
    async function getLines(source: string, core: string): Promise<Ilinetable[]> {
        if (source === '' || core === '') {
            return [];
        }
        const res = await queryApiLine({ sourceName: source, coreName: core });
        const records: Iline[] = res?.lines ?? [];
        const list = records.map((item: Iline, index: number) => ({
            ...item,
            Cycles: item.Cycle,
            'Instructions Executed': item['Instruction Executed'],
        }));
        return list.reverse();
    };

    function clear(): void {
        // 文件源码
        setCode('');
        // 指令记录
        setInstrsData([]);
        // 代码行记录
        setLoggedCodeLines([]);
        setCodeLines([]);
        setDoneQuery(false);
    }

    async function updateData(): Promise<void> {
        Promise.all([
            getCode(condition.source),
            getLines(condition.source, condition.core),
        ]).then(([newCode, newLoggedCodeLines]) => {
            // 文件源码
            setCode(newCode);
            // 代码行记录
            setLoggedCodeLines(newLoggedCodeLines);
            // 全部代码行
            const sourceCodeList = newCode === '' ? [] : newCode.split(BREAK_LINE_REGEXP);
            const sourceCodeLines = sourceCodeList.map((codeItem: string, index: number) => {
                const line = index + 1;
                const lineInfo = newLoggedCodeLines.find((item: Ilinetable) => item.Line === line) ?? {};
                return { Line: line, ...lineInfo };
            });
            setCodeLines(sourceCodeLines);
        });
        // 指令记录
        getInstrs(condition.core).then((newInstrlist) => {
            setInstrsData(newInstrlist);
        });
    }

    function updateCode(): void {
        getCode(condition.source).then(newCode => {
            // 文件源码
            setCode(newCode);
        });
    }
    // 初始化
    useEffect(() => {
        reset();
        // 同步滚动条
        syncScroller(document.getElementById('CodeTable'),
            document.querySelector('#CodeAttrTable .ant-table-body'));
        resizeHeight();

        window.addEventListener('resize', event => {
            resizeHeight();
        });
        limitInput();
    }, []);
    useEffect(() => {
        reset();
        if (!session.parseStatus) {
            clear();
            return;
        }
        updateData();
    }, [condition.core, condition.source, session.parseStatus, session.updateId]);

    useEffect(() => {
        resizeHeight();
    }, [code]);

    useEffect(() => {
        updateInstrsColumns();
    }, [instrsData, t]);
    useEffect(() => {
        updateCode();
    }, [t]);

    return <div id={domId} style={{ height: '100%', width: '100%' }} className={'th35'}>
        <Layout padding={0}>
            <HeaderFixedContainer
                headerStyle={{ padding: '10px' }}
                header={
                    <>
                        <Filter session={session} handleFilterChange={handleFilterChange}/>
                        <LeftRightContainer right={
                            <>
                                <div className="hit-label">
                                    <span>
                                        {t('Line')} :
                                        <span>
                                            {selectedline >= 0 ? selectedline : ''},
                                        </span>
                                        {t('RelatedInstructionsCount')} :
                                        <span>
                                            {getRelatedInstrs().length}
                                        </span>
                                    </span>
                                    <Checkbox
                                        style={{ float: 'right' }}
                                        checked={condition.onlyRelated}
                                        onChange={(e: CheckboxChangeEvent): void => {
                                            handleFilterChange({ ...condition, onlyRelated: e.target.checked });
                                        }}
                                    > {t('OnlyRelatedInstructions')}</Checkbox>
                                </div>
                                { instrLimit.overlimit
                                    ? (<div style={{ color: 'red', padding: '0 10px 0 20px' }} >
                                        {t('ExceedInstructions', { max: instrLimit.maxSize })}
                                    </div>)
                                    : <></> }
                            </>
                        }/>
                    </>
                }
                body={
                    <LeftRightContainer
                        left={<div style={{ height: '100%', width: '100%', overflow: 'auto' }}>
                            <LeftRightContainer
                                headerStyle={{ flex: '0 0 70%' }}
                                bodyStyle={{ flex: '0 0 30%' }}
                                left={
                                    <HeaderFixedContainer
                                        header={<div className={'table-header'}>
                                            <div style={{ width: '45px', textAlign: 'right' }}><span>#</span></div>
                                            <div><span>{t('Source')}</span></div>
                                        </div>}
                                        bodyProps={{ id: 'CodeTable' }}
                                        bodyStyle={{ overflowX: 'scroll' }}
                                        body={
                                            <CodeViewer
                                                code={code}
                                                handleLineClick={(line: number): void => {
                                                    setSelectedline(line);
                                                    setLineClickListener((lineClickListener + 1) % 100);
                                                }}
                                                selectedline={selectedline}
                                            />
                                        }
                                    />
                                }
                                rightProps={{ id: 'CodeAttrTable' }}
                                right={
                                    <ResizeTable
                                        size="small"
                                        minThWidth={50}
                                        pagination={false}
                                        columns={useCodeColumns()}
                                        dataSource={codeLines}
                                        rowClassName={(record: Ilinetable, index: number): string => (selectedline === index + 1 ? 'selected' : '')}
                                        onRow={ (record: Ilinetable): {onClick: (event: React.MouseEvent<HTMLElement>) => void} => {
                                            return {
                                                onClick: (event: React.MouseEvent<HTMLElement>): void => {
                                                    setSelectedline(record.Line);
                                                    setLineClickListener((lineClickListener + 1) % 100);
                                                },
                                            };
                                        }}
                                        scroll={{ y: tableHeight }}
                                    />
                                }/>
                        </div>}
                        right={
                            <HeaderFixedContainer
                                id={'Instructions'}
                                style={{ paddingLeft: '15px' }}
                                body={<InstructionTable
                                    tableHeight={tableHeight}
                                    columns={filterInstrsColumns}
                                    condition={condition}
                                    dataSource={condition.onlyRelated ? getRelatedInstrs() : instrsData}
                                    isRelatedInstr={isRelatedInstr}
                                    handleInstrsClick={handleInstrsClick}
                                    selectedline={selectedline}
                                    lineClickListener={lineClickListener}
                                    isShowPage ={instrsData.length > PAGE_LIMIT}
                                />}
                            />
                        }
                    />
                }
            />
        </Layout>
    </div>;
});

interface IinstrProp {
    condition: ConditionType;
    columns: ColumnsType<InstrsColumnType>;
    dataSource: InstrsColumnType[];
    isRelatedInstr: (instr: InstrsColumnType) => boolean;
    handleInstrsClick: (instr: InstrsColumnType) => void;
    tableHeight: number;
    selectedline: number;
    lineClickListener: number;
    isShowPage?: boolean;
}

const srcollToView = ({ condition, selectedline, showDataSource, isRelatedInstr }:
{
    condition: ConditionType;
    selectedline: number;
    showDataSource: InstrsColumnType[];
    isRelatedInstr: (instr: InstrsColumnType) => boolean;
},
): void => {
    setTimeout(() => {
        if (condition.onlyRelated === true || selectedline < 0) {
            return;
        }
        const index = showDataSource.findIndex(isRelatedInstr);
        if (index < 0) {
            return;
        }
        const top = index * ROW_HEIGHT;
        const parentNode = document.querySelector('#Instructions .ant-table-body') as HTMLElement;
        parentNode.scrollTo({ top });
    });
};

function InstructionTable({
    columns, dataSource, isRelatedInstr, handleInstrsClick, tableHeight, selectedline, condition, lineClickListener, isShowPage,
}: IinstrProp): JSX.Element {
    return isShowPage
        ? <InstructionTablePage
            tableHeight={tableHeight}
            columns={columns}
            condition={condition}
            dataSource={dataSource}
            isRelatedInstr={isRelatedInstr}
            handleInstrsClick={handleInstrsClick}
            selectedline={selectedline}
            lineClickListener={lineClickListener}
        />
        : <InstructionTableNopage
            tableHeight={tableHeight}
            columns={columns}
            condition={condition}
            dataSource={dataSource}
            isRelatedInstr={isRelatedInstr}
            handleInstrsClick={handleInstrsClick}
            selectedline={selectedline}
            lineClickListener={lineClickListener}/>;
};

const getShowData = (dataSource: InstrsColumnType[], filters: Record<string, any[]>, sorter: Record<string, any>): InstrsColumnType[] => {
    let newDataSource = [...dataSource];
    // 筛选
    newDataSource = filterData(newDataSource, filters);
    // 排序
    newDataSource = sortData(newDataSource, sorter);
    return newDataSource;
};

const filterData = (dataSource: InstrsColumnType[], filters: Record<string, any[]>): InstrsColumnType[] => {
    const fields = Object.keys(filters).filter(field => filters[field] !== null);
    if (fields.length === 0) {
        return dataSource;
    }
    return dataSource.filter(row => {
        let fit = true;
        for (let i = 0; i < fields.length; i++) {
            const field = fields[i];
            const value = (row as any)[field];
            fit = fit && filters[field].find((filterValue: string | number) => filterValue === value);
            if (!fit) {
                break;
            }
        }
        return fit;
    });
};

const sortData = (dataSource: InstrsColumnType[], sorter: Record<string, any>): InstrsColumnType[] => {
    if (sorter?.order === undefined || sorter?.order === null || dataSource?.length === 0) {
        return dataSource;
    }
    const sign = sorter.order === 'ascend' ? 1 : -1;
    const field: keyof InstrsColumnType = sorter.field;
    const filedType = typeof dataSource[0][field];
    return [...dataSource].sort((a, b) => {
        switch (filedType) {
            case 'number':
                return sign * (Number(a[field]) - Number(b[field]));
            default:
                return sign * String(a[field]).localeCompare(String(b[field]));
        }
    });
};
function InstructionTableNopage({
    columns, dataSource, isRelatedInstr, handleInstrsClick, tableHeight, selectedline, lineClickListener, condition,
}: IinstrProp): JSX.Element {
    const [showDataSource, setShowDataSource] = useState(dataSource);
    const [filters, setFilters] = useState<Record<string, any[]>>({});
    const [sorter, setSorter] = useState<Record<string, any>>({});

    useEffect(() => {
        setShowDataSource(getShowData(dataSource, filters, sorter));
    }, [dataSource, filters, sorter]);

    useEffect(() => {
        srcollToView({ condition, selectedline, showDataSource, isRelatedInstr });
    }, [lineClickListener, showDataSource]);

    return <ResizeTable
        size="small"
        minThWidth={50}
        columns={columns}
        dataSource={showDataSource}
        rowClassName={(record: InstrsColumnType): string => (isRelatedInstr(record) ? 'selected' : '')}
        onRow={ (record: InstrsColumnType): { onClick: () => void } => ({
            onClick: (): void => {
                handleInstrsClick(record);
            },
        })}
        pagination={false}
        scroll={{ y: tableHeight, rowHeight: ROW_HEIGHT, scrollToFirstRowOnChange: false }}
        virtual={true}
        onChange={(pagination: any, newFilters: {[p: string]: any[]}, newSorter: any, extra: any): void => {
            switch (extra.action) {
                case 'filter':
                    setFilters(newFilters);
                    break;
                case 'sort':
                    setSorter(newSorter);
                    break;
                default:
                    break;
            }
        }}
    />;
}

function InstructionTablePage({
    columns, dataSource, isRelatedInstr, handleInstrsClick, tableHeight, selectedline, lineClickListener, condition,
}: IinstrProp): JSX.Element {
    const [showDataSource, setShowDataSource] = useState<InstrsColumnType[]>([]);
    const [filters, setFilters] = useState<Record<string, any[]>>({});
    const [sorter, setSorter] = useState<Record<string, any>>({});
    const [page, setPage] = useState({ current: 1, pageSize: PAGE_LIMIT, total: dataSource.length });
    const [pageData, setPageData] = useState(showDataSource.slice((page.current - 1) * page.pageSize, page.current * page.pageSize));

    useEffect(() => {
        setShowDataSource(getShowData(dataSource, filters, sorter));
    }, [dataSource, filters, sorter]);
    useEffect(() => {
        setPage({ ...page, total: showDataSource.length });
    }, [showDataSource]);
    useEffect(() => {
        const index = showDataSource.findIndex(isRelatedInstr);
        const onPage = Number(page.pageSize) > 0 ? Math.ceil((index + 1) / page.pageSize) : 1;
        if (index > 0 && onPage !== page.current) {
            setPage({ ...page, current: onPage, total: showDataSource.length });
        }
    }, [showDataSource, selectedline, lineClickListener]);

    useEffect(() => {
        const curPageData = showDataSource.slice((page.current - 1) * page.pageSize, page.current * page.pageSize);
        setPageData(curPageData);
    }, [showDataSource, page.pageSize, page.current]);
    useEffect(() => {
        srcollToView({ condition, selectedline, showDataSource: pageData, isRelatedInstr });
    }, [lineClickListener, pageData, page.current]);

    return <ResizeTable
        size="small"
        minThWidth={50}
        columns={columns}
        dataSource={pageData}
        rowClassName={(record: InstrsColumnType): string => (isRelatedInstr(record) ? 'selected' : '')}
        onRow={ (record: InstrsColumnType): { onClick: () => void } => ({
            onClick: (): void => {
                handleInstrsClick(record);
            },
        })}
        pagination={ GetPageConfigWhithPageData(page, setPage, [PAGE_LIMIT]) }
        scroll={{ y: tableHeight - 50, rowHeight: ROW_HEIGHT }}
        virtual={true}
        onChange={(pagination: any, newFilters: {[p: string]: any[]}, newSorter: any, extra: any): void => {
            switch (extra.action) {
                case 'filter':
                    setFilters(newFilters);
                    break;
                case 'sort':
                    setSorter(newSorter);
                    break;
                default:
                    break;
            }
        }}
    />;
}

export default Index;
