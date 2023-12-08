import { type Page,type Frame } from '@playwright/test'

export async function selectFolder({ page, path }: { page: Page;path?: string}) {
    await page.locator('.el-aside > .header > .icon-button').click();
    let newPath = path ?? 'D:\\workspace\\data\\16ka_gpt3';
    newPath = newPath.replace(/:/g, '');
    const list = newPath.split('\\');
    for (let i = 0; i < list.length; i++) {
        const curPath = list.slice(0, i + 1).join('');
        await page.locator('.custom-tree-node').locator(`span[name=${curPath}]`).click();
    }
    await page.getByText('Confirm').click();
}

export async function clickSelect({ locator, cur, target }: { locator: Page|Frame;cur: string;target:string;}) {
    await locator.locator('.ant-select-selector')?.getByText(cur,{exact:true})?.click();
    await locator.locator('.ant-select-item.ant-select-item-option')?.getByText(target,{exact:true})?.click();
}
