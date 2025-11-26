/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { store } from '../../store/rootStore';
import Filter from '../communication/Filter';
import { render, screen } from '@testing-library/react';
import React from 'react';

it('testCommunicationFilterComponent', () => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    if (session === undefined) {
        return;
    }
    render(<Filter session={session} handleFilterChange={(): void => {}}/>);
    // 查看筛选条件选项是否都在
    expect(screen.queryAllByText('Step')).toBeInTheDocument();
    expect(screen.queryAllByText('Communication Group')).toBeInTheDocument();
    expect(screen.queryAllByText('Operator Name')).toBeInTheDocument();
    expect(screen.getByText('Communication Matrix')).toBeInTheDocument();
    expect(screen.getByText('Communication Duration Analysis')).toBeInTheDocument();
});
