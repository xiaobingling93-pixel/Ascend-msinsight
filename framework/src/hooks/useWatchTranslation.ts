/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { watch, ref, type Ref } from 'vue';
import { t } from '@/i18n';
import { type TFunctionDetailedResult } from 'i18next';
import { useSession } from '@/stores/session';

export const useWatchTranslation = (keys: string[]): Array<Ref<string>> => {
    const { session } = useSession();
    const updatedKeys = keys.map((key) => ref(t(key)));

    watch(
        () => session.language,
        () => {
            keys.forEach((key, index) => {
                updatedKeys[index].value = t(key);
            });
        },
    );

    return updatedKeys as Array<Ref<string>>;
};

export default useWatchTranslation;
