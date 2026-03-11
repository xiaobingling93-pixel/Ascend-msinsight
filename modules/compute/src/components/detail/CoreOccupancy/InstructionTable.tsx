import React from 'react';
import { useTranslation } from 'react-i18next';
import styled from '@emotion/styled';
import { ResizeTable } from '@insight/lib/resize';
import { type ICoreOccupancy } from './Index';

interface ITableData {
    vector: string;
    [key: string]: string | number;
}

const Container = styled.div`
    margin-top: 20px;
`;

const Title = styled.div`
    font-size: 14px;
    font-weight: bold;
    margin-bottom: 8px;
`;

const NoData = styled.div`
    text-align: center;
    color: var(--grey15);
`;

const transformData = (data: ICoreOccupancy): { columns: any[]; dataSource: ITableData[] } => {
    const { opDetails = [] } = data;

    if (opDetails.length === 0) {
        return { columns: [], dataSource: [] };
    }

    const vectorNames: string[] = [];
    const coreMap = new Map<number, Map<string, number>>();
    const coreIds: number[] = [];

    opDetails.forEach(core => {
        const { coreId, subCoreDetails = [] } = core;
        const vectorMap = new Map<string, number>();
        coreIds.push(coreId);

        subCoreDetails.forEach(subCore => {
            const { subCoreName, simtVfInstructionPerCycle } = subCore;
            if (subCoreName.startsWith('vector') && !vectorNames.includes(subCoreName)) {
                vectorNames.push(subCoreName);
            }
            const value = simtVfInstructionPerCycle?.value?.compare ?? 0;
            vectorMap.set(subCoreName, value);
        });

        coreMap.set(coreId, vectorMap);
    });

    const sortedCoreIds = [...new Set(coreIds)].sort((a, b) => a - b);

    const columns = [
        { title: '', dataIndex: 'vector', ellipsis: true },
        ...sortedCoreIds.map(coreId => ({
            title: `Core${coreId}`,
            dataIndex: `Core${coreId}`,
            ellipsis: true,
        })),
    ];

    const dataSource: ITableData[] = vectorNames.map(vectorName => {
        const row: ITableData = { vector: vectorName };
        sortedCoreIds.forEach(coreId => {
            const coreData = coreMap.get(coreId);
            row[`Core${coreId}`] = coreData?.get(vectorName) ?? 0;
        });
        return row;
    });

    return { columns, dataSource };
};

const InstructionTable = ({ data }: { data: ICoreOccupancy }): JSX.Element => {
    const { t } = useTranslation('details');
    const { columns, dataSource } = transformData(data);

    return (
        <Container>
            <Title>{t('Instruction Per Cycle')}</Title>
            {dataSource.length === 0
                ? (
                    <NoData>No data</NoData>
                )
                : (
                    <ResizeTable
                        size="small"
                        columns={columns}
                        dataSource={dataSource.map((row, index) => ({ ...row, key: index }))}
                        pagination={false}
                    />
                )}
        </Container>
    );
};

export default InstructionTable;
