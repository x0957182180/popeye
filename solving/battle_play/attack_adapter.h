#if !defined(SOLVING_BATTLE_PLAY_ATTACK_ADAPTER_H)
#define SOLVING_BATTLE_PLAY_ATTACK_ADAPTER_H

#include "stipulation/structure_traversal.h"
#include "solving/machinery/solve.h"

/* STAttackAdapter slices switch from generic solving to attack solving.
 */

/* Spin the intro slices off a nested battle branch
 * @param adapter identifies adapter slice of the nested help branch
 * @param state address of structure holding state
 */
void battle_spin_off_intro(slice_index adapter, spin_off_state_type *state);

/* Wrap the slices representing the nested slices
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void attack_adapter_make_intro(slice_index adapter,
                               stip_structure_traversal *st);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void attack_adapter_solve(slice_index si);

#endif
