import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import { getSearchParams } from '../utils/localUrl';
import en from './en.json';
import zh from './zh.json';

const resources = {
    en,
    zh,
};

i18n.use(initReactI18next).init({
    resources,
    lng: getSearchParams('language') ?? 'en',
    interpolation: {
        escapeValue: false, // react already safes from xss
    },
});

export default i18n;
