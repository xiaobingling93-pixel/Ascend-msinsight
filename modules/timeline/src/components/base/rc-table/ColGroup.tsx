import { ColumnType } from './types';

export interface ColGroupProps<RecordType> {
    colWidths: readonly (number | string | undefined)[];
    columns?: readonly ColumnType<RecordType>[];
    columCount?: number;
}

export function ColGroup<RecordType>({ colWidths, columns, columCount }: ColGroupProps<RecordType>): JSX.Element {
    const cols: React.ReactElement[] = [];
    const len = columCount || columns!.length;
  
    let shouldInsert = false;
    for (let i = len - 1; i >= 0; i--) {
        const width = columns?.[i]?.width ?? colWidths[i];
    
        if (width || shouldInsert) {
            cols.unshift(<col key={i} width={width} />);
            shouldInsert = true;
        }
    }
  
    return <colgroup>{cols}</colgroup>;
}
