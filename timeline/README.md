# HiInsight frontend

## 配置

### 为tsc开启css module支持

默认情况下，向tsx文件中导入css module对象时，tsc对这个对象的字段没有进行识别，不能补全和检查，因此工程中使用了[typescript-plugin-css-modules插件](https://github.com/mrmckeb/typescript-plugin-css-modules)。

为了配合这个插件，在vscode中进行开发时，需要手动将vscode使用的ts编译器配置为workspace版，参考[官方文档](https://code.visualstudio.com/docs/typescript/typescript-compiling#_using-the-workspace-version-of-typescript)：打开任意ts文件，鼠标悬浮在vscode右下角状态栏的ts版本号处，点击选择版本，然后选择"workspace"即可。

## 开发

### 增加一种chart

1. 首先在entity/chart.ts中增加一行定义，用来描述chart的名称与其配置数据、输入数据的类型，比如
```ts
type ChartDataDefinition = {
    area: ChartDataType<{ color: string }, [number, number][]>,
    bar: ChartDataType<{ legend: string[] }, BarData[]>,
 
    newChartType: ChartDataType<NewConfigType, NewDataType[]>, // 增加的新行,其中newChartType、NewConfigType、NewDataType替换成相应的type
    // to be extended...
};
```

2. 在`components/charts`中增加一个tsx文件，实现该chart的绘图，该组件的props类型应使用`ChartProps<newChartType>`，可参考已有chart

3. 在`components/charts/index.tsx`中找到`chartMap`对象，用刚才的chart名称为key新增一个字段，返回一个JSX，实现draw函数，需要传入的props字段可以从ChartProps定义处找到

4. 在定义insight unit时，需要设置其所用的chartConfig，config类型就是步骤1中的NewConfigType，在步骤3中传递props时，需要将unit.chartConfig一起传入，可参考已有unit

5. 结束。步骤1和2定义了chart的用法和约束，在步骤3和4中如果传参不正确或者缺少字段，编译器会给出提示

### 增加一种DIC数据

1. 确保DIC中支持这个数据
2. 在engine/dataType.ts的SupportedFields中增加DIC已经支持的field和对应类型
```ts
export interface SupportedFields {
    ts: TimeStamp
    cpuUsage: number
    memoryUsage: number

    newType: NewType // 增加的新行
}
```
3. 在fieldSample中增加数据示例（方便运行时做校验）
4. 使用fetchData接口即可从DIC获取数据
```ts
const raw = await engine.fetchData<{ ts: number, newType: NewType}>({ // 这里的泛型必填，用于编译时校验类型，确保你想要获取的类型在SupportedFields中存在
    sessionID,
    startTime,
    endTime,
    fields: { // fields 属性名代表需要的数据，属性值未用到，先统一为true
        ts: true,
        newType: true,
    },
});
```