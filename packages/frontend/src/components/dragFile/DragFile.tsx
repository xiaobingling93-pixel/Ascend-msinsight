/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect } from 'react';
import { FolderFilled } from '@ant-design/icons';
import { notification } from 'antd';
export const DragFileInit = (id: string): void => {
    const dropZone = document.getElementById(id);
    if (dropZone) {
        dropZone.addEventListener('dragenter', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
        }, false);

        dropZone.addEventListener('dragover', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.add('fileshover');
        }, false);

        dropZone.addEventListener('dragleave', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.remove('fileshover');
        }, false);
        // 处理拖拽文件
        dropZone.addEventListener('drop', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.remove('fileshover');
            handleFile(e.dataTransfer.items);
        }, false);
    }
};

async function handleFile(items: DataTransferItemList): Promise<void> {
    const allFiles = await getFilelist(items);
    allFiles.forEach((fileinfo: UploadFileDataType) => {
        readFile(fileinfo);
    });
}
interface UploadFileDataType{
    file: any;// 二进制流
    name: string;// 文件名
    path: string;// 文件路径
}

function readFile(fileInfo: any): void {
    const reader = new FileReader();
    // 二进制流
    reader.readAsArrayBuffer(fileInfo.file);
    reader.onload = event => {
        const buffer = event.target?.result;
        notification.success({
            message: 'Start Reading:',
            description: '【File】' + fileInfo.path,
            duration: null,
        });
        window.request('towingImport', { ...fileInfo, file: buffer }).catch(error => {
            notification.error({
                message: 'Read File Failed:',
                description: <div>
                    <div>{error.message}</div>
                    <div>{'【File】' + fileInfo.path}</div></div>,
                duration: null,
            });
        });
    };
}

async function getFilelist(items: any): Promise<any> {
    return new Promise((resolve, reject) => {
        const list = [];
        for (const item of items) {
            if (item.kind === 'file') {
                const entry = item.webkitGetAsEntry();
                list.push(getFileFromEntryRecusively(entry));
            }
        }
        Promise.all(list).then(values => {
            resolve(values.flat(1));
        });
    });
}

async function getFileFromEntryRecusively(entry: any): Promise<any> {
    return new Promise((resolve, reject) => {
        if (entry.isFile === true) {
            entry.file((file: any) => {
                const fileInfo = { file, path: entry.fullPath.slice(1), name: file.name };
                resolve(fileInfo);
            }, (e: any) => {
                console.log(e);
            });
        } else {
            const reader = entry.createReader();
            reader.readEntries(
                (entries: any) => {
                    entries.forEach((entry: any) => getFileFromEntryRecusively(entry));
                    Promise.all(entries.map((entry: any) => getFileFromEntryRecusively(entry))).then(values => {
                        resolve(values.flat(1));
                    });
                },
                (e: any) => {
                    console.log(e);
                },
            );
        }
    });
}

const DragFile: React.FC = () => {
    const id = 'drag';
    useEffect(() => {
        DragFileInit(id);
    }, []);

    const openSelect = async(): Promise<void> => {};
    return (
        <div className={'drag-zone'} onClick={() => openSelect()} id={id}>
            <FolderFilled className={'drag-zone-icon'}/>
            <div>点击或拖拽文件到此</div>
        </div>
    );
};

export default DragFile;
