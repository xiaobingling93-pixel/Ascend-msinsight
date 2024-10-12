/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import { Session } from '@/entity/session';
import { Menu } from 'antd';
import type { MenuProps } from 'antd';
import { type ModuleConfig, modulesConfig } from '../../moduleConfig';
import styled from '@emotion/styled';

const Container = styled.div` 
    width: 100%;
    height: 100%;
    display: flex;
    flex-direction: column;
    .ant-menu{
        background: ${(props): string => props.theme.bgColorLight};
    }
    .ant-menu-item{
        color: ${(props): string => props.theme.textColorPrimary};
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

// Scene：数据场景,默认、集群、算子调优、Jupter
type Scene = 'Default' | 'Cluster' | 'Compute' | 'Jupyter';

function getScene(session: Session): Scene {
    let scene: Scene;
    if (session.isBinary) {
        scene = 'Compute';
    } else if (session.isCluster) {
        scene = 'Cluster';
    } else if (session.isIpynb) {
        scene = 'Jupyter';
    } else {
        scene = 'Default';
    }
    return scene;
}

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
    const [scene, setScene] = useState<Scene>('Default');
    const [activeModule, setActiveModule] = useState('Timeline');

    const availableModules = useMemo(() => modulesConfig.filter(config => isAvailable(config, scene)), [scene]);
    const items: MenuProps['items'] = useMemo(() => availableModules.map(config => ({ label: config.name, key: config.name })), [availableModules]);
    const onClick: MenuProps['onClick'] = e => {
        setActiveModule(e.key);
    };

    useEffect(() => {
        if (session.isBinary === null && session.isCluster === null) {
            return;
        }
        setScene(getScene(session));
    }, [session.isBinary, session.isCluster, session.isIpynb]);
    useEffect(() => {
        const newActiveModule = getActive(session, scene, activeModule, availableModules);
        setActiveModule(newActiveModule);
    }, [scene]);
    return <Container>
        <Menu onClick={onClick} selectedKeys={[activeModule]} mode="horizontal" items={items} />
        <div className="tab-body">
            {!session.isVscode
                ? availableModules.map(moduleConfig => (
                    <iframe
                        {...moduleConfig.attributes}
                        key={`frame-${moduleConfig.name}`}
                        id={moduleConfig.name}
                        style={{ display: activeModule === moduleConfig.name ? 'block' : 'none' }}
                    />
                ))
                : <></>
            }
        </div>
    </Container>;
});

export default Index;
