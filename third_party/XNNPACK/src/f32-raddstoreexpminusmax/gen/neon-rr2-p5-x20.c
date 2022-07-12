// Auto-generated file. Do not edit!
//   Template: src/f32-raddstoreexpminusmax/neon-p5.c.in
//   Generator: tools/xngen
//
// Copyright 2020 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include <arm_neon.h>

#include <xnnpack/common.h>
#include <xnnpack/raddstoreexpminusmax.h>


void xnn_f32_raddstoreexpminusmax_ukernel__neon_rr2_p5_x20(
    size_t elements,
    const float* input,
    const float* max,
    float* output,
    float* sum,
    const union xnn_f32_expminus_params params[restrict XNN_MIN_ELEMENTS(1)]) XNN_OOB_READS
{
  assert(elements % sizeof(float) == 0);

  const float32x4_t vi_max = vld1q_dup_f32(max);
  const float32x4_t vlog2e = vld1q_dup_f32(&params->neon_rr2_p5.log2e);
  const float32x4_t vmagic_bias = vld1q_dup_f32(&params->neon_rr2_p5.magic_bias);
  const float32x4_t vminus_ln2_hi = vld1q_dup_f32(&params->neon_rr2_p5.minus_ln2_hi);
  const float32x4_t vminus_ln2_lo = vld1q_dup_f32(&params->neon_rr2_p5.minus_ln2_lo);
  const float32x4_t vc5 = vld1q_dup_f32(&params->neon_rr2_p5.c5);
  const float32x4_t vc4 = vld1q_dup_f32(&params->neon_rr2_p5.c4);
  const float32x4_t vc3 = vld1q_dup_f32(&params->neon_rr2_p5.c3);
  const float32x4_t vc2 = vld1q_dup_f32(&params->neon_rr2_p5.c2);
  const float32x4_t vc1 = vld1q_dup_f32(&params->neon_rr2_p5.c1);
  const float32x4_t vdenorm_cutoff = vld1q_dup_f32(&params->neon_rr2_p5.denorm_cutoff);

  float32x4_t vacc0 = vmovq_n_f32(0.0f);
  for (; elements >= 20 * sizeof(float); elements -= 20 * sizeof(float)) {
    const float32x4_t vi0123 = vld1q_f32(input); input += 4;
    const float32x4_t vi4567 = vld1q_f32(input); input += 4;
    const float32x4_t vi89AB = vld1q_f32(input); input += 4;
    const float32x4_t viCDEF = vld1q_f32(input); input += 4;
    const float32x4_t viGHIJ = vld1q_f32(input); input += 4;

    const float32x4_t vx0123 = vsubq_f32(vi0123, vi_max);
    const float32x4_t vx4567 = vsubq_f32(vi4567, vi_max);
    const float32x4_t vx89AB = vsubq_f32(vi89AB, vi_max);
    const float32x4_t vxCDEF = vsubq_f32(viCDEF, vi_max);
    const float32x4_t vxGHIJ = vsubq_f32(viGHIJ, vi_max);

    float32x4_t vn0123 = vmlaq_f32(vmagic_bias, vx0123, vlog2e);
    float32x4_t vn4567 = vmlaq_f32(vmagic_bias, vx4567, vlog2e);
    float32x4_t vn89AB = vmlaq_f32(vmagic_bias, vx89AB, vlog2e);
    float32x4_t vnCDEF = vmlaq_f32(vmagic_bias, vxCDEF, vlog2e);
    float32x4_t vnGHIJ = vmlaq_f32(vmagic_bias, vxGHIJ, vlog2e);

    const float32x4_t vs0123 = vreinterpretq_f32_s32(vshlq_n_s32(vreinterpretq_s32_f32(vn0123), 23));
    const float32x4_t vs4567 = vreinterpretq_f32_s32(vshlq_n_s32(vreinterpretq_s32_f32(vn4567), 23));
    const float32x4_t vs89AB = vreinterpretq_f32_s32(vshlq_n_s32(vreinterpretq_s32_f32(vn89AB), 23));
    const float32x4_t vsCDEF = vreinterpretq_f32_s32(vshlq_n_s32(vreinterpretq_s32_f32(vnCDEF), 23));
    const float32x4_t vsGHIJ = vreinterpretq_f32_s32(vshlq_n_s32(vreinterpretq_s32_f32(vnGHIJ), 23));

    vn0123 = vsubq_f32(vn0123, vmagic_bias);
    vn4567 = vsubq_f32(vn4567, vmagic_bias);
    vn89AB = vsubq_f32(vn89AB, vmagic_bias);
    vnCDEF = vsubq_f32(vnCDEF, vmagic_bias);
    vnGHIJ = vsubq_f32(vnGHIJ, vmagic_bias);

    float32x4_t vt0123 = vmlaq_f32(vx0123, vn0123, vminus_ln2_hi);
    float32x4_t vt4567 = vmlaq_f32(vx4567, vn4567, vminus_ln2_hi);
    float32x4_t vt89AB = vmlaq_f32(vx89AB, vn89AB, vminus_ln2_hi);
    float32x4_t vtCDEF = vmlaq_f32(vxCDEF, vnCDEF, vminus_ln2_hi);
    float32x4_t vtGHIJ = vmlaq_f32(vxGHIJ, vnGHIJ, vminus_ln2_hi);

    vt0123 = vmlaq_f32(vt0123, vn0123, vminus_ln2_lo);
    vt4567 = vmlaq_f32(vt4567, vn4567, vminus_ln2_lo);
    vt89AB = vmlaq_f32(vt89AB, vn89AB, vminus_ln2_lo);
    vtCDEF = vmlaq_f32(vtCDEF, vnCDEF, vminus_ln2_lo);
    vtGHIJ = vmlaq_f32(vtGHIJ, vnGHIJ, vminus_ln2_lo);

    float32x4_t vp0123 = vmlaq_f32(vc4, vc5, vt0123);
    float32x4_t vp4567 = vmlaq_f32(vc4, vc5, vt4567);
    float32x4_t vp89AB = vmlaq_f32(vc4, vc5, vt89AB);
    float32x4_t vpCDEF = vmlaq_f32(vc4, vc5, vtCDEF);
    float32x4_t vpGHIJ = vmlaq_f32(vc4, vc5, vtGHIJ);

    vp0123 = vmlaq_f32(vc3, vp0123, vt0123);
    vp4567 = vmlaq_f32(vc3, vp4567, vt4567);
    vp89AB = vmlaq_f32(vc3, vp89AB, vt89AB);
    vpCDEF = vmlaq_f32(vc3, vpCDEF, vtCDEF);
    vpGHIJ = vmlaq_f32(vc3, vpGHIJ, vtGHIJ);

    vp0123 = vmlaq_f32(vc2, vp0123, vt0123);
    vp4567 = vmlaq_f32(vc2, vp4567, vt4567);
    vp89AB = vmlaq_f32(vc2, vp89AB, vt89AB);
    vpCDEF = vmlaq_f32(vc2, vpCDEF, vtCDEF);
    vpGHIJ = vmlaq_f32(vc2, vpGHIJ, vtGHIJ);

    vp0123 = vmlaq_f32(vc1, vp0123, vt0123);
    vp4567 = vmlaq_f32(vc1, vp4567, vt4567);
    vp89AB = vmlaq_f32(vc1, vp89AB, vt89AB);
    vpCDEF = vmlaq_f32(vc1, vpCDEF, vtCDEF);
    vpGHIJ = vmlaq_f32(vc1, vpGHIJ, vtGHIJ);

    vt0123 = vmulq_f32(vt0123, vs0123);
    vt4567 = vmulq_f32(vt4567, vs4567);
    vt89AB = vmulq_f32(vt89AB, vs89AB);
    vtCDEF = vmulq_f32(vtCDEF, vsCDEF);
    vtGHIJ = vmulq_f32(vtGHIJ, vsGHIJ);

    float32x4_t vf0123 = vmlaq_f32(vs0123, vp0123, vt0123);
    float32x4_t vf4567 = vmlaq_f32(vs4567, vp4567, vt4567);
    float32x4_t vf89AB = vmlaq_f32(vs89AB, vp89AB, vt89AB);
    float32x4_t vfCDEF = vmlaq_f32(vsCDEF, vpCDEF, vtCDEF);
    float32x4_t vfGHIJ = vmlaq_f32(vsGHIJ, vpGHIJ, vtGHIJ);

    vf0123 = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vf0123), vcltq_f32(vx0123, vdenorm_cutoff)));
    vf4567 = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vf4567), vcltq_f32(vx4567, vdenorm_cutoff)));
    vf89AB = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vf89AB), vcltq_f32(vx89AB, vdenorm_cutoff)));
    vfCDEF = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vfCDEF), vcltq_f32(vxCDEF, vdenorm_cutoff)));
    vfGHIJ = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vfGHIJ), vcltq_f32(vxGHIJ, vdenorm_cutoff)));

    vst1q_f32(output, vf0123); output += 4;
    vst1q_f32(output, vf4567); output += 4;
    vst1q_f32(output, vf89AB); output += 4;
    vst1q_f32(output, vfCDEF); output += 4;
    vst1q_f32(output, vfGHIJ); output += 4;

    vacc0 = vaddq_f32(vacc0, vf0123);
    vacc0 = vaddq_f32(vacc0, vf4567);
    vacc0 = vaddq_f32(vacc0, vf89AB);
    vacc0 = vaddq_f32(vacc0, vfCDEF);
    vacc0 = vaddq_f32(vacc0, vfGHIJ);
  }

  float32x4_t vacc = vacc0;
  for (; elements >= 4 * sizeof(float); elements -= 4 * sizeof(float)) {
    const float32x4_t vi = vld1q_f32(input); input += 4;

    const float32x4_t vx = vsubq_f32(vi, vi_max);

    float32x4_t vn = vmlaq_f32(vmagic_bias, vx, vlog2e);

    const float32x4_t vs = vreinterpretq_f32_s32(vshlq_n_s32(vreinterpretq_s32_f32(vn), 23));

    vn = vsubq_f32(vn, vmagic_bias);

    float32x4_t vt = vmlaq_f32(vx, vn, vminus_ln2_hi);
    vt = vmlaq_f32(vt, vn, vminus_ln2_lo);

    float32x4_t vp = vmlaq_f32(vc4, vc5, vt);
    vp = vmlaq_f32(vc3, vp, vt);
    vp = vmlaq_f32(vc2, vp, vt);
    vp = vmlaq_f32(vc1, vp, vt);

    vt = vmulq_f32(vt, vs);
    float32x4_t vf = vmlaq_f32(vs, vp, vt);

    vf = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vf), vcltq_f32(vx, vdenorm_cutoff)));

    vst1q_f32(output, vf); output += 4;

    vacc = vaddq_f32(vacc, vf);
  }
#if XNN_ARCH_ARM64
  float vacc_lo = vaddvq_f32(vacc);
#else
  float32x2_t vacc_lo = vadd_f32(vget_high_f32(vacc), vget_low_f32(vacc));
#endif
  if (elements != 0) {
    assert(elements >= 1 * sizeof(float));
    assert(elements <= 3 * sizeof(float));
    const float32x4_t vi = vld1q_f32(input); input += 4;

    const float32x4_t vx = vsubq_f32(vi, vi_max);

    float32x4_t vn = vmlaq_f32(vmagic_bias, vx, vlog2e);

    const float32x4_t vs = vreinterpretq_f32_s32(vshlq_n_s32(vreinterpretq_s32_f32(vn), 23));

    vn = vsubq_f32(vn, vmagic_bias);

    float32x4_t vt = vmlaq_f32(vx, vn, vminus_ln2_hi);
    vt = vmlaq_f32(vt, vn, vminus_ln2_lo);

    float32x4_t vp = vmlaq_f32(vc4, vc5, vt);
    vp = vmlaq_f32(vc3, vp, vt);
    vp = vmlaq_f32(vc2, vp, vt);
    vp = vmlaq_f32(vc1, vp, vt);

    vt = vmulq_f32(vt, vs);
    float32x4_t vf = vmlaq_f32(vs, vp, vt);

    vf = vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(vf), vcltq_f32(vx, vdenorm_cutoff)));

    float32x2_t vf_lo = vget_low_f32(vf);
    if (elements & (2 * sizeof(float))) {
      vst1_f32(output, vf_lo); output += 2;

      #if XNN_ARCH_ARM64
        vacc_lo += vaddv_f32(vf_lo);
      #else
        vacc_lo = vadd_f32(vacc_lo, vf_lo);
      #endif

      vf_lo = vget_high_f32(vf);
    }
    if (elements & (1 * sizeof(float))) {
      vst1_lane_f32(output, vf_lo, 0);

      #if XNN_ARCH_ARM64
        vacc_lo += vget_lane_f32(vf_lo, 0);
      #else
        vacc_lo = vadd_f32(vacc_lo, vreinterpret_f32_u64(vshl_n_u64(vreinterpret_u64_f32(vf_lo), 32)));
      #endif
    }
  }
#if XNN_ARCH_ARM64
  *sum = vacc_lo;
#else
  vst1_lane_f32(sum, vpadd_f32(vacc_lo, vacc_lo), 0);
#endif
}
