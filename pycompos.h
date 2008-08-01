#if !defined(PYCOMPOS_H)
#define PYCOMPOS_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with composite
 * (i.e. non-leaf) stipulation slices.
 */

/* Determine whether attacker can end in n moves of direct play.
 * This is a recursive function.
 * @param n number of moves left until the end state has to be reached
 * @return true iff attacker can end in n moves
 */
boolean d_composite_does_attacker_win(int n, slice_index si);

/* Determine whether the defender wins in a self/reflex stipulation in
 * n.
 * @param defender defending side (at move)
 * @param n number of moves until end state has to be reached
 * @return "how much or few" the defending side wins
 */
d_defender_win_type d_composite_does_defender_win(int n, slice_index si);

/* Determine and write set play of a direct/self/reflex stipulation
 * @param si slice index
 */
void d_composite_solve_setplay(slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any), starting at the end
 * of a quodlibet slice.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param is_try true iff what we are solving is a try
 */
void d_composite_write_key_solve_postkey(int refutations,
                                         slice_index si,
                                         boolean is_try);

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param refutations table containing refutations after move just
 *                    played
 */
void d_composite_solve_variations(int n,
                                  int len_threat,
                                  int threats,
                                  int refutations,
                                  slice_index si);

/* Determine and write the solutions and tries in the current position
 * in direct play.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_composite_solve(boolean restartenabled, slice_index si);

/* Determine and write the post-key solution in the current position
 * in direct/self/reflex play.
 * @param n number of moves until end state has to be reached
 * @param si slice index
 */
void d_composite_solve_postkey(int n, slice_index si);

/* Determine and write the continuations in the current position in
 * direct/self/reflex play (i.e. attacker's moves winning after a
 * defender's move that refuted the threat).
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached
 * @param continuations table where to store continuing moves
 *                      (i.e. threats)
 * @param si slice index
 */
void d_composite_solve_continuations(int n,
                                     int continuations,
                                     slice_index si);

/* Determine and write the solutions in the current position in series
 * play.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_exact_solve(boolean restartenabled, slice_index si);

/* Solve a composite clide with series play
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_solve(boolean restartenabled, slice_index si);

/* Solve the composite slice with index 0 with series play
 * @param restartenabled true iff option movenum is active
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_slice0_solve(int n, boolean restartenabled);

/* Determine and write the solutions in the current position in help
 * play.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean h_composite_solve(boolean restartenabled, slice_index si);

#endif
