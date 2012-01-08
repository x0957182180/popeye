#if !defined(STIPULATION_END_OF_BRANCH_GOAL_H)
#define STIPULATION_END_OF_BRANCH_GOAL_H

/* In reflex stipulations, guard against tries that would allow the
 * wrong side to reach the goal.
 */

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"

/* Allocate a STEndOfBranchGoal slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_goal(slice_index proxy_to_goal);

/* Allocate a STEndOfBranchForced slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_forced(slice_index proxy_to_goal);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
end_of_branch_goal_can_attack(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
end_of_branch_goal_attack(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type end_of_branch_goal_help(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type end_of_branch_goal_can_help(slice_index si,
                                             stip_length_type n);

#endif
