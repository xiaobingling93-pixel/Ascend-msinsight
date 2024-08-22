/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
    const dom = render(<CoreChart condition={{ showAs: 'cycles' }} data={coreData.opDetails.slice(0, 3)}/>);
    // 对比快照
    expect(dom.baseElement).toMatchSnapshot();
});
