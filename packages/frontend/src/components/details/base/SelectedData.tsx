import React from 'react';
import styled from '@emotion/styled';
import { observer } from 'mobx-react';

interface timeDetailProps {
    renderer?: Array<[string, string | JSX.Element]>;
    hasTitle?: boolean;
}

const StyledSliceDetailDiv = styled.div`
    flex: 1;
    color: ${props => props.theme.fontColor};
    font-size: 12px;

    .sliceDetailTitle {
        text-align: start;
        font-weight: bold;
        background: ${props => props.theme.contentBackgroundColor};
    }
    .sliceDetail {
        display: flex;
        text-align: left;
        line-height: 32px;
        .sliceDetailName {
            flex: 1;
            padding-left: 24px;
            font-weight: bold;
        }
        .sliceDetailMsg {
            flex: 4;
            user-select: text;
            .iconContainer{
                display: flex;
                align-items: center;
                .jumpingIcon {
                    cursor: pointer;
                    margin: 2px 8px 0 5px;
                    g {
                        #Rectangle {
                            fill: ${props => props.theme.svgPlayBackgroundColor};
                        }
                        path {
                            fill: ${props => props.theme.selectedChartColor};
                        }
                    }
                }
            }
        }
    }
    .sliceDetail: hover {
        background-color: ${props => props.theme.tableRowSelect};
    }
`;

export const SelectedDataBase = observer(function SelectedDataBase(props: timeDetailProps): JSX.Element {
    const { renderer, hasTitle = false } = props;
    return <StyledSliceDetailDiv>
        {hasTitle && <div className = "sliceDetail">
            <div className = "sliceDetailName">Event(s)</div>
            <div className = "sliceDetailMsg">Link</div>
        </div>}
        {renderer?.map((item, index) => <div className = "sliceDetail" key={`${item[0]}-${index}`}>
            <div style={{ width: '30%' }} className = "sliceDetailName">{item[0]}</div>
            <div style={{ width: '70%' }} className = "sliceDetailMsg">{item[1]}</div>
        </div>)}
    </StyledSliceDetailDiv>;
});
