/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import DOMIterator from './domiterator';
import RegExpTool from './RegExpTool';

export default class Mark {
    /**
     * @param { HTMLElement | HTMLElement[]} searchContext - 高亮操作的上下文范围
     */
    constructor(searchContext) {
        this.ctx = searchContext;
        this.ie = /MSIE|Trident/.test(window.navigator.userAgent);
    }

    set opt(val) {
        this._opt = Object.assign({}, {
            highlightTagName: '',
            className: '',
            indexName: 'index',
            exclude: [],
            separateWordSearch: true,
            acrossElements: false,
            ignoreGroups: 0,
            each: () => {},
            noMatch: () => {},
            filter: () => true,
            done: () => {},
        }, val);
    }

    get opt() {
        return this._opt;
    }

    get iterator() {
        return new DOMIterator(
            this.ctx,
            this.opt.exclude,
        );
    }

    getSeparatedKeywords(sv) {
        // 使用Set自动去重
        const uniqueKeywords = new Set();

        // 统一处理数组，支持字符串和数组输入
        const processKeyword = (keyword) => {
            const trimmed = keyword.trim();
            if (!trimmed) {
                return;
            }

            if (this.opt.separateWordSearch) {
                trimmed.split(/\s+/) // 使用正则处理多个空格
                    .forEach(word => uniqueKeywords.add(word));
            } else {
                uniqueKeywords.add(trimmed);
            }
        };

        // 处理输入数组
        (Array.isArray(sv) ? sv : [sv]).forEach(processKeyword);

        // 转换为有序数组
        const sortedKeywords = Array.from(uniqueKeywords)
            .sort((a, b) => b.length - a.length || a.localeCompare(b));

        return {
            keywords: sortedKeywords,
            length: sortedKeywords.length,
        };
    }

    getTextNodes(onComplete) {
        let combinedValue = '';
        const textNodes = [];

        const processNode = (node) => {
            const start = combinedValue.length;
            combinedValue += node.textContent;
            textNodes.push({
                start,
                end: combinedValue.length,
                node,
            });
        };

        const filterNode = (node) => {
            return this.matchesExclude(node.parentNode)
                ? NodeFilter.FILTER_REJECT
                : NodeFilter.FILTER_ACCEPT;
        };

        this.iterator.forEachNode(
            NodeFilter.SHOW_TEXT,
            processNode,
            filterNode,
            () => onComplete({
                value: combinedValue,
                nodes: textNodes,
            }),
        );
    }

    matchesExclude(el) {
        return DOMIterator.matches(el, this.opt.exclude.concat([
            // ignores the elements itself, not their childrens (selector *)
            'script', 'style', 'title', 'head', 'html',
        ]));
    }

    wrapRangeInTextNode(node, start, end, index) {
        // 保留原始边界检查
        if (start < 0 || end < start) {
            throw new Error(`Invalid start/end positions: ${start}-${end}`);
        }

        // 保持原始配置获取方式
        const tagName = this.opt.highlightTagName || 'mark';
        const className = this.opt.className;

        // 分步执行分割操作 (关键修复点)
        const middleNode = node.splitText(start); // 第一次分割
        const afterNode = middleNode.splitText(end - start); // 在中间节点上二次分割

        // 保持原始元素创建逻辑
        const wrapper = document.createElement(tagName);
        wrapper.setAttribute('data-markjs', 'true');
        wrapper.setAttribute(`data-${this.opt.indexName}`, index);
        if (className) {
            wrapper.setAttribute('class', className);
        }
        wrapper.textContent = middleNode.textContent;

        // 保持原始DOM替换顺序
        middleNode.parentNode.replaceChild(wrapper, middleNode);

        return afterNode; // 保持原始返回类型
    }

    wrapRangeInMappedTextNode([dict, start, end, filterCb, eachCb, index]) {
        let startOffset = start;
        let endOffset = end;
        for (let i = 0; i < dict.nodes.length; i++) {
            const currentNode = dict.nodes[i];
            const nextNode = dict.nodes[i + 1];

            // 确定是否需要处理当前节点
            if (nextNode === undefined || nextNode.start > startOffset) {
                if (!filterCb(currentNode.node)) {
                    break; // 不满足过滤条件，终止处理
                }

                const relativeStart = startOffset - currentNode.start;
                const relativeEnd = Math.min(endOffset, currentNode.end) - currentNode.start;

                // 包裹文本节点并更新字典值
                this.wrapNodeAndUpdateDict(currentNode, relativeStart, relativeEnd, dict, index);

                // 调整后续节点的偏移量
                this.adjustSubsequentOffsets(dict, i, relativeEnd);

                // 更新结束偏移量
                endOffset -= relativeEnd;

                // 执行回调
                eachCb(currentNode.node.previousSibling, currentNode.start);

                // 判断是否需要继续处理后续节点
                if (endOffset > currentNode.end) {
                    startOffset = currentNode.end; // 更新起始偏移以继续处理
                } else {
                    break; // 当前范围处理完成
                }
            }
        }
    }

    // 包裹指定范围的文本节点并更新字典值
    wrapNodeAndUpdateDict(nodeInfo, relStart, relEnd, dict, index) {
        const preservedStart = nodeInfo.start;

        // 分割原始文本
        const prefix = dict.value.substring(0, preservedStart);
        const suffix = dict.value.substring(preservedStart + relEnd);

        // 包裹文本节点
        nodeInfo.node = this.wrapRangeInTextNode(nodeInfo.node, relStart, relEnd, index);

        // 更新字典中的完整文本值
        dict.value = prefix + suffix;
    }

    // 调整后续节点的起止偏移量
    adjustSubsequentOffsets(dict, startIndex, adjustLength) {
        for (let j = startIndex; j < dict.nodes.length; j++) {
            const node = dict.nodes[j];

            // 当前节点之后的节点需要调整偏移量
            if (j > startIndex) {
                // 防止出现负偏移量
                if (node.start > 0) {
                    node.start -= adjustLength;
                }
            }

            // 所有后续节点都需要调整结束偏移量
            node.end -= adjustLength;
        }
    }

    wrapMatches(regex, ignoreGroups, filterCb, eachCb, endCb) {
        const matchIdx = ignoreGroups === 0 ? 0 : ignoreGroups + 1;

        const handleValidMatch = (match, currentNode) => {
            // 计算原始位置
            let pos = match.index;
            if (matchIdx !== 0) {
                for (let i = 1; i < matchIdx; i++) {
                    pos += match[i].length;
                }
            }

            // 执行节点包裹操作
            const wrappedNode = this.wrapRangeInTextNode(
                currentNode,
                pos,
                pos + match[matchIdx].length,
            );

            // 触发包裹完成回调
            eachCb(wrappedNode.previousSibling);
            return wrappedNode;
        };

        const processNode = nodeInfo => {
            let currentNode = nodeInfo.node;
            let match;

            while ((match = regex.exec(currentNode.textContent)) !== null) {
                // 处理空匹配项提前终止
                if (match[matchIdx] === '') {
                    break;
                }

                // 执行过滤逻辑
                if (!filterCb(match[matchIdx], currentNode)) {
                    continue;
                }

                // 处理有效匹配并更新当前节点
                currentNode = handleValidMatch(match, currentNode);

                // 重置正则表达式状态
                regex.lastIndex = 0;
            }
        };

        const processTextNodes = dict => {
            dict.nodes.forEach(processNode);
            endCb();
        };

        this.getTextNodes(processTextNodes);
    }

    wrapMatchesAcrossElements(regex, ignoreGroups, filterCb, eachCb, endCb) {
        const matchGroupIndex = this.getMatchGroupIndex(ignoreGroups);

        this.processTextNodes(textNodeDict => {
            const count = this.findAndWrapMatches(regex, textNodeDict, matchGroupIndex, filterCb, eachCb);
            endCb(count);
        });
    }

    // 辅助方法：获取有效匹配组索引
    getMatchGroupIndex(ignoreGroups) {
        return ignoreGroups === 0 ? 0 : ignoreGroups + 1;
    }

    // 辅助方法：处理文本节点并查找匹配
    processTextNodes(callback) {
        this.getTextNodes(textNodeDict => callback(textNodeDict));
    }

    // 辅助方法：核心匹配查找和包装逻辑
    findAndWrapMatches(regex, textNodeDict, matchGroupIndex, filterCb, eachCb) {
        let matchResult;
        let index = 0;
        while ((matchResult = this.getNextMatch(regex, textNodeDict.value)) !== null) {
            const { matchedText, startPos, endPos } = this.parseMatchResult(
                matchResult,
                matchGroupIndex,
            );

            if (!this.isValidMatch(matchedText)) {
                break;
            }

            this.wrapRangeInMappedTextNode(
                [textNodeDict,
                    startPos,
                    endPos,
                    currentNode => filterCb(matchedText, currentNode),
                    (wrappedNode, lastIndexPosition) => {
                        this.updateRegexIndex(regex, lastIndexPosition);
                        eachCb(wrappedNode);
                    },
                    index++],
            );
        }
        return index;
    }

    // 辅助方法：获取下一个正则匹配
    getNextMatch(regex, textValue) {
        return regex.exec(textValue);
    }

    // 辅助方法：解析匹配结果
    parseMatchResult(matchResult, groupIndex) {
        let startOffset = matchResult.index;

        // 计算考虑忽略组后的起始位置
        for (let i = 1; i < groupIndex; i++) {
            startOffset += matchResult[i].length;
        }

        return {
            matchedText: matchResult[groupIndex],
            startPos: startOffset,
            endPos: startOffset + matchResult[groupIndex].length,
        };
    }

    // 辅助方法：验证匹配有效性
    isValidMatch(matchedText) {
        return matchedText !== '';
    }

    // 辅助方法：更新正则表达式索引
    updateRegexIndex(regex, lastIndex) {
        regex.lastIndex = lastIndex;
    }

    unwrapMatches(node) {
        const parent = node.parentNode;
        const documentFragment = document.createDocumentFragment();

        // 将子节点全部移动到文档片段
        while (node.firstChild) {
            documentFragment.appendChild(node.firstChild);
        }
        parent.replaceChild(documentFragment, node);

        // 根据浏览器类型选择标准化方式
        this.getNormalizationStrategy(parent);
    }

    getNormalizationStrategy(parent) {
        this.ie ? this.normalizeTextNode(parent) : parent.normalize();
    }

    mergeAdjacentTextNodes(textNode) {
        let nextSibling;
        while ((nextSibling = textNode.nextSibling)?.nodeType === Node.TEXT_NODE) {
            textNode.nodeValue += nextSibling.nodeValue;
            textNode.parentNode.removeChild(nextSibling);
        }
    }

    normalizeTextNode(node) {
        if (!node) {
            return;
        }

        if (node.nodeType === Node.TEXT_NODE) {
            this.mergeAdjacentTextNodes(node);
        } else {
            this.normalizeTextNode(node.firstChild);
        }

        this.normalizeTextNode(node.nextSibling);
    }

    /* markRegExp 函数进行现代化重构后的版本 */
    markRegExp(regexp, opt) {
        // 合并配置并验证参数
        this.opt = { ...this.opt, ...opt };
        this.#validateRegex(regexp);

        // 状态跟踪
        let matchCount = 0;

        // 方法选择逻辑 (Strategy 模式)
        const processMethod = this.opt.acrossElements
            ? this.wrapMatchesAcrossElements
            : this.wrapMatches;

        // 回调函数组合
        const callbacks = {
            each: element => {
                matchCount++;
                this.opt.each(element);
            },
            done: () => {
                if (matchCount === 0) {
                    this.opt.noMatch(regexp);
                }
                this.opt.done(matchCount);
            },
            filter: (node, matchText) => {
                return this.opt.filter(
                    node,
                    matchText,
                    matchCount + 1, // 保持原逻辑，在计数前预加
                );
            },
        };

        // 执行核心处理流程
        processMethod.call(
            this,
            regexp,
            this.opt.ignoreGroups,
            callbacks.filter,
            callbacks.each,
            callbacks.done,
        );

        // 返回实例支持链式调用
        return this;
    }

    // 新增私有方法进行正则验证
    #validateRegex(regex) {
        if (!(regex instanceof RegExp)) {
            throw new TypeError('Expected a RegExp object');
        }
    }

    mark(sv, config) {
        this.opt = config;
        const { keywords, length: keywordsLen } = this.getSeparatedKeywords(Array.isArray(sv) ? sv : [sv]);
        const searchFunctionName = this.opt.acrossElements ? 'wrapMatchesAcrossElements' : 'wrapMatches';
        const isCaseInsensitive = this.opt.caseSensitive;
        let totalMatches = 0;

        const handleKeyword = kw => {
            const regStr = new RegExpTool(config).createRegExp(kw);
            const regex = new RegExp(regStr, `gm${isCaseInsensitive ? '' : 'i'}`);
            let matcheCount = 0;
            this[searchFunctionName](regex, 1, (term, node) => this.opt.filter(node, kw, totalMatches, matcheCount),
                ele => {
                    matcheCount++;
                    totalMatches++;
                    this.opt.each(ele);
                }, (strMatchesCount) => {
                    if (matcheCount === 0) {
                        this.opt.noMatch(kw);
                    }
                    if (keywords[keywordsLen - 1] === kw) {
                        this.opt.done(strMatchesCount);
                    } else {
                        handleKeyword(keywords[keywords.indexOf(kw) + 1]);
                    }
                });
        };
        if (keywordsLen === 0) {
            this.opt.done(totalMatches);
        } else {
            handleKeyword(keywords[0]);
        }
    }

    /**
     * 移除上下文范围内所有标记元素及其HTML内容，并在操作完成后规范化父元素
     */
    unmark(config = {}) {
        this.opt = config;
        const { highlightTagName, className } = this.opt;
        const selector = `${highlightTagName ?? '*'}[data-markjs]${className !== '' && className !== undefined ? `.${className}` : ''}`;
        this.iterator.forEachNode(NodeFilter.SHOW_ELEMENT, item => {
            this.unwrapMatches(item);
        },
        node => {
            const matchesSelector = DOMIterator.matches(node, selector);
            const matchesExclude = this.matchesExclude(node);
            return !matchesSelector || matchesExclude ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT;
        }, this.opt.done);
    }
}
