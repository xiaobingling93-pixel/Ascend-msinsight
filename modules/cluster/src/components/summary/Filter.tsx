/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Select } from 'antd';
import { Label } from '../Common';
import _ from 'lodash';
import { communicator } from '../communicatorContainer/ContainerUtils';
import { queryTopSummary } from '../../utils/RequestUtils';
import { Session } from '../../entity/session';

export interface ConditionDataType {
    step: string ;
    rankIds: string[];
    orderBy: string ;
    top: number;
    group?: string;
}

interface optionDataType{
    key?: string;
    label: React.ReactNode;
    value: string | number ;
    data?: string[];
}

interface optionMapDataType{
    [props: string]: optionDataType[];
}
const orderOptions = [
    { label: 'Total Computing', value: 'computingTime' },
    { label: 'Pure Computing', value: 'pureComputingTime' },
    { label: 'Communication(Overlapped)', value: 'communicationOverLappedTime' },
    { label: 'Communication(Not Overlapped)', value: 'communicationNotOverLappedTime' },
    { label: 'Free', value: 'freeTime' },
    { label: 'Rank ID', value: 'rankId' },
];

// Top可选项： 1、2、4、8.......n(All)
const getTopOptions = (count: number): optionDataType[] => {
    const logIndex = Math.ceil(Math.log2(count > 0 ? count : 1));
    const toplist = [];
    for (let i = 0; i < logIndex; i++) {
        toplist.push(Math.pow(2, i));
    }
    const topOptions: optionDataType[] = toplist.map(item => ({ value: item, label: item }));
    if (count > 0) {
        topOptions.push({ value: count, label: `${count} ( All )` });
    }
    return topOptions;
};

export const defaultConditions = { step: 'All', rankIds: [], orderBy: 'computingTime', top: 0 };

const getStepOptions = async(): Promise<optionDataType[]> => {
    const res = await window.requestData('parallelism/pipeline/getAllSteps', {}, 'summary');
    const list: string[] = res?.data ?? [];
    const options: optionDataType[] = ['All', ...list].map(item => ({ value: item, label: item }));
    return options;
};

// eslint-disable-next-line max-lines-per-function
const Filter = observer((props: any) => {
    const session: Session = props.session;
    const [conditions, setConditions] = useState<ConditionDataType>(defaultConditions);
    const [options, setOptions] = useState<optionMapDataType>({});
    // 初始化
    useEffect(() => {
        if (!session.clusterCompleted) {
            setConditions(defaultConditions);
            setOptions({ ...options, stepOptions: [], topOptions: [], groupOptions: [] });
            return;
        }
        initDefault();
    }, [session.communicatorData]);
    useEffect(() => {
        if (_.find(options.groupOptions, item => item.value === conditions.group) !== undefined) {
            conditions.rankIds = _.find(options.groupOptions, item => item.value === conditions.group)?.data as string[];
        }
        props.handleFilterChange(conditions);
    }, [conditions.step, conditions.orderBy, conditions.group, props.visible]);
    useEffect(() => {
        props.handleFilterChange(conditions, false);
    }, [conditions.top]);
    useEffect(() => {
        const find = _.find(options.groupOptions, item => item.key === props.session.activeCommunicator?.name);
        if (find !== undefined) {
            setConditions({ ...conditions, group: find.value as string });
        }
    }, [props.session.activeCommunicator]);
    const initDefault = async (): Promise<void> => {
        const stepOptions = await getStepOptions();
        const summaryRes: any = await queryTopSummary(conditions);
        const rankList = summaryRes?.rankList ?? [];
        const communicators: communicator[] = [];
        _.filter(props.session.communicatorData.partitionModes, data => data.mode !== 'pp')
            .map(data => data.communicators).forEach((item) => {
                _.forEach(item, tmp => {
                    communicators.push(tmp);
                });
            });
        const groupOptions: optionDataType[] = _.map(communicators, value => ({
            value: value.value as string,
            label: value.value,
            data: value.ranks.map(item => item.toString()),
            key: value.name,
        }));
        const group = groupOptions.length > 0 ? groupOptions[0].value as string : '';
        const topOptions = getTopOptions(rankList.length);
        setOptions({ stepOptions, topOptions, groupOptions, orderOptions });
        setConditions({ ...conditions, top: rankList.length, group });
    };

    const handleChange = (prop: keyof ConditionDataType, val: string | number | string[]): void => {
        setConditions({ ...conditions, [prop]: val });
    };

    return (<FilterCom conditions={conditions} handleChange={handleChange} options={options} session={props.session}/>);
});

const FilterCom = (props: any): JSX.Element => {
    const { conditions, handleChange = [], options = {} } = props;
    const session: Session = props.session;
    const { t } = useTranslation('summary');
    const tOrderOptions = options?.orderOptions?.map((item: any) => {
        return {
            ...item,
            label: t(item.label),
        };
    });
    return (<div style={ { margin: '0 20px 10px' }}>
        {
            !(session.isFullDb)
                ? <Label name={t('Step')} />
                : <></>
        }
        {
            !(session.isFullDb)
                ? <Select
                    value={conditions.step}
                    style={{ width: 120 }}
                    onChange={(val: any) => handleChange('step', val)}
                    options={options.stepOptions}
                />
                : <></>
        }
        <Label name={t('RankGroup')}/>
        <Select
            defaultValue={conditions.group}
            value={conditions.group}
            style={{ width: 200 }}
            onChange={(val: any) => handleChange('group', val)}
            options={options.groupOptions}
        />
        <Label name={t('OrderBy')}/>
        <Select
            value={conditions.orderBy}
            style={{ width: 280 }}
            onChange={(val: any) => handleChange('orderBy', val)}
            options={tOrderOptions}
        />
        <Label name={t('Top')}/>
        <Select
            value={conditions.top}
            style={{ width: 120 }}
            onChange={(val: any) => handleChange('top', val)}
            options={options.topOptions}
        />
    </div>);
};

export default Filter;
