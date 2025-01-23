/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import type { Scene, Session } from '@/entity/session';
import type { MenuProps } from 'antd';
import { Menu } from 'antd';
import { type ModuleConfig, modulesConfig } from '../../moduleConfig';
import styled from '@emotion/styled';
import { SessionAction } from '@/utils/enum';
import { useTranslation } from 'react-i18next';

const Container = styled.div`
    width: 100%;
    height: 100%;
    display: flex;
    flex-direction: column;
    .ant-menu{
        height: 36px;
        background: ${(props): string => props.theme.bgColorLight};
    }
    .ant-menu-item{
        height: 32px;
        line-height: 32px;
        font-weight: bold;
        color: ${(props): string => props.theme.textColorPrimary};
    }
    .ant-menu-item-selected{
        color: ${(props): string => props.theme.primaryColor};
        &:after {
            border-bottom: 1px solid ${(props): string => props.theme.primaryColor};
        }
    }
    .tab-body {
        flex-grow: 1;
        height: calc(100% - 40px);
        background: ${(props): string => props.theme.bgColorDark};
    }
    iframe {
        width: 100%;
        height: 100%;
        border: 0;
        color-scheme: light;
        background: transparent;
    }
`;

function getActive(session: Session, scene: Scene, activeModule: string, availableModules: ModuleConfig[]): string {
    const moduleNameList = availableModules.map(config => config.name);
    if (session.isIpynb && activeModule !== 'Jupyter') {
        return 'Jupyter';
    } else if (!moduleNameList.includes(activeModule)) {
        return moduleNameList[0];
    } else {
        return activeModule;
    }
}
function isAvailable(moduleConfig: ModuleConfig, scene: Scene): boolean {
    return Boolean(moduleConfig[`is${scene}`]);
}

const Index = observer(({ session }: {session: Session}) => {
    const { t } = useTranslation('framework', { keyPrefix: 'tabs' });
    const [scene, setScene] = useState<Scene>('Default');
    const [activeModule, setActiveModule] = useState('Timeline');

    const availableModules = useMemo(() => modulesConfig.filter(config => isAvailable(config, scene)), [scene]);
    const items: MenuProps['items'] = useMemo(() => availableModules.map(config => ({ label: t(config.name), key: config.name }))
        , [availableModules, t]);
    const onClick: MenuProps['onClick'] = e => {
        setActiveModule(e.key);
    };

    useEffect(() => {
        if (session.isBinary === null && session.isCluster === null) {
            return;
        }
        setScene(session.scene);
    }, [session.isBinary, session.isCluster, session.isIpynb]);
    useEffect(() => {
        const newActiveModule = getActive(session, scene, activeModule, availableModules);
        setActiveModule(newActiveModule);
    }, [scene]);
    useEffect(() => {
        const { type, value } = session.actionListener;
        const allModuleName = modulesConfig.map(module => module.name);
        if (type === SessionAction.SWITCH_ACTIVE_MODULE && allModuleName.includes(value)) {
            setActiveModule(value);
        }
    }, [session.actionListener]);
    return <Container>
        <Menu onClick={onClick} selectedKeys={[activeModule]} mode="horizontal" items={items} />
        <div className="tab-body">{availableModules.map(moduleConfig => (
            <iframe
                {...moduleConfig.attributes}
                key={`frame-${moduleConfig.name}`}
                id={moduleConfig.name}
                style={{ display: activeModule === moduleConfig.name ? 'block' : 'none' }}
            />
        ))}
        </div>
    </Container>;
});

export default Index;
