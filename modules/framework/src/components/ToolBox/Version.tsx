/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useState } from 'react';
import { Modal } from 'antd';
import VersionInfo from '@/version_info.json';
import { useTranslation } from 'react-i18next';
import { HelpIcon } from 'ascend-icon';
import styled from '@emotion/styled';

// 软件首发年分
const LAUNCH_YEAR = '2024';
const { version, modifyTime } = VersionInfo;
const modifyYear = modifyTime.split('/')[0];
const copyrightYear = modifyYear === LAUNCH_YEAR ? modifyYear : `${LAUNCH_YEAR}-${modifyYear}`;

const VersionContainer = styled.ul`
    padding: 0;
    list-style: none;
    li {
        margin-bottom: 10px;
        line-height: 1.5em;
        color: ${(p): string => p.theme.textColor};
    }
`;

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
        <HelpIcon onClick={showVersion} data-testid="help-icon"/>
        <Modal title={`${t('About')} MindStudio Insight`} open={isModalOpen} onCancel={closeVersion} destroyOnClose={true} footer={null}>
            <VersionContainer className="help-ul">
                <li>{t('buildVersion', { version, modifyTime })}</li>
                <li>{t('copyRight', { copyrightYear })}</li>
            </VersionContainer>
        </Modal>
    </>;
}

export default Version;
