import { test, expect, type Page,type Frame } from '@playwright/test';
import {clickSelect, selectFolder} from './baseOperation';
test.describe('summary',() => {
	test.describe.configure({ mode: 'serial' });
	let page: Page;
	test.beforeAll('Open Page', async ({ browser, baseURL }) => {
		page = await browser.newPage({ viewport: { width: 1700,height: 1200 }});
		await page.goto(baseURL + '?port=9001');
		await selectFolder({ page, path:'D:\\GUI_Windows\\AscendInsight-GUI_Windows\\test_data\\16ka_gpt3' });
		await page.waitForTimeout(20000);
	});

	test.afterAll(async () => {
		await page.close();
	});
	
	test('testImportClusterData', async () => {
		await page.getByText('Summary').click();
		await page.waitForTimeout(5000);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('clusterData.png');
	})
	// 1.导入16卡集群数据
	test('testSelectPpAnalyse', async () => {
		const frame = page.frame({ url: /.summary.*/ });
		const pp = frame.locator('.container-body');
		await expect(Boolean(pp)).toBe(true);
		if (pp !== null) {
			// 修改筛选条件 Stage
			await pp.locator('.ant-select-selector')?.getByText('All',{exact:true})?.click();
			await frame.locator('.ant-select-item.ant-select-item-option').getByText('(0, 1, 2, 3, 4, 5, 6, 7)',{exact:true})?.click();
			await page.waitForTimeout(1500);
			await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('filterChangeStage.png');
		}
	});


	test('testSelectTensor/DataParallel', async () => {
		const frame = page.frame({ url: /.summary.*/ });
		if (frame !== null) {
			await frame.locator('.ant-tabs-tab').getByText('Tensor/Data Parallel').click();
			await page.waitForTimeout(1500);
			await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectTensor/DataParallel.png');
		}
	});

	test('testSelectTpOrDpFilter', async () => {
		const frame = page.frame({ url: /.summary.*/ });
		if (frame !== null) {
			await clickSelect({ locator: frame, cur: '(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)', target: '(0, 8)' });
			await page.waitForTimeout(1500);
			await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectStep.png');
			await clickSelect({ locator: frame, cur: 'Computing', target: 'Communication(Not Overlapped)' });
			await page.waitForTimeout(1500);
			await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectOrderBy.png');
			await clickSelect({ locator: frame, cur: '16 ( All )', target: '1' });
			await page.waitForTimeout(1500);
			await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectTop.png');
		}
	});

	test('testGenerateCommunicationGroup', async () => {
		const frame = page.frame({ url: /.summary.*/ });
		await frame.locator('#ppSize').fill('2');
		await frame.locator('#tpSize').fill('8');
		await frame.locator('#dpSize').fill('1');
		await frame.getByText('Generate').click();
		await page.waitForTimeout(1500);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('GenerateCommunicationGroup.png');
	});

	test('testSelectOtherParallel', async () => {
		const frame = page.frame({ url: /.summary.*/ });
		await frame.locator('.ant-tabs-tab').getByText('Tensor Parallel').click();
		await page.waitForTimeout(1500);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectTensorParallel.png');
		await frame.locator('.ant-tabs-tab').getByText('Data Parallel').click();
		await page.waitForTimeout(1500);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectDataParallel.png');
	});

	test('testSelectAICOREDetails', async () => {
		const frame = page.frame({ url: /.summary.*/ });
		await frame.getByRole('row', { name: 'AI_CORE 35016 details down' }).getByRole('button').click();
		// 滚动到最下方
		await frame.locator('#root').click();
		await page.mouse.wheel(0, 10000);
		await page.waitForTimeout(1500);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectAICOREDetails.png');
		await frame.getByText('Name').first().click();
		await page.waitForTimeout(1500);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SetAICOREDetailsFilterByName.png');
	});

	test('testSelectCommunicationDetails', async () => {
		const frame = page.frame({ url: /.summary.*/ });
		await frame.getByRole('row', { name: 'HCCL 160813 6846 details down' }).getByRole('button').click();
		// 滚动到最下方
		await frame.locator('#root').click();
		await page.mouse.wheel(0, 10000);
		await page.waitForTimeout(1500);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SelectCommunicationDetails.png');
		await frame.getByText('Name').nth(1).click();
		await page.waitForTimeout(1500);
		await expect(await page.screenshot({ fullPage: true })).toMatchSnapshot('SetCommunicationDetailsFilterByName.png');
	});
	 
});