/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { Session } from './entity/session';
import { cleanup } from '@testing-library/react';
import { customConsole as console } from 'ascend-utils';
import '@testing-library/jest-dom';

declare global {
    const session: Session;
}

beforeEach(() => {
    global.session = new Session({
        id: '1',
        name: 'test',
        phase: 'configuring',
        units: [],
        availableUnits: [],
        startRecordTime: 1,
        endTimeAll: 2,
        isNsMode: true,
    });
});

afterEach(() => {
    cleanup();
    jest.clearAllMocks();
    jest.clearAllTimers();
});

process.on('unhandledRejection', (reason, p) => {
    console.log('Unhandled Rejection at: Promise', p, 'reason:', reason);
});
