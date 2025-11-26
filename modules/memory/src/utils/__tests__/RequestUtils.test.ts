/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import {
    memoryTypeGet,
    resourceTypeGet,
    staticOpMemoryListGet,
    staticOpMemoryGraphGet,
    operatorsMemoryGet,
    memoryCurveGet,
    fetchTableDataByComponent,
    fetchOperatorPosition,
    fetchDynamicOperatorMaxMin,
    fetchStaticOperatorMaxMin,
} from '../RequestUtils';
import type { OperatorMemoryCondition, StaticMemoryCondition } from '../../entity/memory';
import { GroupBy } from '../../entity/memorySession';

// Mock window.request
const mockRequest = jest.fn();
window.request = mockRequest;

describe('Memory Service Tests', () => {
    beforeEach(() => {
        mockRequest.mockClear();
    });

    it('memoryTypeGet should call request with correct params', async () => {
        const params = { rankId: '1', dbPath: '/path/to/db' };
        mockRequest.mockResolvedValue('dynamic');

        const result = await memoryTypeGet(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/type',
            params,
        });
        expect(result).toBe('dynamic');
    });

    it('resourceTypeGet should call request with correct params', async () => {
        const params = { rankId: '1', dbPath: '/path/to/db' };
        mockRequest.mockResolvedValue('CPU');

        const result = await resourceTypeGet(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/resourceType',
            params,
        });
        expect(result).toBe('CPU');
    });

    it('staticOpMemoryListGet should call request with correct params', async () => {
        const params = {
            rankId: '1',
            dbPath: '/path/to/db',
        } as Partial<StaticMemoryCondition> as StaticMemoryCondition;
        const mockData = [{ operator: 'op1', memory: 100 }];
        mockRequest.mockResolvedValue(mockData);

        const result = await staticOpMemoryListGet(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/staticOpMemoryList',
            params,
        });
        expect(result).toEqual(mockData);
    });

    it('staticOpMemoryGraphGet should call request with correct params', async () => {
        const params = { rankId: '1', dbPath: '/path/to/db', graphId: '123', isCompare: true };
        const mockData = { curve: [100, 200, 300] };
        mockRequest.mockResolvedValue(mockData);

        const result = await staticOpMemoryGraphGet(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/staticOpMemoryGraph',
            params,
        });
        expect(result).toEqual(mockData);
    });

    it('operatorsMemoryGet should call request with correct params', async () => {
        const params = {
            rankId: '1',
            dbPath: '/path/to/db',
        } as Partial<OperatorMemoryCondition> as OperatorMemoryCondition;
        const mockData = [{ operator: 'op1', memory: 100 }];
        mockRequest.mockResolvedValue(mockData);

        const result = await operatorsMemoryGet(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/operator',
            params,
        });
        expect(result).toEqual(mockData);
    });

    it('memoryCurveGet should call request with correct params', async () => {
        const params = {
            rankId: '1',
            dbPath: '/path/to/db',
            type: GroupBy.DEFAULT,
            isCompare: false,
            start: '2023-01-01',
            end: '2023-01-02',
        };
        const mockData = { curve: [100, 200, 300] };
        mockRequest.mockResolvedValue(mockData);

        const result = await memoryCurveGet(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/memoryUsage',
            params,
        });
        expect(result).toEqual(mockData);
    });

    it('fetchTableDataByComponent should call request with correct params', async () => {
        const params = { rankId: '1', dbPath: '/path/to/db', component: 'comp1', isCompare: false, currentPage: 1, pageSize: 10 };
        const mockData = { data: [{ operator: 'op1', memory: 100 }] };
        mockRequest.mockResolvedValue(mockData);

        const result = await fetchTableDataByComponent(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/component',
            params,
        });
        expect(result).toEqual(mockData);
    });

    it('fetchOperatorPosition should call request with correct params', async () => {
        const params = { rankId: '1', dbPath: '/path/to/db', operator: 'op1', name: '11' };
        const mockData = { position: 100 };
        mockRequest.mockResolvedValue(mockData);

        const result = await fetchOperatorPosition(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/find/slice',
            params,
        });
        expect(result).toEqual(mockData);
    });

    it('fetchDynamicOperatorMaxMin should call request with correct params', async () => {
        const params = { rankId: '1', dbPath: '/path/to/db', operator: 'op1', type: GroupBy.DEFAULT, isCompare: true };
        const mockData = { minSize: 50, maxSize: 200 };
        mockRequest.mockResolvedValue(mockData);

        const result = await fetchDynamicOperatorMaxMin(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/operatorSize',
            params,
        });
        expect(result).toEqual(mockData);
    });

    it('fetchStaticOperatorMaxMin should call request with correct params', async () => {
        const params = { rankId: '1', dbPath: '/path/to/db', operator: 'op1', type: GroupBy.DEFAULT, isCompare: true };
        const mockData = { minSize: 50, maxSize: 200 };
        mockRequest.mockResolvedValue(mockData);

        const result = await fetchStaticOperatorMaxMin(params);

        expect(mockRequest).toHaveBeenCalledWith({
            command: 'Memory/view/staticOpMemorySize',
            params,
        });
        expect(result).toEqual(mockData);
    });
});
