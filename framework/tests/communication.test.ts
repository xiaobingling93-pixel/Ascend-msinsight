import { test, expect, type Page } from '@playwright/test';
import {clickSelect, selectFolder} from './baseOperation';
test.describe('comminucation', () => {
    test.describe.configure({ mode: 'serial' });
    let page: Page;
    test.beforeAll('Open Page', async ({ browser, baseURL }) => {
        page = await browser.newPage({ viewport: { width: 1700, height: 1100 } });
        await page.goto(baseURL);
    });

    test.afterAll(async () => {
        await page.close();
    });

    test('testCommunicationTabWithNoData', async () => {
        await expect(await page.getByText('Communication').count()).toBe(0);
    });

    test('testCommunicationTabWithData', async () => {
        await selectFolder({ page });
        await page.waitForTimeout(3000);
        await page.getByText('Communication')?.click();
        await page.waitForTimeout(1500);
        await expect(page).toHaveScreenshot('communication.png', { maxDiffPixels: 800 });
    });

    test('testDurationFilterChange', async () => {
        const frame = page.frame({ url: /.communication.*/ });
        await expect(Boolean(frame)).toBe(true);
        if (frame !== null) {
            // 修改筛选条件 Communication Group
            await clickSelect({ locator: frame, cur: 'p2p', target: '(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)' });
            await frame.waitForTimeout(1500);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('filterChangeCommunicationGroup.png');
            // 修改筛选条件 Operator Name
            await clickSelect({ locator: frame, cur: 'Total Op Info', target: 'hcom_allGather__822_4' });
            await frame.waitForTimeout(1500);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('filterChangeOperatorName.png');
        }
    });

    test('testDurationTablePageChange', async () => {
        const frame = page.frame({ url: /.communication.*/ });
        if (frame !== null) {
            // 下一页
            await frame.locator('li.ant-pagination-item-2').click();
            await frame.waitForTimeout(500);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('communicationTimeNextPage.png');
            // 切换每页记录数
            await frame.getByText('10 / page').click();
            await frame.getByText('50 / page').click();
            await frame.waitForTimeout(1000);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('communicationTimePageSizeChange.png');
        }
    });

    // 通信表格排序测试
    test('testDurationTableSortChange', async () => {
        const frame = page.frame({ url: /.communication.*/ });
        if (frame !== null) {
            // ElapseTime升序排序
            await frame.locator('th').filter({ hasText: 'Elapse Time(ms)' }).click();
            await frame.waitForTimeout(1500);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('communicationTimeElapseTimeSortAscend.png');
            // ElapseTime降序排序
            await frame.locator('th').filter({ hasText: 'Elapse Time(ms)' }).click();
            await frame.waitForTimeout(1500);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('communicationTimeElapseTimeSortDscend.png');
            // TransitTime升序排序
            await frame.locator('th').filter({ hasText: 'Transit Time(ms)' }).click();
            await frame.waitForTimeout(1500);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('communicationTimeTransitTimeSortAscend.png');
            // TransitTime降序排序
            await frame.locator('th').filter({ hasText: 'Transit Time(ms)' }).click();
            await frame.waitForTimeout(1500);
            await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('communicationTimeTransitTimeSortDscend.png');
        }
    });

    test('testMatrix', async () => {
        const frame = page.frame({ url: /.communication.*/ });
        await frame.getByLabel('Communication Matrix', { exact: true }).check();
        await frame.waitForTimeout(1500);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('communicationMatrix.png');
    });
    test('testMatrixFilterChange', async () => {
        const frame = page.frame({ url: /.communication.*/ });
        // 修改筛选条件 Communication Group
        await clickSelect({ locator: frame, cur: '(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)', target: '(0, 1, 2, 3, 4, 5, 6, 7)' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixFilterChangeCommunicationGroup.png');

        // 修改筛选条件 Operator Name
        await clickSelect({ locator: frame, cur: 'Total Op Info', target: 'hcom_allGather__483_11' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixFilterChangeOperatorName.png');
    });

    test('testMatrixTypeChange', async () => {
        const frame = page.frame({ url: /.communication.*/ });
        await frame.waitForTimeout(1000);
        // MatrixType: Bandwidth
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeBandwidth.png');

        // MatrixType: Transit Size
        await clickSelect({ locator: frame, cur: 'Bandwidth(GB/s)', target: 'Transit Size(MB)' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeTransitSize.png');

        // MatrixType: Transport Type
        await clickSelect({ locator: frame, cur: 'Transit Size(MB)', target: 'Transport Type' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeTransportType.png');

        // MatrixType: Transit Time
        await clickSelect({ locator: frame, cur: 'Transport Type', target: 'Transit Time(ms)' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeTransitTime.png');

        await clickSelect({ locator: frame, cur: 'Transit Time(ms)', target: 'Bandwidth(GB/s)' });
    });

    test('testMatrixOptionChange', async () => {
        const frame = page.frame({ url: /.communication.*/ });
        await frame.getByText('Show Inner Communication').check();
        await frame.waitForTimeout(1000);
        // MatrixType: Bandwidth
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeBandwidthShowInner.png');
        // MatrixType: Transit Size
        await clickSelect({ locator: frame, cur: 'Bandwidth(GB/s)', target: 'Transit Size(MB)' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeTransitSizeShowInner.png');

        // MatrixType: Transport Type
        await clickSelect({ locator: frame, cur: 'Transit Size(MB)', target: 'Transport Type' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeTransportTypeShowInner.png');

        // MatrixType: Transit Time
        await clickSelect({ locator: frame, cur: 'Transport Type', target: 'Transit Time(ms)' });
        await frame.waitForTimeout(1000);
        await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('matrixTypeTransitTimeShowInner.png');
    });
});