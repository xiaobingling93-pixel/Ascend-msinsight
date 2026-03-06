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

import { getColorByIndex } from '@/leaksWorker/tools/color';
import { Program } from './Program';

export class MemoryStateProgram extends Program {
    protected glInstanceData: Float32Array = new Float32Array();
    protected glInstanceDataSize: number = 0;
    hasBuffer = false;

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
        let totalLength = 0;

        // 提前计算所需buffer的长度
        for (let i = 0; i < data.length; i++) {
            totalLength += data[i].blocks.length * 7;
        }

        const needRealloc = !this.glInstanceData || this.glInstanceData.length < totalLength;

        let instanceData: Float32Array;
        if (needRealloc) {
            // 创建新的buffer
            instanceData = new Float32Array(totalLength);
        } else {
            // 复用已有buffer，但只清空/覆盖前 totalLength 个元素
            instanceData = this.glInstanceData;
        }

        let offset = 0;
        for (let i = 0; i < data.length; i++) {
            const segment = data[i];
            for (let j = 0; j < segment.blocks.length; j++) {
                const color = getColorByIndex(j);
                const block = segment.blocks[j];
                instanceData[offset++] = segment.offsetX + block.offset;
                instanceData[offset++] = segment.offsetY;
                instanceData[offset++] = block.size;
                instanceData[offset++] = color[0];
                instanceData[offset++] = color[1];
                instanceData[offset++] = color[2];
                instanceData[offset++] = color[3];
            }
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
        gl.drawArraysInstanced(gl.TRIANGLES, 0, 6, instanceCount);
        gl.bindVertexArray(null);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
}
