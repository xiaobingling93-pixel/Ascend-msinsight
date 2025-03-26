/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useCallback, useEffect, useMemo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { observer } from 'mobx-react';
import { Checkbox } from 'ascend-components';
import type { ColumnsType } from 'antd/es/table';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import './HotMethod.css';
import type { Session } from '../../entity/session';
import { defaultCacheUnit, InstructionSelectSource } from '../../entity/session';
import Filter from './Filter';
import CodeViewer from './codeViewer/CodeViewer';
import { ResizeTable } from 'ascend-resize';
import {
    GetPageConfigWhithPageData,
    HeaderFixedContainer,
    LeftRightContainer,
    limitInput,
    syncScroller,
} from '../Common';
import type { Iline, Ilinetable, InstrsColumnType, JsonInstructionType } from './defs';
import { FieldType, InstructionVersion, ASCENDC_INNER_CODE, NOT_APPLICABLE } from './defs';
import { queryDynamicInstr, queryDynamicLine, querySourceCode } from '../RequestUtils';
import { Layout } from 'ascend-layout';
import { getInstrColumns } from './InstructionTable';
import { getCodeColumns } from './CodeAttrTable';
import { store } from '../../store';
import { runInAction } from 'mobx';
import CodeTextSearch, { CODE_SEARCH_WINDOW_HEIGHT } from './CodeTextSearch';
import { formatDecimal } from 'ascend-utils';
import { ThContainer } from './TableHead';

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

const isRelated = (instr: InstrsColumnType, range: string[][] = []): boolean => {
    // 指令地址是否在代码行地址范围内
    return Boolean(range?.find(item => Number(item[0]) <= Number(instr.Address) && Number(item[1]) >= Number(instr.Address)));
};

// 恢复默认的指令高亮来源
const recoverDefaultInstructionSource = (): void => {
    const session = store.sessionStore.activeSession;
    if (!session) {
        return;
    }
    runInAction(() => {
        session.instructionSelectSource = InstructionSelectSource.DEFAULT;
        // cache信息重置
        session.cacheUnit = defaultCacheUnit;
    });
};

const domId = 'hotMethod';
// eslint-disable-next-line max-lines-per-function
const Index = observer(({ session }: { session: Session }) => {
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
        if (session.instructionSelectSource === InstructionSelectSource.CACHE) {
            return isRelated(instr, session.cacheUnit.addressRange);
        } else {
            if (selectedline > 0 && codeLines.length > 0) {
                return isRelated(instr, codeLines[selectedline - 1]['Address Range']);
            }
        }
        return false;
    }, [selectedline, codeLines, session.instructionSelectSource, session.cacheUnit.addressRange, session.instructionUpdateId]);
    const getRelatedInstrs = useCallback((): InstrsColumnType[] => {
        return allInstrData.filter((record: InstrsColumnType) => isRelatedInstr(record));
    }, [allInstrData, isRelatedInstr]);
    const relatedInstrs = useMemo(() => getRelatedInstrs(), [getRelatedInstrs]);
    // 指令表当前显示数据
    const curInstrData = useMemo(() => condition.onlyRelated ? relatedInstrs : allInstrData, [allInstrData, condition.onlyRelated, relatedInstrs]);
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
        let line = -1;
        // 指令数据ASCENDC_INNER_CODE可用版本
        if (session.instrVersion === InstructionVersion.ASCENDC_INNER_CODE) {
            const infoList = String(instr[ASCENDC_INNER_CODE]).split(':');
            const file = infoList[0];
            const fileLine = Number(infoList[1]);
            if (file === condition.source && !isNaN(fileLine)) {
                line = fileLine;
            }
        }
        if (line <= 0) {
            const data = loggedCodeLines.find((codeline: Ilinetable) => isRelated(instr, codeline['Address Range']));
            line = data?.Line ?? -1;
        }
        setSelectedline(line);
        recoverDefaultInstructionSource();
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

    async function getInstrs(coreName: string): Promise<{ instructions: InstrsColumnType[]; fields: Record<string, FieldType> }> {
        if (coreName === '') {
            return { instructions: [], fields: {} };
        }
        const res = await queryDynamicInstr({ coreName });
        // 动态列
        const fields = res?.['Instructions Dtype']?.Instructions ?? {};
        // 指令记录
        const records = res?.Instructions ?? [];
        setInstrLimit({ ...instrLimit, overlimit: records.length > instrLimit.maxSize });
        const percentageFields = Object.keys(fields).filter(fieldName => fields[fieldName] === FieldType.PERCENTAGE);
        const list: InstrsColumnType[] = records.map((item: JsonInstructionType, index: number) => {
            const formatData: Record<string, any> = {};
            percentageFields.forEach(fieldName => {
                formatData[fieldName] = formatDecimal(item[fieldName] as number);
            });
            return {
                ...item,
                ...formatData,
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

    async function getLines(source: string, core: string): Promise<{ lines: Ilinetable[]; fields: Record<string, FieldType> }> {
        if (source === '' || core === '') {
            return { lines: [], fields: {} };
        }
        const res = await queryDynamicLine({ sourceName: source, coreName: core });
        const fields = res?.['Files Dtype']?.Lines ?? {};
        let list: Iline[] = res?.Lines ?? [];
        const percentageFields = Object.keys(fields).filter(fieldName => fields[fieldName] === FieldType.PERCENTAGE);
        if (percentageFields.length > 0) {
            list = list.map(item => {
                const formatData: Record<string, any> = {};
                percentageFields.forEach(fieldName => {
                    formatData[fieldName] = formatDecimal(item[fieldName] as number);
                });
                return { ...item, ...formatData };
            });
        }
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
        syncScroller([document.getElementById('CodeTable'),
            document.querySelector('#CodeAttrTable .ant-table-body'),
            document.querySelector('#CodeLine .ant-table-body'),
        ]);
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
    }, [code, session.openFind]);

    useEffect(() => {
        updateCode();
    }, [t]);
    // Cache跳转高亮指令
    useEffect(() => {
        if (session.instructionSelectSource === InstructionSelectSource.CACHE) {
            // 清理原来选中的代码行
            reset();
        }
    }, [session.instructionSelectSource]);

    useEffect(() => {
        if (session.instructionSelectSource === InstructionSelectSource.CACHE) {
            setLineClickListener(pre => (pre + 1) % 100);
        }
    }, [relatedInstrs]);

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
                                        {
                                            session.instructionSelectSource === InstructionSelectSource.CACHE
                                                ? <>{t('Cacheline Id')}:<span>{session.cacheUnit.cachelineId},</span></>
                                                : <>
                                                    {t('Line')} :
                                                    <span>
                                                        {selectedline >= 0 ? selectedline : ''},
                                                    </span>
                                                </>
                                        }
                                        {t('RelatedInstructionsCount')} :
                                        <span>
                                            {relatedInstrs.length}
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
                                {instrLimit.overlimit
                                    ? (<div style={{ color: 'red', padding: '0 10px 0 20px' }}>
                                        {t('ExceedInstructions', { max: instrLimit.maxSize })}
                                    </div>)
                                    : <></>}
                            </>
                        }/>
                    </>
                }
                body={
                    <LeftRightContainer
                        flex
                        left={<div style={{ height: '100%', width: '100%', overflow: 'auto', paddingRight: '8px' }}
                            className={session.openFind ? 'head-gap' : ''}>
                            <LeftRightContainer
                                flex
                                leftPercent={70}
                                left={<>
                                    <LeftRightContainer
                                        flex
                                        flexStyle={{ right: 0, width: '10px' }}
                                        leftWidth={45}
                                        minWidth={30}
                                        className={codeLines.length === 0 ? 'hiddenEmpty' : ''}
                                        left={<ResizeTable
                                            size="small"
                                            pagination={false}
                                            columns={[{ title: '#', dataIndex: 'Line', align: 'center', ellipsis: true }]}
                                            dataSource={codeLines}
                                            rowClassName={(record: Ilinetable, index: number): string => (selectedline === index + 1 ? 'selected' : '')}
                                            onRow={(record: Ilinetable): { onClick: (event: React.MouseEvent<HTMLElement>) => void } => {
                                                return {
                                                    onClick: (event: React.MouseEvent<HTMLElement>): void => {
                                                        recoverDefaultInstructionSource();
                                                        setSelectedline(record.Line);
                                                        setLineClickListener((lineClickListener + 1) % 100);
                                                    },
                                                };
                                            }}
                                            id="CodeLine"
                                            scroll={{ y: tableHeight }}
                                        />}
                                        right={<HeaderFixedContainer
                                            style={{ overflow: 'hidden' }}
                                            headerStyle={{ marginBottom: session.openFind ? `${CODE_SEARCH_WINDOW_HEIGHT}px` : '0' }}
                                            header={<ThContainer><div className="th"><span>{t('Source')}</span></div></ThContainer>}
                                            bodyProps={{ id: 'CodeTable' }}
                                            bodyStyle={{ overflowX: 'scroll', marginRight: '-8px' }}
                                            body={
                                                <CodeViewer
                                                    code={code}
                                                    handleLineClick={(line: number): void => {
                                                        recoverDefaultInstructionSource();
                                                        setSelectedline(line);
                                                        setLineClickListener((lineClickListener + 1) % 100);
                                                    }}
                                                    selectedline={selectedline}
                                                />
                                            }
                                        />}
                                    />
                                    {session.openFind ? <CodeTextSearch code={code}/> : <></>}
                                </>
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
                                        onRow={(record: Ilinetable): { onClick: (event: React.MouseEvent<HTMLElement>) => void } => {
                                            return {
                                                onClick: (event: React.MouseEvent<HTMLElement>): void => {
                                                    recoverDefaultInstructionSource();
                                                    setSelectedline(record.Line);
                                                    setLineClickListener((lineClickListener + 1) % 100);
                                                },
                                            };
                                        }}
                                        scroll={{ y: tableHeight, x: codeLines.length > 0 ? codeColumns.length * 100 : 0 }}
                                    />
                                }/>
                        </div>}
                        right={
                            <HeaderFixedContainer
                                id={'Instructions'}
                                style={{ paddingLeft: '8px' }}
                                body={<InstructionTable
                                    tableHeight={session.openFind ? tableHeight + CODE_SEARCH_WINDOW_HEIGHT : tableHeight}
                                    columns={instrColumns}
                                    condition={condition}
                                    dataSource={curInstrData}
                                    isRelatedInstr={isRelatedInstr}
                                    handleInstrsClick={handleInstrsClick}
                                    selectedline={selectedline}
                                    lineClickListener={lineClickListener}
                                    isShowPage={curInstrData.length > PAGE_LIMIT}
                                    instructionSelectSource={session.instructionSelectSource}
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
    instructionSelectSource: InstructionSelectSource;
}

const srcollToView = ({
    condition,
    selectedline,
    showDataSource,
    isRelatedInstr,
    instructionSelectSource = InstructionSelectSource.DEFAULT,
}:
{
    condition: ConditionType;
    selectedline: number;
    showDataSource: InstrsColumnType[];
    isRelatedInstr: (instr: InstrsColumnType) => boolean;
    instructionSelectSource: InstructionSelectSource;
},
): void => {
    setTimeout(() => {
        // 只显示关联代码，或默认场景下未选中代码行
        if (condition.onlyRelated === true || (selectedline < 0 && instructionSelectSource === InstructionSelectSource.DEFAULT)) {
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
    columns,
    dataSource,
    isRelatedInstr,
    handleInstrsClick,
    tableHeight,
    selectedline,
    condition,
    lineClickListener,
    isShowPage,
    instructionSelectSource,
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
            instructionSelectSource={instructionSelectSource}
        />
        : <InstructionTableNopage
            tableHeight={tableHeight}
            columns={columns}
            condition={condition}
            dataSource={dataSource}
            isRelatedInstr={isRelatedInstr}
            handleInstrsClick={handleInstrsClick}
            selectedline={selectedline}
            lineClickListener={lineClickListener}
            instructionSelectSource={instructionSelectSource}
        />;
};

const getShowData = (dataSource: InstrsColumnType[], filters: Record<string, any[]>, sorter: Record<string, any>): InstrsColumnType[] => {
    let newDataSource = [...dataSource];
    // 筛选
    newDataSource = filterData(newDataSource, filters);
    // 排序
    newDataSource = sortData(newDataSource, sorter);
    return newDataSource;
};

const isMatchfFilter = (filterValue: any, value: any): boolean => {
    if (!isNaN(Number(value))) {
        // 是数字的时候，筛选条件是 NA，选小于0的数字
        return filterValue === NOT_APPLICABLE ? value < 0 : filterValue === value;
    }
    return filterValue === value;
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
            fit = fit && filters[field].some((filterValue: string | number) => {
                return isMatchfFilter(filterValue, value);
            });
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
    return [...dataSource].sort((a, b) => {
        const aType = isNaN(Number(a[field])) ? typeof a[field] : 'number';
        const bType = isNaN(Number(b[field])) ? typeof b[field] : 'number';
        // 如果都是负数，不修改顺序
        const isAllNegativeNumber = aType === 'number' && bType === 'number' && a[field] < 0 && b[field] < 0;
        if (isAllNegativeNumber) {
            return -1;
        }
        // 数字和字符串混合，把字符串当做数字-1
        if (aType === 'number' || bType === 'number') {
            return sign * ((aType === 'number' ? Number(a[field]) : -1) - (bType === 'number' ? Number(b[field]) : -1));
        }
        return sign * String(a[field]).localeCompare(String(b[field]));
    });
};

function InstructionTableNopage({
    columns,
    dataSource,
    isRelatedInstr,
    handleInstrsClick,
    tableHeight,
    selectedline,
    lineClickListener,
    condition,
    instructionSelectSource,
}: IinstrProp): JSX.Element {
    const [showDataSource, setShowDataSource] = useState(dataSource);
    const [filters, setFilters] = useState({});
    const [sorter, setSorter] = useState({});

    useEffect(() => {
        setShowDataSource(getShowData(dataSource, filters, sorter));
    }, [dataSource, filters, sorter]);

    useEffect(() => {
        srcollToView({ condition, selectedline, showDataSource, isRelatedInstr, instructionSelectSource });
    }, [lineClickListener, showDataSource]);

    return <ResizeTable
        size="small"
        minThWidth={50}
        columns={columns}
        dataSource={showDataSource}
        rowClassName={(record: InstrsColumnType): string => (isRelatedInstr(record) ? 'selected' : '')}
        onRow={(record: InstrsColumnType): { onClick: () => void } => ({
            onClick: (): void => {
                handleInstrsClick(record);
            },
        })}
        pagination={false}
        scroll={{ y: tableHeight, rowHeight: ROW_HEIGHT, scrollToFirstRowOnChange: false, x: showDataSource.length > 0 ? columns.length * 110 : 0 }}
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
    columns,
    dataSource,
    isRelatedInstr,
    handleInstrsClick,
    tableHeight,
    selectedline,
    lineClickListener,
    condition,
    instructionSelectSource,
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
        srcollToView({ condition, selectedline, showDataSource: pageData, isRelatedInstr, instructionSelectSource });
    }, [lineClickListener, pageData, page.current]);

    return <ResizeTable
        size="small"
        minThWidth={50}
        columns={columns}
        dataSource={pageData}
        rowClassName={(record: InstrsColumnType): string => (isRelatedInstr(record) ? 'selected' : '')}
        onRow={(record: InstrsColumnType): { onClick: () => void } => ({
            onClick: (): void => {
                handleInstrsClick(record);
            },
        })}
        pagination={GetPageConfigWhithPageData(page, setPage, [PAGE_LIMIT])}
        scroll={{ y: tableHeight - 50, rowHeight: ROW_HEIGHT, x: pageData.length > 0 ? columns.length * 110 : 0 }}
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
