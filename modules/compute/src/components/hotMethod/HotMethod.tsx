/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import React, { useCallback, useEffect, useMemo, useState } from 'react';
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
    confrimMessage,
} from '../Common';
import type { InstrsColumnType, Iline, Ilinetable, JsonInstructionType } from './defs';
import { queryApiInstr, queryApiLine, querySourceCode } from '../RequestUtils';
import { runInAction } from 'mobx';

const BREAK_LINE_REGEXP = /\r\n|\r|\n/g;
const MAX_FILE_SIZE = 1000000; // 100万
const MAX_FILE_SIZE_LABLE = '1 million';
const MAX_LINE_LENGTH = 10000; // 1万
const MAX_LINE = 10000; // 1万
const MAX_INSTRUCTION = 1000000; // 100万

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
    const DomId = 'hotMethod';
    const [condition, setCondition] = useState<ConditionType>({ core: '', source: '', onlyRelated: false });
    const [code, setCode] = useState('');
    const [codeLines, setCodeLines] = useState<Ilinetable[]>([]);
    const [loggedCodeLines, setLoggedCodeLines] = useState<Ilinetable[]>([]);
    const [instrsData, setInstrsData] = useState<InstrsColumnType[]>([]);
    const [selectedline, setSelectedline] = useState<number>(-1);
    const [tableHeight, setTableHeight] = useState<number>(1000);
    const instrsColumns = useInstrsColumns();
    const [filterInstrsColumns, setFilterInstrsColumns] = useState<ColumnsType<InstrsColumnType>>(instrsColumns);
    const [doneQuery, setDoneQuery] = useState(false);
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
                    filterMode: 'tree',
                    filterSearch: true,
                    onFilter: (value: string | number, record: InstrsColumnType) => {
                        if (typeof value === 'string') {
                            return String(record[col.dataIndex as keyof InstrsColumnType]).startsWith(value);
                        } else {
                            return record[col.dataIndex as keyof InstrsColumnType] === value;
                        }
                    },
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
            str += `${linebreak}----------【Characters Exceed ${MAX_FILE_SIZE_LABLE} , Hide the rest content.】----------`;
        }
        if (str.length > 0) {
            let splitlines: string[] = str.split(BREAK_LINE_REGEXP);
            if (splitlines.length > MAX_LINE) {
                splitlines = splitlines.slice(0, MAX_LINE);
                splitlines.push(`----------【Exceed ${MAX_LINE} lines , Hide the rest content.】----------`);
            }
            splitlines = splitlines.map(codeline => {
                if (codeline.length > MAX_LINE_LENGTH) {
                    return `${codeline.slice(0, MAX_LINE_LENGTH)} 【Exceed ${MAX_LINE_LENGTH} , Hide the rest content.】`;
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
            runInAction(() => {
                let list = JSON.parse(res.instructions)?.Instructions;
                if (list?.length > MAX_INSTRUCTION) {
                    list = list.slice(0, MAX_INSTRUCTION);
                    confrimMessage('warn', `Only display the first ${MAX_INSTRUCTION} Instructions`);
                }
                session.Instructions = list;
            });
            setDoneQuery(true);
        }
        const records = session.Instructions;
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
        const records: Iline[] = res.lines ?? [];
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
                    const Line = index + 1;
                    const lineInfo = newLoggedCodeLines.find((item: Ilinetable) => item.Line === Line) ?? {};
                    return { Line, ...lineInfo };
                });
                setCodeLines(sourceCodeLines);
            });
            // 指令记录
            getInstrs(condition.core).then((newInstrlist) => {
                setInstrsData(newInstrlist);
            });
        }
    }, [condition.core, condition.source, session.parseStatus, session.updateId]);

    useEffect(() => {
        resizeHeight();
    }, [code]);

    useEffect(() => {
        updateInstrsColumns();
    }, [condition.onlyRelated, instrsData, t]);

    return <div id={DomId} style={{ height: '100%', width: '100%' }} className={'th35'}>
        <HeaderFixedContainer
            headerStyle={{ padding: '10px' }}
            header={
                <>
                    <Filter session={session} handleFilterChange={handleFilterChange}/>
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
                                            handleLineClick={(line: number) => {
                                                setSelectedline(line);
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
                                            onClick: (event: React.MouseEvent<HTMLElement>) => {
                                                setSelectedline(record.Line);
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
                            />}
                        />
                    }
                />
            }
        />
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
}
const pagelimit = 500000;
const rowHeight = 29;
const srcollToView = ({ condition, selectedline, showDataSource, isRelatedInstr }:
{
    condition: ConditionType;
    selectedline: number;
    showDataSource: InstrsColumnType[];
    isRelatedInstr: (instr: InstrsColumnType) => boolean;
},
): void => {
    if (condition.onlyRelated === true || selectedline < 0) {
        return;
    }
    const index = showDataSource.findIndex(isRelatedInstr);
    if (index < 0) {
        return;
    }
    const top = index * rowHeight;
    const parentNode = document.querySelector('#Instructions .ant-table-body') as HTMLElement;
    parentNode.scrollTo({ top });
};

function InstructionTable({ columns, dataSource, isRelatedInstr, handleInstrsClick, tableHeight, selectedline, condition }: IinstrProp): JSX.Element {
    const [page, setPage] = useState({ current: 1, pageSize: pagelimit, total: dataSource.length });
    const isShowPage = dataSource.length > pagelimit;
    const showDataSource = useMemo(() => {
        const pageDataSource = dataSource.slice((page.current - 1) * page.pageSize, page.current * page.pageSize);
        return isShowPage ? pageDataSource : dataSource;
    }, [dataSource, isShowPage, page.current, page.pageSize]);
    useEffect(() => {
        if (isShowPage) {
            setPage({ ...page, current: 1, total: dataSource.length });
        }
    }, [dataSource.length]);
    useEffect(() => {
        if (isShowPage) {
            const index = dataSource.findIndex(isRelatedInstr);
            const onPage = Math.floor(index / page.pageSize) + 1;
            if (index > 0 && onPage !== page.current) {
                setPage({ ...page, current: onPage });
            }
        }
    }, [selectedline]);
    useEffect(() => {
        srcollToView({ condition, selectedline, showDataSource, isRelatedInstr });
    }, [selectedline, showDataSource]);

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
        pagination={isShowPage ? GetPageConfigWhithPageData(page, setPage, [pagelimit, pagelimit / 2]) : false}
        scroll={{ y: isShowPage ? tableHeight - 50 : tableHeight, rowHeight }}
        virtual={true}
    />;
};

export default Index;
