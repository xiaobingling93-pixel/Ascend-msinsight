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

type ConsoleType = 'log' | 'error' | 'warn' | 'info';
class CustomConsole {
    index: number = 0;
    record: Record<number, unknown> = {};
    print(_: ConsoleType, msg: unknown): void {
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
