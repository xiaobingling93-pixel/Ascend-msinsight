/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

// eslint-disable-next-line max-lines-per-function
const CodeViewer = ({ code, selectedline, hoverline, style, className = '', handleLineClick, ...restProps }: {
    code: string;
    style?: React.CSSProperties;
    className?: string;
    selectedline?: number;
    hoverline?: number;
    handleLineClick?: (line: number) => void;
}): JSX.Element => {
    useEffect(() => {
        highlightAllWithNumber({ showLine: true });
    }, [code]);

    useEffect(() => {
        document.querySelectorAll('code li').forEach((node, index) => {
            if (selectedline === index + 1) {
                addClass(node, 'selected');
                if (!isViewable(node,
                    { fixedtop: document.getElementById('CodeTable')?.getBoundingClientRect().top ?? 120 })) {
                    node.scrollIntoView();
                }
            } else {
                removeClass(node, 'selected');
            }
        });
    }, [selectedline]);
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
