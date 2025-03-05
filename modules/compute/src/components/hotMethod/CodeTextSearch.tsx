/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import styled from '@emotion/styled';
import { Input } from 'ascend-components';
import { ArrowUpOutlined, ArrowDownOutlined, CloseOutlined, AlignLeftOutlined } from '@ant-design/icons';
import { CaseIcon, FullTextIcon, RegularIcon } from 'ascend-icon';
import Mark from '../../utils/DomSearch/mark';
import { store } from '../../store';
import { observable, runInAction } from 'mobx';
import { useTranslation } from 'react-i18next';
import { themeInstance } from 'ascend-theme';

const TextSearchContainer = styled.div`
    background:${(p): string => p.theme.bgColorLight};
    box-shadow : 0 8px 16px 0 rgba( 0,0,0,0.10 ) ;
    height:48px;
    min-width: 430px;
    width:80%;
    border-radius : 2px ;
    position: absolute;
    top: 32px;
    right: 5px;
    padding: 8px 16px;
    .btn{
        color:${(p): string => p.theme.iconColor};
        cursor:pointer;
        width:20px;
        height:20px;
        padding:2px;
        margin-left:4px;
        &.active{
            background:${(p): string => p.theme.radioSelectedColor};
            color:#FFFFFF;
        }
        &:not(.active):hover{
            color:${(p): string => p.theme.textColorPrimary};
        }
    }
    div{
        display:inline-block;
    }
    .search-result{
        width:100px;
        padding:0 5px;
        text-align: center;
        height:32px;
    }
`;
const IconBtn = styled.div`
    color:${(p): string => p.theme.iconColor};
    display: inline-block;
    margin-left: 5px;
    border-radius: 1px;
    cursor: pointer;
    text-align: center;
    padding: 2px 5px;

    &.active{
        color:#FFFFFF;
        background:${(p): string => p.theme.radioSelectedColor};
    }
`;

// 清理查询结果
const clearMark = (): void => {
    const instance = new Mark(getCodeDom());
    instance.unmark();
};
// 代码查询
const serachInCode = ({ text, inRange, condition, handleAfterSearch }:
{text: string;inRange: boolean;condition: Record<string, boolean>;handleAfterSearch: (count: number) => void}): void => {
    clearMark();
    const instance = new Mark(inRange ? getCodeRangeDom() : getCodeDom());
    const option: any = {
        className: HIGHTLIGHT_CLASSNAME,
        caseSensitive: condition.case,
        accuracy: condition.fullText ? 'exactly' : 'partially',
        done: handleAfterSearch,
        separateWordSearch: false,
        acrossElements: true,
    };
    if (condition.regular) {
        instance.markRegExp(new RegExp(text), option);
    } else {
        instance.mark(text, option);
    }
};

export const openFind = (): void => {
    const session = store.sessionStore.activeSession;
    if (session) {
        runInAction(() => {
            session.openFind = true;
        });
    }
};

export const closeFind = (): void => {
    clearMark();
    const session = store.sessionStore.activeSession;
    if (session) {
        runInAction(() => {
            session.openFind = false;
        });
    }
};

// 工具栏高度
export const CODE_SEARCH_WINDOW_HEIGHT = 48;
// 高亮类名
const HIGHTLIGHT_CLASSNAME = 'code-hightlight';
// 当前查看元素类名
const CURRENT_DOM_CLASSNAME = 'current';
// 选中范围类名
const CODE_RANGE_CLASSNAME = 'range';
// 启用选中范围
const CODE_RANGE_ACTIVE_CLASSNAME = 'active-range';
// 代码Dom
const getCodeDom = (): HTMLElement => {
    return document.querySelector('#CodeTable code') as HTMLElement;
};
// 选中范围内的代码Dom
const getCodeRangeDom = (): HTMLElement[] => {
    const nodelist = document.querySelectorAll(`#CodeTable code li.${CODE_RANGE_CLASSNAME}`);
    const list: HTMLElement[] = [];
    nodelist.forEach(item => {
        list.push(item as HTMLElement);
    });
    return list;
};
// 跳转到第index个结果，并高亮
const goTo = (index: number): void => {
    const elements = document.querySelector('#CodeTable code')?.querySelectorAll(`.${HIGHTLIGHT_CLASSNAME}`);
    elements?.forEach(ele => ele.classList.remove(CURRENT_DOM_CLASSNAME));
    const ele = elements?.[index];
    ele?.scrollIntoView({ block: 'center' });
    ele?.classList.add(CURRENT_DOM_CLASSNAME);
};

// 获取上级Li元素
const getParentLi = (node: Node | null): Node | null => {
    const maxCyle = 1000;
    let index = 0;
    let liNode: Node | null = node;
    while (liNode !== null && liNode.nodeName !== 'LI' && index < maxCyle) {
        liNode = liNode.parentNode;
        index++;
    }
    return liNode;
};

// 选中区域
const observeRange = observable<{ index: number }>({ index: 0 });
const updateRangeIndex = (): void => {
    observeRange.index = (observeRange.index + 1) % 100;
};
// 鼠标选中代码块
const handleMouseSelection = (): void => {
    const codeDom = getCodeDom();
    if (codeDom === null) {
        return;
    }
    // 清理上次选中
    codeDom.querySelectorAll(`li.${CODE_RANGE_CLASSNAME}`).forEach(li =>
        (li as HTMLElement).classList.remove(CODE_RANGE_CLASSNAME));
    const selection = window.getSelection();
    if (selection?.anchorNode != null && selection?.focusNode !== null) {
        const { anchorNode, focusNode } = selection;
        // 从前往后选择
        const front2Back = anchorNode.compareDocumentPosition(focusNode) === Node.DOCUMENT_POSITION_FOLLOWING;
        const startLi = front2Back ? getParentLi(anchorNode) : getParentLi(focusNode);
        const endLi = front2Back ? getParentLi(focusNode) : getParentLi(anchorNode);
        if (startLi !== null && endLi !== null) {
            let li: Node | ChildNode | null = startLi;
            let index = 0;
            const maxCycle = 100000;
            // 标记选中范围
            (startLi as HTMLElement).classList.add(CODE_RANGE_CLASSNAME);
            (endLi as HTMLElement).classList.add(CODE_RANGE_CLASSNAME);
            while (index < maxCycle && li !== null && li.compareDocumentPosition(endLi) !== 0) {
                (li as HTMLElement).classList.add(CODE_RANGE_CLASSNAME);
                li = li.nextSibling;
                index++;
            };
        }
    }
    updateRangeIndex();
};
const registerCodeSelectEvent = (): () => void => {
    const codeDom = getCodeDom();
    // 鼠标抬起
    codeDom?.addEventListener('mouseup', handleMouseSelection);
    return (): void => {
        codeDom?.removeEventListener('mouseup', handleMouseSelection);
        clearCodeRange();
    };
};
// 清理选中代码
const clearCodeRange = (): void => {
    const codeDom = getCodeDom();
    codeDom?.querySelectorAll(`li.${CODE_RANGE_CLASSNAME}`).forEach(li => (li as HTMLElement).classList.remove(CODE_RANGE_CLASSNAME));
};

// 匹配条件
enum MatchOption {
    CASE = 'case',
    FULL_TEXT = 'fullText',
    REGULAR = 'regular',
}
const defaultCondition: Record<MatchOption, boolean> = { case: false, fullText: false, regular: false };

// 代码搜索工具栏
const CodeTextSearch = observer((): JSX.Element => {
    const { t } = useTranslation('source');
    const [condition, setCondition] = useState<Record<string, boolean>>(defaultCondition);
    const [inRange, setInRange] = useState(false);
    const [text, setText] = useState('');
    const [searchText, setSearchText] = useState('');
    const [total, setTotal] = useState(0);
    const [curIndex, setCurIndex] = useState(0);

    const reset = (): void => {
        setText('');
        setSearchText('');
        setTotal(0);
        setCurIndex(0);
        clearMark();
    };
    const handleAfterSearch = (count: number): void => {
        setTotal(count);
        setCurIndex(0);
        goTo(0);
    };
    const handleSearch = (force?: boolean): void => {
        if (text === '') {
            reset();
        } else if (text !== searchText || force) {
            setSearchText(text);
            serachInCode({ text, inRange, condition, handleAfterSearch });
        } else {
            if (total > 0) {
                setCurIndex(pre => (pre + 1) % total);
            }
        }
    };

    const handleEsc = (): void => {
        closeFind();
    };

    const switchRange = (): void => {
        getCodeDom().classList.toggle(CODE_RANGE_ACTIVE_CLASSNAME);
        setInRange(pre => !pre);
    };

    useEffect(() => {
        return registerCodeSelectEvent();
    }, []);

    useEffect(() => {
        goTo(curIndex);
    }, [curIndex]);

    useEffect(() => {
        handleSearch(true);
    }, [condition, inRange]);

    useEffect(() => {
        if (inRange) {
            handleSearch(true);
        }
    }, [observeRange.index]);
    return <TextSearchContainer onClick={(e): void => e.stopPropagation()}>
        <Input style={{ width: 'calc(100% - 200px)' }} value={text} onChange={(e): void => { setText(e.target.value); }} onPressEnter={(): void => handleSearch()}
            suffix={ <Filter onChange={(val): void => setCondition(val)} theme={themeInstance.getCurrentTheme()}/>} allowClear/>
        <div className="search-result">
            { total === 0 ? t('No Result') : '' }
            { total > 0 ? <div>{curIndex + 1} of {total}</div> : <></> }
        </div>
        <ArrowUpOutlined className={'btn'} onClick={(): void => setCurIndex(pre => total > 0 ? (pre - 1 + total) % total : pre)}/>
        <ArrowDownOutlined className={'btn'} onClick={(): void => setCurIndex(pre => total > 0 ? (pre + 1) % total : pre)}/>
        <AlignLeftOutlined className={`btn ${inRange ? 'active' : ''}`} onClick={switchRange}/>
        <CloseOutlined className={'btn'} onClick={handleEsc}/>
    </TextSearchContainer>;
});

const btnGroup: Record<MatchOption, (active: boolean) => React.ReactNode> = {
    case: (active: boolean) => <CaseIcon active={active}/>,
    fullText: (active: boolean) => <FullTextIcon active={active}/>,
    regular: (active: boolean) => <RegularIcon active={active}/>,
};

const Filter = ({ onChange }: {onChange: (condition: Record<string, boolean>) => void;theme?: string}): JSX.Element => {
    const [condition, setCondition] = useState<Record<string, boolean>>(defaultCondition);

    useEffect(() => {
        onChange(condition);
    }, [condition]);

    return <div>
        {
            Object.keys(btnGroup).map(key =>
                (<IconBtn
                    key={key}
                    onClick={(): void => { setCondition(pre => ({ ...pre, [key]: !pre[key] })); }}
                >{btnGroup[key as MatchOption]?.(condition[key])}</IconBtn>),
            )
        }
    </div>;
};

export default CodeTextSearch;
