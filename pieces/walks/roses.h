#if !defined(PIECES_WALKS_ROSES_H)
#define PIECES_WALKS_ROSES_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* This module implements Rose and derived pieces.
 * The circle lines are generated in a clever way (which leads to
 * simple code):
 *
 * At the positions vec_knight_start to
 * vec_knight_start+(vec_knight_end-vec_knight_start) of the array
 * vec, we have the knight vectors twice in a row, in the order of the
 * knight's wheel. Each rose line is generated by sequentally adding
 * vec[k1], vec[k1+1] ... vec[k1+(vec_knight_end-vec_knight_start)]
 * (or the same vectors in opposite order) to the initial square.
 *
 * In calls to the generator functions, k2 should either be 0 or
 * (vec_knight_end-vec_knight_start)+1, and delta_k +1 or -1,
 * respectively, to cause k2, k2+delta_k, ... to assume values 0 ...
 * (vec_knight_end-vec_knight_start) or
 * (vec_knight_end-vec_knight_start)+1 ... 1.
 */

typedef enum
{
  rose_rotation_counterclockwise = -1,
  rose_rotation_clockwise = +1
} rose_rotation_sense;

/* Generate moves for a Rose
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void rose_generate_moves(vec_index_type vec_range_start,
                         vec_index_type vec_range_end);

/* Detect observation by a Rose
 * @return true iff there is an observation
 */
boolean rose_check(validator_id evaluate);

/* Generate moves for a Rao
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void rao_generate_moves(vec_index_type vec_range_start,
                        vec_index_type vec_range_end);

/* Generate moves for a Rose Lion
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void roselion_generate_moves(vec_index_type vec_range_start,
                             vec_index_type vec_range_end);

/* Detect observation by a Rose Lion or Rao
 * @return true iff there is an observation
 */
boolean roselion_check(validator_id evaluate);

/* Generate moves for a Rose Hopper
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void rosehopper_generate_moves(vec_index_type vec_range_start,
                               vec_index_type vec_range_end);

/* Detect observation by a Rose Hopper
 * @return true iff there is an observation
 */
boolean rosehopper_check(validator_id evaluate);

/* Generate moves for a Rose Locust
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void roselocust_generate_moves(vec_index_type vec_range_start,
                               vec_index_type vec_range_end);

/* Detect observation by a Rose Locust
 * @return true iff there is an observation
 */
boolean roselocust_check(validator_id evaluate);

#endif
