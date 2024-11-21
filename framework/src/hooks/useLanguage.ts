/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import {onMounted, type Ref, ref} from 'vue';
import i18n from '@/i18n';
import connector from '@/connection';
import { LocalStorageKeys, localStorageService } from '@/utils/local-storage';
import { useSession } from '@/stores/session';

export enum Languages {
    ZH = 'zhCN',
    EN = 'enUS',
}
interface IUseLanguage {
    language: Ref<string>;
    switchLanguage: (newLanguage: Languages) => void;
}
export function useLanguage(): IUseLanguage {
    const { session } = useSession();
    const language = ref('');

    onMounted((): void => {
        const savedLanguage = localStorageService.getItem(LocalStorageKeys.LANGUAGE);
        if (savedLanguage) {
            const lang = savedLanguage as Languages;
            i18n.changeLanguage(lang);
            language.value = lang;
            session.language = lang;
        }
    });

    function switchLanguage(newLanguage: Languages): void {
        i18n.changeLanguage(newLanguage);
        language.value = newLanguage;
        session.language = newLanguage;
        localStorageService.setItem(LocalStorageKeys.LANGUAGE, newLanguage);
        sendMessage(newLanguage);
    }

    function sendMessage(lang: Languages): void {
        connector.send({
            event: 'switchLanguage',
            body: {
                lang,
            },
            target: 'plugin',
        });
    }

    return {
        language,
        switchLanguage,
    };
}
