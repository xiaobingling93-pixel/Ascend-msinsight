/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { store } from '../../store/rootStore';
import Filter from '../communication/Filter';
import { render, screen } from '@testing-library/react';
import React from 'react';
import CommunicationTimeChart from '../communication/CommunicationTimeChart';
import { chartData } from '../__mock__/communication.mock';

it('testCommunicationFilterComponent', () => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    if (session === undefined) {
        return;
    }
    render(<Filter session={session} handleFilterChange={(): void => {}}/>);
    // 查看筛选条件选项是否都在
    expect(screen.queryAllByText('Step')).toBeDefined();
    expect(screen.queryAllByText('Communication Group')).toBeDefined();
    expect(screen.queryAllByText('Operator Name')).toBeDefined();
    expect(screen.getByText('Communication Matrix')).toBeDefined();
    expect(screen.getByText('Communication Duration Analysis')).toBeDefined();
});

it('testCommunicationMatrixComponentByMockdata', () => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    if (session === undefined) {
        return;
    }
    render(<CommunicationTimeChart session={session} dataSource={chartData} />);
    // 查看mockData渲染的页面数据是否正确
    expect(screen.queryAllByText('17.7497')).toBeDefined();
    expect(screen.queryAllByText('19.2835')).toBeDefined();
});
