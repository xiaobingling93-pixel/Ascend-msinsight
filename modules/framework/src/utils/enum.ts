/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

export enum ProjectError {
    NO_ERRORS = 0,
    PROJECT_NAME_CONFLICT = 1,
    IS_UNSAFE_PATH = 2,
    EXISTING_LARGE_FILES = 3,
    // 文件/文件夹作已导入
    IMPORTED = 4,
    FILE_NOT_EXIST = 6,
    OTHER = -1,
}

export enum ProjectAction {
    // 切换项目
    SWITCH_PROJECT = 0,
    // 导入新文件
    ADD_FILE = 1,
};

export enum SessionAction {
    ADD_DATA_UNDER_PROJECT = 'add data under project',
    SWITCH_ACTIVE_MODULE = 'switch active module',
    NO_ACTION = 'no action',
}

export enum ThemeName {
    DARK = 'dark',
    LIGHT = 'light',
}

export enum Language {
    ZH = 'zhCN',
    EN = 'enUS',
}
