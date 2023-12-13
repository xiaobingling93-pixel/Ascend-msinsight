import {test, expect, type Page, FrameLocator, Frame} from '@playwright/test';
import {selectFolder} from './baseOperation';
test.describe('timeline',() => {
    test.describe.configure({ mode: 'serial' });
    let page: Page;
    let timelineFrameLocator: FrameLocator;
    const basePath = 'D:\\GUI_Windows\\AscendInsight-GUI_Windows\\test_data\\';
    test.beforeAll('Open Page', async ({ browser, baseURL }) => {
        page = await browser.newPage({ viewport: { width: 1700,height: 1100 }});
        await page.goto(baseURL + '?port=9000');
        // import file
        const path = basePath + '16ka_gpt3\\master47_1993648_20230919172551_ascend_pt';
        await selectFolder({page, path});
        await waiting(5000);
        timelineFrameLocator = page.frameLocator('[id="Timeline\\ View"]');
    });

    test.afterAll(async () => {
        await page.close();
    });

    // 测试收藏泳道第一层展开功能
    test('testSwimlaneTopLayerExpand', async () => {
        // 选择Timeline页签
        await page.getByRole('menuitem', { name: 'Timeline View' })?.click();
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.locator('svg.insight-unit-fold').first().click();
        await expect(await frame.getByText('Python (1993648)')).toBeVisible();
        // 复原
        await frame.locator('svg.insight-unit-expanded').first().click();
        await compare2Snapshot('resetting.png');
    });

    // 测试收藏泳道功能
    test('testSwimlaneTopLayerPinnedUnpinned', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.locator('svg.insight-unit-fold').first().click();
        await frame.locator('div.unit-info.css-8avfj3').first().hover();
        await frame.locator('div.unit-info.css-8avfj3').locator('button.ant-btn.ant-btn-default.ant-btn-icon-only.css-b5a2p1').first().click();
        await expect(page).toHaveScreenshot('top_layer_pinned.png', { maxDiffPixels: 800 });
        // 取消收藏
        await frame.locator('div.unit-info.css-8avfj3').locator('button.ant-btn.ant-btn-default.ant-btn-icon-only.css-b5a2p1').first().click();
        await expect(page).toHaveScreenshot('top_layer_unpinned.png', { maxDiffPixels: 800 });
        await frame.locator('svg.insight-unit-expanded').first().click();
        await expect(page).toHaveScreenshot('resetting.png', { maxDiffPixels: 800 });
    });

    // 测试泳道展开到第三层
    test('testSwimlaneSecondLayerExpand', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.locator('svg.insight-unit-fold').first().click();
        await frame.locator('svg.insight-unit-fold').nth(2).click();
        await frame.waitForTimeout(1500);
        // 预期第二层数据呈现
        await expect(await frame.getByText('Stream 12')).toBeVisible();
        await frame.locator('svg.insight-unit-expanded').nth(1).click();
        await frame.waitForTimeout(1500);
        await frame.locator('svg.insight-unit-expanded').first().click();
        await expect(page).toHaveScreenshot('resetting.png', { maxDiffPixels: 800 });
    });

    // 测试泳道复位功能
    test('testSwimlaneThirdLayerReset', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.locator('svg.insight-unit-fold').first().click();
        await frame.locator('svg.insight-unit-fold').nth(0).click();
        await frame.waitForTimeout(1500);
        await frame.locator('svg.css-1x2odwv').click();
        await frame.waitForTimeout(1500);
        await compare2Snapshot('third_layer_reset.png');
        await frame.locator('svg.insight-unit-expanded').nth(1).click();
        await frame.waitForTimeout(1500);
        await frame.locator('svg.insight-unit-expanded').first().click();
        await expect(page).toHaveScreenshot('resetting.png', { maxDiffPixels: 800 });
    });

    // 测试数据块详情展示
    test('testSwimlaneThirdLayerSingleSelect', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.locator('svg.insight-unit-fold').first().click();
        await frame.locator('svg.insight-unit-fold').nth(0).click();
        await frame.waitForTimeout(1500);
        await frame.locator('svg.css-1x2odwv').click();
        await frame.waitForTimeout(1500);
        // 点击要展示的数据块
        await page.locator('body').click({
            position: { x: 690, y: 160 },
        });
        await frame.waitForTimeout(1500);
        // 展示详情数据
        await expect(await frame.getByText('Self Time')).toBeVisible();
        await frame.locator('svg.insight-unit-expanded').nth(1).click();
        await frame.waitForTimeout(1500);
        await frame.locator('svg.insight-unit-expanded').first().click();
        await frame.locator('span.anticon.anticon-caret-right.caret').click();
    });

    // 测试顶层泳道数据收起功能
    test('testSwimlaneTopLayerFold', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.locator('svg.insight-unit-fold').first().click();
        await frame.locator('svg.insight-unit-expanded').first().click();
        await expect(await frame.getByText('Python (1993648)')).not.toBeVisible();
    });

    // 测试泳道offset功能
    test('testSwimlaneThirdLayerOffset', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        // 展开泳道
        await frame.locator('svg.insight-unit-fold').first().click();
        await frame.locator('svg.insight-unit-fold').nth(2).click();
        await frame.waitForTimeout(1500);
        // reset展示数据
        await frame.locator('svg.css-1x2odwv').click();
        await frame.waitForTimeout(1500);
        await frame.getByText('Offset').first().click();
        await frame.waitForTimeout(1500);
        // 输入正数泳道数据向左偏移
        await frame.locator('.css-4lza26').locator('input').fill('1000000');
        await frame.waitForTimeout(1500);
        await page.keyboard.down("Enter");
        await expect(page).toHaveScreenshot('layer_offset_left.png', { maxDiffPixels: 900 });
        // 输入0泳道数据向右偏移
        await frame.locator('.css-4lza26').locator('input').fill('0');
        await frame.waitForTimeout(1500);
        await page.keyboard.down("Enter");
        await expect(page).toHaveScreenshot('layer_offset_right.png', { maxDiffPixels: 900 });
        // 复位页面
        await frame.locator('svg.insight-unit-expanded').nth(1).click();
        await frame.locator('svg.insight-unit-expanded').first().click();
    });

    // 测试systemView表格初始化
    test('testSystemViewExist', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.locator('span.anticon.anticon-caret-left.caret').click();
        await frame.getByText('StatsSystemView').click();
        await expect(await frame.getByText('Python API Summary')).toBeVisible();
    });

    // 测试表格切换
    test('testSystemViewChangeTag', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.getByText('CANN API Summary').click();
        await expect(page).toHaveScreenshot('system_view_static_table_cann.png', { maxDiffPixels: 1500 });
        await frame.getByText('Ascend HardWare Task Summary').click();
        await expect(page).toHaveScreenshot('system_view_static_table_task.png', { maxDiffPixels: 1500 });
        await frame.getByText('HCCL Summary').click();
        await expect(page).toHaveScreenshot('system_view_static_table_hccl.png', { maxDiffPixels: 1500 });
        await frame.getByText('Overlap Analysis').click();
        await expect(page).toHaveScreenshot('system_view_static_table_overlap.png', { maxDiffPixels: 1500 });
        await frame.getByText('Kernel Details').click();
        await expect(page).toHaveScreenshot('system_view_static_table_kernel.png', { maxDiffPixels: 1500 });
    });

    // 测试统计表格按照Num Calls进行排序
    test('testSystemViewStaticSort', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.getByText('Python API Summary').click();
        // Num Calls升序
        await frame.locator('th').filter({ hasText: 'Num Calls' }).click();
        await expect(page).toHaveScreenshot('system_view_table_aes.png', { maxDiffPixels: 1500 });
        // Num Calls降序
        await frame.locator('th').filter({ hasText: 'Num Calls' }).click();
        await expect(page).toHaveScreenshot('system_view_table_des.png', { maxDiffPixels: 1500 });
        // 恢复
        await frame.locator('th').filter({ hasText: 'Num Calls' }).click();
        await expect(page).toHaveScreenshot('system_view_table_reset.png', { maxDiffPixels: 1500 });
    });

    // 测试kernel detail表格按照Block Dim进行排序
    test('testSystemViewKernelSort', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.getByText('Kernel Details').click();
        // Block Dim升序
        await frame.locator('th').filter({ hasText: 'Block Dim' }).click();
        await expect(page).toHaveScreenshot('system_view_table_kernel_aes.png', { maxDiffPixels: 1500 });
        // Block Dim降序
        await frame.locator('th').filter({ hasText: 'Block Dim' }).click();
        await expect(page).toHaveScreenshot('system_view_table_kernel_des.png', { maxDiffPixels: 1500 });
        // 恢复
        await frame.locator('th').filter({ hasText: 'Block Dim' }).click();
        await expect(page).toHaveScreenshot('system_view_table_kernel_reset.png', { maxDiffPixels: 1500 });
    });

    // 测试统计数据表切换数据和切换页面显示个数
    test('testSystemViewStaticTablePageChange', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.getByText('Python API Summary').click();
        // 下一页
        await frame.locator('li.ant-pagination-item-2').click();
        await frame.waitForTimeout(500);
        await expect(page).toHaveScreenshot('system_view_table_page_to_2.png', { maxDiffPixels: 1500 });
        // 切换每页记录个数
        await frame.getByText('10 / page').click();
        await frame.getByText('20 / page').click();
        await frame.waitForTimeout(1000);
        await frame.locator('div.ant-tabs-content.ant-tabs-content-top').click();
        await page.mouse.wheel(0, 1000);
        await expect(page).toHaveScreenshot('system_view_table_page_20.png', { maxDiffPixels: 1500 });
    });

    // 测试kernel detail表格切换数据和切换页面显示个数
    test('testSystemViewKernelTablePageChange', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.getByText('Kernel Details').click();
        // 下一页
        await frame.locator('li.ant-pagination-item-2').click();
        await frame.waitForTimeout(500);
        await expect(page).toHaveScreenshot('system_view_kernel_table_page_to_2.png', { maxDiffPixels: 1500 });
        // 切换每页记录个数
        await frame.getByText('10 / page').click();
        await frame.getByText('20 / page').click();
        await frame.waitForTimeout(1000);
        await frame.locator('div.ant-tabs-content.ant-tabs-content-top').click();
        await page.mouse.wheel(0, 1000);
        await expect(page).toHaveScreenshot('system_view_kernel_table_page_20.png', { maxDiffPixels: 1500 });
    });

    // 测试kernel detail表格数据和timeline联动
    test('testSystemViewKernelLinkTimeline', async () => {
        const frame = page.frame({ url: /.Timeline.*/  });
        await frame.getByText('Kernel Details').click();
        // 点击click
        await frame.getByRole('button', { name: 'click' }).first().click();
        await expect(page).toHaveScreenshot('system_view_kernel_link_timeline.png', { maxDiffPixels: 1500 });
        // 还原成初始导入数据的样子
        await frame.locator('svg.insight-unit-expanded').nth(1).click();
        await frame.locator('svg.insight-unit-expanded').first().click();
        await frame.locator('span.anticon.anticon-caret-right.caret').click();
        await expect(page).toHaveScreenshot('resetting.png', { maxDiffPixels: 1500 });
    });

    async function addMarker(xPosition: number, yPosition: number, frameLocator: FrameLocator) {
        await timelineFrameLocator.locator('#timelineFlagCnvas').click({
            position: {
                x: xPosition,
                y: yPosition
            }
        });
    }

    async function clearMarker( timelineFrameLocator: FrameLocator) {
        await openMarkerList(timelineFrameLocator);
        await timelineFrameLocator.getByText('Clear').click();
        await timelineFrameLocator.getByRole('button', { name: 'OK' }).click();
    }

    async function openMarkerList( timelineFrameLocator: FrameLocator) {
        await timelineFrameLocator.getByRole('button').first().click();
    }

    async function closeMarkerList( timelineFrameLocator: FrameLocator) {
        await timelineFrameLocator.locator('#timeMakerList').getByRole('button').click();
    }

    async function compare2Snapshot(snapshotName: string, maxDiff: number = 800) {
        expect(await page.screenshot({ fullPage: true })).toMatchSnapshot(snapshotName, {maxDiffPixels: maxDiff});
    }

    async function resetTimeline(timelineFrameLocator: FrameLocator) {
        await timelineFrameLocator.locator('.css-1x2odwv').click();
    }

    async function waiting(time: number = 150) {
        await page.waitForTimeout(time);
    }

    async function clickRoomInBtn(timelineFrameLocator: FrameLocator) {
        await timelineFrameLocator.locator('svg:nth-child(4)').first().click();
        await waiting();
    }

    async function clickRoomOutBtn(timelineFrameLocator: FrameLocator) {
        await timelineFrameLocator.locator('svg:nth-child(2)').first().click();
        await waiting();
    }

    test('testToolbarZoomInAndOutAndReset', async () => {
        await resetTimeline(timelineFrameLocator);
        // 展开第一张卡的泳道
        await timelineFrameLocator.locator('svg.insight-unit-fold').first().click();
        // 展开到Python层
        await timelineFrameLocator.locator('svg.insight-unit-fold').nth(0).click();

        // 点击放大按钮
        await clickRoomInBtn(timelineFrameLocator);
        await clickRoomInBtn(timelineFrameLocator);
        await clickRoomInBtn(timelineFrameLocator);
        await compare2Snapshot('timelineToolbarZoomIn.png');

        // 点击缩小按钮
        await clickRoomOutBtn(timelineFrameLocator);
        await clickRoomOutBtn(timelineFrameLocator);
        await clickRoomOutBtn(timelineFrameLocator);
        await compare2Snapshot('timelineToolbarZoomOut.png');

        // 点击重置按钮
        await resetTimeline(timelineFrameLocator);
        await waiting();
        await compare2Snapshot('timelineToolbarZoomReset.png');
        // 收起第一张卡的泳道
        await timelineFrameLocator.locator('.insight-unit-expanded').nth(1).click();
        await timelineFrameLocator.locator('.insight-unit-expanded').first().click();
    });

    async function roomInByHotkey(timelineFrameLocator: FrameLocator) {
        await timelineFrameLocator.locator('div.chart-empty').press('W');
        await waiting();
    }

    async function roomOutByHotkey(timelineFrameLocator: FrameLocator) {
        await timelineFrameLocator.locator('div.chart-empty').press('S');
        await waiting();
    }

    test('testToolbarZoomInAndOutAndResetWithHotkey', async () => {
        // 展开第一张卡的泳道
        await timelineFrameLocator.locator('svg.insight-unit-fold').first().click();
        // 展开到Python层
        await timelineFrameLocator.locator('svg.insight-unit-fold').nth(0).click();

        // 点击快捷键W放大
        await roomInByHotkey(timelineFrameLocator);
        await roomInByHotkey(timelineFrameLocator);
        await roomInByHotkey(timelineFrameLocator);
        await compare2Snapshot('timelineToolbarZoomInWithHotkey.png');

        // 点击快捷键S缩小按钮
        await roomOutByHotkey(timelineFrameLocator);
        await roomOutByHotkey(timelineFrameLocator);
        await roomOutByHotkey(timelineFrameLocator);
        await compare2Snapshot('timelineToolbarZoomOutWithHotkey.png');
        // 收起第一张卡的泳道
        await timelineFrameLocator.locator('.insight-unit-expanded').nth(1).click();
        await timelineFrameLocator.locator('.insight-unit-expanded').first().click();
    });

    test('testToolbarSearch', async () => {
        // 点击搜索按钮
        await timelineFrameLocator.locator('button:nth-child(3)').click();
        await timelineFrameLocator.getByRole('textbox').click();
        // 输入关键词进行搜索
        await timelineFrameLocator.getByRole('textbox').fill('aten::item');
        await timelineFrameLocator.getByRole('tooltip').locator('button').click();
        await waiting();
        await expect(timelineFrameLocator.getByTitle('1/17')).toBeVisible();

        // 跳转到下一个
        await timelineFrameLocator.getByRole('button', { name: 'right' }).click();
        await waiting(500);
        await compare2Snapshot('timelineToolbarSearchNextOne.png');

        // 跳转到上一个
        await timelineFrameLocator.getByRole('button', { name: 'left' }).click();
        await waiting(500);
        await compare2Snapshot('timelineToolbarSearchLastOne.png');

        // 跳转到第三个
        await timelineFrameLocator.getByRole('listitem', { name: '1/17' }).getByRole('textbox').click();
        await timelineFrameLocator.getByRole('listitem', { name: '1/17' }).getByRole('textbox').fill('3');
        await timelineFrameLocator.getByRole('listitem', { name: '1/17' }).getByRole('textbox').press('Enter');
        await waiting(300);
        await compare2Snapshot('timelineToolbarSearchRedirectTo.png');

        // 点击取消按钮
        await timelineFrameLocator.getByRole('tooltip', { name: 'left /17 right' }).getByRole('button').first().click();
        await waiting(300);
        await compare2Snapshot('timelineToolbarSearchCancel.png');

        // 收起card 0的泳道
        await timelineFrameLocator.locator('.insight-unit-expanded').nth(1).click();
        await timelineFrameLocator.locator('.insight-unit-expanded').first().click();
    });

    test('testAddAndRemoveMarker', async () => {
        await page.locator('span.deleteIcon').first().click();
        await page.getByRole('button', { name: 'Confirm' }).click();
        await selectFolder({page, path: basePath + '16ka_gpt3\\master47_1993648_20230919172551_ascend_pt'});
        await resetTimeline(timelineFrameLocator);
        // 添加marker
        await addMarker(242, 9, timelineFrameLocator);
        await waiting();
        await compare2Snapshot('timelineAddMarker.png');

        // 删除marker
        await timelineFrameLocator.locator('#timelineFlagCnvas').dblclick({
            position: {
                x: 242,
                y: 9
            }
        });
        await waiting();
        await timelineFrameLocator.getByRole('button', { name: 'Delete' }).click();
        await waiting();
        await compare2Snapshot('timelineRemoveMarker.png');
    });

    test('testEditMarker', async () => {
        await resetTimeline(timelineFrameLocator);
        // 添加marker
        await addMarker(242, 9, timelineFrameLocator);
        await waiting();
        await compare2Snapshot('timelineEditMarkerAdd.png');

        // 修改marker名称
        await timelineFrameLocator.locator('#timelineFlagCnvas').dblclick({
            position: {
                x: 242,
                y: 9
            }
        });
        await timelineFrameLocator.getByRole('textbox').fill('testMarker');
        await timelineFrameLocator.getByRole('button', { name: 'OK' }).click();

        // 修改旗标颜色
        await timelineFrameLocator.locator('#timelineFlagCnvas').dblclick({
            position: {
                x: 242,
                y: 9
            }
        });
        await waiting();
        // check修改名称是否生效
        await expect(timelineFrameLocator.getByRole('textbox')).toHaveValue('testMarker');

        await timelineFrameLocator.locator('#colorBoxes div').nth(1).click();
        await timelineFrameLocator.getByRole('button', { name: 'OK' }).click();
        await waiting();
        // check 修改颜色是否生效
        await compare2Snapshot('timelineEditMarkerSetColor.png');

        // 删除marker
        await timelineFrameLocator.locator('#timelineFlagCnvas').dblclick({
            position: {
                x: 242,
                y: 9
            }
        });
        await waiting(100);
        await timelineFrameLocator.getByRole('button', { name: 'Delete' }).click();
        await waiting(100);
        await compare2Snapshot('timelineEditMarkerRemove.png');
    });

    test('testToolbarMarkerListEdit', async () => {
        // 添加三个marker
        let locations = [
            [100, 6],
            [150, 6],
            [200, 6]
        ];
        for (let i = 0; i < locations.length; i++) {
            await addMarker(locations[i][0], locations[i][1], timelineFrameLocator);
        }
        await page.mouse.move(1, 1);
        await compare2Snapshot('timelineToolbarMarkerListAdd.png', 810);

        // 打开marker列表
        await timelineFrameLocator.getByRole('button').first().click();

        // 删除第一个旗标
        let rows = page.frameLocator('[id="Timeline\\ View"]').locator('.singleTimeMakerRow');
        let sizeBeforeDelete = await rows.count();
        await timelineFrameLocator.locator('#deleteButton').first().click();
        let sizeAfterDelete = await rows.count();
        expect(sizeBeforeDelete - sizeAfterDelete).toBe(1);

        // 修改第一个旗标的颜色
        const divs = await timelineFrameLocator.locator('.singleTimeMakerRow').first()
            .locator("div").all();
        for (const div of divs) {
            const id = await div.getAttribute('id');
            if (id != null && id.toString().includes('color')) {
                await div.click();
                break;
            }
        }
        await timelineFrameLocator.locator('#colorBoxes div').first().click();
        await timelineFrameLocator.getByRole('button', { name: 'OK' }).click();
        await waiting(100);
        await compare2Snapshot('timelineToolbarMarkerListChangeColor.png');

        // 点击clear并取消
        await timelineFrameLocator.getByText('Clear').click();
        await timelineFrameLocator.getByRole('button', { name: 'Cancel' }).click();
        await waiting(100);
        await compare2Snapshot('timelineToolbarMarkerListClearThenCancel.png');

        // 删除所有marker
        await timelineFrameLocator.getByText('Clear').click();
        await timelineFrameLocator.getByRole('button', { name: 'OK' }).click();
        await waiting();
        await compare2Snapshot('timelineToolbarMarkerListClear.png');
    });

    test('testToolbarMarkerRedirectToMarker', async () => {
        // 添加三个marker
        let locations = [
            [100, 6],
            [300, 6],
            [600, 6]
        ];
        for (let i = 0; i < locations.length; i++) {
            await addMarker(locations[i][0], locations[i][1], timelineFrameLocator);
        }
        await page.mouse.move(1, 1);

        // 点击快捷键W放大
        await timelineFrameLocator.locator('div.chart-empty').press('W');
        await waiting(100);
        await timelineFrameLocator.locator('div.chart-empty').press('W');
        await waiting(100);
        await timelineFrameLocator.locator('div.chart-empty').press('W');
        await waiting(100);

        // 打开marker列表
        await openMarkerList(timelineFrameLocator);

        // 跳转到第一个旗标
        await timelineFrameLocator.locator('svg.JumpSvgIcon').nth(0).click();
        await closeMarkerList(timelineFrameLocator);
        await waiting();
        await compare2Snapshot('timelineToolbarMarkerRedirectToMarkerJump2FirstOne.png');
        await openMarkerList(timelineFrameLocator);
        await timelineFrameLocator.locator('svg.JumpSvgIcon').nth(2).click();
        await closeMarkerList(timelineFrameLocator);
        await waiting();
        await compare2Snapshot('timelineToolbarMarkerRedirectToMarkerJump2ThirdOne.png');


        // 删除所有marker
        await clearMarker(timelineFrameLocator);
        await waiting();
        await compare2Snapshot('timelineToolbarMarkerRedirectToMarkerClear.png')
    });

    test('testToolbarMarkerListCompare', async () => {
        await resetTimeline(timelineFrameLocator);
        // 添加三个marker
        let locations = [
            [100, 6],
            [300, 6],
            [600, 6]
        ];
        for (let i = 0; i < locations.length; i++) {
            await addMarker(locations[i][0], locations[i][1], timelineFrameLocator);
        }
        await page.mouse.move(1, 1);

        // 打开marker列表
        await openMarkerList(timelineFrameLocator);

        // 点击第二个marker
        await timelineFrameLocator.locator('.singleTimeMakerRow').nth(1).click();
        await waiting();
        // 将鼠标移动到第一个marker
        await timelineFrameLocator.locator('.singleTimeMakerRow').nth(0).hover();
        let timeDiff = await timelineFrameLocator.locator('#timeDiffDisplay').textContent();
        expect(timeDiff).toBe('173.9ms');

        // 将鼠标移动到第三个marker
        await timelineFrameLocator.locator('.singleTimeMakerRow').nth(2).hover();
        timeDiff = await timelineFrameLocator.locator('#timeDiffDisplay').textContent();
        expect(timeDiff).toBe('260.8ms');

        // 关闭marker列表
        await closeMarkerList(timelineFrameLocator);
        // 删除所有marker
        await clearMarker(timelineFrameLocator);
        await waiting();
        await compare2Snapshot('timelineToolbarMarkerListCompareClear.png')
    });

    /** 多卡数据测试用例 **/
    test('testToolbarFilterByCardAndCancel', async () => {
        // 导入多卡数据
        await selectFolder({page, path: basePath + '16ka_gpt3'});
        await waiting(3000);

        // 点击filter图标
        await timelineFrameLocator.locator('button:nth-child(2)').first().click();
        await expect(timelineFrameLocator.getByTitle('Filter')).toBeVisible()
        await timelineFrameLocator.getByTitle('Filter').click();

        // 按card过滤,过滤出3号卡
        await timelineFrameLocator.getByText('Card Filter').click();
        await timelineFrameLocator.locator('input.ant-input').nth(1).click();
        await timelineFrameLocator.getByTitle('3').getByText('3').nth(1).click();
        await waiting(800);
        await compare2Snapshot('timelineToolbarFilterCardNumber3.png');

        // 点击取消按钮
        await timelineFrameLocator.getByRole('tooltip', { name: 'Card Filter 3' }).getByRole('button').click();
        await compare2Snapshot('timelineToolbarFilterByCardThenCancel.png');
    });

    test('testToolbarFilterByUnitAndCancel', async () => {
        // 展开card 0的泳道
        await timelineFrameLocator.locator('.insight-unit-fold').first().click();

        // 点击filter图标
        await timelineFrameLocator.locator('button:nth-child(2)').first().click();

        // 按units过滤
        await timelineFrameLocator.getByTitle('Card Filter').click();
        await timelineFrameLocator.getByText('Units Filter').click();

        // 输入"hardware",选择"Ascend Hardware"
        await timelineFrameLocator.locator('input.ant-input').nth(1).click();
        await timelineFrameLocator.getByTitle('Ascend Hardware').getByText('Ascend Hardware').click();
        await waiting(800);
        await compare2Snapshot('timelineToolbarFilterUnitAscendHardware.png');

        // 点击取消按钮
        await timelineFrameLocator.getByRole('tooltip', { name: 'Units Filter Ascend Hardware' }).getByRole('button').click();
        await compare2Snapshot('timelineToolbarFilterByUnitThanCancel.png');
        // 收起card 0的泳道
        await timelineFrameLocator.locator('.insight-unit-expanded').first().click();
    });
});
