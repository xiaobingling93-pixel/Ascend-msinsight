import * as React from 'react';
import { render, screen } from '@testing-library/react';

it('HorizontalScrollbar test', () => {
    const TestComponent = (): JSX.Element => {
        return <div>Test</div>;
    };
    render(<TestComponent />);
    expect(screen.getByText('Test')).toBeDefined();
});
