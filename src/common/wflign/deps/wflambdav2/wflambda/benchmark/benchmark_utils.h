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
 * DESCRIPTION: Benchmark utils
 */

#ifndef BENCHMARK_UTILS_H_
#define BENCHMARK_UTILS_H_

#include "wflambda/utils/commons.h"
#include "wflambda/system/mm_allocator.h"
#include "wflambda/system/profiler_timer.h"
#include "wflambda/alignment/score_matrix.h"
#include "wflambda/wavefront/wavefront_aligner.h"
#include "wflambda/wavefront/wavefront_aligner.h"

/*
 * Constants
 */
#define ALIGN_DEBUG_CHECK_CORRECT   0x00000001
#define ALIGN_DEBUG_CHECK_SCORE     0x00000002
#define ALIGN_DEBUG_CHECK_ALIGNMENT 0x00000004
#define ALIGN_DEBUG_DISPLAY_INFO    0x00000008

#define ALIGN_DEBUG_CHECK_DISTANCE_METRIC_EDIT       0x00000010
#define ALIGN_DEBUG_CHECK_DISTANCE_METRIC_GAP_LINEAL 0x00000040
#define ALIGN_DEBUG_CHECK_DISTANCE_METRIC_GAP_AFFINE 0x00000080

/*
 * Alignment Input
 */
typedef struct {
  // Sequences
  int sequence_id;
  char* pattern;
  int pattern_length;
  char* text;
  int text_length;
  // Output
  FILE* output_file;
  // MM
  wavefront_aligner_t* wf_aligner; // WF-Aligner (Pre-allocated)
  mm_allocator_t* mm_allocator;
  // PROFILE/STATS
  profiler_timer_t timer;
  profiler_counter_t align;
  profiler_counter_t align_correct;
  profiler_counter_t align_score;
  profiler_counter_t align_score_total;
  profiler_counter_t align_score_diff;
  profiler_counter_t align_cigar;
  profiler_counter_t align_bases;
  profiler_counter_t align_matches;
  profiler_counter_t align_mismatches;
  profiler_counter_t align_del;
  profiler_counter_t align_ins;
  // DEBUG
  int debug_flags;
  lineal_penalties_t* check_lineal_penalties;
  affine_penalties_t* check_affine_penalties;
  int check_bandwidth;
  bool verbose;
} align_input_t;

/*
 * Setup
 */
void benchmark_align_input_clear(
    align_input_t* const align_input);

/*
 * Display
 */
void benchmark_print_alignment(
    FILE* const stream,
    align_input_t* const align_input,
    const int score_computed,
    cigar_t* const cigar_computed,
    const int score_correct,
    cigar_t* const cigar_correct);
void benchmark_print_alignment_short(
    FILE* const stream,
    const int score,
    cigar_t* const cigar);

/*
 * Stats
 */
void benchmark_print_stats(
    FILE* const stream,
    align_input_t* const align_input,
    const bool print_wf_stats);

#endif /* BENCHMARK_UTILS_H_ */
