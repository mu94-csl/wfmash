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
 * DESCRIPTION: WaveFront-Alignment module for display and report
 */

#include "wflambda/wavefront/wavefront_display.h"
#include "wflambda/wavefront/wavefront_compute.h"

#ifdef WFLAMBDA_NAMESPACE
namespace wflambda {
#endif

#define WF_DISPLAY_YLABEL_LENGTH 8

/*
 * Compute dimensions
 */
int wavefront_display_compute_row_width(
    const distance_metric_t distance_metric,
    const int bt_length) {
  // Compute number of components
  int num_components = 1;
  switch (distance_metric) {
    case gap_affine: num_components = 3; break;
    case gap_affine_2p: num_components = 5; break;
    default: break;
  }
  // Return
  return ((num_components)*(4+(bt_length)));
}
void wavefront_display_compute_limits(
    wavefront_aligner_t* const wf_aligner,
    const int score_begin,
    const int score_end,
    int* const out_max_k,
    int* const out_min_k) {
  // Parameters
  const distance_metric_t distance_metric = wf_aligner->distance_metric;
  // Compute min/max k
  int i, max_k=0, min_k=0;
  for (i=score_begin;i<=score_end;++i) {
    const int s = (wf_aligner->memory_modular) ? i%wf_aligner->max_score_scope : i;
    wavefront_t* const mwavefront = wf_aligner->mwavefronts[s];
    if (mwavefront != NULL) {
      max_k = MAX(max_k,mwavefront->hi);
      min_k = MIN(min_k,mwavefront->lo);
    }
    if (distance_metric==edit || distance_metric==gap_lineal) continue;
    wavefront_t* const i1wavefront = wf_aligner->i1wavefronts[s];
    if (i1wavefront != NULL) {
      max_k = MAX(max_k,i1wavefront->hi);
      min_k = MIN(min_k,i1wavefront->lo);
    }
    wavefront_t* const d1wavefront = wf_aligner->d1wavefronts[s];
    if (d1wavefront != NULL) {
      max_k = MAX(max_k,d1wavefront->hi);
      min_k = MIN(min_k,d1wavefront->lo);
    }
    if (distance_metric==gap_affine) continue;
    wavefront_t* const i2wavefront = wf_aligner->i2wavefronts[s];
    if (i2wavefront != NULL) {
      max_k = MAX(max_k,i2wavefront->hi);
      min_k = MIN(min_k,i2wavefront->lo);
    }
    wavefront_t* const d2wavefront = wf_aligner->d2wavefronts[s];
    if (d2wavefront != NULL) {
      max_k = MAX(max_k,d2wavefront->hi);
      min_k = MIN(min_k,d2wavefront->lo);
    }
  }
  // Return
  *out_max_k = max_k;
  *out_min_k = min_k;
}
/*
 * Display components
 */
void wavefront_display_print_element(
    FILE* const stream,
    wavefront_aligner_t* const wf_aligner,
    wavefront_t* const wavefront,
    const int k,
    const int bt_length) {
  // Check limits
  if (wavefront!=NULL && wavefront->lo <= k && k <= wavefront->hi) {
    const wf_offset_t offset = wavefront->offsets[k];
    if (offset >= 0) {
      fprintf(stream,"[%2d]",(int)offset);
      // Print pcigar
      if (bt_length > 0) {
        char cigar_buffer[32];
        const int pcigar_length = pcigar_unpack(wavefront->bt_pcigar[k],cigar_buffer);
        const int effective_pcigar_length = MIN(bt_length,pcigar_length);
        fprintf(stream,"%.*s",effective_pcigar_length,cigar_buffer);
        PRINT_CHAR_REP(stream,' ',bt_length-effective_pcigar_length);
      }
    } else {
      fprintf(stream,"[  ]");
      PRINT_CHAR_REP(stream,' ',bt_length);
    }
  } else {
    PRINT_CHAR_REP(stream,' ',bt_length+4);
  }
}
void wavefront_display_print_frame(
    FILE* const stream,
    const int score_begin,
    const int score_end,
    const int row_width,
    const int bt_length) {
  // Align Y-label [k=000]
  PRINT_CHAR_REP(stream,' ',WF_DISPLAY_YLABEL_LENGTH);
  // Frame
  fprintf(stream,"+");
  const int num_blocks = score_end-score_begin+1;
  const int frame_length = (num_blocks*row_width) + (num_blocks-1);
  PRINT_CHAR_REP(stream,'-',frame_length);
  fprintf(stream,"+\n");
}
void wavefront_display_print_score(
    FILE* const stream,
    const int score_begin,
    const int score_end,
    const int row_width,
    const int bt_length) {
  // Score label
  PRINT_CHAR_REP(stream,' ',WF_DISPLAY_YLABEL_LENGTH);
  int s;
  for (s=score_begin;s<=score_end;++s) {
    fprintf(stream,"|");
    PRINT_CHAR_REP(stream,' ',row_width-10);
    fprintf(stream,"%4d-score",s);
  }
  fprintf(stream,"|\n");
}
void wavefront_display_print_header(
    FILE* const stream,
    const distance_metric_t distance_metric,
    const int score_begin,
    const int score_end,
    const int bt_length) {
  const int row_width = wavefront_display_compute_row_width(distance_metric,bt_length);
  // Score header
  fprintf(stream,"\n>[SCORE %d-%d]\n",score_begin,score_end);
  // Frame
  wavefront_display_print_frame(stream,score_begin,score_end,row_width,bt_length);
  // Score header
  wavefront_display_print_score(stream,score_begin,score_end,row_width,bt_length);
  // Frame
  wavefront_display_print_frame(stream,score_begin,score_end,row_width,bt_length);
  // Wavefront labels
  PRINT_CHAR_REP(stream,' ',WF_DISPLAY_YLABEL_LENGTH); // Align [k=   ]
  int s;
  for (s=score_begin;s<=score_end;++s) {
    fprintf(stream,"|");
    fprintf(stream,"[ M]");
    PRINT_CHAR_REP(stream,' ',bt_length);
    if (distance_metric==edit || distance_metric==gap_lineal) continue;
    fprintf(stream,"[I1]");
    PRINT_CHAR_REP(stream,' ',bt_length);
    fprintf(stream,"[D1]");
    PRINT_CHAR_REP(stream,' ',bt_length);
    if (distance_metric==gap_affine) continue;
    fprintf(stream,"[I2]");
    PRINT_CHAR_REP(stream,' ',bt_length);
    fprintf(stream,"[D2]");
    PRINT_CHAR_REP(stream,' ',bt_length);
  }
  fprintf(stream,"|\n");
  // Frame
  wavefront_display_print_frame(stream,score_begin,score_end,row_width,bt_length);
}
/*
 * Display
 */
void wavefront_aligner_print_block(
    FILE* const stream,
    wavefront_aligner_t* const wf_aligner,
    const int score_begin,
    const int score_end,
    int bt_length) {
  // Parameters
  const distance_metric_t distance_metric = wf_aligner->distance_metric;
  if (!wf_aligner->bt_piggyback) bt_length = 0; // Check BT
  // Compute dinmensions
  int max_k, min_k;
  wavefront_display_compute_limits(wf_aligner,score_begin,score_end,&max_k,&min_k);
  // Header
  wavefront_display_print_header(stream,distance_metric,score_begin,score_end,bt_length);
  // Traverse all diagonals
  int k;
  for (k=max_k;k>=min_k;k--) {
    fprintf(stream,"[k=%3d] ",k);
    // Traverse all scores
    int i;
    for (i=score_begin;i<=score_end;++i) {
      const int s = (wf_aligner->memory_modular) ? i%wf_aligner->max_score_scope : i;
      fprintf(stream,"|");
      // Fetch wavefront
      wavefront_t* const mwavefront = wf_aligner->mwavefronts[s];
      wavefront_display_print_element(stream,wf_aligner,mwavefront,k,bt_length);
      if (distance_metric==edit || distance_metric==gap_lineal) continue;
      wavefront_t* const i1wavefront = wf_aligner->i1wavefronts[s];
      wavefront_t* const d1wavefront = wf_aligner->d1wavefronts[s];
      wavefront_display_print_element(stream,wf_aligner,i1wavefront,k,bt_length);
      wavefront_display_print_element(stream,wf_aligner,d1wavefront,k,bt_length);
      if (distance_metric==gap_affine) continue;
      wavefront_t* const i2wavefront = wf_aligner->i2wavefronts[s];
      wavefront_t* const d2wavefront = wf_aligner->d2wavefronts[s];
      wavefront_display_print_element(stream,wf_aligner,i2wavefront,k,bt_length);
      wavefront_display_print_element(stream,wf_aligner,d2wavefront,k,bt_length);
    }
    fprintf(stream,"|\n");
  }
  // Footer
  const int row_width = wavefront_display_compute_row_width(distance_metric,bt_length);
  wavefront_display_print_frame(stream,score_begin,score_end,row_width,bt_length);
}
void wavefront_aligner_print(
    FILE* const stream,
    wavefront_aligner_t* const wf_aligner,
    const int score_begin,
    const int score_end,
    const int wf_block_num,
    const int wf_bt_length) {
  // Print wavefronts by chunks
  int s;
  for (s=score_begin;s<=score_end;s+=wf_block_num-1) {
    const int block_score_end = MIN(s+wf_block_num-1,score_end);
    wavefront_aligner_print_block(stream,wf_aligner,s,block_score_end,wf_bt_length);
    if (block_score_end == score_end) break;
  }
}

#ifdef WFLAMBDA_NAMESPACE
}
#endif
