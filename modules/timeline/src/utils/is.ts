/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { getOperatingSystem } from './index';

export const isMac = getOperatingSystem() === 'Mac OS';
