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
import React, { useEffect } from 'react';
import './highlight.scss';
import './CodeViewer.css';
import { addClass, removeClass, isViewable } from '../../Common';
import { highlightAllWithNumber } from './highlightLineNumbers';

// 修改样式，行数的宽度
export function changeIndexWidth(width: number): void {
    document.querySelectorAll('#CodeTable ul>li .index').forEach(indexElement => {
        (indexElement as HTMLElement).style.width = `${width}px`;
    });
}

const CodeViewer = ({ code, selectedLine, hoverline, style, className = '', handleLineClick, ...restProps }: {
    code: string;
    style?: React.CSSProperties;
    className?: string;
    selectedLine?: number;
    hoverline?: number;
    handleLineClick?: (line: number) => void;
}): JSX.Element => {
    useEffect(() => {
        highlightAllWithNumber({ showLine: true });
    }, [code]);

    useEffect(() => {
        document.querySelectorAll('code li').forEach((node, index) => {
            if (selectedLine === index + 1) {
                addClass(node, 'selected');
                if (!isViewable(node,
                    { fixedtop: document.getElementById('CodeTable')?.getBoundingClientRect().top ?? 120 })) {
                    node.scrollIntoView();
                }
            } else {
                removeClass(node, 'selected');
            }
        });
    }, [selectedLine]);
    const handleClick = (event: React.MouseEvent<HTMLElement>): void => {
        event.stopPropagation();
        let dom: any = event.target as HTMLElement;
        while (dom !== undefined && dom !== null && dom.nodeName !== 'LI') {
            dom = dom.parentElement;
        }
        if (dom === null) {
            return;
        }
        document.querySelectorAll('code li.selected').forEach(node => removeClass(node, 'selected'));
        addClass(dom, 'selected');
        let i = 1;
        while ((dom = dom.previousSibling) != null) {
            i++;
        }
        if (handleLineClick !== undefined) {
            handleLineClick(i);
        }
    };
    return (
        <div style={ { ...style ?? {} }} className={className} {...restProps} >
            {
                (code !== '' && code !== null && code !== undefined) && (
                    <pre>
                        <code
                            className={'language-cpp'}
                            onClick={handleClick}>
                            {code}
                        </code>
                    </pre>
                )
            }
        </div>
    );
};

export default CodeViewer;
