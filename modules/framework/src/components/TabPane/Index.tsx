/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React, { useEffect, useMemo, useState } from 'react';
import { observer } from 'mobx-react';
import type { Scene, Session } from '@/entity/session';
import { type MenuProps, message, Menu } from 'antd';
import { safeJSONParse } from 'ascend-utils';

import { type ModuleConfig, modulesConfig } from '../../moduleConfig';
import styled from '@emotion/styled';
import { SessionAction } from '@/utils/enum';
import { useTranslation } from 'react-i18next';
import { getModuleConfig } from '@/utils/Request';
import { updateSession } from '@/connection/notificationHandler';

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

export function updateDataScene(data: Record<string, any>): void {
    const scenceInfo = {
        isCluster: data.isCluster ?? false,
        isReset: data.reset ?? false,
        isIpynb: data.isIpynb ?? false,
        isBinary: data.isBinary ?? false,
        hasCachelineRecords: data.hasCachelineRecords ?? false,
        isOnlyTraceJson: data.isOnlyTraceJson ?? false,
        instrVersion: data.instrVersion ?? -1,
    };
    updateSession(scenceInfo);
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
function isAvailable(moduleConfig: ModuleConfig, scene: Scene, dataCompose: Record<string, boolean>): boolean {
    // 根据包含某种数据，控制页签显隐
    const composeList = Object.keys(dataCompose);
    for (const name of composeList) {
        if (dataCompose[name] && Boolean((moduleConfig as any)[name])) {
            return true;
        }
    }
    return Boolean(moduleConfig[`is${scene}`]);
}

const Index = observer(({ session }: {session: Session}) => {
    const { t } = useTranslation('framework', { keyPrefix: 'tabs' });
    const [scene, setScene] = useState<Scene>('Default');
    const [dataCompose, setDataCompose] = useState<Record<string, boolean>>({});
    const [activeModule, setActiveModule] = useState('Timeline');
    const [mergedModulesConfig, setMergedModulesConfig] = useState(modulesConfig);

    const availableModules = useMemo(() => mergedModulesConfig.filter(config => isAvailable(config, scene, dataCompose))
        , [scene, dataCompose, mergedModulesConfig]);
    const items: MenuProps['items'] = useMemo(() => availableModules.map(config => ({ label: t(config.name, { defaultValue: config.name }), key: config.name }))
        , [availableModules, t]);
    const onClick: MenuProps['onClick'] = e => {
        setActiveModule(e.key);
    };

    useEffect(() => {
        const fetchModuleConfigData = async (): Promise<void> => {
            try {
                const { configs }: any = await getModuleConfig();
                const pluginModulesConfig: ModuleConfig[] = [];
                (configs as string[]).forEach(item => {
                    const config: ModuleConfig = { name: '', requestName: '', attributes: {} };
                    Object.assign(config, safeJSONParse(item));
                    if ((config.attributes.src != null) && config.attributes.src !== '' && !config.attributes.src.startsWith('http')) {
                        pluginModulesConfig.push(config);
                    }
                });

                setMergedModulesConfig(prevConfig => ([
                    ...prevConfig,
                    ...pluginModulesConfig,
                ]));
            } catch (error) {
                message.error('Plugin load error');
            }
        };

        if (session.defaultConnected) {
            fetchModuleConfigData();
        }
    }, [session.defaultConnected]);

    useEffect(() => {
        // 删除工程的场景：不改变页签
        if (session.isBinary === null && session.isCluster === null) {
            return;
        }
        setScene(session.scene);
        setDataCompose({ hasCachelineRecords: session.hasCachelineRecords });
    }, [session.isBinary, session.isCluster, session.isIpynb, session.hasCachelineRecords, session.isOnlyTraceJson]);
    useEffect(() => {
        const newActiveModule = getActive(session, scene, activeModule, availableModules);
        setActiveModule(newActiveModule);
    }, [scene, availableModules]);
    useEffect(() => {
        const { type, value } = session.actionListener;
        const allModuleName = mergedModulesConfig.map(module => module.name);
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
                name={moduleConfig.name}
                style={{ display: activeModule === moduleConfig.name ? 'block' : 'none' }}
            />
        ))}
        </div>
    </Container>;
});

export default Index;
