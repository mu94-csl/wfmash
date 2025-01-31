/*
 *                             The MIT License
 *
 * Wavefront Alignments Algorithms
 * Copyright (c) 2017 by Santiago Marco-Sola  <santiagomsola@gmail.com>
 *
 * This file is part of Wavefront Alignments Algorithms.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * PROJECT: Wavefront Alignments Algorithms
 * AUTHOR(S): Santiago Marco-Sola <santiagomsola@gmail.com>
 * DESCRIPTION: Dynamic-programming algorithm for computing gap-affine
 *   pairwise alignment (Smith-Waterman-Gotoh - SWG)
 */

#pragma once

#include "WFA/utils/commons.h"
#include "WFA/gap_affine/affine_matrix.h"

#ifdef WFA_NAMESPACE
namespace wfa {
#endif

/*
 * SWG alignment
 */
    void swg_compute(
            affine_matrix_t* const affine_matrix,
            affine_penalties_t* const penalties,
            const char* const pattern,
            const int pattern_length,
            const char* const text,
            const int text_length,
            cigar_t* const cigar);

/*
 * SWG alignment (ends-free)
 */
    void swg_compute_endsfree(
            affine_matrix_t* const affine_matrix,
            affine_penalties_t* const penalties,
            const char* const pattern,
            const int pattern_length,
            const char* const text,
            const int text_length,
            const int pattern_begin_free,
            const int pattern_end_free,
            const int text_begin_free,
            const int text_end_free,
            cigar_t* const cigar);

/*
 * SWG alignment (banded)
 */
    void swg_compute_banded(
            affine_matrix_t* const affine_matrix,
            affine_penalties_t* const penalties,
            const char* const pattern,
            const int pattern_length,
            const char* const text,
            const int text_length,
            const int bandwidth,
            cigar_t* const cigar);

#ifdef WFA_NAMESPACE
}
#endif
