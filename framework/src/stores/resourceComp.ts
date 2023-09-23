import { defineStore } from "pinia";
import { reactive } from 'vue';
import { request } from "@/centralServer/server";
import { LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';

export type ResourceItem = {
    path: string;
    name: string;
    childrenFolders?: ResourceItem[];
    childrenFiles?: ResourceItem[],
    children?: ResourceItem[];
};

interface Body {
    name: string;
    path: string;
    childrenFolders: ResourceItem[];
    childrenFiles: ResourceItem[];
}


export const useResource = defineStore('resource', () => {

    const resourceState = reactive({
        resource: [] as ResourceItem[],
        currentPath: "",
    });

    const dealResource = (newResource: ResourceItem): ResourceItem['children'] => {
        newResource.childrenFolders?.forEach(item => {
            item.children = dealResource(item);
        })
        return [...(newResource.childrenFolders ? newResource.childrenFolders : []), ...(newResource.childrenFiles ? newResource.childrenFiles : [])]
    }

    const doMerge = (resource: ResourceItem[], newResource: ResourceItem): boolean => {
        const path = newResource.path;
        const target = resource.find(item => item.path === path);
        if (target) {
            target.childrenFolders = newResource.childrenFolders || [];
            target.childrenFiles = newResource.childrenFiles || [];
            target.children = newResource.children;
            return true;
        } else {
            for (let i = 0; i < resource.length; i++) {
                const item = resource[i];
                if (item.childrenFolders) {
                    const result = doMerge(item.childrenFolders, newResource);
                    if (result) return true;
                }
            }
            return false;
        }
    }

    const setResource = (body: Body) => {
        const { name, path, childrenFolders, childrenFiles } = body;
        const newResource: ResourceItem = {
            path,
            name,
            childrenFolders,
            childrenFiles,
        }
        newResource.children = dealResource(newResource)
        if (path === "" && newResource.children) {
            resourceState.resource = [...newResource.children];
            return;
        }
        const mergeResult = doMerge(resourceState.resource, newResource)
    }

    const loadFiles = async (path: string) => {
        if (path === undefined) {
            return;
        }
        const result = await request({ remote: LOCAL_HOST, port: PORT, dataPath: [] }, 'global', {
            command: 'files/get',
            params: {
                path,
            }
        });
        setResource(result as Body)
    }

    const setCurrentPath = (path: string) => {
        resourceState.currentPath = path;
    }

    return { resourceState, loadFiles, setCurrentPath }
});
