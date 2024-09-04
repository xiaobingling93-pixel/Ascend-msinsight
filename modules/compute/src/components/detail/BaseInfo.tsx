/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { type ReactNode, useEffect, useState, useMemo } from 'react';
import { useTranslation } from 'react-i18next';
import type { TFunction } from 'i18next';
import { Tooltip } from 'ascend-components';
import { observer } from 'mobx-react';
import { MIDescriptions, MIDescriptionsItem } from 'ascend-utils';
import { ResizeTable } from 'ascend-resize';
import { type Session } from '../../entity/session';
import { queryBaseInfo } from '../RequestUtils';
import { LimitHit } from '../LimitSet';
import CollapsiblePanel from 'ascend-collapsible-panel';

interface Iprops {
    session: Session;
}
interface Ilabel {
    label: ReactNode;
    key: string;
    isMix?: boolean;
}
interface IblockDetail {
    headerName: string[] ;
    row: Array<{value: string[]}>;
}
interface Ibaseinfo {
    name?: string;
    soc?: string;
    opType?: string;
    duration?: string ;
    blockDim?: string ;
    blockDetail?: IblockDetail ;
    mixBlockDim?: string ;
    pid?: string;
    deviceId?: string;

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
            label: t('DeviceId'),
            key: 'deviceId',
        },
        {
            label: t('Pid'),
            key: 'pid',
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
    return allLabellist.filter(label => {
        if (label.key === 'pid' && dataObj.pid === '') {
            return false;
        } else {
            return label.isMix === undefined || label.isMix === (dataObj.opType?.toLowerCase() === 'mix');
        }
    });
};

function BlockDetail({ blockDetail = { headerName: [], row: [] }, translate }: Ibaseinfo & {translate: any}): JSX.Element {
    const [limit, setLimit] = useState({ maxSize: 10000, overlimit: false, current: 0 });
    const tableset = useMemo(() => {
        const { headerName = [], row = [] } = blockDetail ?? {};
        const cols = getFullCols(headerName, translate);
        const dataset = row.slice(0, limit.maxSize).map(item => {
            const arr = item.value;
            const obj: Record<string, string> = {};
            headerName.forEach((header, index) => {
                obj[header] = arr[index];
            });
            return obj;
        });
        return { cols, dataset };
    }, [blockDetail]);
    useEffect(() => {
        setLimit({ ...limit, overlimit: blockDetail.row.length > limit.maxSize, current: blockDetail.row.length });
    }, [blockDetail]);
    return (<div style={{ width: '600px' }}>
        {limit.overlimit && (<LimitHit maxSize={limit.maxSize} name={`${translate('Block Detail Records')} (${limit.current})`}/>)}
        <ResizeTable
            size="small"
            columns={tableset.cols}
            dataSource={tableset.dataset}
            scroll={tableset.dataset.length > 10 ? { y: 400 } : undefined}
            pagination={false}
        />
    </div>);
}
function getFullCols(headerName: string[], tanslate: any): any[] {
    return headerName.map((item, index) => (
        {
            title: tanslate(item),
            dataIndex: item,
            ellipsis: true,
        }
    ));
}

const getInfoItem = (item: Ilabel, dataObj: Ibaseinfo, translate: any): Record<string, ReactNode> => {
    if (item.key === 'blockDetail') {
        return {
            ...item,
            value: <BlockDetail {...dataObj} translate={translate}/>,
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
    blockDetail: { headerName: [], row: [] },
    pid: '',
    deviceId: '',
};

const index = observer(({ session }: Iprops): JSX.Element => {
    const [data, setData] = useState<Ibaseinfo>(defaultData);
    const [items, setItems] = useState<Array<Record<string, ReactNode>>>([]);
    const { t } = useTranslation();
    const { t: tDetails } = useTranslation('details');

    const getBaseInfo = async (): Promise<void> => {
        const res = await queryBaseInfo();
        setData(res.compare ?? defaultData);
    };

    const showBaseInfo = (dataObj: Ibaseinfo): void => {
        const labellist = getLabellist(dataObj, tDetails);
        const disaplayList = labellist.map(labelItem => getInfoItem(labelItem, dataObj, tDetails));
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
        <CollapsiblePanel title={t('BaseInfo')}>
            <MIDescriptions title={''}>
                {
                    items.map((item, itemIndex) => <MIDescriptionsItem key={itemIndex} label={item.label}>
                        {item.value}
                    </MIDescriptionsItem>)
                }
            </MIDescriptions>
        </CollapsiblePanel>
    );
});

export default index;
