# MindStudio Insight GUI Test

## Project Setup

```sh
npm install
```

### 准备测试数据

测试数据可以在此仓库下载：[测试数据地址](https://codehub-y.huawei.com/q30057178/GUI_TEST_DATA/files?ref=master)

### Run End-to-End Tests with [Playwright](https://playwright.dev)

```sh
# 首先执行该命令，下载测试浏览器，执行后会自动在C盘生成必要目录
# 注意：由于内网无法自动下载对应的浏览器，需要手动下载并放在指定目录
# 下载链接：https://playwright.azureedge.net/builds/chromium/1129/chromium-win64.zip
# 指定目录：C:\Users\q30057178\AppData\Local\ms-playwright\chromium-1129\chrome-win\chrome.exe
#（其中 chromium-1129 目录需要手动创建，1129 为当前 playwright 版本对应的测试浏览器版本）
npx playwright install

# 运行所有用例
npm run test
# 在 UI 模式下运行
npm run test -- --ui
# 运行单个测试文件
npm run test timeline.spec.ts
# 运行单个测试用例
npm run test -- -g test_unitsExpandAndCollapse_when_click
```

### 自动化写用例

```sh
npx playwright codegen localhost:5174 --viewport-size=1920,1080
```

### Lint with [ESLint](https://eslint.org/)

```sh
npm run lint
```

### 常见问题

#### Q: 如何禁用无头模式？

A: 在 playwright.config.ts 配置文件中，将 headless 设置为 false，即可显示浏览器窗口。

```ts
use: {
    headless: false;
}
```
