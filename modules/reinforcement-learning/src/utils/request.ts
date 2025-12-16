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

import { connector } from '@/connection';
import type { Request, ResponseBody } from '@/types/request';
import { message } from 'antd';

export const request: Request = async ({ command, params, module }) => {
    const moduleName = module ?? command.split('/')?.[0]?.toLowerCase() ?? '';

    try {
        const res = await connector.fetch({
            args: {
                command,
                params,
            },
            module: moduleName,
        });

        const result = res as ResponseBody;

        return result.body;
    } catch (error: any) {
        message.error(error.message || '请求异常');
        throw error;
    }
};
