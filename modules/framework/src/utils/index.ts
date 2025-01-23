/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { store } from '@/store/rootStore';
import { modulesConfig } from '@/moduleConfig';

export function getModuleIndex(name = ''): number {
    const session = store.sessionStore.activeSession;
    const availableModules = modulesConfig.filter(module => module[`is${session.scene}`]);
    return availableModules.findIndex(item => item.name.toLowerCase() === name.toLowerCase());
}

export function firstLetterUpper(value: string): string {
    const word = String(value);
    return word.charAt(0).toUpperCase() + word.slice(1);
}
