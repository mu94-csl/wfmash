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
 * DESCRIPTION: Individual WaveFront data structure
 */

#include "WFA/wavefront/wavefront.h"

#ifdef WFA_NAMESPACE
namespace wfa {
#endif

/*
 * Setup
 */
void wavefront_allocate(
    wavefront_t* const wavefront,
    const int wf_elements_allocated,
    const bool allocate_backtrace,
    mm_allocator_t* const mm_allocator) {
  // Allocate memory
  wavefront->wf_elements_allocated = wf_elements_allocated;
  wavefront->offsets_mem = mm_allocator_calloc(
      mm_allocator,wf_elements_allocated,wf_offset_t,false);
  if (allocate_backtrace) {
    wavefront->bt_pcigar_mem = mm_allocator_calloc(
        mm_allocator,wf_elements_allocated,pcigar_t,false);
    wavefront->bt_prev_mem = mm_allocator_calloc(
        mm_allocator,wf_elements_allocated,block_idx_t,false);
  } else {
    wavefront->bt_pcigar_mem = NULL;
  }
}
void wavefront_resize(
    wavefront_t* const wavefront,
    const int wf_elements_allocated,
    mm_allocator_t* const mm_allocator) {
  // Set new size
  wavefront->wf_elements_allocated = wf_elements_allocated;
  // Reallocate offsets (Content is lost)
  mm_allocator_free(mm_allocator,wavefront->offsets_mem);
  wavefront->offsets_mem = mm_allocator_calloc(
      mm_allocator,wf_elements_allocated,wf_offset_t,false);
  // Reallocate backtrace (Content is lost)
  if (wavefront->bt_pcigar_mem) {
    mm_allocator_free(mm_allocator,wavefront->bt_pcigar_mem);
    mm_allocator_free(mm_allocator,wavefront->bt_prev_mem);
    wavefront->bt_pcigar_mem = mm_allocator_calloc(
        mm_allocator,wf_elements_allocated,pcigar_t,false);
    wavefront->bt_prev_mem = mm_allocator_calloc(
        mm_allocator,wf_elements_allocated,block_idx_t,false);
  }
}
void wavefront_free(
    wavefront_t* const wavefront,
    mm_allocator_t* const mm_allocator) {
  mm_allocator_free(mm_allocator,wavefront->offsets_mem);
  if (wavefront->bt_pcigar_mem) {
    mm_allocator_free(mm_allocator,wavefront->bt_pcigar_mem);
    mm_allocator_free(mm_allocator,wavefront->bt_prev_mem);
  }
}
/*
 * Initialization
 */
void wavefront_init(
    wavefront_t* const wavefront,
    const int lo,
    const int hi) {
  // Configure limits
  wavefront->null = false;
  wavefront->lo = lo;
  wavefront->hi = hi;
  wavefront->k_alignment_end = WAVEFRONT_DIAGONAL_NULL;
  // Setup elements
  wavefront->offsets = wavefront->offsets_mem - lo; // Center at k=0
  if (wavefront->bt_pcigar_mem) {
    wavefront->bt_pcigar = wavefront->bt_pcigar_mem - lo; // Center at k=0
    wavefront->bt_prev = wavefront->bt_prev_mem - lo; // Center at k=0
  }
  // Internals
  wavefront->wf_elements_used_min = lo;
  wavefront->wf_elements_used_max = hi;
}
void wavefront_init_null(
    wavefront_t* const wavefront,
    const int lo,
    const int hi) {
  // Configure limits
  wavefront->null = true;
  wavefront->lo =  1;
  wavefront->hi = -1;
  wavefront->k_alignment_end = WAVEFRONT_DIAGONAL_NULL;
  // Setup elements
  wavefront->offsets = wavefront->offsets_mem - lo; // Center at k=0
  if (wavefront->bt_pcigar_mem) {
    wavefront->bt_pcigar = wavefront->bt_pcigar_mem - lo; // Center at k=0
    wavefront->bt_prev = wavefront->bt_prev_mem - lo; // Center at k=0
  }
  // Initialize
  const int wf_elements_allocated = wavefront->wf_elements_allocated;
  int i;
  for (i=0;i<wf_elements_allocated;++i) {
    wavefront->offsets_mem[i] = WAVEFRONT_OFFSET_NULL;
  }
  if (wavefront->bt_pcigar_mem) {
      memset(wavefront->bt_pcigar_mem,0,wf_elements_allocated*sizeof(pcigar_t));
      memset(wavefront->bt_prev_mem,0,wf_elements_allocated*sizeof(block_idx_t));
  }
  // Internals
  wavefront->wf_elements_used_min = lo; // To keep track of limits
  wavefront->wf_elements_used_max = hi; // To keep track of limits
}
void wavefront_init_victim(
    wavefront_t* const wavefront,
    const int lo,
    const int hi) {
  // Configure limits
  wavefront->null = true;
  wavefront->lo =  1;
  wavefront->hi = -1;
  wavefront->k_alignment_end = WAVEFRONT_DIAGONAL_NULL;
  // Setup offsets
  wavefront->offsets = wavefront->offsets_mem - lo; // Center at k=0
  if (wavefront->bt_pcigar_mem) {
    wavefront->bt_pcigar = wavefront->bt_pcigar_mem - lo; // Center at k=0
    wavefront->bt_prev = wavefront->bt_prev_mem - lo; // Center at k=0
  }
  // Internals
  wavefront->wf_elements_used_min = lo; // To keep track of limits
  wavefront->wf_elements_used_max = hi; // To keep track of limits
}
/*
 * Utils
 */
uint64_t wavefront_get_size(
    wavefront_t* const wavefront) {
  uint64_t total_size = wavefront->wf_elements_allocated*sizeof(wf_offset_t);
  if (wavefront->bt_pcigar_mem) {
    total_size += wavefront->wf_elements_allocated*(sizeof(pcigar_t)+sizeof(block_idx_t));
  }
  return total_size;
}

#ifdef WFA_NAMESPACE
}
#endif


