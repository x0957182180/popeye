#if !defined(STIPULATION_HELP_PLAY_BRANCH_H)
#define STIPULATION_HELP_PLAY_BRANCH_H

#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with STHelp*
 * stipulation slices.
 */

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length);

/* Shorten a help slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void help_branch_shorten_slice(slice_index si);

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 * @return entry slice of shortened branch
 *         no_slice if shortening isn't applicable
 */
slice_index help_branch_shorten(slice_index si);

/* Instrument a branch leading to a goal to be a help goal branch
 * @param si identifies entry slice of branch
 */
void stip_make_help_goal_branch(slice_index si);

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a help branch
 * @param n indicates at what n slice next should kick in
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_next_slice(slice_index si,
                                stip_length_type n,
                                slice_index next);

/* Determine the position where to insert a slice into an help branch.
 * @param si entry slice of help branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_help_slice_insertion_pos(slice_index si, SliceType type);

#endif
