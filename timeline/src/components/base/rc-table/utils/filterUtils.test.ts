import { customizeFilterData } from './filterUtil';

const onFilterFunc = <T extends Record<string, unknown>>(dataIndex: keyof T | Array<keyof T>): typeof onFilter => {
    const onFilter = (value: string | number | boolean, record: T): boolean => {
        if (typeof dataIndex === 'string') {
            return filterIndex(String(record[dataIndex]), String(value));
        } else if (dataIndex instanceof Array) {
            const filteredDataIndex = dataIndex.filter(item => record[item] !== undefined);
            return filteredDataIndex.some(index => filterIndex(String(record[index]), String(value)));
        }
        return false;
    };
    return onFilter;
};

const filterIndex = (data: string, value: string): boolean => {
    return data.toLowerCase().includes(value.toLowerCase());
};

const threadData = {
    key: 'Symbol Name',
            symbolName: "Thread 16449",
            children: [
                {
                    symbolName: '/system/lib64/libace.z.so+0xf4bd78',
                    children: [
                        { symbolName: '/system/lib64/libace.z.so+0xf4c2b8', }
                    ]
                },
                {
                    symbolName: "/system/lib64/libace.z.so+0xf47a08",
                    children: [
                        { symbolName: "/system/lib64/libace.z.so+0xf4bd78", }
                    ]
                }
            ]
}

const data1 = {
    data: [
        {
            key: 'Symbol Name',
            symbolName: "1.ui",
            children: [
                {
                    symbolName: '/system/lib64/libark_jsruntime.so+0x37d560',
                    children: [
                        { symbolName: 'ArkNativeFunction::NativeFunctionCallBack(panda::JsiRuntimeCallInfo*)', }
                    ]
                },
                {
                    symbolName: "/system/lib64/libark_jsruntime.so+0x3d1500",
                    children: [
                        { symbolName: "/system/lib64/libark_jsruntime.so+0x116310", }
                    ]
                }
            ]
        },
        threadData,
    ],
    result1: [threadData],
    result2: [{
        key: 'Symbol Name',
        symbolName: "1.ui",
        children: [
            {
                symbolName: '/system/lib64/libark_jsruntime.so+0x37d560',
                children: [
                    { symbolName: 'ArkNativeFunction::NativeFunctionCallBack(panda::JsiRuntimeCallInfo*)', }
                ]
            }
        ]
    }],
    result3: [{
        key: 'Symbol Name',
        symbolName: "1.ui",
        children: [
            {
                symbolName: "/system/lib64/libark_jsruntime.so+0x3d1500",
                children: [
                    { symbolName: "/system/lib64/libark_jsruntime.so+0x116310", }
                ]
            }
        ]
    }],
    result4: [],
    filterState: (key: string[]) => [
        {
            column: { onFilter: onFilterFunc("symbolName") },
            filteredKeys: key,
        },
    ]
}

const data2 = {
    data: [
        { template: 'Allocation', device: '018TRP206T000364', process: 'com.example.demoapp(16438)' },
        { template: 'Allocation', device: '018TRP206T000365', process: 'com.example.demoapp(16439)' },
        { template: 'Time', device: '018TRP206T000364', process: 'com.example.demoapp(16439)' },
        { template: 'Time', device: '018TRP206T000365', process: 'com.example.demoapp(16438)' },
    ],
    result1: [
        { template: 'Allocation', device: '018TRP206T000364', process: 'com.example.demoapp(16438)' }
    ],
    result2: [
        { template: 'Time', device: '018TRP206T000364', process: 'com.example.demoapp(16439)' }
    ],
    result3: [
        { template: 'Time', device: '018TRP206T000365', process: 'com.example.demoapp(16438)' }
    ],
    result4: [
        { template: 'Time', device: '018TRP206T000364', process: 'com.example.demoapp(16439)' },
        { template: 'Time', device: '018TRP206T000365', process: 'com.example.demoapp(16438)' },
    ],
    result5: [
        { template: 'Allocation', device: '018TRP206T000364', process: 'com.example.demoapp(16438)' },
        { template: 'Time', device: '018TRP206T000364', process: 'com.example.demoapp(16439)' },
    ],
    result6: [
        { template: 'Allocation', device: '018TRP206T000364', process: 'com.example.demoapp(16438)' },
        { template: 'Time', device: '018TRP206T000365', process: 'com.example.demoapp(16438)' },
    ],
    result7: [],
    result8: [],
    result9: [],

    filterState: (key: Array<string[] | undefined>) => [
        {
            column: { onFilter: onFilterFunc("template") },
            filteredKeys: key[0],
        },
        {
            column: { onFilter: onFilterFunc("device") },
            filteredKeys: key[1],
        },
        {
            column: { onFilter: onFilterFunc("process") },
            filteredKeys: key[2],
        },
    ]
}

describe('customizeFilterData test', () => {
    // 测试树形结构下的过滤，单个字段过滤场景
    it('single field group', () => {
        expect(customizeFilterData(data1.data, data1.filterState(['Thread']))).toEqual(data1.result1);

        expect(customizeFilterData(data1.data, data1.filterState(['so+0x37d560']))).toEqual(data1.result2);

        expect(customizeFilterData(data1.data, data1.filterState(['so+0x116310']))).toEqual(data1.result3);

        expect(customizeFilterData(data1.data, data1.filterState(['abcde']))).toEqual(data1.result4);
    });

    // 测试非树形结构下的过滤，多个字段过滤场景
    it('multiple field group', () => {
        expect(customizeFilterData(data2.data, data2.filterState([['Allocation'], ['364'], undefined]))).toEqual(data2.result1);

        expect(customizeFilterData(data2.data, data2.filterState([['Time'], undefined, ['16439']]))).toEqual(data2.result2);

        expect(customizeFilterData(data2.data, data2.filterState([undefined, ['365'], ['16438']]))).toEqual(data2.result3);

        expect(customizeFilterData(data2.data, data2.filterState([['Time'], undefined, undefined]))).toEqual(data2.result4);

        expect(customizeFilterData(data2.data, data2.filterState([undefined, ['364'],  undefined]))).toEqual(data2.result5);

        expect(customizeFilterData(data2.data, data2.filterState([undefined, undefined, ['16438']]))).toEqual(data2.result6);

        expect(customizeFilterData(data2.data, data2.filterState([undefined, undefined, ['16440']]))).toEqual(data2.result7);

        expect(customizeFilterData(data2.data, data2.filterState([undefined, ['366'],  undefined]))).toEqual(data2.result8);

        expect(customizeFilterData(data2.data, data2.filterState([['Time1'], undefined,  undefined]))).toEqual(data2.result9);
    });
});
