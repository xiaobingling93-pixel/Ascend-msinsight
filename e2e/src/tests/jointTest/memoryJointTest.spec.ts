/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

import { expect, test as baseTest, WebSocket } from '@playwright/test';
import { MemoryPage } from '@/page-object';
import { clearAllData, importData, setupWebSocketListener, waitForWebSocketEvent } from '@/utils';
import { FilePath } from '@/utils/constants';

interface TestFixtures {
    memoryPage: MemoryPage;
    ws: Promise<WebSocket>;
}
const test = baseTest.extend<TestFixtures>({
    memoryPage: async ({ page }, use) => {
        const memoryPage = new MemoryPage(page);
        await use(memoryPage);
    },
    ws: async ({ page }, use) => {
        const ws = setupWebSocketListener(page);
        await use(ws);
    },
});

test.describe('Memory(Joint)', () => {
    
    test.beforeEach(async ({ page, memoryPage, ws }) => {
        const allCardParsedPromise = waitForWebSocketEvent(page, (res) => res?.event === 'allPagesSuccess');
        await memoryPage.goto();
        await clearAllData(page);
        await importData(page, FilePath.JOINT_DATA);
        await allCardParsedPromise;
    });

    test.afterEach(async ({ page, ws }) => {
        await clearAllData(page, ws);
    });

    test('loadMemoryPageSuccess_when_filters_how', async ({ page, memoryPage }) => {
        const { memoryFrame } = memoryPage;
        await expect(memoryFrame.getByText('Host Name')).toBeVisible();
        await expect(memoryFrame.getByText('Rank ID')).toBeVisible();
        await expect(memoryFrame.getByText('Group By')).toBeVisible();
        await page.mouse.move(0, 0);
    });

    test('loadMemoryPageSuccess_when_anaylysis_data_show', async ({ page, memoryPage }) => {
        const { memoryFrame } = memoryPage;
        await expect(memoryFrame.getByText('Peak Memory Usage')).toBeVisible();
        await page.mouse.move(0, 0);
    });

    test('loadMemoryPageSuccess_when_allocation_release_Details', async ({ page, memoryPage }) => {
        const { memoryFrame } = memoryPage;
        await expect(memoryFrame.getByText('Peak Memory Usage')).toBeVisible();
        await expect(memoryFrame.getByText('Min Size(KB)')).toBeVisible();
        await expect(memoryFrame.getByText('Max Size(KB)')).toBeVisible();
        await expect(memoryFrame.getByText('Only show allocated or released within the selected interval')).toBeVisible();
        const rows = await memoryFrame.locator('.ant-table-row').count();
        expect(rows).toBeGreaterThan(0);
        await memoryFrame.getByRole('cell', { name: 'aten::empty_strided' }).first().click({
            button: 'right',
        });
        await memoryFrame.getByText('Find in Timeline').click();
        await page.mouse.move(0, 0);
    });

}); 