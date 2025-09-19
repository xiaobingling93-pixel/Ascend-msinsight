import React from 'react';
/*
*  UpdateTableAsync 会在表格渲染的时候，先获取一遍表格数据，如果表格数据有loading的值，则将会记录在updateMap中
*  当loading完成后，会遍历updateMap中的所有数据重新发起请求渲染表格
* */
class UpdateTableAsync {
    // 用来记录最后需要更新的表格
    updateMap: {
        [key: string]: any;
    };

    constructor() {
        // 初始化为空对象
        this.updateMap = {};
    }

    // 为表格添加loading的样式
    addRenderMethod (columns: any[]): any[] {
        return columns.map((item) => {
            if (item.render) {
                return item;
            }
            item.render = (text: any) => {
                if (text === 'Loading') {
                    return <div className="table-column-load"></div>;
                } else {
                    return text;
                }
            };
            return item;
        });
    }

    // 遍历表格中的数据，如果有loading字段则返回true
    replaceEmptyValuesWithLoa (tableData: any[]): boolean {
        let result: boolean = false;
        tableData.map((item: any) => {
            Object.entries(item).forEach(([key, value]) => {
                // 检查值是否为空（null、undefined、空字符串）
                if (value === 'Loading') {
                    result = true;
                }
            });
            return item;
        });
        return result;
    }

    /**
     * @pkey 用getData方法的入参对象转换成string当key
     * @getData 获取表格数据的方法
     * @dataProcessing 获取到数据后，对数据的处理方法
     * @setData  设置表格数据的方法
     */
    addUpdateList(key: string, getData: any, dataProcessing: any, setData: any): void {
        this.updateMap[key] = {
            getData,
            dataProcessing,
            setData,
        };
    }

    // 更新所有数据
    update(): boolean {
        const updateMapKeysList = Object.keys(this.updateMap);
        if (!updateMapKeysList.length) {
            return false;
        }
        updateMapKeysList.forEach((key) => {
            const {
                getData,
                dataProcessing,
                setData,
            } = this.updateMap[key];
            const parmes = JSON.parse(key);
            getData(parmes.fullCondition, parmes.condition, parmes.opDetail, parmes.pageStatus).then((res: any) => {
                const realData = dataProcessing(res);
                setData(realData);
            });
        });
        return true;
    }
}

export default UpdateTableAsync;
