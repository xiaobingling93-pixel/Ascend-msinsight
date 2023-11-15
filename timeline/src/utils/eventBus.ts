import { pull } from 'lodash';
import { useEffect } from 'react';

export type EventHandler<T> = (arg?: T) => void;

export enum EventType {
    UNITWRAPPERSCROLL = 'UNITWRAPPERSCROLL', // 泳道区滚动
    PINNEDUNITWRAPPERSCROLL = 'PINNEDUNITWRAPPERSCROLL', // 置顶泳道区滚动
    SCROLLTOLOCATEUNIT = 'SCROLLTOLOCATEUNIT', // 滚动到特定泳道
    GLOBALSEARCH = 'GLOBALSEARCH', // 键盘输入搜索快捷键
}

class EventBus<T> {
    private readonly _events: Map<string, Array<EventHandler<T>>>;
    private readonly _maxListeners: number = 10; // 设立监听上限

    constructor() {
        this._events = new Map(); // 储存事件/回调键值对
    }

    private _getHandlers(event: string): Array<EventHandler<T>> {
        let handlers = this._events.get(event);
        if (!handlers) {
            handlers = [];
            this._events.set(event, handlers);
        };
        return handlers;
    }

    // 触发事件
    public emit(event: string, args?: T): void {
        const handler = this._getHandlers(event);
        for (let i = 0; i < handler.length; i++) {
            const fn = handler[i];
            fn(args);
        }
    }

    // 监听事件
    public on(event: string, handler: EventHandler<T>): void {
        const handlers = this._getHandlers(event);
        handlers.push(handler);
    }

    // 移除事件
    public off(event: string, handler?: EventHandler<T>): void {
        if (handler) {
            const handlers = this._getHandlers(event);
            pull(handlers, handler);
        } else {
            this._events.delete(event);
        }
    }

    // 一次监听
    public once(event: string, handler: EventHandler<T>): void {
        const cb = (arg?: T): void => {
            this.off(event, cb);
            handler(arg);
        };
        this.on(event, cb);
    }
}

// 事件总线
const eventBus = new EventBus();

export function useEventBus(event: string, handler: EventHandler<unknown>): void {
    useEffect(() => {
        eventBus.on(event, handler);
        return () => {
            eventBus.off(event, handler);
        };
    }, []);
}

export default eventBus;
