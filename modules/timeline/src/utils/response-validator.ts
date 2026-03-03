/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

/**
 * 响应校验器：通过版本号机制确保仅处理最新请求的响应
 * 适用场景：useEffect 监听值快速变化时避免旧响应覆盖状态
 */
export class ResponseValidator {
    private currentVersion = 0;

    /**
     * 标记状态更新（每次 selectedRow 变化时调用）
     * @returns 新的版本号（用于本次请求的校验标识）
     */
    markUpdate(): number {
        this.currentVersion += 1;
        return this.currentVersion;
    }

    /**
     * 校验响应是否属于最新状态
     * @param responseVersion 响应发起时记录的版本号
     * @returns true 表示响应有效（应更新状态），false 表示过时（应丢弃）
     */
    isValid(responseVersion: number): boolean {
        return responseVersion === this.currentVersion;
    }

    /**
     * （可选）重置校验器（如组件卸载前清理）
     */
    reset(): void {
        this.currentVersion = 0;
    }
}
