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
import hljs from 'highlight.js';
export const BREAK_LINE_REGEXP = /\r\n|\r|\n/;
const BREAK = 'break';
export const highlightAllWithNumber = function(set = { showLine: false }): void {
    const codes = document.querySelectorAll('pre code');
    codes.forEach((el: Element) => {
        (el as HTMLElement).dataset.highlighted = '';
        hljs.highlightElement(el as HTMLElement);
    });
    const e = document.querySelectorAll('code');
    const elen = e.length;
    for (let i = 0; i < elen; i++) {
        const linebreakStr = e[i].innerText.match(BREAK_LINE_REGEXP)?.[0] ?? '';
        const ul = splitNode(e[i]);
        if (set.showLine) {
            for (let j = 0; j < ul.children.length; j++) {
                const li = ul.children[j];
                const linebreak = document.createTextNode(linebreakStr);
                li.appendChild(linebreak);
            }
        }
        e[i].innerHTML = '';
        e[i].append(ul);
    }
};
function splitNode(node: Node): HTMLElement {
    const { childNodes } = node;
    const grouplist = [];
    let group = [];
    for (let i = 0; i < childNodes.length; i++) {
        const lines = singleNode2Line([childNodes[i]]);
        for (let j = 0; j < lines.length; j++) {
            if (lines[j] !== BREAK) {
                group.push(lines[j]);
            } else if (lines[j] === BREAK && group.length > 0) {
                grouplist.push(group);
                group = [];
            } else { /* empty */ }
        }
    }

    if (group.length > 0) {
        grouplist.push(group);
    }

    const ul = document.createElement('ul');
    for (let i = 0; i < grouplist.length; i++) {
        const li = document.createElement('li');
        const groupItem = grouplist[i];
        groupItem.forEach((nodePath: any) => {
            const dom = createNode(nodePath);
            li.append(dom);
        });
        ul.append(li);
    }
    return ul;
}
function singleNode2Line(nodePath: Node[]): Node[][] | string {
    if (nodePath.length === 0) {
        return [];
    }
    const node = nodePath[nodePath.length - 1].cloneNode(true);
    let list: any[] = [];
    let lnlist = [];
    if (node.nodeName === '#text') {
        if (node.nodeValue !== null) {
            list = node.nodeValue.split(BREAK_LINE_REGEXP).map((item: string) =>
                [...nodePath.slice(0, -1), document.createTextNode(item)]);
            lnlist = [list[0]];
            for (let i = 1; i < list.length; i++) {
                lnlist.push(BREAK, list[i]);
            }
        }
    } else if (node.childNodes.length > 0) {
        const span = node.cloneNode();
        node.childNodes.forEach((item: Node) => {
            list.push(...singleNode2Line([...nodePath.slice(0, -1), span, item]));
        });
        lnlist = list;
    } else { /* empty */ }

    return lnlist;
}
function createNode(nodePath: Node[]): Node {
    let dom;
    let p;
    for (let i = 0; i < nodePath.length; i++) {
        const node = nodePath[i].cloneNode();
        if (i === 0) {
            dom = node;
        } else {
            p?.append(node);
        }
        p = (node as HTMLElement);
    }
    return dom ?? document.createTextNode('');
}
