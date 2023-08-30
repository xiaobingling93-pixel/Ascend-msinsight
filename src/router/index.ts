import { createRouter, createWebHistory } from 'vue-router';
import { request2 as request } from '@centralServer';

const modules = import.meta.glob('@plugins/*/*.html');
const test = () => {
    console.log(123);
}

export const routes = Object.entries(modules).map(([path]) => {
    const name = path.replace('/plugins/', '').replace(/\/\w+.html/, '');
    return {
        path: `/${name}`,
        name,
        component: {
            template: `<iframe id=${name} src=${path} allow=""></iframe>`,
            mounted: () => {
                setTimeout(() => {
                    const iframe = document.getElementById(name) as HTMLIFrameElement;
                    console.log(iframe);
                    iframe.contentWindow?.postMessage({
                        test
                    }, '*');
                    localStorage.setItem('request', JSON.stringify(test));
                }, 1000);
            },
        },
        meta: {
            path,
        },
    };
});

const router = createRouter({
    history: createWebHistory(import.meta.env.BASE_URL),
    routes: [
        {
            path: '/*',
            redirect: '/Timeline',
        },
        ...routes,
    ],
});

export default router;
