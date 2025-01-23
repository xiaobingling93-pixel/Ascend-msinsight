/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState } from 'react';
import { Modal } from 'antd';
import VersionInfo from '@/version_info.json';
import { useTranslation } from 'react-i18next';
import { HelpIcon } from 'ascend-icon';

// 软件首发年分
const LAUNCH_YEAR = '2024';
const { version, modifyTime } = VersionInfo;
const modifyYear = modifyTime.split('/')[0];
const copyrightYear = modifyYear === LAUNCH_YEAR ? modifyYear : `${LAUNCH_YEAR}-${modifyYear}`;

// 版本信息
function Version(): JSX.Element {
    const { t } = useTranslation('framework');
    const [isModalOpen, setIsModalOpen] = useState(false);
    const showVersion = (): void => {
        setIsModalOpen(true);
    };
    const closeVersion = (): void => {
        setIsModalOpen(false);
    };

    return <>
        <HelpIcon onClick={showVersion}/>
        <Modal title={`${t('About')} MindStudio Insight`} open={isModalOpen} onCancel={closeVersion} destroyOnClose={true} footer={null}>
            <ul className="help-ul">
                <li>{t('buildVersion', { version, modifyTime })}</li>
                <li>{t('copyRight', { copyrightYear })}</li>
            </ul>
        </Modal>
    </>;
}

export default Version;
