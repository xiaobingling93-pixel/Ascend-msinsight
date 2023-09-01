import { FixedType, StickyOffsets } from '../types';

export interface FixedInfo {
    fixLeft?: number | false;
    fixRight?: number | false;
    lastFixLeft?: boolean;
    firstFixRight?: boolean;
    isSticky?: boolean;
}

export function getCellFixedInfo(
    start: number,
    end: number,
    columns: readonly { fixed?: FixedType }[],
    stickyOffsets: StickyOffsets,
): FixedInfo {
    const startColumn = columns[start] || {};
    const endColumn = columns[end] || {};
  
    let fixLeft: number | undefined;
    let fixRight: number | undefined;
  
    if (startColumn.fixed === 'left') {
        fixLeft = stickyOffsets.left[start];
    } else if (endColumn.fixed === 'right') {
        fixRight = stickyOffsets.right[end];
    }
  
    let lastFixLeft = false;
    let firstFixRight = false;
    const nextColumn = columns[end + 1];
    const prevColumn = columns[start - 1];
  
    if (fixLeft !== undefined) {
        lastFixLeft = !(nextColumn && nextColumn.fixed === 'left');
    } else if (fixRight !== undefined) {
        firstFixRight = !(prevColumn && prevColumn.fixed === 'right');
    }
  
    return {
        isSticky: stickyOffsets.isSticky,
        fixLeft,
        fixRight,
        lastFixLeft,
        firstFixRight,
    };
}
