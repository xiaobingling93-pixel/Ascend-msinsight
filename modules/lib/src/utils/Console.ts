/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

type ConsoleType = 'log' | 'error' | 'warn' | 'info';
class CustomConsole {
    index: number = 0;
    record: Record<number, unknown> = {};
    print(type: ConsoleType, msg: unknown): void {
        this.record[this.index++ % 1000] = msg;
    }

    log(...message: any[]): void {
        this.print('log', message);
    }

    error(...message: any[]): void {
        this.print('error', message);
    }

    warn(...message: any[]): void {
        this.print('warn', message);
    }

    info(...message: any[]): void {
        this.print('info', message);
    }
}

export const customConsole = new CustomConsole();
