/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import { loopActionFactory } from '../FactoryActions';

describe('loopActionFactory', () => {
    it('should start and stop loop action correctly', () => {
        jest.useFakeTimers();
        // 模拟回调函数
        const callBackFunc = jest.fn();

        // 创建循环控制对象
        const loopControl = loopActionFactory(callBackFunc, 100);

        // 开始循环
        loopControl.beginLoop();

        // 等待一段时间，以便循环执行
        jest.advanceTimersByTime(150);

        // 断言回调函数被调用了多次（说明循环在执行）
        expect(callBackFunc).toHaveBeenCalled();

        // 停止循环
        loopControl.clearLoop();

        // 等待一段时间，以便确认循环已停止
        jest.advanceTimersByTime(200);

        // 断言回调函数没有再被调用（说明循环已停止）
        expect(callBackFunc).toHaveBeenCalledTimes(1);
    });
});
