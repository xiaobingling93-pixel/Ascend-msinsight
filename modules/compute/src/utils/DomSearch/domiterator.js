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
export default class DOMIterator {
    #ctx;
    #exclude;

    constructor(ctx, exclude = []) {
        this.#ctx = ctx;
        this.#exclude = exclude;
    }

    static #getMatcher(element) {
        return (
            element.matches ??
            element.matchesSelector ??
            element.msMatchesSelector ??
            element.mozMatchesSelector ??
            element.oMatchesSelector ??
            element.webkitMatchesSelector
        );
    }

    static matches(element, selector) {
        const selectors = [].concat(selector);
        const matcher = this.#getMatcher(element);

        if (!matcher) {
            return false;
        }

        return selectors.some(sel =>
            matcher.call(element, sel),
        );
    }

    #normalizeContext() {
        if (this.#ctx === undefined) {
            return [];
        }

        if (this.#ctx instanceof NodeList) {
            return [...this.#ctx];
        }

        if (Array.isArray(this.#ctx)) {
            return this.#ctx;
        }

        if (typeof this.#ctx === 'string') {
            return [...document.querySelectorAll(this.#ctx)];
        }

        return [this.#ctx];
    }

    getContexts() {
        const contexts = this.#normalizeContext();
        const uniqueContexts = [];

        contexts.forEach(current => {
            const isExisting = uniqueContexts.some(existing =>
                existing.contains?.(current),
            );

            if (!isExisting && !uniqueContexts.includes(current)) {
                uniqueContexts.push(current);
            }
        });

        return uniqueContexts;
    }

    createIterator(context, whatToShow, filter) {
        return document.createNodeIterator(
            context,
            whatToShow,
            filter,
            false,
        );
    }

    #getIteratorState(iterator) {
        const prevNode = iterator.previousNode();
        const node = prevNode === null
            ? iterator.nextNode()
            : Boolean(iterator.nextNode()) && iterator.nextNode();

        return { prevNode, node };
    }

    #processNodes(whatToShow, context, eachCallback, filterCallback, doneCallback) {
        const iterator = this.createIterator(context, whatToShow, filterCallback);
        const nodes = [];

        let state;
        while (Boolean((state = this.#getIteratorState(iterator)).node) === true) {
            nodes.push(state.node);
        }

        nodes.forEach(eachCallback);
        doneCallback();
    }

    forEachNode(whatToShow, each, filter, done = () => {}) {
        const contexts = this.getContexts();
        let remaining = contexts.length;

        if (!remaining) {
            done();
            return;
        }

        contexts.forEach(context => {
            this.#processNodes(
                whatToShow,
                context,
                each,
                filter,
                () => --remaining <= 0 && done(),
            );
        });
    }
}
