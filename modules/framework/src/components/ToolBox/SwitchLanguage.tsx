/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { Tooltip } from 'ascend-components';
import { localStorageService, LocalStorageKey } from 'ascend-local-storage';
import i18n from 'ascend-i18n';
import { sendLanguage } from '@/connection/sendNotification';
import { LangEnIcon, LangZhIcon } from 'ascend-icon';
import { Language } from '@/utils/enum';

const useLanguage = (): [Language, (val: Language) => void] => {
    const [lang, setLang] = useState(localStorageService.getItem(LocalStorageKey.LANGUAGE) ?? Language.EN);
    useEffect(() => {
        i18n.changeLanguage(lang);
        localStorageService.setItem(LocalStorageKey.LANGUAGE, lang);
        sendLanguage();
    }, [lang]);
    return [lang, setLang];
};

// 切换语言
function SwitchLanguage(): JSX.Element {
    const [lang, setLang] = useLanguage();
    const isChinese = lang === Language.ZH;

    const handleToggleLang = (): void => {
        setLang(isChinese ? Language.EN : Language.ZH);
    };

    return <Tooltip placement={'bottom'} title={'中文/English'}>
        <div onClick={handleToggleLang} >
            { isChinese ? <LangZhIcon/> : <LangEnIcon/> }
        </div>
    </Tooltip>;
}

export default SwitchLanguage;
