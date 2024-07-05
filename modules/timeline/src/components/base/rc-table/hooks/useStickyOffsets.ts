import { useMemo } from 'react';
import { StickyOffsets } from '../types';

export function useStickyOffsets(colWidths: number[], columnCount: number): StickyOffsets {
    const stickyOffsets: StickyOffsets = useMemo(() => {
        const leftOffsets: number[] = [];
        const rightOffsets: number[] = [];
        let left = 0;
        let right = 0;
    
        for (let start = 0; start < columnCount; start += 1) {
            // Left offset
            leftOffsets[start] = left;
            left += colWidths[start] || 0;
    
            // Right offset
            const end = columnCount - start - 1;
            rightOffsets[end] = right;
            right += colWidths[end] || 0;
        }
    
        return {
            left: leftOffsets,
            right: rightOffsets,
        };
    }, [colWidths, columnCount]);
  
    return stickyOffsets;
}
