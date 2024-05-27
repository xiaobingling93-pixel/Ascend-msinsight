/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { type ReactNode, useEffect, useState, useMemo } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { Tooltip } from 'antd';
import type { ColumnsType } from 'antd/es/table';
import { observer } from 'mobx-react';
import { BaseContainer, BaseDescription } from 'lib/CommonUtils';
import ResizeTable from 'lib/ResizeTable';
import { type Session } from '../../entity/session';
import { queryBaseInfo } from '../RequestUtils';
import { LimitHit } from '../LimitSet';
interface Iprops {
    session: Session;
}
interface Ilabel {
    label: ReactNode;
    key: string;
    isMix?: boolean;
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
const getAllLabellist = (t: TFunction): Ilabel[] => {
    return [
        {
            label: t('Name'),
            key: 'name',
        },
        {
            label: t('Soc'),
            key: 'soc',
        },
        {
            label: `${t('Duration')} (μs)`,
            key: 'duration',
        },
        {
            label: t('OpType'),
            key: 'opType',
        },
        {
            label: t('BlockDim'),
            key: 'blockDim',
            isMix: false,
        },
        {
            label: t('BlockDetail'),
            key: 'blockDetail',
            isMix: false,
        },
        {
            label: t('MixBlockDim'),
            key: 'blockDim',
            isMix: true,
        },
        {
            label: t('MixBlockDetail'),
            key: 'blockDetail',
            isMix: true,
        },
    ];
};

const getLabellist = (dataObj: Ibaseinfo, t: TFunction): Ilabel[] => {
    const allLabellist = getAllLabellist(t);
    return allLabellist.filter(label => label.isMix === undefined || label.isMix === (dataObj.opType?.toLowerCase() === 'mix'));
};
const useBlockCol = (): ColumnsType<IblockDuration> => {
    const { t } = useTranslation('details');
    return [{
        title: t('BlockID'),
        dataIndex: 'blockId',
        ellipsis: true,
        width: 100,
    },
    {
        title: t('CoreType'),
        dataIndex: 'coreType',
        ellipsis: true,
    },
    {
        title: `${t('Duration')} (μs)`,
        dataIndex: 'duration',
        ellipsis: true,
    },
    ];
};

const useMixBlockCol = (): ColumnsType<IblockDuration> => {
    const { t } = useTranslation('details');
    return [
        {
            title: t('BlockID'),
            dataIndex: 'blockId',
            ellipsis: true,
            width: 100,
        },
        {
            title: `CUBE0 ${t('Duration')} (μs)`,
            dataIndex: 'aicDuration',
            ellipsis: true,
        },
        {
            title: `VECTOR0 ${t('Duration')} (μs)`,
            dataIndex: 'aiv0Duration',
            ellipsis: true,
        },
        {
            title: `VECTOR1 ${t('Duration')} (μs)`,
            dataIndex: 'aiv1Duration',
            ellipsis: true,
        },
    ];
};
function BlockDetail({ opType = '', blockDetail = [] }: Ibaseinfo): JSX.Element {
    const [limit, setLimit] = useState({ maxSize: 10000, overlimit: false, current: 0 });

    const dataset = useMemo(() => {
        const list = blockDetail.slice(0, limit.maxSize);
        if (opType?.toLowerCase() === 'mix') {
            return list.map(item => ({
                ...item,
                aicDuration: item?.duration?.[0],
                aiv0Duration: item?.duration?.[1],
                aiv1Duration: item?.duration?.[2],
            }));
        } else {
            return list.map(item => ({
                ...item,
                duration: item?.duration?.[0],
            }));
        }
    }, [blockDetail]);
    useEffect(() => {
        setLimit({ ...limit, overlimit: blockDetail.length > limit.maxSize, current: blockDetail.length });
    }, [blockDetail]);
    const mixBlockCol = useMixBlockCol();
    const blockCol = useBlockCol();
    const col = useMemo(() => opType?.toLowerCase() === 'mix' ? mixBlockCol : blockCol, [opType]);
    return (<div style={{ width: '600px' }}>
        {limit.overlimit && (<LimitHit maxSize={limit.maxSize} name={`Block Detail Records (${limit.current})`}/>)}
        <ResizeTable
            size="small"
            columns={col}
            dataSource={dataset}
            scroll={dataset.length > 10 ? { y: 400 } : false}
            pagination={false}
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
        let text = (dataObj[item.key as keyof Ibaseinfo] ?? '') as string;
        const maxSize = 10000; // 1万
        if (text.length > maxSize) {
            text = `${text.slice(0, maxSize)} 【Exceed ${maxSize} , Hide the rest content.】`;
        }
        let node: ReactNode = text;
        if (text.length > 50) {
            node = (<Tooltip placement="topLeft" title={<div style={{ maxWidth: '800px', maxHeight: '400px', overflow: 'auto' }}>{text}</div>}>{text}</Tooltip>);
        }
        return { ...item, value: node };
    }
};

const defaultData: Ibaseinfo = {
    name: '',
    soc: '',
    opType: '',
    blockDim: '',
    mixBlockDim: '',
    duration: '',
    blockDetail: [],
};

const index = observer(({ session }: Iprops): JSX.Element => {
    const [data, setData] = useState<Ibaseinfo>(defaultData);
    const [items, setItems] = useState<Array<Record<string, unknown>>>([]);
    const { t } = useTranslation();
    const { t: tDetails } = useTranslation('details');

    const getBaseInfo = async (): Promise<void> => {
        const res = await queryBaseInfo();
        setData(res ?? defaultData);
    };

    const showBaseInfo = (dataObj: Ibaseinfo): void => {
        const labellist = getLabellist(dataObj, tDetails);
        const disaplayList = labellist.map(item => getInfoItem(item, dataObj));
        setItems(disaplayList);
    };

    useEffect(() => {
        if (!session.parseStatus) {
            setTimeout(() => {
                if (!session.parseStatus) {
                    setData(defaultData);
                }
            }, 100);
            return;
        }
        getBaseInfo();
    }, [session.updateId, session.parseStatus]);
    useEffect(() => {
        showBaseInfo(data);
    }, [JSON.stringify(data), t]);

    return (
        <BaseContainer
            header={t('BaseInfo')}
            body={<BaseDescription items={items}/>}
        />
    );
});

export default index;
