#if !defined(PYNOT_H)
#define PYNOT_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with STNot stipulation
 * slices.
 */

/* Allocate a not slice.
 * @param op 1st operand
 * @return index of allocated slice
 */
slice_index alloc_not_slice(slice_index op);

/* Is there no chance left for the starting side to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
void not_write_unsolvability(slice_index si);

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @return true iff no chance is left
 */
boolean not_must_starter_resign_hashed(slice_index si);

/* Determine and write the solution
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean not_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean not_has_solution(slice_index si);

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of slice
 */
void not_solve_continuations(table continuations, slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean not_has_starter_apriori_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean not_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean not_has_starter_reached_goal(slice_index si);

/* Determine whether a slice.has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean not_has_non_starter_solved(slice_index si);

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index not_root_make_setplay_slice(slice_index si);

/* Determine and write the solution of a slice
 * @param slice index
 * @return true iff >=1 solution was found
 */
boolean not_root_solve(slice_index si);

#endif
