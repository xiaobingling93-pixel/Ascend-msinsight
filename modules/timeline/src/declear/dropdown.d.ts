declare module 'antd' {
    import type { InputRef, TooltipProps } from 'antd';
    import { DropDownProps } from 'antd/lib/dropdown/dropdown';
    import { ReactNode } from 'react';
    import DropdownButton from 'antd/lib/dropdown/dropdown-button';

    type MyDropDownProps = DropDownProps & {
        children?: ReactNode;
    };
    interface MyDropdownInterface extends React.FC<MyDropDownProps> {
        Button: typeof DropdownButton;
    }
    export const Dropdown: MyDropdownInterface;
    export const Menu;
    export const Pagination;
    export const Select;
    export const Tooltip;
    export const Button;
    export const ConfigProvider;
    export const Empty;
    export const Input;
    export const List;
    export const Spin;
    export const Checkbox;
    export const Radio;
    export const Card;
    export const Row;
    export const Col;
    export const Switch;
    export const Modal;
    export const TreeSelect;
    export const Tag;
    export const AutoComplete;
    export const message;
    export type TooltipProps = TooltipProps;
    export type InputRef = InputRef;
    export type AutoCompleteProps = AutoCompleteProps;
    export const Table;
}
