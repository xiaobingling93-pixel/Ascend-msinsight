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
import { RenderEngine } from './index';

const RENDER_FREQUENCY = 16;
describe('RenderEngine', () => {
    let renderEngine: RenderEngine;

    beforeEach(() => {
        jest.useFakeTimers();
        renderEngine = new RenderEngine();
    });

    afterEach(() => {
        jest.useRealTimers();
    });

    it('should start in running state', () => {
        // 应该以运行状态开始
        expect((renderEngine as any)._status).toBe('running');
    });

    it('should add and execute a once task', () => {
        // 应该添加并执行一次性任务
        const mockAction = jest.fn();
        const renderID = renderEngine.addTask(mockAction, 'once');

        // 检查任务是否被添加
        expect((renderEngine as any)._renderTasks.has(renderID)).toBe(true);
        expect((renderEngine as any)._renderTasks.get(renderID).status).toBe('pending');

        jest.advanceTimersByTime(RENDER_FREQUENCY); // 模拟时间推进

        // 检查任务是否被执行了一次
        expect(mockAction).toHaveBeenCalledTimes(1);
        expect((renderEngine as any)._renderTasks.get(renderID).status).toBe('fullfilled');
    });

    it('should add and execute an always task', () => {
        // 应该添加并执行持续任务
        const mockAction = jest.fn();
        const renderID = renderEngine.addTask(mockAction, 'always');

        // 检查任务是否被添加
        expect((renderEngine as any)._renderTasks.has(renderID)).toBe(true);
        expect((renderEngine as any)._renderTasks.get(renderID).status).toBe('pending');

        jest.advanceTimersByTime(RENDER_FREQUENCY); // 模拟时间推进

        // 检查任务是否被执行并保持为pending状态
        expect(mockAction).toHaveBeenCalledTimes(1);
        expect((renderEngine as any)._renderTasks.get(renderID).status).toBe('pending');
    });

    it('should delete a task', () => {
        // 应该删除一个任务
        const mockAction = jest.fn();
        const renderID = renderEngine.addTask(mockAction, 'always');

        // 检查任务是否被添加
        expect((renderEngine as any)._renderTasks.has(renderID)).toBe(true);

        renderEngine.deleteTask(renderID);

        // 检查任务是否被删除
        expect((renderEngine as any)._renderTasks.has(renderID)).toBe(false);
    });

    it('should stop and start the engine', () => {
        // 应该停止和启动引擎
        renderEngine.stop();
        expect((renderEngine as any)._status).toBe('waiting');

        renderEngine.start();
        expect((renderEngine as any)._status).toBe('running');
    });

    it('should not execute tasks when stopped', () => {
        // 在停止时不应该执行任务
        const mockAction = jest.fn();
        renderEngine.addTask(mockAction, 'always');

        renderEngine.stop();
        jest.advanceTimersByTime(RENDER_FREQUENCY); // 模拟时间推进

        // 检查任务是否被执行了一次
        expect(mockAction).toHaveBeenCalledTimes(1);

        jest.advanceTimersByTime(RENDER_FREQUENCY); // 模拟时间推进

        // 检查任务是否不会增加
        expect(mockAction).toHaveBeenCalledTimes(1);
    });

    it('should execute tasks periodically when running', () => {
        // 应该在运行时周期性地执行任务
        const mockAction = jest.fn();
        renderEngine.addTask(mockAction, 'always');

        jest.advanceTimersByTime(RENDER_FREQUENCY); // 模拟时间推进

        // 检查任务是否被执行了一次
        expect(mockAction).toHaveBeenCalledTimes(1);

        jest.advanceTimersByTime(RENDER_FREQUENCY); // 模拟更多时间推进

        // 检查任务是否被执行了两次
        expect(mockAction).toHaveBeenCalledTimes(2);
    });
});
