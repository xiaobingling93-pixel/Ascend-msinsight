/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useCallback, useEffect, useState, useMemo } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { Checkbox } from 'ascend-components';
import type { ColumnsType } from 'antd/es/table';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import './HotMethod.css';
import type { Session } from '../../entity/session';
import Filter from './Filter';
import CodeViewer from './codeViewer/CodeViewer';
import { ResizeTable } from 'ascend-resize';
import {
    HeaderFixedContainer,
    LeftRightContainer,
    syncScroller,
    limitInput,
    GetPageConfigWhithPageData,
} from '../Common';
import type { InstrsColumnType, Iline, Ilinetable, JsonInstructionType } from './defs';
import { queryDynamicInstr, queryDynamicLine, querySourceCode } from '../RequestUtils';
import { Layout } from 'ascend-layout';
import { getInstrColumns } from './InstructionTable';
import { getCodeColumns } from './CodeAttrTable';
import TableHead, { type Col } from './TableHead';
import { FieldType } from './defs';

const BREAK_LINE_REGEXP = /\r\n|\r|\n/g;
const MAX_FILE_SIZE = 1000000; // 100,0000
const MAX_LINE_LENGTH = 10000; // 10000
const MAX_LINE = 10000;
const MAX_INSTRUCTION = 1000000; // 100,0000
const PAGE_LIMIT = 500000; // 50,0000
const ROW_HEIGHT = 32;

interface ConditionType {
    core: string;
    source: string;
    onlyRelated?: boolean;
};

const useSourceColumns = (): Col[] => {
    const { t } = useTranslation('source');
    return [{ width: 45, textAlign: 'right', name: '#' }, { name: t('Source') }];
};
const isRelated = (codeline: Ilinetable, instr: InstrsColumnType): boolean => {
    // 指令地址是否在代码行地址范围内
    return Boolean(codeline?.['Address Range']?.find(item => Number(item[0]) <= Number(instr.Address) && Number(item[1]) >= Number(instr.Address)));
};

// eslint-disable-next-line max-lines-per-function
const Index = observer(({ session }: { session: Session }) => {
    const domId = 'hotMethod';
    const { t } = useTranslation('source');
    const [condition, setCondition] = useState<ConditionType>({ core: '', source: '', onlyRelated: false });
    const [code, setCode] = useState('');
    const [codeLines, setCodeLines] = useState<Ilinetable[]>([]);
    const [loggedCodeLines, setLoggedCodeLines] = useState<Ilinetable[]>([]);
    const [dynamicCodeFields, setDynamicCodeFields] = useState<Record<string, FieldType>>({});
    const codeColumns = useMemo(() => getCodeColumns(t, dynamicCodeFields), [dynamicCodeFields, t]);
    // 选中代码行
    const [selectedline, setSelectedline] = useState<number>(-1);
    // 指令表
    const [dynamicInstrFields, setDynamicInstrFields] = useState<Record<string, FieldType>>({});
    const [allInstrData, setAllInstrData] = useState<InstrsColumnType[]>([]);
    // 是否关联指令
    const isRelatedInstr = useCallback((instr: InstrsColumnType): boolean => {
        if (selectedline > 0 && codeLines.length > 0) {
            return isRelated(codeLines[selectedline - 1], instr);
        }
        return false;
    }, [selectedline, codeLines]);
    const getRelatedInstrs = useCallback((): InstrsColumnType[] => {
        return allInstrData.filter((record: InstrsColumnType) => isRelatedInstr(record));
    }, [allInstrData, isRelatedInstr]);
    // 指令表当前显示数据
    const curInstrData = useMemo(() => condition.onlyRelated ? getRelatedInstrs() : allInstrData, [allInstrData, condition.onlyRelated, getRelatedInstrs]);
    const instrColumns = useMemo(() => getInstrColumns(dynamicInstrFields, t, curInstrData), [dynamicInstrFields, t, curInstrData]);
    const [lineClickListener, setLineClickListener] = useState<number>(0);
    const [tableHeight, setTableHeight] = useState<number>(1000);
    const [instrLimit, setInstrLimit] = useState({ maxSize: MAX_INSTRUCTION, overlimit: false, current: 0 });
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

    function resizeHeight(): void {
        const codeTable = document.getElementById('CodeTable');
        const height = codeTable?.clientHeight ?? 1000;
        if (height === 0) {
            return;
        }
        setTableHeight(height);
    }

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

    async function getInstrs(coreName: string): Promise<{instructions: InstrsColumnType[] ;fields: Record<string, FieldType>}> {
        if (coreName === '') {
            return { instructions: [], fields: {} };
        }
        const res = await queryDynamicInstr({ coreName });
        // 动态列
        const fields = res?.['Instructions Dtype']?.Instructions ?? {};
        // 指令记录
        const records = res?.Instructions ?? [];
        setInstrLimit({ ...instrLimit, overlimit: records.length > instrLimit.maxSize });
        const list: InstrsColumnType[] = records.map((item: JsonInstructionType, index: number) => {
            return {
                ...item,
                index: index + 1,
                maxCycles: 0,
            };
        });
        let maxCycles = 1;
        list.forEach(item => {
            if (!isNaN(Number(item.Cycles))) {
                maxCycles = Math.max(maxCycles, Number(item.Cycles));
            }
        });
        list.forEach(item => {
            item.maxCycles = maxCycles;
        });
        return { instructions: list, fields };
    };
    async function getLines(source: string, core: string): Promise<{ lines: Ilinetable[];fields: Record<string, FieldType> }> {
        if (source === '' || core === '') {
            return { lines: [], fields: {} };
        }
        const res = await queryDynamicLine({ sourceName: source, coreName: core });
        const fields = res?.['Files Dtype']?.Lines ?? {};
        const list: Iline[] = res?.Lines ?? [];
        return { lines: list.reverse(), fields };
    };

    function clear(): void {
        // 文件源码
        setCode('');
        // 代码行记录
        setLoggedCodeLines([]);
        setDynamicCodeFields({});
        // 选中行
        setCodeLines([]);
        // 指令记录
        setAllInstrData([]);
        setDynamicInstrFields({});
    }

    async function updateData(): Promise<void> {
        Promise.all([
            getCode(condition.source),
            getLines(condition.source, condition.core),
        ]).then(([newCode, { lines: newLoggedCodeLines, fields }]) => {
            // 文件源码
            setCode(newCode);
            // 代码行动态列
            setDynamicCodeFields(fields);
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
        getInstrs(condition.core).then(({ instructions: newInstrlist, fields }) => {
            setDynamicInstrFields(fields);
            setAllInstrData(newInstrlist);
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
        updateCode();
    }, [t]);

    return <div id={domId} style={{ height: '100%', width: '100%' }} className={'th35'}>
        <Layout>
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
                        flex
                        left={<div style={{ height: '100%', width: '100%', overflow: 'auto', paddingRight: '8px' }}>
                            <LeftRightContainer
                                flex
                                leftPercent={70}
                                left={
                                    <HeaderFixedContainer
                                        style={{ overflow: 'hidden' }}
                                        header={<TableHead columns={useSourceColumns()}/>}
                                        bodyProps={{ id: 'CodeTable' }}
                                        bodyStyle={{ overflowX: 'scroll', marginRight: '-8px' }}
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
                                        columns={codeColumns}
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
                                style={{ paddingLeft: '8px' }}
                                body={<InstructionTable
                                    tableHeight={tableHeight}
                                    columns={instrColumns}
                                    condition={condition}
                                    dataSource={curInstrData}
                                    isRelatedInstr={isRelatedInstr}
                                    handleInstrsClick={handleInstrsClick}
                                    selectedline={selectedline}
                                    lineClickListener={lineClickListener}
                                    isShowPage ={curInstrData.length > PAGE_LIMIT}
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
    const [filters, setFilters] = useState({});
    const [sorter, setSorter] = useState({});

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
        onChange={(pagination, newFilters, newSorter, extra): void => {
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
    const [filters, setFilters] = useState({});
    const [sorter, setSorter] = useState({});
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
        onChange={(pagination, newFilters, newSorter, extra): void => {
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
