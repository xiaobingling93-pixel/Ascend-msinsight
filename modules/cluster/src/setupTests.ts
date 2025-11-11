/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import { Session } from './entity/session';
import type { Device, Process } from './entity/device';
import { cleanup } from '@testing-library/react';
import { customConsole as console } from '@insight/lib/utils';

declare global {
    const session: Session;
}

beforeEach(() => {
    const device: Device = {
        deviceKey: 'device',
        status: 'Online',
        connectType: 'connect',
        cpuAbi: 'cpu',
        apiVersion: 1,
        category: 'category',
        deviceName: 'device-name',
        productModel: 'model',
        deviceType: 'type',
        softwareVersion: 'version',
        productBrand: 'brand',
    };
    const process: Process = {
        pid: 1,
        uid: '1',
        tid: 1001,
        name: 'process',
        status: 'Alive',
        debuggable: true,
    };
    global.session = new Session({
        id: '1',
        name: 'test',
        phase: 'configuring',
        device,
        process,
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
