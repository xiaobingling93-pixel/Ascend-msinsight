/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useRef } from 'react';
import './Resizor.css';

// eslint-disable-next-line max-lines-per-function
const Resizor = (props: any): JSX.Element => {
    const divRef: any = useRef(null);
    let isDown = false;
    let offsetX: number;
    let initalWidth: number;
    let initalNextWidth: number;

    function handleMouseDown(event: any): void {
        event.preventDefault();
        if (!isDown) {
            isDown = true;
            offsetX = event.clientX;
            const dom = divRef.current.parentNode;
            dom.style.pointerEvents = 'none';
            initalWidth = dom.offsetWidth;
            const nextBrother = dom.nextElementSibling;
            if (nextBrother !== null) {
                initalNextWidth = nextBrother.offsetWidth;
            }

            window.addEventListener('mousemove', handleMouseMove);
            window.addEventListener('mouseup', handleMouseUp);
            // 如果当前在iframe中
            if (self !== top) {
                window.addEventListener('message', handleTopWindow);
            }
            // 如果页面中有iframe
            if (window.document.querySelectorAll('iframe').length !== 0) {
                window.document.querySelectorAll('iframe').forEach((item: any) => {
                    item.style['pointer-events'] = 'auto';
                });
            }
        }
    };

    function handleMouseMove(event: any): void {
        event.preventDefault();
        if (isDown && divRef?.current as boolean) {
            const deltaX = event.clientX - offsetX;
            const dom = divRef.current.parentNode;
            const width = initalWidth + deltaX;
            const nextBrother = dom.nextElementSibling;
            if (nextBrother !== null) {
                const nextWidth = initalNextWidth - deltaX;
                props.onResize(deltaX, width, nextWidth);
                return;
            }
            props.onResize(deltaX, width);
        }
    }
    function handleMouseUp(event?: any): void {
        if (event !== undefined) {
            event.preventDefault();
        }
        isDown = false;
        offsetX = 0;
        window.removeEventListener('mousemove', handleMouseMove);
        window.removeEventListener('mouseup', handleMouseUp);
        const parentNode = divRef.current.parentNode;
        parentNode.style.pointerEvents = null;
        // 如果当前在iframe中
        if (self !== top) {
            window.removeEventListener('message', handleTopWindow);
        }
        // 如果页面中有iframe
        if (window.document.querySelectorAll('iframe').length !== 0) {
            window.document.querySelectorAll('iframe').forEach((item: any) => {
                item.style['pointer-events'] = 'auto';
            });
        }
    }

    function handleTopWindow(event?: any): void {
        try {
            if (typeof event.data !== 'string') {
                return;
            }
            const data = JSON.parse(event.data);
            if (data.from === 'framework' && data.event === 'mouseover') {
                handleMouseUp();
            }
        } catch (error) {
            console.log(error);
        }
    }

    return <div ref={divRef} className={'resizor'} onMouseDown={handleMouseDown} ></div>;
};
export default Resizor;
