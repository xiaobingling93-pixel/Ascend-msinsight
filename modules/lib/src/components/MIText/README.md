## 特性
- 支持多种文本类型：`default | primary | info | success | warning | error`
- 支持不同层级对比度（`depth=1/2/3`）
- 支持 **strong / italic / underline / delete** 样式
- 支持 **code** 特殊样式
- 支持 **ellipsis（单行省略）** 与 **lineClamp（多行收起）**
- 可通过 `as` 改变渲染标签（如 `span`, `p`, `div` 等）

## 使用方式

```tsx
import { Text } from "./Text";

<Text>默认文本</Text>
<Text strong>加粗文本</Text>
<Text italic underline>斜体+下划线</Text>
<Text delete>删除线</Text>
<Text type="primary">Primary 类型</Text>
<Text type="success">Success 类型</Text>
```

## Props

| Prop        | 类型                                     | 默认值      | 说明 |
|-------------|------------------------------------------|-------------|------|
| `as`        | `keyof JSX.IntrinsicElements`            | `span`      | 渲染标签 |
| `type`      | `'default' \| 'primary' \| 'info' \| 'success' \| 'warning' \| 'error'` | `default` | 文本类型（决定颜色） |
| `depth`     | `1 \| 2 \| 3`                           | `1`         | 文本对比度，数值越大颜色越浅 |
| `strong`    | `boolean`                               | `false`     | 是否加粗 |
| `italic`    | `boolean`                               | `false`     | 是否斜体 |
| `underline` | `boolean`                               | `false`     | 是否下划线 |
| `delete`    | `boolean`                               | `false`     | 是否删除线 |
| `code`      | `boolean`                               | `false`     | 是否显示为内联代码样式 |
| `ellipsis`  | `boolean`                               | `false`     | 单行省略（需设置容器宽度） |
| `lineClamp` | `number`                                | `0`         | 多行收起（行数） |
| `disabled`  | `boolean`                               | `false`     | 是否禁用样式 |
