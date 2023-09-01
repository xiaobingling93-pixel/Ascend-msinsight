import React, { useEffect } from 'react';
import { observer } from 'mobx-react';
import { runInAction } from 'mobx';
import { Session } from '../../entity/session';
import { SingleDataDesc } from '../../entity/insight';
import { useSelectedDataDetailUpdater } from './hooks';
import { SelectedDataBase } from '../details/base/SelectedData';

export const SimpleSelectedDetail = observer(({ session, detail }: {session: Session; detail: SingleDataDesc<Record<string, unknown>, unknown>}): JSX.Element => {
    useEffect(() => {
        runInAction(() => {
            session.selectedDetailKeys = [];
            session.selectedDetails = [];
        });
    }, [session.selectedData]);
    const { renderFields } = useSelectedDataDetailUpdater(session, detail, session.selectedData);
    return <SelectedDataBase renderer={renderFields} />;
}) as (props: { session: Session; detail: SingleDataDesc<Record<string, unknown>, unknown> }) => JSX.Element;
