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
export const loopActionFactory = (callBackFunc: (args: any) => void, msPerTime: number, msDelay: number = 0):
{ beginLoop: (e?: React.KeyboardEvent<HTMLDivElement>) => void; clearLoop: () => void } => {
    let lastTime = 0;
    let isZoomInning = true;
    let delay = msDelay;
    function actionPerform<T>(args?: T): void {
        const now = performance.now();
        if (now - lastTime >= msPerTime && isZoomInning) {
            lastTime = now + delay;
            callBackFunc(args);
            delay = 0;
        }
        if (isZoomInning) {
            requestAnimationFrame((time) => actionPerform(args));
        }
    };
    function beginAction<T>(args?: T): void {
        delay = msDelay;
        isZoomInning = true;
        actionPerform(args);
    }
    function clearAction(): void {
        isZoomInning = false;
        lastTime = 0;
    }
    return { beginLoop: beginAction, clearLoop: clearAction };
};
