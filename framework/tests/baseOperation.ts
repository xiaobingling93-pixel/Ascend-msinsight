import { type Page, type Frame } from '@playwright/test'

export async function selectFolder({ page, path }: { page: Page;path?: string}) {
    await page.locator('.el-aside > .header > .icon-button').click();
    let newPath = path ?? 'D:\\GUI_Windows\\AscendInsight-GUI_Windows\\test_data\\16ka_gpt3';
    newPath = newPath.replace(/:/g, '');
    newPath = newPath.replace(/\./g, '--dot--');
    const list = newPath.split('\\');
    for (let i = 0; i < list.length; i++) {
        const curPath = list.slice(0, i + 1).join('');
        const nextPath = list.slice(0, i + 2).join('');
        const node = await page.locator('.custom-tree-node').locator(`span[name=${nextPath}]`).count();
        if (node === 0 || curPath === nextPath) {
            await page.locator('.custom-tree-node').locator(`span[name=${curPath}]`).click();
        }
    }
    await page.getByText('Confirm').click();
}

export async function clickSelect({ locator, cur, target }: { locator: Page | Frame; cur: string; target: string; }) {
    await locator.locator('.ant-select-selector')?.getByText(cur, { exact: true })?.click();
    await locator.locator('.ant-select-item.ant-select-item-option')?.getByText(target, { exact: true })?.click();
}
