import { useTheme } from '@emotion/react';
import { Tooltip } from 'antd';
import { TooltipPlacement } from 'antd/lib/tooltip';
import * as React from 'react';

interface AbbreviatureType {
    content: string | JSX.Element;
    availableWidth?: number | undefined;
    fontSize?: string | undefined;
    placement?: string | undefined;
};

const DFT_FONT_SIZE = '1rem';

const containerStyle: React.CSSProperties = {
    overflow: 'hidden',
    width: '100%',
    textOverflow: 'ellipsis',
    whiteSpace: 'nowrap',
};

const calcTextWidth = (container: HTMLDivElement, contentWidth: number,
    setContentWidth: React.Dispatch<React.SetStateAction<number>>): void => {
    if (contentWidth !== 0) return;
    const ele = container.querySelector('span');
    if (!ele) return;
    setContentWidth(ele.offsetWidth);
};

export const Abbreviature = ({ content, availableWidth, fontSize, placement }: AbbreviatureType): JSX.Element => {
    const parent = React.useRef<HTMLDivElement | null>(null);
    const theme = useTheme();
    const [ isOverstep, setIsOverstep ] = React.useState<boolean>(false);
    const [ windowWidth, setWindowWidth ] = React.useState<number>(window.innerWidth);
    const [ contentWidth, setContentWidth ] = React.useState<number>(0);

    if (fontSize === undefined) fontSize = DFT_FONT_SIZE;

    // 监听窗口宽度
    const adjustWidth = (): void => setWindowWidth(window.innerWidth);
    React.useEffect(() => {
        window.addEventListener('resize', adjustWidth);
        return () => {
            window.removeEventListener('resize', adjustWidth);
        };
    }, []);

    React.useEffect(() => {
        const parentWidth = availableWidth ?? parent.current?.clientWidth;
        parent.current && calcTextWidth(parent.current, contentWidth, setContentWidth);
        if (parentWidth !== undefined && contentWidth !== undefined &&
            contentWidth !== 0 && parentWidth !== 0) {
            setIsOverstep(parentWidth <= contentWidth);
        }
    }, [ parent.current, windowWidth, availableWidth, contentWidth ]);

    return <>
        {isOverstep
            ? (<Tooltip
                mouseEnterDelay={0.3}
                overlayStyle={{ maxHeight: 200, overflow: 'hidden', fontSize }}
                placement={(placement ?? 'topLeft') as TooltipPlacement }
                color={theme.selectBackgroundColor}
                overlayInnerStyle={{ color: theme.fontColor, borderRadius: 10, boxShadow: 'none' }}
                title={content}
                destroyTooltipOnHide>
                <div style={{ ...containerStyle, fontSize }}>
                    <span>{content}</span>
                </div>
            </Tooltip>)
            : (<div ref={parent} style={{ ...containerStyle, fontSize }}>
                <span>{content}</span>
            </div>)
        }
    </>;
};
