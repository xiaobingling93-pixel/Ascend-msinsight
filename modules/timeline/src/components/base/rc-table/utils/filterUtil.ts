import Item from "antd/lib/list/Item";

// 修改antd过滤模块
type FilterProps = { text: string; value: string; children: FilterProps[] };
type KeyType = string | number | boolean;

const flattenKeys = (filters: Array<FilterProps>) => {
    let keys: KeyType[] = [];
    (filters || []).forEach(function (_ref2) {
        const value = _ref2.value, children = _ref2.children;
        keys.push(value);
        if (children !== undefined) {
            keys = keys.concat(flattenKeys(children));
        }
    });
    return keys;
};

// 判断data数组是否有含有children属性的元素
const judgeChildren = (data: any[]) => {
    return data.some((item: any) => item !== undefined && item.children !== undefined && item.children.length > 0);
};

// 过滤树的某个字段,返回过滤后的数组
const recurFilter = (data: any[] | any, keyword: string, onFilter: Function) => {
    if (data instanceof Array) {
        // data为数组
        let arr: any[] = [];
        data.forEach(item => {
            if (onFilter(keyword, item)) {
                arr.push(item);
            } else {
                const node = recurFilter(item, keyword, onFilter);
                if (node !== undefined) {
                    arr.push(node);
                }
            }
        });
        return arr.length > 0 ? arr : [];
    }
    // data为非数组
    if (data.children !== undefined && data.children.length > 0) {
        const children: any[] = recurFilter(data.children, keyword, onFilter);
        return children.length === 0 ? undefined : {...data, children};
    }
    return onFilter(keyword, data) ? {...data} : undefined;
};

export const customizeFilterData = (data: any[], filterStates: any[]) => {
    if (data.length === 0 || filterStates.length === 0) {
        return data;
    }
    if (filterStates.length === 1 && filterStates[0].column.filters === undefined && judgeChildren(data)) {
        // 判断为输入文本形式的过滤,并且有children属性，为树形结构过滤
        const { column: { onFilter }, filteredKeys, key } = filterStates[0];
        if (onFilter !== undefined && filteredKeys !== null && filteredKeys.length > 0) {
            return recurFilter(data, filteredKeys[0], onFilter);
        }
        return data;
    }
    // 默认的过滤方式
    return filterStates.reduce((currentData, filterState: any) => {
        const { column: { onFilter, filters }, filteredKeys } = filterState;
        if (onFilter !== undefined && filteredKeys !== null && filteredKeys !== undefined && filteredKeys.length > 0) {
            return currentData.filter((record: unknown) => {
                return filteredKeys.some((key: KeyType) => {
                    const keys = flattenKeys(filters);
                    const keyIndex = keys.findIndex(function (k) {
                        return String(k) === String(key);
                    });
                    const realKey = keyIndex !== -1 ? keys[keyIndex] : key;
                    return onFilter(realKey, record);
                });
            });
        }
        return currentData;
    }, data);
};
