/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
