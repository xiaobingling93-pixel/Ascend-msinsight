/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useCallback, useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { Checkbox, Layout } from '@insight/lib/components';
import { ResizeTable } from '@insight/lib/resize';
import { formatDecimal } from '@insight/lib/utils';
import type { ColumnsType } from 'antd/es/table';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';
import Filter from './Filter';
import { CODE_SEARCH_WINDOW_HEIGHT } from './CodeTextSearch';
import CodeTable, { UNSELECTED } from './CodeAttrTable';
import { getInstrColumns } from './InstructionTable';
import { ASCENDC_INNER_CODE, NOT_APPLICABLE, InstructionVersion, FieldType } from './defs';
import type { Ilinetable, InstrsColumnType, JsonInstructionType, Limit } from './defs';
import { queryDynamicInstr } from '../RequestUtils';
import { GetPageConfigWhithPageData, HeaderFixedContainer, LeftRightContainer, syncScroller } from '../Common';
import { updateSession } from '../../connection/handler';
import { defaultCacheUnit, InstructionSelectSource } from '../../entity/session';
import type { Session } from '../../entity/session';
import './HotMethod.css';

const MAX_INSTRUCTION = 1000000; // 100,0000
const PAGE_LIMIT = 500000; // 50,0000
const ROW_HEIGHT = 32;

export interface ConditionType {
    core: string;
    source: string;
    onlyRelated?: boolean;
};

// 指令是否关联
export const isRelated = (instr: InstrsColumnType, range: string[][] = []): boolean => {
    // 指令地址是否在代码行地址范围内
    return Boolean(range?.find(item => Number(item[0]) <= Number(instr.Address) && Number(item[1]) >= Number(instr.Address)));
};

interface IRelatedLineParams {
    instr: InstrsColumnType;
    instrVersion: InstructionVersion;
    condition: ConditionType;
    loggedCodeLines: Ilinetable[];
}

// 指令查找代码行
function getRelatedLine({ instr, instrVersion, condition, loggedCodeLines }: IRelatedLineParams): number {
    let line = UNSELECTED;
    // 指令数据ASCENDC_INNER_CODE可用版本
    if (instrVersion === InstructionVersion.ASCENDC_INNER_CODE) {
        const infoList = String(instr[ASCENDC_INNER_CODE]).split(':');
        const file = infoList[0];
        const fileLine = Number(infoList[1]);
        if (file === condition.source && !isNaN(fileLine)) {
            line = fileLine;
        }
    }
    if (line <= 0) {
        const data = loggedCodeLines.find((codeline: Ilinetable) => isRelated(instr, codeline['Address Range']));
        line = data?.Line ?? UNSELECTED;
    }
    return line;
}

// 恢复默认的指令高亮来源
export const recoverDefaultInstructionSource = (): void => {
    updateSession({
        instructionSelectSource: InstructionSelectSource.DEFAULT,
        cacheUnit: defaultCacheUnit,
    });
};

const Index = observer(({ session }: { session: Session }) => {
    const { t } = useTranslation('source');
    const [condition, setCondition] = useState<ConditionType>({ core: '', source: '', onlyRelated: false });
    // 选中代码行
    const [selectedLine, setSelectedLine] = useState<number>(UNSELECTED);
    const [selectedCodeLine, setSelectedCodeLine] = useState<Ilinetable | null>(null);
    // 指令表跳转到关联指令
    const [jumpListener, setJumpListener] = useState<number>(0);
    const [codeTableHeight, setCodeTableHeight] = useState<number>(1000);
    const [instructionTableHeight, setInstructionTableHeight] = useState<number>(1000);
    // 安全防护
    const [instrLimit, setInstrLimit] = useState({ maxSize: MAX_INSTRUCTION, overlimit: false, current: 0 });
    const [relatedInstrsLength, setRelatedInstrsLength] = useState<number>();

    const handleFilterChange = (newConditions: ConditionType): void => {
        setCondition({ ...condition, ...newConditions });
    };

    function resizeHeight(): void {
        const height = document.getElementById('CodeTable')?.clientHeight ?? 1000;
        if (height === 0) {
            return;
        }
        setCodeTableHeight(height);
        setInstructionTableHeight(session.openFind ? height + CODE_SEARCH_WINDOW_HEIGHT : height);
    }

    // 指令表跳转到关联指令
    function activeJump(): void {
        setTimeout(() => {
            setJumpListener(pre => (pre + 1) % 100);
        });
    }

    function cancelSelected(): void {
        // 清除选中
        setSelectedLine(UNSELECTED);
    };

    // 初始化
    useEffect(() => {
        // 同步滚动条
        syncScroller([document.getElementById('CodeTable'),
            document.querySelector('#CodeAttrTable .ant-table-body'),
            document.querySelector('#CodeLine .ant-table-body'),
        ]);

        const resizeFunc = (): void => {
            resizeHeight();
        };
        window.addEventListener('resize', resizeFunc);
        return (): void => {
            window.removeEventListener('resize', resizeFunc);
        };
    }, []);

    // Cache跳转过来
    useEffect(() => {
        if (session.instructionSelectSource === InstructionSelectSource.CACHE) {
            // 清理原来选中的代码行
            cancelSelected();
            activeJump();
        }
    }, [session.instructionSelectSource, session.cacheUnit]);

    return <div id="hotMethod" style={{ height: '100%', width: '100%' }} className={'th35'}>
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
                                                        {selectedLine >= 0 ? selectedLine : ''},
                                                    </span>
                                                </>
                                        }
                                        {t('RelatedInstructionsCount')} :
                                        <span>
                                            {relatedInstrsLength}
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
                        left={<CodeTable
                            session={session}
                            tableHeight={codeTableHeight}
                            activeJump={activeJump}
                            condition={condition}
                            resizeHeight={resizeHeight}
                            selectedLine={selectedLine}
                            setSelectedLine={setSelectedLine}
                            setSelectedCodeLine={setSelectedCodeLine}
                        />}
                        right={
                            <HeaderFixedContainer
                                id={'Instructions'}
                                style={{ paddingLeft: '8px' }}
                                body={<InstructionTable
                                    session={session}
                                    tableHeight={instructionTableHeight}
                                    condition={condition}
                                    selectedLine={selectedLine}
                                    setSelectedLine={setSelectedLine}
                                    selectedCodeLine={selectedCodeLine}
                                    jumpListener={jumpListener}
                                    setInstrLimit={setInstrLimit}
                                    setRelatedInstrsLength={setRelatedInstrsLength}
                                />}
                            />
                        }
                    />
                }
            />
        </Layout>
    </div>;
});

// 查询指令
async function getInstrs(coreName: string): Promise<{ instructions: InstrsColumnType[]; fields: Record<string, FieldType>;count: number }> {
    if (coreName === '') {
        return { instructions: [], fields: {}, count: 0 };
    }
    const res = await queryDynamicInstr({ coreName });
    // 动态列
    const fields = res?.['Instructions Dtype']?.Instructions ?? {};
    // 指令记录
    let records = res?.Instructions ?? [];
    // 安全防护
    const count = records.length;
    records = records.slice(0, MAX_INSTRUCTION);
    // percentage类型值小数位保留2位(从小数点后第一个非0数开始）
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
    // Cycles
    let maxCycles = 1;
    list.forEach(item => {
        if (!isNaN(Number(item.Cycles))) {
            maxCycles = Math.max(maxCycles, Number(item.Cycles));
        }
    });
    list.forEach(item => {
        item.maxCycles = maxCycles;
    });

    return { instructions: list, fields, count };
};

function isRelatedInstruction({ instr, session, selectedCodeLine }: {
    instr: InstrsColumnType;
    session: Session;
    selectedCodeLine: Ilinetable | null;
}): boolean {
    if (session.instructionSelectSource === InstructionSelectSource.CACHE) {
        return isRelated(instr, session.cacheUnit.addressRange);
    }
    if (selectedCodeLine !== null && selectedCodeLine !== undefined) {
        return isRelated(instr, selectedCodeLine['Address Range']);
    }
    return false;
}

interface IProps {
    session: Session;
    condition: ConditionType;
    tableHeight: number;
    selectedLine: number;
    selectedCodeLine: Ilinetable | null;
    jumpListener: number;
    setInstrLimit: (val: (pre: Limit) => Limit) => void ;
    setSelectedLine: (val: number) => void;
    setRelatedInstrsLength: (val: number) => void;
}

const InstructionTable = observer(({
    session, condition, tableHeight, selectedCodeLine, setInstrLimit, selectedLine, jumpListener, setSelectedLine, setRelatedInstrsLength,
}: IProps) => {
    const { t } = useTranslation('source');
    const [allInstrs, setAllInstrs] = useState<InstrsColumnType[]>([]);
    // 是否关联指令
    const isRelatedInstr = useCallback((instr: InstrsColumnType): boolean => isRelatedInstruction({ instr, session, selectedCodeLine })
        , [selectedCodeLine, session.instructionSelectSource, session.cacheUnit.addressRange]);
    const getRelatedInstrs = useCallback((): InstrsColumnType[] => allInstrs.filter((record: InstrsColumnType) => isRelatedInstr(record))
        , [allInstrs, isRelatedInstr]);
    const relatedInstrs = useMemo(() => getRelatedInstrs(), [getRelatedInstrs]);
    // 表格数据
    const showInstrs = useMemo(() => condition.onlyRelated ? relatedInstrs : allInstrs, [allInstrs, relatedInstrs, condition.onlyRelated]);

    // 动态列
    const [dynamicInstrFields, setDynamicInstrFields] = useState<Record<string, FieldType>>({});
    const instrColumns = useMemo(() => getInstrColumns(dynamicInstrFields, t, showInstrs), [dynamicInstrFields, t, showInstrs]);

    async function updateInstrs(): Promise<void> {
        // 指令记录
        getInstrs(condition.core).then(({ instructions: newInstrlist, fields, count }) => {
            setDynamicInstrFields(fields);
            setAllInstrs(newInstrlist);
            setInstrLimit(pre => ({ ...pre, overlimit: count > MAX_INSTRUCTION, current: count }));
        });
    }

    const handleInstrsClick = (instr: InstrsColumnType): void => {
        setSelectedLine(getRelatedLine({ instr, instrVersion: session.instrVersion, condition, loggedCodeLines: session.loggedCodeLines }));
        recoverDefaultInstructionSource();
    };

    useEffect(() => {
        updateInstrs();
    }, [condition.core]);

    useEffect(() => {
        setRelatedInstrsLength(relatedInstrs.length);
    }, [relatedInstrs.length]);

    return <InstructionTableComp
        tableHeight={tableHeight}
        columns={instrColumns}
        condition={condition}
        dataSource={showInstrs}
        isRelatedInstr={isRelatedInstr}
        handleInstrsClick={handleInstrsClick}
        selectedLine={selectedLine}
        jumpListener={jumpListener}
        isShowPage={showInstrs.length > PAGE_LIMIT}
        instructionSelectSource={session.instructionSelectSource}
    />;
});

interface IinstrProp {
    condition: ConditionType;
    columns: ColumnsType<InstrsColumnType>;
    dataSource: InstrsColumnType[];
    isRelatedInstr: (instr: InstrsColumnType) => boolean;
    handleInstrsClick: (instr: InstrsColumnType) => void;
    tableHeight: number;
    selectedLine: number;
    jumpListener: number;
    isShowPage?: boolean;
    instructionSelectSource: InstructionSelectSource;
}

const srcollToView = ({
    condition,
    selectedLine,
    showDataSource,
    isRelatedInstr,
    instructionSelectSource = InstructionSelectSource.DEFAULT,
}:
{
    condition: ConditionType;
    selectedLine: number;
    showDataSource: InstrsColumnType[];
    isRelatedInstr: (instr: InstrsColumnType) => boolean;
    instructionSelectSource: InstructionSelectSource;
},
): void => {
    // 只显示关联代码，或默认场景下未选中代码行
    if (condition.onlyRelated === true || (selectedLine < 0 && instructionSelectSource === InstructionSelectSource.DEFAULT)) {
        return;
    }
    const index = showDataSource.findIndex(isRelatedInstr);
    if (index < 0) {
        return;
    }
    const top = index * ROW_HEIGHT;
    const parentNode = document.querySelector('#Instructions .ant-table-body') as HTMLElement;
    parentNode.scrollTo({ top });
};

function InstructionTableComp({
    columns,
    dataSource,
    isRelatedInstr,
    handleInstrsClick,
    tableHeight,
    selectedLine,
    condition,
    jumpListener,
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
            selectedLine={selectedLine}
            jumpListener={jumpListener}
            instructionSelectSource={instructionSelectSource}
        />
        : <InstructionTableNopage
            tableHeight={tableHeight}
            columns={columns}
            condition={condition}
            dataSource={dataSource}
            isRelatedInstr={isRelatedInstr}
            handleInstrsClick={handleInstrsClick}
            selectedLine={selectedLine}
            jumpListener={jumpListener}
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
        const isAllNegativeNumber = aType === 'number' && bType === 'number' && Number(a[field]) < 0 && Number(b[field]) < 0;
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
    selectedLine,
    jumpListener,
    condition,
    instructionSelectSource,
}: IinstrProp): JSX.Element {
    const [showDataSource, setShowDataSource] = useState(dataSource);
    const [filters, setFilters] = useState({});
    const [sorter, setSorter] = useState({});

    const doSrcollToView = useCallback((): void => srcollToView({ condition, selectedLine, showDataSource, isRelatedInstr, instructionSelectSource })
        , [condition, selectedLine, showDataSource, isRelatedInstr, instructionSelectSource]);
    useEffect(() => {
        setShowDataSource(getShowData(dataSource, filters, sorter));
    }, [dataSource, filters, sorter]);

    useEffect(() => {
        doSrcollToView();
    }, [jumpListener]);

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
    selectedLine,
    jumpListener,
    condition,
    instructionSelectSource,
}: IinstrProp): JSX.Element {
    const [showDataSource, setShowDataSource] = useState<InstrsColumnType[]>([]);
    const [filters, setFilters] = useState({});
    const [sorter, setSorter] = useState({});
    const [page, setPage] = useState({ current: 1, pageSize: PAGE_LIMIT, total: dataSource.length });
    const [pageData, setPageData] = useState(showDataSource.slice((page.current - 1) * page.pageSize, page.current * page.pageSize));
    const doSrcollToView = useCallback((): void => srcollToView({ condition, selectedLine, showDataSource: pageData, isRelatedInstr, instructionSelectSource })
        , [condition, selectedLine, pageData, isRelatedInstr, instructionSelectSource]);

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
    }, [showDataSource, jumpListener]);

    useEffect(() => {
        const curPageData = showDataSource.slice((page.current - 1) * page.pageSize, page.current * page.pageSize);
        setPageData(curPageData);
    }, [showDataSource, page.pageSize, page.current]);
    useEffect(() => {
        doSrcollToView();
    }, [jumpListener, pageData, page.current]);

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
