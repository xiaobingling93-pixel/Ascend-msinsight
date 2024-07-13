/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
export const loopActionFactory = (callBackFunc: (args: any) => void, msPerTime: number, msDelay: number = 0):
{ beginLoop: (e?: React.KeyboardEvent<HTMLDivElement>) => void; clearLoop: () => void } => {
    let lastTime = 0;
    let isZoomInning = true;
    let delay = msDelay;
    function actionPerform<T>(args?: T): void {
        const now = performance.now();
        if (now - lastTime >= msPerTime) {
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
        setTimeout(() => {
            lastTime = 0;
        }, msPerTime);
    }
    return { beginLoop: beginAction, clearLoop: clearAction };
};
