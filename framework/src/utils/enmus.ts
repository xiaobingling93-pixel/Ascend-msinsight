/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

export enum ProjectErrorType {
    NO_ERRORS = 0,
    PROJECT_NAME_CONFLICT = 1,
    IS_UNSAFE_PATH = 2,
    EXISTING_LARGE_FILES = 3,
    TRANSFER_PROJECT = 4,
    EXCEEDS_MXIMUN_LENGTH = 5,
    OTHER = -1
}