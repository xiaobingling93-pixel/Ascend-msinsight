/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { type ReactNode, useEffect, useState, useMemo } from 'react';
import type { ColumnsType } from 'antd/es/table';
import { observer } from 'mobx-react';
import { BaseContainer, BaseDescription } from 'lib/CommonUtils';
import ResizeTable from 'lib/ResizeTable';
import { type Session } from '../../entity/session';
import { queryBaseInfo } from '../RequestUtils';
interface Iprops {
    session: Session;
}
interface Ilabel {
    label: ReactNode;
    key: string;
}
interface IblockDuration {
    blockId: string ;
    coreType?: string;
    duration: string[];
}
interface Ibaseinfo {
    name?: string;
    soc?: string;
    opType?: string;
    duration?: string ;
    blockDim?: string ;
    blockDetail?: IblockDuration[] ;
    mixBlockDim?: string ;

}
const allLabellist = [
    {
        label: 'Name',
        key: 'name',
    },
    {
        label: 'Soc',
        key: 'soc',
    },
    {
        label: 'Duration (μs)',
        key: 'duration',
    },
    {
        label: 'Op Type',
        key: 'opType',
    },
    {
        label: 'Block Dim',
        key: 'blockDim',
        isMix: false,
    },
    {
        label: 'Block Detail',
        key: 'blockDetail',
        isMix: false,
    },
    {
        label: 'Mix Block Dim',
        key: 'blockDim',
        isMix: true,
    },
    {
        label: 'Mix Block Detail',
        key: 'blockDetail',
        isMix: true,
    },
];

const getLabellist = (dataObj: Ibaseinfo): Ilabel[] => {
    return allLabellist.filter(label => label.isMix === undefined || label.isMix === (dataObj.opType?.toLowerCase() === 'mix'));
};
const blockCol: ColumnsType<IblockDuration> = [
    {
        title: 'Block ID',
        dataIndex: 'blockId',
        ellipsis: true,
        width: 100,
    },
    {
        title: 'Core Type',
        dataIndex: 'coreType',
        ellipsis: true,
    },
    {
        title: 'Duration (μs)',
        dataIndex: 'duration',
        ellipsis: true,
    },
];

const mixBlockCol: ColumnsType<IblockDuration> = [
    {
        title: 'Block ID',
        dataIndex: 'blockId',
        ellipsis: true,
        width: 100,
    },
    {
        title: 'AIC Duration (μs)',
        dataIndex: 'aicDuration',
        ellipsis: true,
    },
    {
        title: 'AIV0 Duration (μs)',
        dataIndex: 'aiv0Duration',
        ellipsis: true,
    },
    {
        title: 'AIV1 Duration (μs)',
        dataIndex: 'aiv1Duration',
        ellipsis: true,
    },
];
function BlockDetail({ opType = '', blockDetail = [] }: Ibaseinfo): JSX.Element {
    const dataset = useMemo(() => {
        if (opType?.toLowerCase() === 'mix') {
            return blockDetail.map(item => ({
                ...item,
                aicDuration: item?.duration?.[0],
                aiv0Duration: item?.duration?.[1],
                aiv1Duration: item?.duration?.[2],
            }));
        } else {
            return blockDetail.map(item => ({
                ...item,
                duration: item?.duration?.[0],
            }));
        }
    }, [blockDetail]);
    const col = useMemo(() => opType?.toLowerCase() === 'mix' ? mixBlockCol : blockCol, [opType]);
    return (<div style={{ width: '600px' }}>
        <ResizeTable
            size="small"
            columns={col}
            dataSource={dataset}
            scroll={dataset.length > 10 ? { y: 400 } : false}
            pagination={{ showTotal: () => true, size: 'small', hideOnSinglePage: true }}
        />
    </div>);
}

const getInfoItem = (item: Ilabel, dataObj: Ibaseinfo): Record<string, unknown> => {
    if (item.key === 'blockDetail') {
        return {
            ...item,
            value: <BlockDetail {...dataObj}/>,
        };
    } else {
        return { ...item, value: dataObj[item.key as keyof Ibaseinfo] ?? '' };
    }
};

const index = observer(({ session }: Iprops): JSX.Element => {
    const [data, setData] = useState<Ibaseinfo>({});
    const [items, setItems] = useState<Array<Record<string, unknown>>>([]);

    const getBaseInfo = async (): Promise<void> => {
        const res = await queryBaseInfo();
        setData(res ?? {});
    };

    const showBaseInfo = (dataObj: Ibaseinfo): void => {
        const labellist = getLabellist(dataObj);
        const disaplayList = labellist.map(item => getInfoItem(item, dataObj));
        setItems(disaplayList);
    };

    useEffect(() => {
        getBaseInfo();
    }, [session.updateId]);
    useEffect(() => {
        showBaseInfo(data);
    }, [JSON.stringify(data)]);

    return (
        <BaseContainer
            header="Base Info"
            body={<BaseDescription items={items}/>}
        />
    );
});

export default index;
