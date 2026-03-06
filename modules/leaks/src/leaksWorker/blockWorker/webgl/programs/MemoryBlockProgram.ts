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

import { getColorByAddr } from '@/leaksWorker/tools/color';
import { Program } from './Program';

export class MemoryBlockProgram extends Program {
    protected batches: Float32Array[] = [];
    protected maxInstanceDataSize: number = 10000000;
    hasBuffer = false;

    bindBuffer(): void {
        const gl = this.gl;
        if (this.instanceBuffer) {
            return;
        }
        this.instanceBuffer = this.createBuffer(4 * this.maxInstanceDataSize);
        gl.bindVertexArray(this.vao);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceBuffer);
        const stride = 9 * 4;
        gl.enableVertexAttribArray(0);
        gl.vertexAttribPointer(0, 2, gl.FLOAT, false, stride, 0);
        gl.vertexAttribDivisor(0, 1);
        gl.enableVertexAttribArray(1);
        gl.vertexAttribPointer(1, 2, gl.FLOAT, false, stride, 8);
        gl.vertexAttribDivisor(1, 1);
        gl.enableVertexAttribArray(2);
        gl.vertexAttribPointer(2, 1, gl.FLOAT, false, stride, 16);
        gl.vertexAttribDivisor(2, 1);
        gl.enableVertexAttribArray(3);
        gl.vertexAttribPointer(3, 4, gl.FLOAT, false, stride, 20);
        gl.vertexAttribDivisor(3, 1);
        gl.bindVertexArray(null);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }

    processData(data: RenderData['blocks'], isHighlight: boolean = false): void {
        this.batches = [];
        let batch = [];
        for (let i = 0; i < data.length; i++) {
            const { path, size, addr } = data[i];
            if (batch.length + path.length * 9 > this.maxInstanceDataSize) {
                this.batches.push(new Float32Array(batch));
                batch = [];
            }
            for (let j = 0; j < path.length - 1; j++) {
                batch.push(...path[j], ...path[j + 1], size, ...getColorByAddr(addr, isHighlight));
            }
        }
        this.batches.push(new Float32Array(batch));
        this.bindBuffer();
        this.hasBuffer = true;
    }

    render(options: RenderOptions): void {
        if (!this.hasBuffer || this.instanceBuffer === null) {
            return;
        }
        const gl = this.gl;
        const uniformData = this.uniformData;
        gl.useProgram(this.program);
        gl.uniform2f(this.uniformLoc.uScale, uniformData[0], uniformData[1]);
        gl.uniform2f(this.uniformLoc.uTranslate, uniformData[2], uniformData[3]);
        gl.uniform2f(this.uniformLoc.uResolution, uniformData[4], uniformData[5]);
        gl.uniform2f(this.uniformLoc.uZoom, uniformData[6], uniformData[7]);
        gl.uniform1f(this.uniformLoc.uOffset, uniformData[8]);
        gl.bindVertexArray(this.vao);
        for (let i = 0; i < this.batches.length; i++) {
            gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceBuffer);
            gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.batches[i], 0);
            const instanceCount = this.batches[i].length / 9;
            gl.drawArraysInstanced(gl.TRIANGLES, 0, 6, instanceCount);
        }
        gl.bindVertexArray(null);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
}
