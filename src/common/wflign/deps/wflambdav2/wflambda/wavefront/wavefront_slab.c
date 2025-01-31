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
 * DESCRIPTION: WaveFront Slab for fast pre-allocated wavefronts' memory handling
 */

#include "wflambda/wavefront/wavefront_slab.h"

#ifdef WFLAMBDA_NAMESPACE
namespace wflambda {
#endif

/*
 * Constants
 */
#define WF_SLAB_EXPAND_FACTOR    (3.0f/2.0f)
#define WF_SLAB_SIZE_INIT        100

/*
 * Slab Setup
 */
wavefront_slab_t* wavefront_slab_new(
    const int init_max_wavefront_elements,
    const bool allocate_backtrace,
    mm_allocator_t* const mm_allocator) {
  // Allocate
  wavefront_slab_t* const wavefront_slab =
      mm_allocator_alloc(mm_allocator,wavefront_slab_t);
  // Attributes
  wavefront_slab->allocate_backtrace = allocate_backtrace;
  // Wavefront Slabs
  wavefront_slab->max_wavefront_elements = init_max_wavefront_elements;
  wavefront_slab->wavefronts = vector_new(WF_SLAB_SIZE_INIT,wavefront_t*);
  wavefront_slab->wavefronts_free = vector_new(WF_SLAB_SIZE_INIT,wavefront_t*);
  // MM
  wavefront_slab->mm_allocator = mm_allocator;
  // Return
  return wavefront_slab;
}
void wavefront_slab_resize(
    wavefront_slab_t* const wavefront_slab,
    const int max_wavefront_elements) {
  // DEBUG
  //  const int element_size = sizeof(wf_offset_t)+sizeof(pcigar_t)+sizeof(block_idx_t);
  //  fprintf(stderr,"Resizing wavefronts to %d elements (%ld MB)\n",
  //      max_wavefront_elements,
  //      CONVERT_B_TO_MB(max_wavefront_elements*element_size));
  // Parameters
  mm_allocator_t* const mm_allocator = wavefront_slab->mm_allocator;
  // Deallocate short wavefronts (unfit)
  if (wavefront_slab->max_wavefront_elements < max_wavefront_elements) {
    wavefront_t** const wavefronts_free =
        vector_get_mem(wavefront_slab->wavefronts_free,wavefront_t*);
    const int num_wavefronts_free = vector_get_used(wavefront_slab->wavefronts_free);
    int i;
    for (i=0;i<num_wavefronts_free;++i) {
      wavefronts_free[i]->status = wavefront_status_deallocated;
      wavefront_free(wavefronts_free[i],mm_allocator);
    }
    vector_clear(wavefront_slab->wavefronts_free);
  }
  // Set new max-elements wavefront length
  wavefront_slab->max_wavefront_elements = max_wavefront_elements;
}
void wavefront_slab_clear(
    wavefront_slab_t* const wavefront_slab,
    const bool deallocate_oversized) {
  // Parameters
  const int max_wavefront_elements = wavefront_slab->max_wavefront_elements;
  mm_allocator_t* const mm_allocator = wavefront_slab->mm_allocator;
  // Clear free
  vector_clear(wavefront_slab->wavefronts_free);
  // Strict adjustment of wavefronts (No matter the status: busy, free, etc)
  wavefront_t** const wavefronts =
      vector_get_mem(wavefront_slab->wavefronts,wavefront_t*);
  const int num_wavefronts = vector_get_used(wavefront_slab->wavefronts);
  int i, valid_idx = 0;
  for (i=0;i<num_wavefronts;++i) {
    // Check wavefront status
    if (wavefronts[i]->status == wavefront_status_deallocated) {
      mm_allocator_free(mm_allocator,wavefronts[i]); // Delete handler
    } else {
      // Busy or free
      const bool unfit = wavefronts[i]->max_wavefront_elements < max_wavefront_elements;
      const bool oversized =  wavefronts[i]->max_wavefront_elements > max_wavefront_elements;
      if (unfit || (oversized && deallocate_oversized)) {
        wavefront_free(wavefronts[i],mm_allocator); // Free wavefront
        mm_allocator_free(mm_allocator,wavefronts[i]); // Delete handler
      } else {
        wavefronts[valid_idx++] = wavefronts[i]; // Valid wavefront
        wavefronts[i]->status = wavefront_status_free; // Set free
        vector_insert(wavefront_slab->wavefronts_free,wavefronts[i],wavefront_t*);
      }
    }
  }
  vector_set_used(wavefront_slab->wavefronts,valid_idx);
}
void wavefront_slab_delete(
    wavefront_slab_t* const wavefront_slab) {
  // Parameters
  mm_allocator_t* const mm_allocator = wavefront_slab->mm_allocator;
  // Delete free vector
  vector_delete(wavefront_slab->wavefronts_free);
  // Free wavefronts
  wavefront_t** const wavefronts =
      vector_get_mem(wavefront_slab->wavefronts,wavefront_t*);
  const int num_wavefronts = vector_get_used(wavefront_slab->wavefronts);
  int i;
  for (i=0;i<num_wavefronts;++i) {
    if (wavefronts[i]->status == wavefront_status_deallocated) {
      mm_allocator_free(mm_allocator,wavefronts[i]); // Delete handler
    } else {
      wavefront_free(wavefronts[i],mm_allocator); // Free wavefront
      mm_allocator_free(mm_allocator,wavefronts[i]); // Delete handler
    }
  }
  vector_delete(wavefront_slab->wavefronts);
  // Handler
  mm_allocator_free(wavefront_slab->mm_allocator,wavefront_slab);
}
/*
 * Slab Allocator
 */
wavefront_t* wavefront_slab_allocate(
    wavefront_slab_t* const wavefront_slab,
    const int lo,
    const int hi) {
  // Parameters
  vector_t* const wavefronts_free = wavefront_slab->wavefronts_free;
  const int wavefront_length = hi - lo + 2;
  // Check max-length of pre-allocated wavefronts
  if (wavefront_length > wavefront_slab->max_wavefront_elements) {
    // Compute new slab size & redim
    const int proposed_length = (float)wavefront_length * WF_SLAB_EXPAND_FACTOR;
    wavefront_slab_resize(wavefront_slab,proposed_length);
  }
  // Check for a free wavefront (pre-allocated in the slab)
  wavefront_t* wavefront;
  if (vector_get_used(wavefronts_free) > 0) {
    // Reuse wavefront
    wavefront = *(vector_get_last_elm(wavefronts_free,wavefront_t*));
    vector_dec_used(wavefronts_free);
  } else {
    // Allocate new wavefront
    wavefront = mm_allocator_alloc(wavefront_slab->mm_allocator,wavefront_t);
    wavefront_allocate(wavefront,wavefront_slab->max_wavefront_elements,
        wavefront_slab->allocate_backtrace,wavefront_slab->mm_allocator);
    vector_insert(wavefront_slab->wavefronts,wavefront,wavefront_t*);
  }
  // Init wavefront
  wavefront->status = wavefront_status_busy;
  wavefront_init(wavefront,lo,hi);
  // Return
  return wavefront;
}
void wavefront_slab_free(
    wavefront_slab_t* const wavefront_slab,
    wavefront_t* const wavefront) {
  // Check wavefront length
  if (wavefront->max_wavefront_elements >= wavefront_slab->max_wavefront_elements) {
    // Return wavefront to slab as free
    wavefront->status = wavefront_status_free;
    vector_insert(wavefront_slab->wavefronts_free,wavefront,wavefront_t*);
  } else {
    // Delete wavefront
    wavefront->status = wavefront_status_deallocated;
    wavefront_free(wavefront,wavefront_slab->mm_allocator);
  }
}

#ifdef WFLAMBDA_NAMESPACE
}
#endif
