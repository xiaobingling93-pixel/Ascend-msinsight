import { onExpandForChildren, parseColDef, selectRow, treeAttachInfo } from "./utils";

describe('utils test', () => {
    it('parseColDef test', () => {
        expect(() => parseColDef({ columns: [['name', () => 'render', 0]] }, session)).toThrow(new Error('columnsWidth at least one of the columns should have width "max-content" or "auto"'));
        expect(JSON.stringify(parseColDef({
            columns: [
                ['name2', () => 'render2', 'max-content', 'scroll']
            ]
        }, session))).toEqual(JSON.stringify([
            {
                title: "name2",
                key: "name2",
                colSpan: 1,
                ellipsis: {
                    showTitle: false
                },
                render: () => { }
            }
        ]));
    })

    it('selectRow test', () => {
        const row = {};
        const tableState = {
            data: [],
            rowKey: (row: object) => 'testkey',
            columns: [],
            isLoading: false
        }
        selectRow(row, session, tableState);

        expect(session.selectedDetails).toStrictEqual([row]);

        expect(session.selectedDetailKeys).toStrictEqual(['testkey']);
    })

    it('treeAttachInfo test', () => {
        let root = undefined;
        expect(treeAttachInfo(root)).toStrictEqual(root);
        root = {};
        treeAttachInfo(root);
        expect(Object.getOwnPropertySymbols(root).length).toBe(2);
        root = { children: [{}] };
        treeAttachInfo(root);
        expect(Object.getOwnPropertySymbols(root).length).toBe(2);
        expect(Object.getOwnPropertySymbols(root.children[0]).length).toBe(2);
    })

    it('onExpandForChildren test', async () => {
        const setTableState = jest.fn();
        let node = {};
        const onExpand = jest.fn().mockResolvedValue(node);
        const keys: [] | [string] = ['1'];
        session.selectedDetailKeys = keys;
        expect(onExpandForChildren(session, undefined, setTableState)).toBeUndefined();
        let fn = onExpandForChildren(session, onExpand, setTableState) as Function;
        expect(fn(false, {})).toBeUndefined();
        await fn(true, node);
        expect(onExpand).toBeCalledTimes(1);
        expect(Object.getOwnPropertySymbols(node).length).toBe(2);
        expect(session.selectedDetailKeys).not.toBe(keys);
        expect(session.selectedDetailKeys).toStrictEqual(keys);
    })
})
