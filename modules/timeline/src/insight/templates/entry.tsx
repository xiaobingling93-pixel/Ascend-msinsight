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
import React from 'react';
import type { InsightTemplate } from '../../entity/insight';
import { ReactComponent as HomePageIcon } from '../../assets/images/insights/HomePageIcon.svg';

export const entryTemplate: InsightTemplate = {
    id: 'entry',
    name: 'entry',
    source: '<internal>' as const,
    description: '',
    icon: <HomePageIcon className="homePageIcon"/>,
    units: [],
    availableUnits: [],
    isNsMode: true,
};
