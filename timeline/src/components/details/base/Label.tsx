import * as React from 'react';
import styled from '@emotion/styled';
import type { Theme } from '@emotion/react';

interface LabelPropType {
    name: keyof Theme['categories'];
}
const Label = ({ name }: LabelPropType): JSX.Element => {
    const StyledLabel = styled.span`
        color: ${p => p.theme.categories[name].color};
        background-color: ${p => p.theme.categories[name].background};
        font-weight: 400;
        font-size: .85rem;
        border-radius: 4px;
        padding: 3px 8px;
    `;

    return name !== undefined
        ? <StyledLabel> { name } </StyledLabel>
        : <div />;
};

export default Label;
