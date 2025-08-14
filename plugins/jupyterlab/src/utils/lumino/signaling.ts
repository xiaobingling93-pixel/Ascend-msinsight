/**
 * Copyright (c) Jupyter Development Team.
 * Distributed under the terms of the BSD-3-Clause License.
 *
 * This file is based on code from the @lumino/signaling package:
 * https://github.com/jupyterlab/lumino/tree/master/packages/signaling
 *
 * Modifications made by Huawei Technologies Co., Ltd., 2025.
 */

import { ArrayExt, find } from "./algorithm";

export interface ISignal<T, U> {
    connect(slot: Slot<T, U>, thisArg?: any): boolean;

    disconnect(slot: Slot<T, U>, thisArg?: any): boolean;
}

export type Slot<T, U> = (sender: T, args: U) => void;

export class Signal<T, U> implements ISignal<T, U> {
    constructor(sender: T) {
      this.sender = sender;
    }

    readonly sender: T;

    connect(slot: Slot<T, U>, thisArg?: unknown): boolean {
      return Private.connect(this, slot, thisArg);
    }

    disconnect(slot: Slot<T, U>, thisArg?: unknown): boolean {
      return Private.disconnect(this, slot, thisArg);
    }

    emit(args: U): void {
      Private.emit(this, args);
    }
}

/**
 * The namespace for the `Signal` class statics.
 */
export namespace Signal {
    export function disconnectBetween(sender: unknown, receiver: unknown): void {
      Private.disconnectBetween(sender, receiver);
    }

    export function disconnectSender(sender: unknown): void {
      Private.disconnectSender(sender);
    }

    export function disconnectReceiver(receiver: unknown): void {
      Private.disconnectReceiver(receiver);
    }

    export function disconnectAll(object: unknown): void {
      Private.disconnectAll(object);
    }

    export function clearData(object: unknown): void {
      Private.disconnectAll(object);
    }

    export type ExceptionHandler = (err: Error) => void;
}

namespace Private {
    const exceptionHandler: Signal.ExceptionHandler = (err: Error) => {
        console.error(err);
    };

    export function connect<T, U>(
        signal: Signal<T, U>,
        slot: Slot<T, U>,
        thisArg?: unknown
    ): boolean {
        thisArg = thisArg || undefined;

        let receivers = receiversForSender.get(signal.sender);
        if (!receivers) {
            receivers = [];
            receiversForSender.set(signal.sender, receivers);
        }

        if (findConnection(receivers, signal, slot, thisArg)) {
            return false;
        }

        let receiver = thisArg || slot;

        let senders = sendersForReceiver.get(receiver);
        if (!senders) {
            senders = [];
            sendersForReceiver.set(receiver, senders);
        }

        let connection = { signal, slot, thisArg };
        receivers.push(connection);
        senders.push(connection);

        return true;
    }

    export function disconnect<T, U>(
        signal: Signal<T, U>,
        slot: Slot<T, U>,
        thisArg?: unknown
    ): boolean {
        thisArg = thisArg || undefined;

        let receivers = receiversForSender.get(signal.sender);
        if (!receivers || receivers.length === 0) {
            return false;
        }

        let connection = findConnection(receivers, signal, slot, thisArg);
        if (!connection) {
            return false;
        }

        let receiver = thisArg || slot;

        let senders = sendersForReceiver.get(receiver)!;

        connection.signal = null;
        scheduleCleanup(receivers);
        scheduleCleanup(senders);

        return true;
    }

    export function disconnectBetween(sender: unknown, receiver: unknown): void {
        let receivers = receiversForSender.get(sender);
        if (!receivers || receivers.length === 0) {
            return;
        }

        let senders = sendersForReceiver.get(receiver);
        if (!senders || senders.length === 0) {
            return;
        }

        for (const connection of senders) {
            if (!connection.signal) {
                continue;
            }

            if (connection.signal.sender === sender) {
                connection.signal = null;
            }
        }

        scheduleCleanup(receivers);
        scheduleCleanup(senders);
    }

    export function disconnectSender(sender: unknown): void {
        let receivers = receiversForSender.get(sender);
        if (!receivers || receivers.length === 0) {
            return;
        }

        for (const connection of receivers) {
            if (!connection.signal) {
                continue;
            }

            let receiver = connection.thisArg || connection.slot;

            connection.signal = null;

            scheduleCleanup(sendersForReceiver.get(receiver)!);
        }

        scheduleCleanup(receivers);
    }

    export function disconnectReceiver(receiver: unknown): void {
        let senders = sendersForReceiver.get(receiver);
        if (!senders || senders.length === 0) {
            return;
        }

        for (const connection of senders) {
            if (!connection.signal) {
                continue;
            }

            let sender = connection.signal.sender;

            connection.signal = null;

            scheduleCleanup(receiversForSender.get(sender)!);
        }

        scheduleCleanup(senders);
    }

    export function disconnectAll(object: unknown): void {
        disconnectSender(object);

        disconnectReceiver(object);
    }

    export function emit<T, U>(signal: Signal<T, U>, args: U): void {
        let receivers = receiversForSender.get(signal.sender);
        if (!receivers || receivers.length === 0) {
            return;
        }

        for (let i = 0, n = receivers.length; i < n; ++i) {
            let connection = receivers[i];
            if (connection.signal === signal) {
                invokeSlot(connection, args);
            }
        }
    }

    interface IConnection {
        signal: Signal<any, any> | null;

        readonly slot: Slot<any, any>;

        readonly thisArg: any;
    }

    const dirtySet = new Set<IConnection[]>();

    const sendersForReceiver = new WeakMap<any, IConnection[]>();

    const receiversForSender = new WeakMap<any, IConnection[]>();

    const schedule = (() => {
        let ok = typeof requestAnimationFrame === 'function';
        return ok ? requestAnimationFrame : (cb: () => void) => setTimeout(cb, 0);
    })();

    function invokeSlot(connection: IConnection, args: any): void {
        let { signal, slot, thisArg } = connection;
        try {
            slot.call(thisArg, signal!.sender, args);
        } catch (err) {
            exceptionHandler(new Error(String(err)));
        }
    }

    function findConnection(
        connections: IConnection[],
        signal: Signal<any, any>,
        slot: Slot<any, any>,
        thisArg: any
    ): IConnection | undefined {
        return find(
        connections,
        (connection: any) =>
            connection.signal === signal &&
            connection.slot === slot &&
            connection.thisArg === thisArg
        );
    }

    function cleanupDirtySet(): void {
        dirtySet.forEach(cleanupConnections);
        dirtySet.clear();
    }

    function scheduleCleanup(array: IConnection[]): void {
        if (dirtySet.size === 0) {
        schedule(cleanupDirtySet);
        }
        dirtySet.add(array);
    }
    
    function isDeadConnection(connection: IConnection): boolean {
        return connection.signal === null;
    }

    function cleanupConnections(connections: IConnection[]): void {
        ArrayExt.removeAllWhere(connections, isDeadConnection);
    }
}