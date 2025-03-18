/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { runInAction } from 'mobx';
import { store } from '@/store';
import type { Session } from '@/entity/session';
import connector from '@/connection';
import { firstLetterUpper, getModuleIndex } from '@/utils';
import {
    sendMap,
    sendLanguage,
    sendModuleReset,
    sendStatus,
    sendTheme,
} from './sendNotification';
import { NotificationMessage } from './notification';
import { SessionAction } from '@/utils/enum';
import { customConsole as console } from 'ascend-utils';

export const updateSessionHandler = (e: NotificationMessage): void => {
    const session = store.sessionStore.activeSession;
    const receiver = e.data.body;
    if (receiver === undefined) {
        console.warn('data.body is undefined, please check your params');
    }
    const updateState = updateSession(receiver);

    setTimeout(() => {
        const isSend =
            (updateState.parseCompleted !== undefined ||
                updateState.clusterCompleted !== undefined ||
                updateState.unitcount !== undefined ||
                updateState.isBinary === true ||
                updateState.durationFileCompleted === true ||
                updateState.isIpynb === true ||
                updateState.ipynbUrl !== '') &&
            receiver.broadcast !== false;
        if (isSend) {
            connector.send({
                event: 'updateSession',
                body: {
                    parseCompleted: session.parseCompleted,
                    clusterCompleted: session.clusterCompleted,
                    unitcount: session.unitcount,
                    instrVersion: session.instrVersion,
                    ...updateState,
                },
            });
        }
    });
};

export const updateSession = (receiver: Record<string, any>): Record<string, any> => {
    if (receiver === undefined || receiver == null) {
        return {};
    }
    const session = store.sessionStore.activeSession;
    const receiverPropKeys = Object.keys(receiver);
    const sessionPropKeys = Object.keys(session);
    const updateState: Record<string, any> = {};
    for (const key of receiverPropKeys) {
        // 1.receiver的字段key在session中存在
        // 2.receiver[key]的类型（例如string、boolean)与session[key]也相同，或者session[key]当前为null
        const isSameType = Object.prototype.toString.call(receiver[key]) === Object.prototype.toString.call(session[key as keyof Session]);
        const valid = sessionPropKeys.includes(key) && (isSameType || session[key as keyof Session] === null);
        if (valid) {
            Object.assign(updateState, { [key]: receiver[key] });
        } else {
            console.warn(`you just send a invalid data: {${key}: ${receiver[key]}} to update session, please check it`);
        }
    }
    runInAction(() => {
        Object.entries(updateState).forEach(([key, value]) => {
            (session as any)[key] = value;
        });
    });
    return updateState;
};

export const getParseStatusHandler = (e: NotificationMessage): void => {
    const session = store.sessionStore.activeSession;
    if (session.isIpynb) {
        sendModuleReset();
    }
    // 请求特定数据
    const receiver = e.data.body;
    const requestList = receiver?.requests as string[];
    if (requestList?.length > 0) {
        requestList.forEach(key => {
            sendMap[key]?.(e.data.from);
        });
        return;
    }
    const requestKey = receiver?.request as string;
    if (requestKey !== undefined && requestKey !== null && (session as any)[requestKey] !== undefined) {
        connector.send({
            event: 'updateSession',
            body: { [requestKey]: (session as any)[requestKey] },
            to: getModuleIndex(receiver?.from),
        });
        return;
    }
    // 请求通用数据
    sendStatus();
    // 主题
    sendTheme();
};

export const getThemeHandler = (): void => {
    sendTheme();
};

export const deleteRankHandler = (e: NotificationMessage): void => {
    const receiver = e.data.body;
    if (receiver === null || receiver === undefined) {
        console.warn('data.body is undefined, please check your params');
        return;
    }
    connector.send({ event: 'deleteRank', body: receiver });
    const deleteIds: string[] = receiver.rankId as string[];
    if (deleteIds.length > 0) {
        const session = store.sessionStore.activeSession;
        const memoryRankIds = session.memoryRankIds.filter((item: string) => !deleteIds?.includes(item));
        updateSession({ memoryRankIds });
    }
};

export const switchModuleHandler = (e: NotificationMessage): void => {
    const session = store.sessionStore.activeSession;
    const body = e.data.body;
    if (body?.switchTo === null || body?.switchTo === undefined) {
        return;
    }
    const moduleName = firstLetterUpper(body.switchTo);
    const moduleIndex = getModuleIndex(moduleName);
    if (moduleIndex > -1) {
        // 切换到此模块
        runInAction(() => {
            session.actionListener = { type: SessionAction.SWITCH_ACTIVE_MODULE, value: moduleName };
        });
        if (body.toModuleEvent !== undefined) {
            // 向目标模块发送消息
            connector.send({ event: body.toModuleEvent, to: body.broadcast ? undefined : moduleIndex, body: body.params });
        }
    }
};

export const getLanguageHandler = (e: NotificationMessage): void => {
    sendLanguage(e.data.from);
};
