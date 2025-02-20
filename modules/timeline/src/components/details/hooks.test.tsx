/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { renderHook, act } from '@testing-library/react-hooks';
import { useDetailUpdater, useMoreUpdater, useSelectedParamsDetailUpdater, useExtraDataUpdater, useSelectedDataDetailUpdater } from './hooks';
import { EMPTY_TABLE_STATE } from './types';
import type { ColumnDef, DetailDescriptor, InsightUnit, MoreDescriptor, SingleDataDesc } from '../../entity/insight';
import { FilterType, TabState } from '../../entity/tabDependency';

describe('hooks test', () => {
    const unit: InsightUnit = {
        collapsible: false,
        name: 'unit',
        height: () => 1,
        expandable: false,
        isExpanded: false,
        isDisplay: true,
        isUnitVisible: true,
        type: 'basic',
        phase: 'download',
        isParseLoading: false,
        shouldParse: false,
        progress: 0,
        showProgress: true,
        metadata: {},
    };
    const expectedTablestate = {
        dataSource: [],
        columns: [
            {
                title: 'name2',
                key: 'name2',
                colSpan: 1,
                ellipsis: {
                    showTitle: true,
                },
                width: 'max-content',
                fixed: undefined,
                render: (): void => { },
            },
        ],
        rowKey: undefined,
        onExpand: undefined,
        loading: false,
    };
    const col: ColumnDef<Record<string, unknown>> = ['name2', (): string => 'render2', 'max-content'];

    it('useDetailUpdater test', async () => {
        let detailDescriptor: DetailDescriptor<unknown> = {
            columns: [
                col,
            ],
            fetchData: jest.fn().mockResolvedValue([]),
        };
        const tabStateLocal: TabState | undefined = new TabState({
            filter: {
                type: FilterType.INPUT_FILTER,
                field: 'name2',
                filterKeys: ['name2'],
                options: [{ label: 'name', value: '' }],
            },
        });
        const tabDep: unknown = [tabStateLocal];
        const onDataLoadFinish = jest.fn();
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail, tabState, dep, onDataLoaded }) => useDetailUpdater(session, detail, tabState, dep, onDataLoaded),
            { initialProps: { session, detail: detailDescriptor, tabState: tabStateLocal, dep: tabDep, onDataLoaded: onDataLoadFinish } },
        );
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify(expectedTablestate));

        session.selectedUnits = [unit];
        session.selectedRange = [2, 3];
        session.phase = 'download';
        await act(async () => {
            rerender({ session, detail: detailDescriptor, tabState: tabStateLocal as TabState, dep: tabDep, onDataLoaded: onDataLoadFinish });
            await waitForNextUpdate();
        });
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify(expectedTablestate));

        detailDescriptor = {
            columns: [
                col,
            ],
            fetchData: jest.fn().mockResolvedValue([]),
        };
        await act(async () => {
            rerender({ session, detail: detailDescriptor, tabState: {} as TabState, dep: tabDep, onDataLoaded: onDataLoadFinish });
            await waitForNextUpdate();
        });
        expect(onDataLoadFinish).toBeCalled();

        detailDescriptor = {
            columns: [],
            fetchData: jest.fn().mockRejectedValue('error'),
        };
        await act(async () => {
            rerender({ session, detail: detailDescriptor, tabState: tabStateLocal as TabState, dep: tabDep, onDataLoaded: onDataLoadFinish });
            await waitForNextUpdate();
        });
        expect(detailDescriptor.fetchData).toBeCalled();
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);
    });

    it('useMoreUpdater test', async () => {
        const moreDescriptor: MoreDescriptor = {
            field: 'name',
            columns: [
                col,
            ],
        };
        const { result, rerender } = renderHook(
            ({ session, more }) => useMoreUpdater(session, more),
            { initialProps: { session, more: moreDescriptor } },
        );
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);

        session.selectedDetails = [{ name: [{}] }];
        session.selectedDetailKeys = [''];
        rerender({ session, more: moreDescriptor });
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify({ ...expectedTablestate, dataSource: [{}] }));
    });

    it('useSelectedParamsDetailUpdater test', async () => {
        let detailDescriptor: DetailDescriptor<unknown> | undefined;
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail }) => useSelectedParamsDetailUpdater(session, detail),
            { initialProps: { session, detail: detailDescriptor } },
        );
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);

        detailDescriptor = {
            columns: [
                col,
            ],
            fetchData: jest.fn().mockResolvedValue([]),
        };
        session.phase = 'download';
        session.selectedParams = { baseRawId: 1, curRawId: 1 };
        await act(async () => {
            rerender({ session, detail: detailDescriptor });
            await waitForNextUpdate();
        });
        expect(detailDescriptor.fetchData).toBeCalled();
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify(expectedTablestate));

        detailDescriptor = {
            columns: [],
            fetchData: jest.fn().mockRejectedValue('error'),
        };
        await act(async () => {
            rerender({ session, detail: detailDescriptor });
            await waitForNextUpdate();
        });
        expect(detailDescriptor.fetchData).toBeCalled();
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);
    });

    it('useExtraDataUpdater test', async () => {
        let detailDescriptor: DetailDescriptor<unknown> | undefined;
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail }) => useExtraDataUpdater(session, detail),
            { initialProps: { session, detail: detailDescriptor } },
        );
        expect(result.current).toStrictEqual({});

        detailDescriptor = {
            columns: [],
            fetchData: jest.fn(),
            fetchExtraData: jest.fn().mockResolvedValue('res'),
        };
        session.phase = 'download';
        await act(async () => {
            rerender({ session, detail: detailDescriptor });
            await waitForNextUpdate();
        });
        expect(detailDescriptor.fetchExtraData).toBeCalled();
        expect(result.current).toStrictEqual({ result: 'res' });

        detailDescriptor = {
            columns: [],
            fetchData: jest.fn(),
            fetchExtraData: jest.fn().mockRejectedValue('error'),
        };
        await act(async () => {
            rerender({ session, detail: detailDescriptor });
            await waitForNextUpdate();
        });
        expect(detailDescriptor.fetchExtraData).toBeCalled();
        expect(result.current).toStrictEqual({});
    });

    it('useSelectedDataDetailUpdater test', async () => {
        let dataDesc: SingleDataDesc<Record<string, unknown>, unknown> = {
            fetchData: jest.fn().mockResolvedValue({ result: '' }),
            renderFields: [],
        };
        let selectedRecord: Record<string, string> | undefined;
        session.phase = 'download';
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail, selectedData }) => useSelectedDataDetailUpdater(session, detail, selectedData),
            { initialProps: { session, detail: dataDesc, selectedData: selectedRecord } },
        );
        expect(result.current).toStrictEqual({ renderFields: undefined, data: {} });

        session.selectedUnits = [unit];
        selectedRecord = { name: 'test' };
        await act(async () => {
            rerender({ session, detail: dataDesc, selectedData: selectedRecord });
            await waitForNextUpdate();
        });
        expect(dataDesc.fetchData).toBeCalled();
        expect(result.current).toStrictEqual({
            renderFields: [],
            data: { result: '' },
        });

        const expectedRes = {
            renderFields: [
                ['Name', 'Name'],
            ],
            data: { result: '' },
        };
        const render = jest.fn().mockReturnValue('Name');
        dataDesc = {
            fetchData: jest.fn().mockResolvedValue({ result: '' }),
            renderFields: [
                ['Name', render],
            ],
        };
        await act(async () => {
            rerender({ session, detail: dataDesc, selectedData: selectedRecord });
            await waitForNextUpdate();
        });
        expect(dataDesc.fetchData).toBeCalled();
        expect(render).toBeCalled();
        expect(result.current).toStrictEqual(expectedRes);

        const hidden = jest.fn().mockReturnValue(false);
        dataDesc = {
            fetchData: jest.fn().mockResolvedValue({ result: '' }),
            renderFields: [
                ['Name', render, hidden],
            ],
        };
        await act(async () => {
            rerender({ session, detail: dataDesc, selectedData: selectedRecord });
            await waitForNextUpdate();
        });
        expect(dataDesc.fetchData).toBeCalled();
        expect(hidden).toBeCalled();
        expect(result.current).toStrictEqual(expectedRes);

        dataDesc = {
            fetchData: jest.fn().mockRejectedValue('error'),
            renderFields: [],
        };
        await act(async () => {
            rerender({ session, detail: dataDesc, selectedData: selectedRecord });
            await waitForNextUpdate();
        });
        expect(dataDesc.fetchData).toBeCalled();
        expect(result.current).toStrictEqual(expectedRes);
    });
});
