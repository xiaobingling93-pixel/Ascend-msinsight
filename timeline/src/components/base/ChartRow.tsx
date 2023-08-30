import * as React from 'react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import { RowProps } from 'antd/lib/grid';

const ChartRowContainer = styled.div`
    display: flex;
    align-items: center;
`;
interface ChartRowLeftProps {
    width: number;
}

export const ChartRowLeft = styled.div<ChartRowLeftProps>`
    width: ${(props) => props.width}px;
    height: 100%;
    position: relative;
`;

interface ChartRowRightProps {
    width?: React.CSSProperties['width'];
}

export const ChartRowRight = styled.div<ChartRowRightProps>`
    flex-grow: 1;
    width: ${(props) => props?.width !== undefined ? `${props.width}` : 'unset'};
    height: 100%;
    position: relative;
`;

export interface ChartRowProps extends RowProps {
    className?: string;
    key?: React.Key;
    children: [ JSX.Element | null, JSX.Element ];
    leftWidth: number;
    rightWidth?: React.CSSProperties['width'];
    rightAreaName?: string;
}

export const ChartRow = observer((props: ChartRowProps) => {
    return <ChartRowContainer className={ props.className }>
        <ChartRowLeft width={props.leftWidth}>
            { props.children[0] ?? <div/> }
        </ChartRowLeft>
        <ChartRowRight id={props.rightAreaName} width={props.rightWidth}>
            { props.children[1] }
        </ChartRowRight>
    </ChartRowContainer>;
});
