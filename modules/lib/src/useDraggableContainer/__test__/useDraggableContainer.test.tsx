import React from 'react';
import { render, fireEvent } from '@testing-library/react';
import { useDraggableContainer, ViewProps } from '../useDraggableContainer';
import { renderHook } from '@testing-library/react-hooks';
import '@testing-library/jest-dom/extend-expect';
import type { Theme } from '@emotion/react';
const light = {
    contentBackgroundColor: '#F1F3F5',
    closeDragContainerBG: 'rgb(229, 230, 232)',
    switchIconColor: '#18181A',
    dividerColor: '#fff',
    maskColor: 'rgb(255, 255, 255, 0.55)',
    fontColor: '#fff',
} as Theme;

describe('DraggableContainer test', () => {
    const viewProps: ViewProps = {
        mainContainer: React.createElement('div', null, 'Mock main Element'),
        draggableContainer: React.createElement('div', null, 'Mock draggable Element'),
        slot: React.createElement('div', null, 'Mock slot Element'),
        id: 'Mock Draggable',
    };
    for (let i = 0; i < 4; i++) {
        it('should pass when view Rendering correctly', function () {
            const { result } = renderHook(() => useDraggableContainer({ dragDirection: i, draggableWH: 300, open: true, theme: light }));
            const [view] = result.current;
            const { container } = render(view(viewProps));
            const draggableContainer = container.querySelector('div');
            const draggableButton = container.querySelector('.buttonShow');
            const caretButton = container.querySelector('.caret');
            expect(draggableButton).toBeInTheDocument();
            expect(caretButton).toBeInTheDocument();
            expect(draggableContainer).toBeInTheDocument();
            if (draggableContainer) {
                fireEvent.mouseDown(draggableContainer);
                fireEvent.mouseMove(draggableContainer);
                fireEvent.mouseUp(draggableContainer);
            }
            if (draggableButton) {
                fireEvent.click(draggableButton);
            }
            if (caretButton) {
                fireEvent.click(caretButton);
            }
        });
    }
});
