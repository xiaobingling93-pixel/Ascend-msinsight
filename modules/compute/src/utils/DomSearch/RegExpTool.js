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
export default class RegExpTool {
    constructor(opt = {}) {
        /* 初始化配置：合并用户配置和默认配置 */
        this.opt = Object.assign(
            {
                accuracy: 'partially',
                keepReg: false,
            },
            opt,
        );
    }

    /* 主入口：按固定流程生成正则表达式 */
    createRegExp(val) {
        let str = val;
        str = this._escapeStr(str); // 转义特殊字符
        str = this._createMergedBlanksRegExp(str); // 合并空白符
        str = this._createAccuracyRegExp(str); // 精度控制处理
        return str;
    }

    /* 转义正则元字符：-[]/{}()*+?.\\^$| */
    _escapeStr(str) {
        if (this.opt.keepReg) {
            return str;
        }
        return str.replace(/[-[\]/{}()*+?.\\^$|]/g, '\\$&');
    }

    /* 合并连续空白符：[\\s]+ */
    _createMergedBlanksRegExp(str) {
        return str.replace(/[\s]+/gim, '[\\s]+');
    }

    /* 精度控制：添加边界匹配约束 */
    _createAccuracyRegExp(str) {
        const boundaryChars = '!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~¡¿'.split('');
        const { value: accuracy, limiters = boundaryChars } = typeof this.opt.accuracy === 'object'
            ? this.opt.accuracy
            : { value: this.opt.accuracy };

        switch (accuracy) {
            case 'exactly': {
                const boundary = limiters.map(l => this._escapeStr(l)).join('|');
                return `(^|\\s${boundary ? `|${boundary}` : ''})(${str})(?=$|\\s${boundary ? `|${boundary}` : ''})`;
            }
            default: // 'partially'
                return `()(${str})`;
        }
    }
}
