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
import React from 'react';
import { render } from '@testing-library/react';
import { coreData } from '../__mock__/data.mock';
import { store } from '@/store/rootStore';
import CoreChart from '../detail/CoreOccupancy/CoreChart';
import '@testing-library/jest-dom';

it('testCoreChartComponent', () => {
    const { sessionStore } = store;
    const session = sessionStore.activeSession;
    if (session === undefined) {
        return;
    }
    const dom = render(<CoreChart condition={{ showAs: 'cycles', isCompared: false }} data={coreData}/>);
    // 对比快照
    const res = dom.baseElement.getElementsByClassName('core');
    expect(res[15]).toMatchSnapshot();
    expect(res[19]).toMatchSnapshot();
});
