import { renderHook, act } from '@testing-library/react-hooks';
import { useDetailUpdater, useMoreUpdater, useSelectedParamsDetailUpdater, useExtraDataUpdater, useSelectedDataDetailUpdater } from './hooks';
import { EMPTY_TABLE_STATE } from './types';
import { ColumnDef, DetailDescriptor, InsightUnit, MoreDescriptor, SingleDataDesc } from '../../entity/insight';
import { FilterType, TabState } from '../../entity/tabDependency';

describe('hooks test', () => {
    const unit: InsightUnit = {
        name: 'unit',
        height: () => 1,
        expandable: false,
        isExpanded: false,
        isDisplay: true,
        type: 'basic',
        phase: 'download'
    };
    const expectedTablestate = {
        data: [],
        columns: [
            {
                title: "name2",
                key: "name2",
                colSpan: 1,
                ellipsis: {
                    showTitle: false
                },
                width: "max-content",
                fixed: undefined,
                render: () => { }
            }
        ],
        rowKey: undefined,
        onExpand: undefined,
        isLoading: false,
    };
    const col: ColumnDef<Record<string, unknown>> = ['name2', () => 'render2', 'max-content'];

    it('useDetailUpdater test', async () => {
        let detail: DetailDescriptor<unknown> = {
            columns: [
                col,
            ],
            fetchData: jest.fn().mockResolvedValue([]),
        };
        let tabState: TabState | undefined = new TabState({
            filter: {
                type: FilterType.INPUT_FILTER,
                field: 'name2',
                filterKeys: ['name2'],
                options: [{ label: 'name', value: '' }]
            }
        });
        let dep: unknown = [tabState];
        let onDataLoaded = jest.fn();
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail, tabState, dep, onDataLoaded }) => useDetailUpdater(session, detail, tabState, dep, onDataLoaded),
            { initialProps: { session, detail, tabState, dep, onDataLoaded } }
        )
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify(expectedTablestate));

        session.selectedUnits = [unit];
        session.selectedRange = [2, 3];
        session.phase = 'download';
        await act(async () => {
            rerender({ session, detail, tabState: tabState as TabState, dep, onDataLoaded });
            await waitForNextUpdate();
        });
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify(expectedTablestate));

        detail = {
            columns: [
                col,
            ],
            fetchData: jest.fn().mockResolvedValue([]),
        }
        await act(async () => {
            rerender({ session, detail, tabState: {} as TabState, dep, onDataLoaded });
            await waitForNextUpdate();
        });
        expect(onDataLoaded).toBeCalled();

        detail = {
            columns: [],
            fetchData: jest.fn().mockRejectedValue('error'),
        }
        await act(async () => {
            rerender({ session, detail, tabState: tabState as TabState, dep, onDataLoaded });
            await waitForNextUpdate();
        });
        expect(detail.fetchData).toBeCalled();
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);
    })

    it('useMoreUpdater test', async () => {
        const more: MoreDescriptor = {
            field: 'name',
            columns: [
                col,
            ]
        }
        const { result, rerender } = renderHook(
            ({ session, more }) => useMoreUpdater(session, more),
            { initialProps: { session, more } }
        )
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);

        session.selectedDetails = [{ name: [{}] }];
        session.selectedDetailKeys = [''];
        rerender({ session, more });
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify({ ...expectedTablestate, data: [{}] }));
    })

    it('useSelectedParamsDetailUpdater test', async () => {
        let detail: DetailDescriptor<unknown> | undefined;
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail }) => useSelectedParamsDetailUpdater(session, detail),
            { initialProps: { session, detail } }
        );
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);

        detail = {
            columns: [
                col,
            ],
            fetchData: jest.fn().mockResolvedValue([]),
        }
        session.phase = 'download';
        session.selectedParams = { baseRawId: 1, curRawId: 1 };
        await act(async () => {
            rerender({ session, detail });
            await waitForNextUpdate();
        });
        expect(detail.fetchData).toBeCalled();
        expect(JSON.stringify(result.current)).toEqual(JSON.stringify(expectedTablestate));

        detail = {
            columns: [],
            fetchData: jest.fn().mockRejectedValue('error'),
        }
        await act(async () => {
            rerender({ session, detail });
            await waitForNextUpdate();
        });
        expect(detail.fetchData).toBeCalled();
        expect(result.current).toStrictEqual(EMPTY_TABLE_STATE);
    })

    it('useExtraDataUpdater test', async () => {
        let detail: DetailDescriptor<unknown> | undefined;
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail }) => useExtraDataUpdater(session, detail),
            { initialProps: { session, detail } }
        );
        expect(result.current).toStrictEqual({});

        detail = {
            columns: [],
            fetchData: jest.fn(),
            fetchExtraData: jest.fn().mockResolvedValue('res'),
        }
        session.phase = 'download';
        await act(async () => {
            rerender({ session, detail });
            await waitForNextUpdate();
        });
        expect(detail.fetchExtraData).toBeCalled();
        expect(result.current).toStrictEqual({ result: 'res' });

        detail = {
            columns: [],
            fetchData: jest.fn(),
            fetchExtraData: jest.fn().mockRejectedValue('error'),
        }
        await act(async () => {
            rerender({ session, detail });
            await waitForNextUpdate();
        });
        expect(detail.fetchExtraData).toBeCalled();
        expect(result.current).toStrictEqual({});
    })

    it('useSelectedDataDetailUpdater test', async () => {
        let detail: SingleDataDesc<Record<string, unknown>, unknown> = {
            fetchData: jest.fn().mockResolvedValue({ result: '' }),
            renderFields: []
        };
        let selectedData: Record<string, string> | undefined;
        session.phase = 'download';
        const { result, rerender, waitForNextUpdate } = renderHook(
            ({ session, detail, selectedData }) => useSelectedDataDetailUpdater(session, detail, selectedData),
            { initialProps: { session, detail, selectedData } }
        );
        expect(result.current).toStrictEqual({ renderFields: undefined, data: {} });

        session.selectedUnits = [unit];
        selectedData = { name: 'test' };
        await act(async () => {
            rerender({ session, detail, selectedData });
            await waitForNextUpdate();
        });
        expect(detail.fetchData).toBeCalled();
        expect(result.current).toStrictEqual({
            renderFields: [],
            data: { result: '' }
        });

        const ExpectedRes = {
            renderFields: [
                ['Name', 'Name']
            ],
            data: { result: '' }
        };
        const render = jest.fn().mockReturnValue('Name');
        detail = {
            fetchData: jest.fn().mockResolvedValue({ result: '' }),
            renderFields: [
                ['Name', render],
            ]
        };
        await act(async () => {
            rerender({ session, detail, selectedData });
            await waitForNextUpdate();
        });
        expect(detail.fetchData).toBeCalled();
        expect(render).toBeCalled();
        expect(result.current).toStrictEqual(ExpectedRes);

        const hidden = jest.fn().mockReturnValue(true);
        detail = {
            fetchData: jest.fn().mockResolvedValue({ result: '' }),
            renderFields: [
                ['Name', render, hidden],
            ]
        };
        await act(async () => {
            rerender({ session, detail, selectedData });
            await waitForNextUpdate();
        });
        expect(detail.fetchData).toBeCalled();
        expect(hidden).toBeCalled();
        expect(result.current).toStrictEqual(ExpectedRes);

        detail = {
            fetchData: jest.fn().mockRejectedValue('error'),
            renderFields: []
        };
        await act(async () => {
            rerender({ session, detail, selectedData });
            await waitForNextUpdate();
        });
        expect(detail.fetchData).toBeCalled();
        expect(result.current).toStrictEqual(ExpectedRes);
    })
})
