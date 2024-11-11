/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

export enum ProjectError {
    NO_ERRORS = 0,
    PROJECT_NAME_CONFLICT = 1,
    IS_UNSAFE_PATH = 2,
    EXISTING_LARGE_FILES = 3,
    TRANSFER_PROJECT = 4,
    IMPORTED = 5,
    FILE_NOT_EXIST = 6,
    OTHER = -1,
}

export enum ProjectAction {
    TRANSFER_PROJECT = 0,
    ADD_FILE = 1,
};
