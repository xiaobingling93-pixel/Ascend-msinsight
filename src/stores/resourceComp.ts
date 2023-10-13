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
    leaf?: boolean;
};

interface Body {
    name: string;
    path: string;
    childrenFolders: ResourceItem[];
    childrenFiles: ResourceItem[];
}

type ResourceTotal = {
    [key: string]: ResourceItem[]
}


export const useResource = defineStore('resource', () => {

    const resourceState = reactive({
        currentPath: "",
        startResource: [] as ResourceItem[],
        resourceTotal: {} as ResourceTotal
    });

    const dealResource = (newResource: ResourceItem): ResourceItem[] => {
        newResource.childrenFolders?.forEach(item => {
            item.leaf = false;
        })
        newResource.childrenFiles?.forEach(item => {
            item.leaf = true;
        })
        return [...(newResource.childrenFolders ? newResource.childrenFolders : []), ...(newResource.childrenFiles ? newResource.childrenFiles : [])];
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
        if (newResource.children) {
            resourceState.resourceTotal[newResource.path] = newResource.children;
            if (path === "") {
                resourceState.startResource = [...newResource.children];
            }
        }
        return newResource.children;
    }

    const loadFiles = async (path: string): Promise<ResourceItem[]> => {
        const result = await request({ remote: LOCAL_HOST, port: PORT, dataPath: [] }, 'global', {
            command: 'files/get',
            params: {
                path,
            }
        });
        return setResource(result as Body)
    }

    const setCurrentPath = (path: string) => {
        resourceState.currentPath = path;
    }

    return { resourceState, loadFiles, setCurrentPath }
});
