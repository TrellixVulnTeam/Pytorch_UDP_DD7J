// Auto-generated file. Do not edit!
//   Template: src/f32-vlrelu/wasmsimd-bitselect.c.in
//   Generator: tools/xngen
//
// Copyright 2020 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include <wasm_simd128.h>

#include <xnnpack/common.h>
#include <xnnpack/vunary.h>


void xnn_f32_vlrelu_ukernel__wasmsimd_bitselect_x4(
    size_t n,
    const float* x,
    float* y,
    const union xnn_f32_lrelu_params params[restrict XNN_MIN_ELEMENTS(1)]) XNN_OOB_READS
{
  assert(n != 0);
  assert(n % sizeof(float) == 0);

  const v128_t vslope = wasm_v128_load64_splat(params->wasmsimd.slope);
  for (; n >= 4 * sizeof(float); n -= 4 * sizeof(float)) {
    const v128_t vx = wasm_v128_load(x);
    x += 4;
    v128_t vacc = wasm_f32x4_mul(vx, vslope);
    const v128_t vmask = wasm_i32x4_shr(vx, 31);
    vacc = wasm_v128_bitselect(vacc, vx, vmask);
    wasm_v128_store(y, vacc);
    y += 4;
  }
  if XNN_UNLIKELY(n != 0) {
    const v128_t vx = wasm_v128_load(x);
    v128_t vacc = wasm_f32x4_mul(vx, vslope);
    const v128_t vmask = wasm_i32x4_shr(vx, 31);
    vacc = wasm_v128_bitselect(vacc, vx, vmask);

    if (n & (2 * sizeof(float))) {
      *((double*) y) = wasm_f64x2_extract_lane(vacc, 0);
      vacc = wasm_v32x4_shuffle(vacc, vacc, 2, 3, 2, 3);
      y += 2;
    }
    if (n & (1 * sizeof(float))) {
      *y = wasm_f32x4_extract_lane(vacc, 0);
    }
  }
}
