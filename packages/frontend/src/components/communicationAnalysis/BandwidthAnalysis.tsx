import { observer } from 'mobx-react-lite';
import React from 'react';
import { Session } from '../../entity/session';

const BandwidthAnalysis = observer(function ({ session }: { session: Session }) {
    return (
        <div>
            <div>Advisor</div>
            <div>Table</div>
            <div>Chart</div>
        </div>
    );
});

export default BandwidthAnalysis;
