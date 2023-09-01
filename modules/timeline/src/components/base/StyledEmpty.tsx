import * as React from 'react';
import { Empty } from 'antd';
import { EmptyProps } from 'antd/lib/empty';
import { useTheme } from '@emotion/react';

export const StyledEmpty = (props: EmptyProps & { descriptor?: string }): JSX.Element => {
    const theme = useTheme();
    return (
        <Empty
            image={Empty.PRESENTED_IMAGE_SIMPLE}
            description={
                <span style={{ color: theme.fontColor }}>
                    {props.descriptor ?? 'No Data'}
                </span>}>
        </Empty>
    );
};
