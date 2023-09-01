import styled from '@emotion/styled';
import { observer } from 'mobx-react';
import React from 'react';

export type ChartInfoProps = {
    title: string;
};

const ChartInfoContainer = styled.div`
    display: flex;
    color: #FFFFFF;
    text-align: start;
    flex-direction: column;
    font-size: 1.5rem;
`;
const ChartInfoBottom = styled.div`
    display: flex;
    flex-grow: 1;
`;
const LeftContainer = styled.div`
        display: flex;
        flex-direction: column;
`;
const ItemTitle = styled.div`
    font-size: 1rem;
    margin: 0.5rem 0 1rem 0;
`;
const Item = styled.div`
    display: flex;
`;
const ItemNum = styled.div`
    font-size: 2rem;
    font-weight: 700;
`;
const ItemPercent = styled.div`
    font-size: .75rem;
    margin: 0.75rem 0 0 0.5rem;
`;

export const ChartInfo = observer(({ title }: ChartInfoProps) => {
    return <ChartInfoContainer>
        <div>{title}</div>
        <ChartInfoBottom>
            <LeftContainer>
                <ItemTitle>Current Used {title}</ItemTitle>
                <Item>
                    <ItemNum>25</ItemNum>
                    <ItemPercent>%</ItemPercent>
                </Item>
            </LeftContainer>
        </ChartInfoBottom>
    </ChartInfoContainer>;
});
