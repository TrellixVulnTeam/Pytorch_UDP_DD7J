// Auto-generated file. Do not edit!
//   Template: src/f16-ibilinear/neonfp16arith.c.in
//   Generator: tools/xngen
//
// Copyright 2022 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include <arm_neon.h>

#include <xnnpack/common.h>
#include <xnnpack/ibilinear.h>


void xnn_f16_ibilinear_ukernel__neonfp16arith_c16(
    size_t output_pixels,
    size_t channels,
    const void**restrict input,
    size_t input_offset,
    const void*restrict weights,
    void*restrict output,
    size_t output_increment) XNN_OOB_READS
{
  assert(output_pixels != 0);
  assert(channels != 0);
  assert(channels % sizeof(__fp16) == 0);

  __fp16* o = (__fp16*) output;
  do {
    const __fp16* i0 = (const __fp16*) ((uintptr_t) input[0] + input_offset);
    const __fp16* i1 = (const __fp16*) ((uintptr_t) input[1] + input_offset);
    const __fp16* i2 = (const __fp16*) ((uintptr_t) input[2] + input_offset);
    const __fp16* i3 = (const __fp16*) ((uintptr_t) input[3] + input_offset);
    input += 4;

    const float16x8_t valphah = vld1q_dup_f16(weights); weights = (const __fp16*) weights + 1;
    const float16x8_t valphav = vld1q_dup_f16(weights); weights = (const __fp16*) weights + 1;

    size_t c = channels;
    for (; c >= 16 * sizeof(__fp16); c -= 16 * sizeof(__fp16)) {
      const float16x8_t vtl456789AB = vld1q_f16(i0); i0 += 8;
      const float16x8_t vtr456789AB = vld1q_f16(i1); i1 += 8;
      const float16x8_t vbl456789AB = vld1q_f16(i2); i2 += 8;
      const float16x8_t vbr456789AB = vld1q_f16(i3); i3 += 8;
      const float16x8_t vtlCDEFGHIJ = vld1q_f16(i0); i0 += 8;
      const float16x8_t vtrCDEFGHIJ = vld1q_f16(i1); i1 += 8;
      const float16x8_t vblCDEFGHIJ = vld1q_f16(i2); i2 += 8;
      const float16x8_t vbrCDEFGHIJ = vld1q_f16(i3); i3 += 8;

      const float16x8_t vtd456789AB = vsubq_f16(vtr456789AB, vtl456789AB);
      const float16x8_t vbd456789AB = vsubq_f16(vbr456789AB, vbl456789AB);
      const float16x8_t vtdCDEFGHIJ = vsubq_f16(vtrCDEFGHIJ, vtlCDEFGHIJ);
      const float16x8_t vbdCDEFGHIJ = vsubq_f16(vbrCDEFGHIJ, vblCDEFGHIJ);

      const float16x8_t vt456789AB = vfmaq_f16(vtl456789AB, vtd456789AB, valphah);
      const float16x8_t vb456789AB = vfmaq_f16(vbl456789AB, vbd456789AB, valphah);
      const float16x8_t vtCDEFGHIJ = vfmaq_f16(vtlCDEFGHIJ, vtdCDEFGHIJ, valphah);
      const float16x8_t vbCDEFGHIJ = vfmaq_f16(vblCDEFGHIJ, vbdCDEFGHIJ, valphah);

      const float16x8_t vd456789AB = vsubq_f16(vb456789AB, vt456789AB);
      const float16x8_t vdCDEFGHIJ = vsubq_f16(vbCDEFGHIJ, vtCDEFGHIJ);

      const float16x8_t vo456789AB = vfmaq_f16(vt456789AB, vd456789AB, valphav);
      const float16x8_t voCDEFGHIJ = vfmaq_f16(vtCDEFGHIJ, vdCDEFGHIJ, valphav);

      vst1q_f16(o, vo456789AB); o += 8;
      vst1q_f16(o, voCDEFGHIJ); o += 8;
    }
    for (; c >= 8 * sizeof(__fp16); c -= 8 * sizeof(__fp16)) {
      const float16x8_t vtl = vld1q_f16(i0); i0 += 8;
      const float16x8_t vtr = vld1q_f16(i1); i1 += 8;
      const float16x8_t vbl = vld1q_f16(i2); i2 += 8;
      const float16x8_t vbr = vld1q_f16(i3); i3 += 8;

      const float16x8_t vtd = vsubq_f16(vtr, vtl);
      const float16x8_t vbd = vsubq_f16(vbr, vbl);

      const float16x8_t vt = vfmaq_f16(vtl, vtd, valphah);
      const float16x8_t vb = vfmaq_f16(vbl, vbd, valphah);

      const float16x8_t vd = vsubq_f16(vb, vt);

      const float16x8_t vo = vfmaq_f16(vt, vd, valphav);

      vst1q_f16(o, vo); o += 8;
    }
    if XNN_UNLIKELY(c != 0) {
      const float16x8_t vtl = vld1q_f16(i0);
      const float16x8_t vtr = vld1q_f16(i1);
      const float16x8_t vbl = vld1q_f16(i2);
      const float16x8_t vbr = vld1q_f16(i3);

      const float16x8_t vtd = vsubq_f16(vtr, vtl);
      const float16x8_t vbd = vsubq_f16(vbr, vbl);

      const float16x8_t vt = vfmaq_f16(vtl, vtd, valphah);
      const float16x8_t vb = vfmaq_f16(vbl, vbd, valphah);

      const float16x8_t vd = vsubq_f16(vb, vt);

      float16x8_t vo = vfmaq_f16(vt, vd, valphav);

      float16x4_t vo_lo = vget_low_f16(vo);
      if (c & (4 * sizeof(__fp16))) {
        vst1_f16(o, vo_lo); o += 4;
        vo_lo = vget_high_f16(vo);
      }
      if (c & (2 * sizeof(__fp16))) {
        vst1_lane_u32(o, vreinterpret_u32_f16(vo_lo), 0); o += 2;
        vo_lo = vext_f16(vo_lo, vo_lo, 2);
      }
      if (c & (1 * sizeof(__fp16))) {
        vst1_lane_f16(o, vo_lo, 0); o += 1;
      }
    }

    o = (__fp16*) ((uintptr_t) o + output_increment);
  } while (--output_pixels != 0);
}
