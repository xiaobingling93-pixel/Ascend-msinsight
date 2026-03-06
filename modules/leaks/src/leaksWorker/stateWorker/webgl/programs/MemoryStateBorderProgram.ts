/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

import { Program } from './Program';

export enum RenderType {
    border = 'border',
    highlightBorder = 'highlightBorder',
};

export class MemoryStateBorderProgram extends Program {
    readonly renderType: RenderType;
    protected glInstanceData: Float32Array = new Float32Array();
    protected glInstanceDataSize: number = 0;
    hasBuffer = false;

    constructor(gl: WebGL2RenderingContext, uniformData: Float32Array, shader: Shader, renderType: RenderType = RenderType.border) {
        super(gl, uniformData, shader);
        this.renderType = renderType;
    }

    bindBuffer(): void {
        const gl = this.gl;
        if (this.instanceBuffer) {
            this.gl.deleteBuffer(this.instanceBuffer);
        }
        this.instanceBuffer = this.createBuffer(4 * this.glInstanceDataSize);
        gl.bindVertexArray(this.vao);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceBuffer);
        const stride = 7 * 4;
        gl.enableVertexAttribArray(0);
        gl.vertexAttribPointer(0, 1, gl.FLOAT, false, stride, 0);
        gl.vertexAttribDivisor(0, 1);
        gl.enableVertexAttribArray(1);
        gl.vertexAttribPointer(1, 1, gl.FLOAT, false, stride, 4);
        gl.vertexAttribDivisor(1, 1);
        gl.enableVertexAttribArray(2);
        gl.vertexAttribPointer(2, 1, gl.FLOAT, false, stride, 8);
        gl.vertexAttribDivisor(2, 1);
        gl.enableVertexAttribArray(3);
        gl.vertexAttribPointer(3, 4, gl.FLOAT, false, stride, 12);
        gl.vertexAttribDivisor(3, 1);
        gl.bindVertexArray(null);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }

    updateSubBuffer(): void {
        const gl = this.gl;
        gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceBuffer);
        gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.glInstanceData.subarray(0, this.glInstanceDataSize));
    }

    processData(data: Segment[]): void {
        if (this.renderType !== RenderType.border) {
            return;
        }
        const totalLength = data.length * 7;

        const needRealloc = !this.glInstanceData || this.glInstanceData.length < totalLength;

        let instanceData: Float32Array;
        if (needRealloc) {
            // 创建新的buffer
            instanceData = new Float32Array(totalLength);
        } else {
            // 复用已有buffer，但只清空/覆盖前 totalLength 个元素
            instanceData = this.glInstanceData;
        }

        const borderColor = [0, 0, 0, 1];
        let offset = 0;
        for (let i = 0; i < data.length; i++) {
            const segment = data[i];
            instanceData[offset++] = segment.offsetX;
            instanceData[offset++] = segment.offsetY;
            instanceData[offset++] = segment.size;
            instanceData[offset++] = borderColor[0];
            instanceData[offset++] = borderColor[1];
            instanceData[offset++] = borderColor[2];
            instanceData[offset++] = borderColor[3];
        }

        this.glInstanceData = instanceData;
        this.glInstanceDataSize = totalLength;
        if (needRealloc) {
            this.bindBuffer();
        } else {
            this.updateSubBuffer();
        }
        this.hasBuffer = true;
    }

    // 复用边框处理的逻辑处理高亮数据,不能在同一个实例里和processData混用
    processHighlightData(highlightData: StateDataHoverResult | null): void {
        if (this.renderType !== RenderType.highlightBorder) {
            return;
        }
        if (this.glInstanceDataSize < 7) {
            // 高亮数据固定一条，只在第一次调用时申请buffer
            this.glInstanceData = new Float32Array(7);
            this.glInstanceDataSize = 7;
            this.bindBuffer();
        }

        const instanceData = this.glInstanceData;
        // 高亮颜色为红色,如果没有高亮数据将颜色则设置为透明
        const highlightColor = [1, 0, 0, highlightData === null ? 0 : 1];
        // let offset = 0;
        if (highlightData?.type === 'segment') {
            const segment = highlightData.data;
            instanceData[0] = segment.offsetX;
            instanceData[1] = segment.offsetY;
            instanceData[2] = segment.size;
        } else if (highlightData?.type === 'block') {
            const segment = highlightData.data;
            const block = segment.blocks[0]; // 内部数据，能走到这里，一定有一个block
            instanceData[0] = segment.offsetX + block.offset;
            instanceData[1] = segment.offsetY;
            instanceData[2] = block.size;
        } else {
            instanceData[0] = 0;
            instanceData[1] = 0;
            instanceData[2] = 0;
        }
        instanceData[3] = highlightColor[0];
        instanceData[4] = highlightColor[1];
        instanceData[5] = highlightColor[2];
        instanceData[6] = highlightColor[3];

        this.hasBuffer = true;
    }

    render(options: RenderOptions): void {
        if (!this.hasBuffer || this.instanceBuffer === null) {
            return;
        }
        const gl = this.gl;
        gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceBuffer);
        gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.glInstanceData, 0);
        const instanceCount = this.glInstanceDataSize / 7;
        const uniformData = this.uniformData;
        gl.useProgram(this.program);
        gl.uniform2f(this.uniformLoc.uScale, uniformData[0], uniformData[1]);
        gl.uniform2f(this.uniformLoc.uTranslate, uniformData[2], uniformData[3]);
        gl.uniform2f(this.uniformLoc.uResolution, uniformData[4], uniformData[5]);
        gl.uniform2f(this.uniformLoc.uZoom, uniformData[6], uniformData[7]);
        gl.bindVertexArray(this.vao);
        gl.drawArraysInstanced(gl.LINES, 0, 8, instanceCount);
        gl.bindVertexArray(null);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
}
