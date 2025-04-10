/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { useTranslation } from 'react-i18next';
import { ResizeTable } from 'ascend-resize';
import { formatDecimal } from 'ascend-utils';
import { type ColumnsType } from 'antd/es/table';
import { type TFunction } from 'i18next';
import { ThContainer } from './TableHead';
import { getColConfig } from './InstructionTable';
import CodeTextSearch, { CODE_SEARCH_WINDOW_HEIGHT } from './CodeTextSearch';
import { type ConditionType, recoverDefaultInstructionSource } from './HotMethod';
import { type Iline, type Ilinetable, FieldType, LINE } from './defs';
import CodeViewer from './codeViewer/CodeViewer';
import { BREAK_LINE_REGEXP } from './codeViewer/highlightLineNumbers';
import { HeaderFixedContainer, LeftRightContainer } from '../Common';
import { queryDynamicLine, querySourceCode } from '../RequestUtils';
import { type Session } from '../../entity/session';
import { updateSession } from '../../connection/handler';

const MAX_FILE_SIZE = 1000000; // 100,0000
const MAX_LINE_LENGTH = 10000; // 10000
const MAX_LINE = 10000;
export const UNSELECTED = -1;

const CodeTableContainer = styled.div`
    height: 100%;
    width: 100%;
    overflow: auto;
    padding-right: 8px;
`;

// 查询源代码
async function getCode(source: string, t: TFunction): Promise<string> {
    if (source === undefined || source === null || source === '') {
        return '';
    }
    const res = await querySourceCode(source);
    return warpCode(res?.fileContent ?? '', t);
}

// 安全防护：限制源码长度
function warpCode(sourcecode: string, t: TFunction): string {
    let str = sourcecode;
    const linebreak = str.match(BREAK_LINE_REGEXP)?.[0] ?? '';
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

// 查询代码行记录
async function getLines({ source, core }: { source: string; core: string }): Promise<{ lines: Ilinetable[]; fields: Record<string, FieldType> }> {
    if (source === '' || core === '') {
        return { lines: [], fields: {} };
    }
    const res = await queryDynamicLine({ sourceName: source, coreName: core });
    const fields = res?.['Files Dtype']?.Lines ?? {};
    let list: Iline[] = res?.Lines ?? [];
    const percentageFields = Object.keys(fields).filter(fieldName => fields[fieldName] === FieldType.PERCENTAGE);
    // percentage类型值小数位保留2位(从小数点后第一个非0数开始）
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

// 源码属性表
function getCodeLines(code: string = '', loggedCodeLines: Ilinetable[] = []): Ilinetable[] {
    const sourceCodeList = code === '' ? [] : code.split(BREAK_LINE_REGEXP);
    return sourceCodeList.map((codeItem: string, index: number) => {
        const line = index + 1;
        const lineInfo = loggedCodeLines.find((item: Ilinetable) => item.Line === line) ?? {};
        return { [LINE]: line, ...lineInfo };
    });
}

// 动态列
// 默认显示列
const defaultCols = ['Instructions Executed', 'Cycles'];
// 不显示的列
const notDisplayedCols = ['Address Range', 'Line'];
const getCodeColumns = (t: TFunction, dynamicFields: Record<string, FieldType> = {}): ColumnsType<Ilinetable> => {
    const dynamicCols = Object.keys(dynamicFields).filter(col => dynamicFields[col] !== FieldType.SKIP);
    const cols = dynamicCols.length === 0
        ? defaultCols
        : [...defaultCols.filter(colName => dynamicCols.includes(colName)),
            ...dynamicCols.filter(colName => !defaultCols.includes(colName) && !notDisplayedCols.includes(colName)),
        ]
    ;
    return cols.map(colName => getColConfig<Ilinetable>({ colName, fieldType: dynamicFields[colName], presetCols: [], defaultSort: false, t }));
};

interface IProps {
    session: Session;
    condition: ConditionType;
    tableHeight: number;
    selectedLine: number;
    setSelectedLine: (val: number) => void;
    setSelectedCodeLine: (val: Ilinetable | null) => void;
    activeJump: () => void;
    resizeHeight: () => void;
}

// 源码表
const CodeAttrTable = observer(({
    session, condition, activeJump, tableHeight, resizeHeight, selectedLine, setSelectedLine, setSelectedCodeLine,
}: IProps) => {
    const { t } = useTranslation('source');
    // 源代码
    const [code, setCode] = useState('');
    // 表格
    const codeLines = useMemo<Ilinetable[]>(() => getCodeLines(code, session.loggedCodeLines), [code, session.loggedCodeLines]);
    // 表格动态列
    const [dynamicCodeFields, setDynamicCodeFields] = useState<Record<string, FieldType>>({});
    const codeColumns = useMemo(() => getCodeColumns(t, dynamicCodeFields), [dynamicCodeFields, t]);

    // 源代码
    function updateCode(): void {
        getCode(condition.source, t).then(newCode => {
            setCode(newCode);
        });
    }
    // 日志记录
    function updateLoggedLines(): void {
        getLines(condition).then(({ lines: loggedCodeLines, fields }) => {
            // 动态列
            setDynamicCodeFields(fields);
            updateSession({ loggedCodeLines });
        });
    }

    function handleLineClick(line: number): void {
        recoverDefaultInstructionSource();
        setSelectedLine(line);
        activeJump();
    }

    useEffect(() => {
        setSelectedLine(UNSELECTED);
    }, [condition.source]);

    useEffect(() => {
        updateCode();
    }, [condition.source, t]);

    useEffect(() => {
        updateLoggedLines();
    }, [condition.core, condition.source]);

    useEffect(() => {
        setSelectedCodeLine(codeLines[selectedLine - 1] ?? null);
    }, [selectedLine, codeLines]);

    useEffect(() => {
        resizeHeight();
    }, [code, session.openFind]);

    return <CodeTableContainer
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
                        rowClassName={(record: Ilinetable, index: number): string => (selectedLine === index + 1 ? 'selected' : '')}
                        onRow={(record: Ilinetable): { onClick: (event: React.MouseEvent<HTMLElement>) => void } => {
                            return { onClick: (): void => handleLineClick(record.Line) };
                        }}
                        id="CodeLine"
                        scroll={{ y: tableHeight }}
                    />}
                    right={<HeaderFixedContainer
                        style={{ overflow: 'hidden' }}
                        headerStyle={{ marginBottom: session.openFind ? `${CODE_SEARCH_WINDOW_HEIGHT}px` : '0' }}
                        header={<ThContainer>
                            <div className="th"><span>{t('Source')}</span></div>
                        </ThContainer>}
                        bodyProps={{ id: 'CodeTable' }}
                        bodyStyle={{ overflowX: 'scroll', marginRight: '-8px' }}
                        body={
                            <CodeViewer
                                code={code}
                                handleLineClick={handleLineClick}
                                selectedLine={selectedLine}
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
                    rowClassName={(record: Ilinetable, index: number): string => (selectedLine === index + 1 ? 'selected' : '')}
                    onRow={(record: Ilinetable): { onClick: (event: React.MouseEvent<HTMLElement>) => void } => {
                        return { onClick: (): void => handleLineClick(record.Line) };
                    }}
                    scroll={{ y: tableHeight, x: codeLines.length > 0 ? codeColumns.length * 100 : 0 }}
                />
            }/>
    </CodeTableContainer>;
});

export default CodeAttrTable;
