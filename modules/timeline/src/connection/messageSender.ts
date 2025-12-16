/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

export interface IMessageSender {
    sendMessage: (ceq: any) => void;
    selectFolder: () => Promise<string>;
    selectFile: () => Promise<string>;
}

export const removeAndAddEventListener = (resolve: (value: (string | PromiseLike<string>)) => void): void => {
    function onMessage(event: MessageEvent): void {
        const message = event.data;
        switch (message.command) {
            case 'ascend.folderSelected':
                resolve(message.path);
                break;
            case 'ascend.folderSelectionCanceled':
                resolve('');
                break;
            default:
        }
    }
    window.removeEventListener('message', onMessage);
    window.addEventListener('message', onMessage);
};
