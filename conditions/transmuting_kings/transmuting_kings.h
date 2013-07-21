#if !defined(CONDITIONS_TRANSMUTING_KINGS_TRANSMUTING_KINGS_H)
#define CONDITIONS_TRANSMUTING_KINGS_TRANSMUTING_KINGS_H

/* This module implements the Transmuting Kings family of fairy conditions */

#include "position/position.h"
#include "pyproc.h"

extern PieNam transmpieces[nr_sides][PieceCount];
extern boolean transmuting_kings_lock_recursion;

/* Initialise the sequence of king transmuters
 * @param side for which side to initialise?
 */
void init_transmuters_sequence(Side side);

typedef enum
{
  king_not_transmuting,
  king_transmuting_no_attack,
  king_transmuting_attack,
} transmuting_kings_attack_type;

/* Does the transmuting king of side trait[nbply] attack a particular square
 * while transmuting?
 * @param evaluate attack evaluator
 * @return how much attack of the transmuting king to sq_target is there?
 */
transmuting_kings_attack_type
transmuting_kings_is_square_attacked_by_transmuting_king(square sq_target,
                                                         evalfunction_t *evaluate);

/* Does the transmuting king of side trait[nbply] attack a particular square
 * (while transmuting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return true iff a king attacks sq_target?
 */
boolean transmuting_kings_is_square_attacked_by_king(square sq_target,
                                                     evalfunction_t *evaluate);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void transmuting_kings_generate_moves_for_piece(slice_index si,
                                                square sq_departure,
                                                PieNam p);

/* Inialise the solving machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 */
void transmuting_kings_initialise_solving(slice_index si);

boolean transmuting_king_is_square_observed(slice_index si,
                                            square sq_target,
                                            evalfunction_t *evaluate);

/* Does the reflective king of side trait[nbply] attack a particular square
 * (while transmuting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return true iff a king attacks sq_target?
 */
boolean reflective_kings_is_square_attacked_by_king(square sq_target,
                                                    evalfunction_t *evaluate);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void reflective_kings_generate_moves_for_piece(slice_index si,
                                     square sq_departure,
                                     PieNam p);

/* Inialise the solving machinery with reflective kings
 * @param si identifies root slice of solving machinery
 */
void reflective_kings_initialise_solving(slice_index si);

boolean reflective_king_is_square_observed(slice_index si,
                                           square sq_target,
                                           evalfunction_t *evaluate);

#endif
