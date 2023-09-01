import React from 'react';
import styled from '@emotion/styled';

const Container = styled.svg`
    height: 120px;
    width: 100%;
    padding: 0;
    margin: 0;
    display: block;
`;

const _SVGChart = (props: React.SVGProps<SVGSVGElement>, ref: React.Ref<SVGSVGElement>): JSX.Element => {
    return <Container ref={ ref } { ...props }/>;
};

export const SVGChart = React.forwardRef(_SVGChart);
