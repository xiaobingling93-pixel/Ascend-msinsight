import {useOrderStatisticTree} from "../useOrderStatisticTree";
import {Key} from "react";
import {renderHook} from "@testing-library/react-hooks";

describe('useOrderStatisticTree test', () => {
    const fakePara = {
        data: [],
        childrenColumnName: '',
        expandedKeySet: new Set<Key>(),
        getRowKey: jest.fn(),
    }
    type MockRecord<T extends Record<string, unknown>> = T;
    const orderData: readonly MockRecord<Record<string, unknown>>[] = [
        { key1: 'value1' },
        { key2: 'value2' },
    ];
    it('should pass when notNull data[] Rendering correctly', function () {
        const { result } = renderHook(() => useOrderStatisticTree(orderData,fakePara.childrenColumnName,fakePara.expandedKeySet,fakePara.getRowKey));
        expect(result.current.getTotalCount()).toEqual(2);
        expect(result.current.getTotalHeight(10)).toEqual(20);
        expect(result.current.getVisibleData(10,20,30,40))
            .toEqual([{
            "children": [],
            "data":  {
                "key1": "value1",
            },
            "depth": 0,
            "parent": undefined,
            "subtreeSize": 1,
        }, {
                "children": [],
                "data":  {
                    "key2": "value2",
                },
                "depth": 0,
                "parent": undefined,
                "subtreeSize": 1,
            },]);
        expect(result.current.findNodeIndex({ key1: 'value1' })).toEqual(2);
        expect(result.current.findNodeIndex({ key2: 'value2' })).toEqual(2);
    });
    it('should pass when Null data[] Rendering correctly', function () {
        const { result } = renderHook(() => useOrderStatisticTree(fakePara.data,fakePara.childrenColumnName,fakePara.expandedKeySet,fakePara.getRowKey));
        expect(result.current.getTotalCount()).toEqual(0);
        expect(result.current.getTotalHeight(10)).toEqual(0);
        expect(result.current.getVisibleData(10,20,30,40)).toEqual([]);
        expect(result.current.findNodeIndex({ key1: 'value1' })).toEqual(0);

    });
    it('should pass when notNull data Rendering correctly', function () {
        const { result } = renderHook(() => useOrderStatisticTree({ key1: 'value1' },fakePara.childrenColumnName,fakePara.expandedKeySet,fakePara.getRowKey));
        expect(result.current.getTotalCount()).toEqual(1);
        expect(result.current.getTotalHeight(10)).toEqual(10);
        expect(result.current.getVisibleData(10,20,30,40)).toEqual([{
            "children": [],
            "data":  {
                "key1": "value1",
            },
            "depth": 0,
            "parent": undefined,
            "subtreeSize": 1,
        }]);
        expect(() => result.current.findNodeIndex({ key1: 'value1' })).toThrowError('Unsupported Data');
    });
});
