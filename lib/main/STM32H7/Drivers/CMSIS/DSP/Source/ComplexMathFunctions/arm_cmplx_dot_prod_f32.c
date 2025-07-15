/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_dot_prod_f32.c
 * Description:  Floating-point complex dot product
 *
 * $Date:        18. March 2019
 * $Revision:    V1.6.0
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2019 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arm_math.h"

/**
  @ingroup groupCmplxMath
 */

/**
  @defgroup cmplx_dot_prod Complex Dot Product

  Computes the dot product of two complex vectors.
  The vectors are multiplied element-by-element and then summed.

  The <code>pSrcA</code> points to the first complex input vector and
  <code>pSrcB</code> points to the second complex input vector.
  <code>numSamples</code> specifies the number of complex samples
  and the data in each array is stored in an interleaved fashion
  (real, imag, real, imag, ...).
  Each array has a total of <code>2*numSamples</code> values.

  The underlying algorithm is used:

  <pre>
  realResult = 0;
  imagResult = 0;
  for (n = 0; n < numSamples; n++) {
      realResult += pSrcA[(2*n)+0] * pSrcB[(2*n)+0] - pSrcA[(2*n)+1] * pSrcB[(2*n)+1];
      imagResult += pSrcA[(2*n)+0] * pSrcB[(2*n)+1] + pSrcA[(2*n)+1] * pSrcB[(2*n)+0];
  }
  </pre>

  There are separate functions for floating-point, Q15, and Q31 data types.
 */

/**
  @addtogroup cmplx_dot_prod
  @{
 */

/**
  @brief         Floating-point complex dot product.
  @param[in]     pSrcA       points to the first input vector
  @param[in]     pSrcB       points to the second input vector
  @param[in]     numSamples  number of samples in each vector
  @param[out]    realResult  real part of the result returned here
  @param[out]    imagResult  imaginary part of the result returned here
  @return        none
 */

void arm_cmplx_dot_prod_f32(
  const float32_t * pSrcA,
  const float32_t * pSrcB,
        uint32_t numSamples,
        float32_t * realResult,
        float32_t * imagResult)
{
        uint32_t blkCnt;                               /* Loop counter */
        float32_t real_sum = 0.0f, imag_sum = 0.0f;    /* Temporary result variables */
        float32_t a0,b0,c0,d0;

#if defined(ARM_MATH_NEON)
    float32x4x2_t vec1,vec2,vec3,vec4;
    float32x4_t accR,accI;
    float32x2_t accum = vdup_n_f32(0);

    accR = vdupq_n_f32(0.0);
    accI = vdupq_n_f32(0.0);

    /* Loop unrolling: Compute 8 outputs at a time */
    blkCnt = numSamples >> 3U;

    while (blkCnt > 0U)
    {
	/* C = (A[0]+jA[1])*(B[0]+jB[1]) + ...  */
        /* Calculate dot product and then store the result in a temporary buffer. */

	vec1 = vld2q_f32(pSrcA);
        vec2 = vld2q_f32(pSrcB);

	/* Increment pointers */
        pSrcA += 8;
        pSrcB += 8;

	/* Re{C} = Re{A}*Re{B} - Im{A}*Im{B} */
        accR = vmlaq_f32(accR,vec1.val[0],vec2.val[0]);
        accR = vmlsq_f32(accR,vec1.val[1],vec2.val[1]);

	/* Im{C} = Re{A}*Im{B} + Im{A}*Re{B} */
        accI = vmlaq_f32(accI,vec1.val[1],vec2.val[0]);
        accI = vmlaq_f32(accI,vec1.val[0],vec2.val[1]);

        vec3 = vld2q_f32(pSrcA);
        vec4 = vld2q_f32(pSrcB);
	
	/* Increment pointers */
        pSrcA += 8;
        pSrcB += 8;

	/* Re{C} = Re{A}*Re{B} - Im{A}*Im{B} */
        accR = vmlaq_f32(accR,vec3.val[0],vec4.val[0]);
        accR = vmlsq_f32(accR,vec3.val[1],vec4.val[1]);

	/* Im{C} = Re{A}*Im{B} + Im{A}*Re{B} */
        accI = vmlaq_f32(accI,vec3.val[1],vec4.val[0]);
        accI = vmlaq_f32(accI,vec3.val[0],vec4.val[1]);

        /* Decrement the loop counter */
        blkCnt--;
    }

    accum = vpadd_f32(vget_low_f32(accR), vget_high_f32(accR));
    real_sum += accum[0] + accum[1];

    accum = vpadd_f32(vget_low_f32(accI), vget_high_f32(accI));
    imag_sum += accum[0] + accum[1];

    /* Tail */
    blkCnt = numSamples & 0x7;

#else
#if defined (ARM_MATH_LOOPUNROLL)

  /* Loop unrolling: Compute 4 outputs at a time */
  blkCnt = numSamples >> 2U;

  while (blkCnt > 0U)
  {
    a0 = *pSrcA++;
    b0 = *pSrcA++;
    c0 = *pSrcB++;
    d0 = *pSrcB++;

    real_sum += a0 * c0;
    imag_sum += a0 * d0;
    real_sum -= b0 * d0;
    imag_sum += b0 * c0;

    a0 = *pSrcA++;
    b0 = *pSrcA++;
    c0 = *pSrcB++;
    d0 = *pSrcB++;

    real_sum += a0 * c0;
    imag_sum += a0 * d0;
    real_sum -= b0 * d0;
    imag_sum += b0 * c0;

    a0 = *pSrcA++;
    b0 = *pSrcA++;
    c0 = *pSrcB++;
    d0 = *pSrcB++;

    real_sum += a0 * c0;
    imag_sum += a0 * d0;
    real_sum -= b0 * d0;
    imag_sum += b0 * c0;

    a0 = *pSrcA++;
    b0 = *pSrcA++;
    c0 = *pSrcB++;
    d0 = *pSrcB++;

    real_sum += a0 * c0;
    imag_sum += a0 * d0;
    real_sum -= b0 * d0;
    imag_sum += b0 * c0;

    /* Decrement loop counter */
    blkCnt--;
  }

  /* Loop unrolling: Compute remaining outputs */
  blkCnt = numSamples % 0x4U;

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = numSamples;

#endif /* #if defined (ARM_MATH_LOOPUNROLL) */
#endif /* #if defined(ARM_MATH_NEON) */

  while (blkCnt > 0U)
  {
    a0 = *pSrcA++;
    b0 = *pSrcA++;
    c0 = *pSrcB++;
    d0 = *pSrcB++;

    real_sum += a0 * c0;
    imag_sum += a0 * d0;
    real_sum -= b0 * d0;
    imag_sum += b0 * c0;

    /* Decrement loop counter */
    blkCnt--;
  }

  /* Store real and imaginary result in destination buffer. */
  *realResult = real_sum;
  *imagResult = imag_sum;
}

/**
  @} end of cmplx_dot_prod group
 */
