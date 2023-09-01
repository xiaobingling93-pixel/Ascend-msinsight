import { ColumnsType } from '../base/rc-table';
import { AutoKey } from '../../utils/dataAutoKey';
import { Session } from '../../entity/session';
import { DetailDescriptor, MoreDescriptor } from '../../entity/insight';
import { TabComponentProps, TabProto, CommonStateProto } from './base/Tabs';
import { TabState } from '../../entity/tabDependency';

export type TableViewProps<Tab extends TabProto, TabsState extends CommonStateProto> = {
    session: Session;
    detail?: DetailDescriptor<unknown>;
    height: number; // The height of the drawing area for this view, which is necessary to correctly draw a scrollbar.
    isTree?: boolean;
    tabState?: TabState; // used for decide what to show in FilterContainer
    commonState?: TabComponentProps<Tab, TabsState>['commonState'];
    depsList?: unknown[];
    tabs?: TabComponentProps<Tab, TabsState>['tabs'];
    interactorProps?: TabComponentProps<Tab, TabsState>['interactorProps'];
    onDataLoaded?: (data: unknown[]) => void;
};

export type MoreTableProps = {
    more?: MoreDescriptor;
    session: Session;
    height: number;
    isTree?: boolean;
};

export type TableState = {
    data: Array<AutoKey<object>>;
    columns: ColumnsType<object>;
    rowKey?: (row: object) => string;
    onExpand?: (expanded: boolean, data: Record<string, unknown>) => void;
    isLoading: boolean;
};

export const EMPTY_TABLE_STATE: TableState = {
    data: [],
    columns: [],
    isLoading: false,
};
