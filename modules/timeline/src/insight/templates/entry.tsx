import React from 'react';
import { InsightTemplate } from '../../entity/insight';
import { ReactComponent as HomePageIcon } from '../../assets/images/insights/HomePageIcon.svg';

export const EntryTemplate: InsightTemplate = {
    id: 'entry',
    name: 'entry',
    source: '<internal>' as const,
    description: '',
    icon: <HomePageIcon className="homePageIcon"/>,
    units: [],
    availableUnits: [],
    isNsMode: true,
};
