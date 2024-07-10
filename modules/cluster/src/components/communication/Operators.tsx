import { observer } from 'mobx-react-lite';
import React from 'react';

const Operators = observer(({ rankId }: { rankId: string }) => {
    return (
        <div>
            <div>Advisor</div>
            <div>Table</div>
            <div>Chart</div>
        </div>
    );
});

export default Operators;
