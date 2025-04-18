/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import { expect, test as baseTest } from '@playwright/test';
import { FrameworkPage, SourcePage } from '@/page-object';
import { clearAllData, importData } from '@/utils';
import { FilePath } from '@/utils/constants';
import { SelectHelpers } from '@/components';

interface TestFixtures {
    sourcePage: SourcePage;
}

const test = baseTest.extend<TestFixtures>({
    sourcePage: async ({ page }, use) => {
        const sourcePage = new SourcePage(page);
        await use(sourcePage);
    },
});

const imgMap = {
    loadDataSuccess: 'loadDataSuccess.png',
    coreSelectChangeSuccess: 'coreSelectChangeSuccess.png',
    codeTableInteractInsructionTableSuccess: 'CodeTable_interact_InstructionTable.png',
    insructionTableInteractCodeTableSuccess: 'InstructionTable_interact_CodeTable.png',
    switchChineseLanguageSuccess: 'Chinese_Language.png',
};
const inputMap = {
    secondFileName: '/home/huangning/code/samples/operator/AddCustomSample/KernelLaunch/AddKernelInvocationNeo/build/auto_gen/ascendc_kernels_sim/auto_gen_add_custom.cpp',
};
const resMap = {
    secondFileContens:'#ifndef __ADD_CUSTOM__KERNEL_FUN_H_',
};

test.describe('Source', () => {
    test.beforeEach(async ({ page, sourcePage }, testInfo) => {
        await page.goto('/');
        const filePath = testInfo.title === 'test_source_sourceSelectChange' ? FilePath.SOURCE_MULTIFILE : FilePath.SOURCE;
        await importData(page, filePath);
        await sourcePage.goto();
        const coreValue = sourcePage.sourceFrame.getByText('core0');
        await expect(coreValue).toBeVisible();
    });

    test.afterEach(async ({ page }) => {
        await clearAllData(page);
    });

    // 源码数据导入成功
    // 预期：界面正确
    test('test_source_loadDataSuccess', async ({ sourcePage }) => {
        const { mainContent } = sourcePage;
        await expect(mainContent).toHaveScreenshot(imgMap.loadDataSuccess);
    });

    // 筛选条件变化，修改Core选项
    // 预期：
    // 1、左侧代码表 Instructions Executed、Cycles列变化
    // 2、右侧指令表 Instructions Executed、Cycles列变化
    test('test_source_coreSelectChange', async ({ page, sourcePage }) => {
        const { sourceFrame, coreSelector, mainContent } = sourcePage;
        const coreSelect = new SelectHelpers(page, coreSelector, sourceFrame);
        await coreSelect.open();
        await coreSelect.selectOption('core10.veccore0');
        await expect(mainContent).toHaveScreenshot(imgMap.coreSelectChangeSuccess);
    });

    // 筛选条件变化，修改Source选项
    // 预期：代码表的源代码Source变化
    test('test_source_sourceSelectChange', async ({ page, sourcePage }) => {
        await sourcePage.goto();
        const { sourceFrame, sourceSelector } = sourcePage;
        const coreSelect = new SelectHelpers(page, sourceSelector, sourceFrame);
        await coreSelect.open();
        await coreSelect.selectOption(inputMap.secondFileName);
        const codeTable = sourceFrame.locator('#CodeTable');
        await expect(codeTable).toContainText(resMap.secondFileContens);
    });

    // 点击代码行，联动指令表
    // 点击左侧代码行（有Instructions Executed、Cycles数据的）
    // 预期：
    // 1、代码行高亮
    // 2、右侧指令表高亮相关指令行，并滚动到高亮行
    test('test_source_CodeTable_interact_InstructionTable', async ({ sourcePage }) => {
        const { mainContent } = sourcePage;
        const codeAttrTable = mainContent.locator('#CodeAttrTable');
        // 代码表中有指令数据的行
        const td = codeAttrTable.locator('td', { hasText: '170' }).first();
        await td.click();
        await expect(codeAttrTable.locator('tr.selected').first()).toBeVisible();
        await sourcePage.mouseOut();
        await expect(mainContent).toHaveScreenshot(imgMap.codeTableInteractInsructionTableSuccess);
    });

    // 点击指令表，联动代码行
    // 点击右侧指令表（不是每个都有关联代码行）
    // 预期：
    // 1、左侧代码表高亮关联代码行（只有一行）
    // 2、此行代码关联的其它指令行高亮
    test('test_source_InstructionTable_interact_CodeTable', async ({ sourcePage }) => {
        const { mainContent } = sourcePage;
        const instructionTable = mainContent.locator('#Instructions');
        // 代码表中有关联代码的指令行
        const tr = instructionTable.locator('tr', { hasText: '1' }).first();
        await tr.click();
        await expect(instructionTable.locator('tr.selected').first()).toBeVisible();
        await sourcePage.mouseOut();
        await expect(mainContent).toHaveScreenshot(imgMap.insructionTableInteractCodeTableSuccess);
    });

    // 切换语言，查看中文界面
    test('test_source_Chinese_Language', async ({ page, sourcePage }) => {
        const { mainContent } = sourcePage;
        const frameworkPage = new FrameworkPage(page);
        await frameworkPage.switchLanguageBtn.click();
        await sourcePage.mouseOut();
        await expect(mainContent).toHaveScreenshot(imgMap.switchChineseLanguageSuccess);
        await frameworkPage.switchLanguageBtn.click();
    });
});
