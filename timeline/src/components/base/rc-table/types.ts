export type Key = React.Key;

export type FixedType = 'left' | 'right' | boolean;

export type DefaultRecordType = Record<string, unknown>;

export type TableLayout = 'auto' | 'fixed';

export type RowClassName<RecordType> = (
    record: RecordType,
    index: number,
    indent: number,
) => string;

export interface CellType<RecordType> {
    key?: Key;
    className?: string;
    style?: React.CSSProperties;
    children?: React.ReactNode;
    column?: ColumnType<RecordType>;
    colSpan?: number;
    colStart?: number;
    colEnd?: number;
}

export type DataIndex = string | number | readonly (string | number)[];

export type CellEllipsisType = { showTitle?: boolean } | boolean;

export type AlignType = 'left' | 'center' | 'right';

export type SvgType = React.FunctionComponent<React.SVGProps<
    SVGSVGElement
> & { title?: string }> & React.ReactNode;

export interface RenderedCell<RecordType> {
    props?: CellType<RecordType>;
    children?: React.ReactNode;
}

export interface ColumnType<RecordType> {
    key?: Key;
    title?: React.ReactNode;
    className?: string;
    fixed?: FixedType;
    ellipsis?: CellEllipsisType;
    align?: AlignType;
    colSpan?: number;
    dataIndex?: DataIndex;
    render?: (value: any, record: RecordType, index: number) => React.ReactNode | RenderedCell<RecordType>;
    onCell?: GetComponentProps<RecordType>;
    onHeaderCell?: GetComponentProps<ColumnType<RecordType>>;
    shouldCellUpdate?: (record: RecordType, prevRecord: RecordType) => boolean;
    width?: number | string;
}

export type ColumnsType<RecordType = unknown> = ColumnType<RecordType>[];

export type GetRowKey<RecordType> = (record: RecordType, index?: number) => Key;

export interface StickyOffsets {
    left: readonly number[];
    right: readonly number[];
    isSticky?: boolean;
}

export type GetComponentProps<DataType> = (
    data: DataType,
    index?: number,
) => React.HTMLAttributes<any> | React.TdHTMLAttributes<any>;

export interface ExpandIconProps<RecordType> {
    prefixCls?: string;
    expanded: boolean;
    record: RecordType;
    expandable: boolean;
    onExpand: TriggerEventHandler<RecordType>;
}

export type RenderExpandIcon<RecordType> = (props: ExpandIconProps<RecordType>) => React.ReactNode;

export interface ExpandableConfig<RecordType> {
    expandedRowKeys?: readonly Key[];
    defaultExpandedRowKeys?: readonly Key[];
    // Convention for expand icon element: renderers should return an element with classname `${prefixCls}-row-expand-icon`
    // this convention is not strictly checked but violating it may lead to some features missing
    expandIcon?: RenderExpandIcon<RecordType>;
    onExpand?: (expanded: boolean, record: RecordType) => void;
    onExpandedRowsChange?: (expandedKeys: readonly Key[]) => void;
    indentSize?: number;
    expandIconColumnIndex?: number;
    showExpandColumn?: boolean;
    childrenColumnName?: string;
    columnWidth?: number | string;
    fixed?: FixedType;
}

// =================== Events ===================
export type TriggerEventHandler<RecordType> = (
    record: RecordType,
    event?: React.MouseEvent<HTMLElement>,
) => void;
