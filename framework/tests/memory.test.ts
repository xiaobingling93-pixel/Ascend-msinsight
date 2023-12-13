import { test, expect, type Page } from '@playwright/test';
import { clickSelect, selectFolder } from './baseOperation';

interface MemoryRankInfo {
  hasMemory: boolean;
  rankId: string;
}
const MEMORY_COUNT = 16;
test.describe('memory', () => {
  test.describe.configure({ mode: 'serial' });
  let page: Page;
  let memoryData: MemoryRankInfo[] = [];
  test.beforeAll(async ({ browser, baseURL }) => {
    page = await browser.newPage({ viewport: { width: 1920, height: 1080 } });
    page.on('websocket', (websocket) => {
      websocket.on('framereceived', (data) => {
        if (data.payload) {
          try {
            let res = JSON.parse(data.payload);
            if (res && res.body && res.body.memoryResult) {
              memoryData = res.body.memoryResult;
            }
          } catch {
            (err) => console.log(`Error: ${err}`);
          }
        }
      });
    });
    await page.goto(`${baseURL}?port=9003`);
    await page.getByRole('menuitem', { name: 'Memory' }).click();
  });

  test.afterAll(async () => {
    await page.close();
  });

  // 1.初始无数据时界面比对
  test('testMemoryTabWithNoData', async () => {
    await expect(page).toHaveScreenshot('noData.png');
  });

  // 2.上传数据后界面初次比对
  test('testMainPageWithData', async () => {
    const frame = page.frame({ url: /.Memory.*/ });
    expect(Boolean(frame)).toBe(true);
    await selectFolder({ page, path: 'D:\\GUI_Windows\\AscendInsight-GUI_Windows\\test_data\\16ka_gpt3' });
    const waitTimes = 120;
    // 通过定时器监听memoryData变化
    const interId: number = await new Promise((resolve) => {
      let loopTimes = 0;
      const intervalId = setInterval(() => {
        loopTimes += 1;
        if (memoryData.filter(item => item.hasMemory).length === MEMORY_COUNT || loopTimes === waitTimes) {
          resolve(intervalId);
        }
      }, 1000);
    });
    clearInterval(interId);
    expect(memoryData.length).toBe(16);
    if (frame !== null) {
      await frame.waitForTimeout(2000);
    }
    await expect(page).toHaveScreenshot('startRank0.png', { maxDiffPixels: 800 });
  })

  // 3.测试切换rankId为1
  test('testSelectRankIdTo1', async () => {
    const frame = page.frame({ url: /.Memory.*/ });
    if (frame !== null) {
      await clickSelect({ locator: frame, cur: '0', target: '1' });
      await frame.waitForTimeout(2000);
      await expect(page).toHaveScreenshot('selectRank1.png');
    }
  })

  // 4.测试输入框和按钮
  test('testInputsAndBtns', async () => {
    const frame = page.frame({ url: /.Memory.*/ });
    if (frame !== null) {
      const queryBtn = frame.locator('.ant-btn').getByText('Query');
      const resetBtn = frame.locator('.ant-btn').getByText('Reset');
      // 通过设置算子名称查询条件为aten::来筛选
      await frame.getByPlaceholder('Search by Name').fill('aten::');
      await queryBtn.click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('searchByName.png');

      // 设置最小内存输入框为64来筛选
      await frame.getByRole('spinbutton').first().fill('64');
      await queryBtn.click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('searchByMinSize.png');

      // 设置最大内存输入框为10000来筛选
      await frame.getByRole('spinbutton').nth(1).fill('10000');
      await queryBtn.click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('searchByMaxSize.png');

      // 重置按钮点击事件
      await resetBtn.click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('selectRank1.png', { maxDiffPixels: 800 });
    }
  })

  // 5.测试表头排序功能
  test('testTableColumnSort', async () => {
    const frame = page.frame({ url: /.Memory.*/ });
    if (frame !== null) {
      // Name列升序排序
      await frame.locator('th').filter({ hasText: 'Name' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableNameSortAscend.png');
      // Name列降序排序
      await frame.locator('th').filter({ hasText: 'Name' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableNameSortDescend.png');
      // Size列升序排序
      await frame.locator('th').filter({ hasText: 'Size(KB)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableSizeSortAscend.png');
      // Size列降序排序
      await frame.locator('th').filter({ hasText: 'Size(KB)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableSizeSortDescend.png');
      // Allocation Time列升序排序
      await frame.locator('th').filter({ hasText: 'Allocation Time(ms)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableAllocationTimeSortAscend.png');
      // Allocation Time列降序排序
      await frame.locator('th').filter({ hasText: 'Allocation Time(ms)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableAllocationTimeSortDescend.png');
      // Release Time列升序排序
      await frame.locator('th').filter({ hasText: 'Release Time(ms)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableReleaseTimeSortAscend.png');
      // Release Time降序排序
      await frame.locator('th').filter({ hasText: 'Release Time(ms)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableReleaseTimeSortDescend.png');
      // Duration列升序排序
      await frame.locator('th').filter({ hasText: 'Duration(ms)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableDurationSortAscend.png');
      // Duration列降序排序
      await frame.locator('th').filter({ hasText: 'Duration(ms)' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTableDurationSortDescend.png');
      // 取消排序
      await frame.locator('th').filter({ hasText: 'Duration(ms)' }).click();
    }
  })

  // 6.测试表格分页栏
  test('testChangePagination', async () => {
    const frame = page.frame({ url: /.Memory.*/ });
    if (frame !== null) {
      // 翻页
      await frame.getByRole('button', { name: 'right' }).click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTablePage2.png');
      // 点击第4页
      await frame.getByRole('listitem', { name: '4' }).locator('a').click();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTablePage4.png');
      // 跳转到第30页
      await frame.getByLabel('Page', { exact: true }).fill('30');
      await frame.getByLabel('Page', { exact: true }).press('Enter');
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryTablePage30.png');
      // 每页显示20条
      await clickSelect({ locator: frame, cur: '10 / page', target: '20 / page' });
      await page.mouse.wheel(0, 400);
      await expect(page).toHaveScreenshot('memoryTablePageSize20.png', { fullPage: true });
      // 重置初始状态
      await clickSelect({ locator: frame, cur: '20 / page', target: '10 / page' });
      await frame.getByLabel('Page', { exact: true }).fill('1');
      await frame.getByLabel('Page', { exact: true }).press('Enter');
    }
  })

  // 7.测试框选折线图
  test('testSelectCurveRange', async () => {
    const frame = page.frame({ url: /.Memory.*/ });
    if (frame !== null) {
      // 框选折线图
      await frame.locator('canvas').hover({ position: { x: 180, y: 200 } });
      await page.mouse.down();
      await frame.locator('canvas').hover({ position: { x: 280, y: 200 } });
      await page.mouse.up();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryCurveSelect.png');
      // 再次框选更细粒度
      await frame.locator('canvas').hover({ position: { x: 180, y: 200 } });
      await page.mouse.down();
      await frame.locator('canvas').hover({ position: { x: 480, y: 200 } });
      await page.mouse.up();
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('memoryCurveSecondSelect.png');
      // 测试图表联动
      await frame.getByRole('cell', { name: 'aten::lt', exact: true }).hover();
      await expect(page).toHaveScreenshot('memoryCurveLinkTable.png');
      // 右键还原
      await frame.locator('canvas').click({ button: 'right' });
      await frame.waitForTimeout(1000);
      await expect(page).toHaveScreenshot('selectRank1.png', { maxDiffPixels: 800 });
    }
  })

  // 8.测试移除数据
  test('testRemoveData', async () => {
    await page.locator('.deleteIcon').first().click();
    await page.getByRole('button', { name: 'Confirm' }).click();
    await page.waitForTimeout(1000);
    await expect(page).toHaveScreenshot('noData.png');
  })
})