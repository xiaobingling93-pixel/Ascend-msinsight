import {renderHook} from "@testing-library/react-hooks";
import { useAsyncState } from "../useAsyncState";
import React, {Key} from "react";
import {act} from "react-dom/test-utils";

describe('useAsyncState test', () => {
    it('should pass when useAsyncState', function () {
        act(() => {
            const { result } = renderHook(() => useAsyncState(new Map<React.Key, number>()));
            const [ immutableStateRefCurrent, setAsyncState ] = result.current;
            expect(immutableStateRefCurrent).toEqual(new Map);
            const mockUpdater = jest.fn(<State>(prev: State) => {
                return new Map<Key, number>();
            });
            setAsyncState(mockUpdater);
        })
    });
});
