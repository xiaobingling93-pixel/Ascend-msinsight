import { observer } from 'mobx-react';
import React, { useState } from 'react';
import { Select, Radio, Form } from 'antd';
import { Label, Space } from './Common';

const options = [
    {
        value: '0',
        label: '0',
    },
    {
        value: '1',
        label: '1',
    },
    {
        value: '2',
        label: '2',
    },
    {
        value: '3',
        label: '3',
    },
];

const Filter = observer((props: any) => {
    const [ iteration, setIteration ] = useState('');
    const [ operator, setOperator ] = useState('');
    const handleChange = (name: string, value: string): void => {
        const obj: {[propName: string]: any} = { iteration, operator };
        obj[name.toLowerCase()] = value;
        if (name === 'Iteration') {
            setIteration(value);
        } else if (name === 'Operator') {
            setOperator(value);
        }

        props.handleFilterChange(obj);
    };
    return (<div style={ { margin: '0 20px', ...props.style ?? {} }}>
        <Label name="Iteration ID"/>
        <Select
            defaultValue="0"
            style={{ width: 120 }}
            onChange={val => handleChange('Iteration', val)}
            options={options}
        />
        <Label name="Operator Name"/>
        <Select
            defaultValue="0"
            style={{ width: 200 }}
            onChange={val => handleChange('Operator', val)}
            options={options}
        />
        <Space length={20}/>
        <Radio.Group value={props.showWindow} onChange={(e) => { props.switchPage(e.target.value); }}>
            <Radio value={'CommunicationDurationAnalysis'}>Communication Duration Analysis</Radio>
            <Radio value={'CommunicationMatrix'}>Communication Matrix</Radio>
        </Radio.Group>
    </div>);
});

export const FilterForm = observer((props: any) => {
    const [form] = Form.useForm();
    const [ page, setPage ] = useState(0);
    const onFinish = (values: any): void => {
        console.log(values);
    };
    const handleChange = (value: string): void => {
        console.log(`selected ${value}`);
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
                    options={options}
                />
            </Form.Item>
            <Form.Item name="OperatorName" label="Operator Name">
                <Select
                    defaultValue="lucy"
                    style={{ width: 120 }}
                    onChange={handleChange}
                    options={options}
                />
            </Form.Item>
            <Form.Item {...tailLayout}>
                <Radio.Group onChange={(e) => {
                    setPage(e.target.value);
                    props.switchPage(e.target.value);
                }} value={page}>
                    <Radio value={0}>Communication Duration Analysis</Radio>
                    <Radio value={1}>Communication Matrix</Radio>
                </Radio.Group>
            </Form.Item>
        </Form>
    );
});

export default Filter;
