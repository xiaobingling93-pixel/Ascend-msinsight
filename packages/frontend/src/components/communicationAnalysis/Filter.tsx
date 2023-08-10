/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { Select, Radio, Form } from 'antd';
import { Label, Space, MultiSelectWithAll } from './Common';
import { optionDataType } from '../../utils/interface';

export interface conditionDataType{
    iterationId: string | number;
    rankIds: string[];
    operatorName: string | number;
    type: string;
}

interface optionMapDataType{
    [props: string]: optionDataType[];
}

const Filter = observer((props: any) => {
    const { handleFilterChange } = props;
    // 获取可选项
    useEffect(() => {
        getOptions();
    }, []);
    const [ options, setOptions ] = useState<optionMapDataType>({});
    const getOptions = (): void => {
        const iterationlist = [ 0, 1, 2, 3, 4, 5 ];
        const iterationOptions: optionDataType[] = iterationlist.map(item => ({ value: item, label: item }));
        const operatorlist = [ 'allGather_1_1', 0, 1 ];
        const operatorOptions: optionDataType[] = operatorlist.map(item => ({ value: item, label: item }));
        const rankIds = [ 0, 1, 2, 3 ];
        const rankIdOptions: optionDataType[] = rankIds.map(item => ({ value: item, label: item }));
        setOptions({ iterationOptions, operatorOptions, rankIdOptions });
        setConditions({ ...conditions, iterationId: iterationlist[0], operatorName: operatorlist[0] });
    };
    // 筛选条件变化
    const [ conditions, setConditions ] = useState<conditionDataType>(
        { iterationId: '', rankIds: [], operatorName: '', type: 'CommunicationDurationAnalysis' });
    const handleChange = (prop: keyof conditionDataType, val: string | number | string[]): void => {
        setConditions({ ...conditions, [prop]: val });
    };
    useEffect(() => {
        handleFilterChange(conditions);
    }, [conditions]);

    return (<FilterCom conditions={conditions} handleChange={handleChange} options={options} />);
});

const FilterCom = (props: any): JSX.Element => {
    const { conditions, handleChange, options = {} } = props;
    return (<div style={ { margin: '0 20px 10px' }}>
        <Label name="Iteration ID" />
        <Select
            value={conditions.iterationId}
            style={{ width: 120 }}
            onChange={val => handleChange('iterationId', val)}
            options={options.iterationOptions}
        />
        <Label name="Rank ID"/>
        <MultiSelectWithAll
            value={conditions.rankIds}
            onChange={(val: any) => handleChange('rankIds', val)}
            options={options.rankIdOptions}
            maxTagCount={2}
            style={{ width: 200 }}
        />
        <Label name="Operator Name"/>
        <Select
            value={conditions.operatorName}
            style={{ width: 300 }}
            onChange={val => handleChange('operatorName', val)}
            options={options.operatorOptions}
        />
        <Space length={20}/>
        <Radio.Group value={conditions.type} onChange={(e) => { handleChange('type', e.target.value); }}>
            <Radio value={'CommunicationDurationAnalysis'}>Communication Duration Analysis</Radio>
        </Radio.Group>
    </div>);
};

export const FilterForm = observer((props: any) => {
    const [form] = Form.useForm();
    const [ page, setPage ] = useState(0);
    const onFinish = (values: any): void => {
    };
    const handleChange = (value: string): void => {

    };

    const layout = {
        labelCol: { span: 8 },
        wrapperCol: { span: 16 },
    };
    const tailLayout = {
        wrapperCol: { offset: 8, span: 16 },
    };

    return (
        <Form {...layout} form={form} name="filter" onFinish={onFinish}>
            <Form.Item name="IterationID" label="Iteration ID">
                <Select
                    defaultValue="lucy"
                    style={{ width: 120 }}
                    onChange={handleChange}
                    options={[]}
                />
            </Form.Item>
            <Form.Item name="OperatorName" label="Operator Name">
                <Select
                    defaultValue="lucy"
                    style={{ width: 120 }}
                    onChange={handleChange}
                    options={[]}
                />
            </Form.Item>
            <Form.Item {...tailLayout}>
                <Radio.Group onChange={(e) => {
                    setPage(e.target.value);
                    props.switchWindow(e.target.value);
                }} value={page}>
                    <Radio value={0}>Communication Duration Analysis</Radio>
                    <Radio value={1}>Communication Matrix</Radio>
                </Radio.Group>
            </Form.Item>
        </Form>
    );
});

export default Filter;
