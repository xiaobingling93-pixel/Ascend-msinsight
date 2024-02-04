/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

type ConsoleType = 'log' | 'error' | 'warn' | 'info';
class CustomConsole {
    print(type: ConsoleType, msg: any): void {
        // 本地调试： console[type](msg)
    }

    log(message: any): void {
       this.print('log', message);
    }

    error(message: any): void {
        this.print('error', message);
    }

    warn(message: any): void {
        this.print('warn', message);
    }

    info(message: any): void {
        this.print('info', message);
    }
}

export const Console = new CustomConsole();
