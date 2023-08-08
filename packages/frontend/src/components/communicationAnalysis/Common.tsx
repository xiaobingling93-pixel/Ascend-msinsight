import React from 'react';
import { DragDirection, useDraggableContainer } from '../../utils/useDraggableContainer';

export const Label = (props: {name: string;style?: object }): JSX.Element => {
    return <span style={{ margin: '0 10px', ...(props.style ?? {}) }}>{props.name} :</span>;
};

export const Space = (props: {length: string | number }): JSX.Element => {
    return <span style={{ display: 'inline-block', marginRight: `${props.length}px` }}/>;
};

export const Container = (props: {title?: JSX.Element | string; content?: JSX.Element;style?: any}): JSX.Element => {
    return <div style={{ height: '100%', margin: '10px', ...(props.style ?? {}) }}>
        <div style={{
            fontWeight: 'bold', background: '#fafafa', height: '3rem', lineHeight: '3rem', fontSize: '1.5rem', paddingLeft: '10px',
        }}>{props.title}</div>
        <div style={{ height: 'calc(100% - 20px', overflow: 'auto' }}>{props.content}</div>
    </div>;
};

export const Tan = (props: {position: string;main: JSX.Element;drag: JSX.Element;id: string;dragSize?: number;style?: object}): JSX.Element => {
    const [view] = useDraggableContainer({ draggableWH: props.dragSize ?? 300, dragDirection: DragDirection.left, open: true });
    const { style = {} } = props;
    return <div style={{ display: 'block', ...style }}>
        <div style={{ display: 'flex', height: '100%', overflow: 'auto' }} className={'tan-box'}>
            {view({
                mainContainer: props.main,
                draggableContainer: props.drag,
                id: props.id,
            })}
        </div>
    </div>;
};
