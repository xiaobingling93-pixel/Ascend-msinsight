/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
import { ElLoading } from 'element-plus';
import type { LoadingInstance } from 'element-plus/lib/components/loading/src/loading';
import { onUnmounted } from 'vue';
import { t } from '@/i18n';

interface LoadingOptions {
    [key: string]: any;
    text?: string;
    background?: string;
}
let loadingInstance: LoadingInstance | null = null;

export const useLoading = (): any => {
    const open = (options?: LoadingOptions): void => {
        // 如果遮罩已存在，则不处理直接返回
        if (loadingInstance) {
            return;
        }
        loadingInstance = ElLoading.service({
            fullscreen: true,
            text: t('Loading') as string,
            background: 'rgba(0,0,0,0.5)',
            ...options,
        });
    };

    const close = (): void => {
        if (loadingInstance) {
            loadingInstance.close();
            loadingInstance = null;
        }
    };

    function status(): boolean {
        return loadingInstance != null;
    }

    return { open, close, status };
};
