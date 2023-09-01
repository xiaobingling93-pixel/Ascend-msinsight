import {renderHook} from "@testing-library/react-hooks";
import {useStickyOffsets} from "../useStickyOffsets";

test('useStickyOffsets works as expected', () => {
    const { result } = renderHook(() => useStickyOffsets([100,200,300], 3));
    const stickyOffsets = result.current;
    expect(stickyOffsets.left).toEqual([0,100,300]);
    expect(stickyOffsets.right).toEqual([500,300,0]);
});
