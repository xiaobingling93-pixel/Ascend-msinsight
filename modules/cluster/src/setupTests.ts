/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { Session } from './entity/session';
import { cleanup } from '@testing-library/react';
import { customConsole as console } from '@insight/lib/utils';

declare global {
    const session: Session;
}

Object.defineProperty(window, 'matchMedia', {
    writable: true,
    value: jest.fn().mockImplementation(query => ({
        matches: false,
        media: query,
        onchange: null,
        addListener: jest.fn(),
        removeListener: jest.fn(),
        addEventListener: jest.fn(),
        removeEventListener: jest.fn(),
        dispatchEvent: jest.fn(),
    })),
});

beforeEach(() => {
    global.session = new Session({
        id: '1',
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
