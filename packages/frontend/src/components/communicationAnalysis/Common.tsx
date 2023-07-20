import React from 'react';

export const Label = (props: {name: string }): JSX.Element => {
    return <span style={{ margin: '0 10px' }}>{props.name} :</span>;
};

export const Space = (props: {length: string | number }): JSX.Element => {
    return <span style={{ display: 'inline-block', marginRight: `${props.length}px` }}/>;
};
