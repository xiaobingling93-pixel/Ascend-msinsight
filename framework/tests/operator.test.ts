import { test, expect, type Page,type Frame } from '@playwright/test';
import {clickSelect, selectFolder} from './baseOperation';
test.describe('operator',() => {
    test.describe.configure({ mode: 'serial' });
    let page: Page;
    test.beforeAll('Open Page', async ({ browser, baseURL }) => {
        page = await browser.newPage({ viewport: { width: 1700,height: 1200 }});
        await page.goto(baseURL);
        await page.getByText('Operator').click();
    });

    test.afterAll(async () => {
        await page.close();
    });
    // 1.导入Pytorch单卡数据
    test('testImportPytorchSingleCardData', async () => {
        await selectFolder({ page,
            path:'D:\\workspace\\data\\16ka_gpt3\\master47_1993653_20230919172551_ascend_pt' });
        await page.waitForTimeout(1500);
        await expect(page).toHaveScreenshot('pytorchSingleCardData.png', { maxDiffPixels: 800 });
    });

    // 2.导入推理单卡数据
    test('testImportInferenceSingleCardData', async () => {
        // 删除数据
        await page.locator('.deleteIcon').first().click();
        await page.getByRole('button', { name: 'Confirm' }).click();
        await page.waitForTimeout(1000);
        await selectFolder({ page,
            path:'D:\\workspace\\data\\16ka_gpt3\\master47_1993649_20230919172551_ascend_pt\\PROF_000001_20230919172552034_IHHJIQFFNNKCDNFA' });
        await page.waitForTimeout(1000);
        await expect(page).toHaveScreenshot('inferenceSingleCardData.png', { maxDiffPixels: 800 });
    });

    // 3.导入推理多卡数据
    test('testImportInferenceMultiCardData', async () => {
        await selectFolder({ page,
            path:'D:\\workspace\\data\\16ka_gpt3\\master47_1993649_20230919172551_ascend_pt\\PROF_000001_20230919172552034_IHHJIQFFNNKCDNFA' });
        await page.waitForTimeout(1000);
        await expect(page).toHaveScreenshot('inferenceMultiCardData.png', { maxDiffPixels: 800 });
    });

    // 4.清理数据
    test('testCleanImportData', async () => {
        await page.locator('.deleteIcon').first().click();
        await page.getByRole('button', { name: 'Confirm' }).click();
        await page.waitForTimeout(1000);
        await expect(page).toHaveScreenshot('cleanImportData.png', { maxDiffPixels: 800 });
    });

    // 5.导入集群数据
    test('testImportPytorchClusterData', async () => {
        await selectFolder({ page });
        await page.waitForTimeout(6000);
        const frame = page.frame({ url: /.Operator.*/ });
        // 切换到RankId 0
        await frame.locator('.ant-select-selector').nth(1).click();
        await frame.locator('.ant-select-dropdown').nth(0).hover();
        await page.mouse.wheel(0, -1000);
        await frame.locator('.ant-select-item.ant-select-item-option')?.getByText('0',{exact:true})?.click();
        await page.waitForTimeout(1000);
        await expect(page).toHaveScreenshot('pytorchClusterData.png', { maxDiffPixels: 800 });
    });

    // 6.Operator Type 筛选条件变化、排序变化
    test('testViewOperatorTypeStatistics', async () => {
        const frame = page.frame({ url: /.Operator.*/ });
        // 修改Top
        await clickSelect({ locator: frame, cur: '15', target: 'All' });
        await frame.waitForTimeout(1000);
        await expect(page).toHaveScreenshot('operatorTypeTopChange.png', { maxDiffPixels: 800 });

        // Type升序排序
        await frame.locator('th').filter({ hasText: 'Type' }).click();
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('typeSortAscend.png');
        // Type降序排序
        await frame.locator('th').filter({ hasText: 'Type' }).click();
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('typeSortDscend.png');
        // Total Time升序排序
        await frame.locator('th').filter({ hasText: 'Total Time(μs)' }).click();
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('totalTimeSortAscend.png');
        // Total Time降序排序
        await frame.locator('th').filter({ hasText: 'Total Time(μs)' }).click();
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('totalTimeSortDscend.png');
        // 取消排序
        await frame.locator('th').filter({ hasText: 'Total Time(μs)' }).click();
    });

    // 7.Operator Type Details
    test('testViewOperatorTypeDetails', async () => {
        const frame = page.frame({ url: /.Operator.*/ });
        // 点击 see more
        await frame.getByText('see more').first().click();
        await frame.waitForTimeout(1000);
        await expect(page).toHaveScreenshot('operatorTypeDetails.png', { maxDiffPixels: 800 });
    });

    // 8.InputShape
    test('testViewInputShapeStatistics', async () => {
        const frame = page.frame({ url: /.Operator.*/ });
        await clickSelect({ locator: frame, cur: 'Operator Type', target: 'Operator Name and Input Shape' });
        await expect(page).toHaveScreenshot('inputShape.png', { maxDiffPixels: 800 });

        // 修改Top
        await clickSelect({ locator: frame, cur: 'All', target: '15' });
        await frame.waitForTimeout(1000);
        await expect(page).toHaveScreenshot('inputShapeTopChange.png', { maxDiffPixels: 800 });
    });

    // 9.InputShape Details
    test('testViewInputShapeDetails', async () => {
        const frame = page.frame({ url: /.Operator.*/ });
        // 点击 see more
        frame.getByText('see more').first().click();
        await expect(page).toHaveScreenshot('inputShapeDetails.png', { maxDiffPixels: 800 });
    });

    // 10.Operator
    test('testViewOperatorDetails', async () => {
        const frame = page.frame({ url: /.Operator.*/ });
        await clickSelect({ locator: frame, cur: 'Operator Name and Input Shape', target: 'Operator' });
        await expect(page).toHaveScreenshot('operator.png', { maxDiffPixels: 800 });

        // 修改Top
        await clickSelect({ locator: frame, cur: '15', target: 'Custom' });
        await frame.locator('input').nth(3).fill('');
        await frame.locator('input').nth(3).pressSequentially('20');
        await frame.waitForTimeout(20000);
        await expect(page).toHaveScreenshot('operatorTopChange.png', { maxDiffPixels: 800 });
    });
});