/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { ProjectAction, ProjectError } from '@/utils/enum';
import i18n from 'ascend-i18n';
import { message } from 'antd';
import { handleProjectAction } from '@/utils/Project';
import { checkPathValid } from '@/utils/Resource';

export function firstLetterUpper(value: string): string {
    const word = String(value);
    return word.charAt(0).toUpperCase() + word.slice(1);
}

// 注册文件拖拽事件
export function registerDragAndDropFile(): void {
    Object.defineProperty(window, 'handleDrop', {
        value: async (path: string) => {
            const project = {
                projectName: path,
                dataPath: [path],
            };
            // 校验
            const validRes: ProjectError = await checkPathValid(project);
            // 校验通过
            if ([ProjectError.NO_ERRORS, ProjectError.IMPORTED].includes(validRes)) {
                const action = validRes === ProjectError.NO_ERRORS ? ProjectAction.ADD_FILE : ProjectAction.SWITCH_PROJECT;
                handleProjectAction({ action, project, isConflict: false });
            } else if (validRes === ProjectError.PROJECT_NAME_CONFLICT) {
                message.info({ content: `${i18n.t('framework:FileConflict')}:${i18n.t('framework:FileConflictContent')}` });
            } else {
                message.info({ content: `Error:${ProjectError[validRes]}` });
            }
        },
        writable: true,
        enumerable: false,
        configurable: true,
    });
}
