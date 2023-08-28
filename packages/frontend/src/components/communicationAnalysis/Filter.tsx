/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import { Select, Radio, Form } from 'antd';
import { Label, MultiSelectWithAll, notNullObj } from '../Common';
import { optionDataType, VoidFunction } from '../../utils/interface';
import { queryIterations, queryOperators, queryRanks } from '../../utils/RequestUtils';
import { Session } from '../../entity/session';

export interface conditionDataType{
    iterationId: string ;
    rankIds: string[];
    operatorName: string ;
    type: string;
    stageId: string;
}

interface optionMapDataType{
    [props: string]: optionDataType[];
}

const getiterationOptions = async(): Promise<optionDataType[]> => {
    const iterationRes: {iterationsOrRanks: Array<{iteration_id: string } > } = await queryIterations();
    const iterationlist: string[] = iterationRes.iterationsOrRanks.map(item => item.iteration_id);
    const iterationOptions: optionDataType[] = iterationlist.map(item => ({ value: item, label: item }));
    return iterationOptions;
};

const getStageOptions = (): optionDataType[] => {
    const stageList = [ 0, 1, 2, 3, 4, 5, 6, 7 ];
    const stageOptions = stageList.map(item => ({ value: item, label: item }));
    return stageOptions;
};

const getOptions = async(): Promise<any> => {
    const iterationOptions: optionDataType[] = await getiterationOptions();
    const defaultIterationId = iterationOptions[0]?.value as string;
    const rankRes: {iterationsOrRanks: Array<{rank_id: number } > } = await queryRanks({ iterationId: defaultIterationId });
    const rankIds: number[] = rankRes.iterationsOrRanks.map(item => item.rank_id);
    const rankIdOptions: optionDataType[] = rankIds.map(item => ({ value: item, label: item }));
    const operatorRes: any = await queryOperators({ iterationId: defaultIterationId, rankIds: [] });
    const operatorlist: string[] = operatorRes.operators.map((item: any) => item.op_name);
    const operatorOptions: optionDataType[] = operatorlist.map(item => ({ value: item, label: item }));
    const stageOptions: optionDataType[] = getStageOptions();
    return {
        iterationOptions,
        rankIdOptions,
        operatorOptions,
        stageOptions,
        iterationId: defaultIterationId,
        stageId: stageOptions[0]?.value,
    };
};

export const totalOperator = 'Total Op Info';

const Filter = observer((props: {session: Session;handleFilterChange: VoidFunction}) => {
    const [ conditions, setConditions ] = useState<conditionDataType>(
        { iterationId: '', rankIds: [], operatorName: '', type: 'CommunicationDurationAnalysis', stageId: '' });
    const [ options, setOptions ] = useState<optionMapDataType>(
        { iterationOptions: [], operatorOptions: [], rankIdOptions: [], stageOptions: [] },
    );

    // 初始化
    useEffect(() => {
        init();
    }, [props.session.allRankIds]);
    // Iteration ID联动Rank ID
    useEffect(() => {
        updateRanks(conditions.iterationId);
    }, [conditions.iterationId]);
    // Rank ID 联动算子选项
    useEffect(() => {
        updateOperator(conditions.rankIds);
    }, [conditions.rankIds]);
    // 筛选条件变化
    useEffect(() => {
        if (notNullObj(conditions)) {
            props.handleFilterChange(conditions);
        }
    }, [conditions]);

    const init = async(): Promise<void> => {
        const optionsObj = await getOptions();
        const { iterationOptions, rankIdOptions, operatorOptions, iterationId, stageOptions, stageId } = optionsObj;
        // 初始可选项
        setOptions({ ...options, iterationOptions, rankIdOptions, operatorOptions, stageOptions });
        // 初始查询条件
        setConditions({ ...conditions, iterationId, operatorName: totalOperator, stageId });
    };

    const updateRanks = async (iterationId: string): Promise<void> => {
        const rankRes: {iterationsOrRanks: Array<{rank_id: number } > } = await queryRanks({ iterationId });
        const rankIds: number[] = rankRes.iterationsOrRanks.map(item => item.rank_id);
        const rankIdOptions: optionDataType[] = rankIds.map(item => ({ value: item, label: item }));
        setOptions({ ...options, rankIdOptions });
        setConditions({ ...conditions, rankIds: [] });
    };

    const updateOperator = async (rankIds: string[]): Promise<void> => {
        const operatorRes = await queryOperators({ iterationId: conditions.iterationId, rankIds });
        const operatorlist: string[] = operatorRes.operators.map((item: any) => item.op_name);
        const operatorOptions: optionDataType[] = operatorlist.map(item => ({ value: item, label: item }));
        setOptions({ ...options, operatorOptions });
        if (conditions.operatorName !== totalOperator && !operatorlist.includes(conditions.operatorName)) {
            setConditions({ ...conditions, operatorName: totalOperator });
        }
    };

    const handleChange = (prop: keyof conditionDataType, val: string | number | string[] | number[]): void => {
        setConditions({ ...conditions, [prop]: val });
    };

    return (<FilterCom conditions={conditions} handleChange={handleChange} options={options} />);
});

const FilterCom = ({ conditions, handleChange, options = {} }: any): JSX.Element => {
    return (<div>
        <FormItem
            name="Step"
            content={(<Select
                value={conditions.iterationId}
                style={{ width: 120 }}
                onChange={val => handleChange('iterationId', val)}
                options={options.iterationOptions}
            />
            )}/>
        <FormItem
            name="Stage"
            content={(<Select
                value={conditions.stageId}
                style={{ width: 120 }}
                onChange={val => handleChange('stageId', val)}
                options={options.stageOptions}
            />
            )}/>
        <FormItem
            name="Rank ID"
            content={(<MultiSelectWithAll
                value={conditions.rankIds}
                onChange={(val: any) => handleChange('rankIds', val)}
                options={options.rankIdOptions}
                maxTagCount={2}
                style={{ width: 200 }}
            />
            )}/>
        <FormItem
            name="Operator Name"
            content={(
                <Select
                    value={conditions.operatorName}
                    style={{ width: 300 }}
                    onChange={val => handleChange('operatorName', val)}
                    options={options.operatorOptions}
                    showSearch={true}
                />)}/>
        <FormItem content={(
            <Radio.Group value={conditions.type}
                onChange={(e) => { handleChange('type', e.target.value); }}>
                <Radio value={'CommunicationDurationAnalysis'}>Communication Duration Analysis</Radio>
                <Radio value={'CommunicationMatrix'}>Communication Matrix</Radio>
            </Radio.Group>)}/>
        <div>
        </div>
    </div>);
};

const FormItem = (props: any): JSX.Element => {
    return (<div style={{ display: 'inline-block', height: '30px', lineHeight: '30px', margin: '0 20px 10px 0' }}>
        <Label name={props.name}/>
        {props.content}
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
