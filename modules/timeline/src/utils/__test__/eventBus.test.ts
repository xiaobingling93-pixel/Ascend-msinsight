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

import EventBus from '../eventBus';

describe('EventBus', () => {
    let eventBus: any;

    beforeEach(() => {
        eventBus = EventBus;
    });

    it('should add a listener to an event', () => {
        const handler = jest.fn();
        eventBus.on('testEvent', handler);
        expect(eventBus._events.get('testEvent')).toContain(handler);
    });

    it('should remove a listener from an event', () => {
        const handler = jest.fn();
        eventBus.on('testEvent', handler);
        eventBus.off('testEvent', handler);
        expect(eventBus._events.get('testEvent')).not.toContain(handler);
    });

    it('should remove all listeners from an event if no handler is provided', () => {
        const handler1 = jest.fn();
        const handler2 = jest.fn();
        eventBus.on('testEvent', handler1);
        eventBus.on('testEvent', handler2);
        eventBus.off('testEvent');
        expect(eventBus._events.get('testEvent')).toBeUndefined();
    });

    it('should emit an event and call all listeners', () => {
        const handler1 = jest.fn();
        const handler2 = jest.fn();
        eventBus.on('testEvent', handler1);
        eventBus.on('testEvent', handler2);
        eventBus.emit('testEvent', 'testArg');
        expect(handler1).toHaveBeenCalledWith('testArg');
        expect(handler2).toHaveBeenCalledWith('testArg');
    });

    it('should call a one-time listener and then remove it', () => {
        const handler = jest.fn();
        eventBus.once('testEvent', handler);
        eventBus.emit('testEvent', 'testArg');
        expect(handler).toHaveBeenCalledWith('testArg');
        eventBus.emit('testEvent', 'testArg2');
        expect(handler).toHaveBeenCalledTimes(1);
    });
});
