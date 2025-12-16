/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
