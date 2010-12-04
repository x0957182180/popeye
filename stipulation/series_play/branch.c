#include "stipulation/series_play/branch.h"
#include "pyslice.h"
#include "pymovein.h"
#include "stipulation/goals/goals.h"
#include "stipulation/series_play/play.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/move_to_goal.h"
#include "stipulation/series_play/shortcut.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types dealing with series moves appear
 * STSeriesFork is not mentioned because it has a variable rank.
 */
static slice_index const series_slice_rank_order[] =
{
  STReadyForSeriesMove,
  STParryFork,
  STMoveInverterSeriesFilter,
  STSeriesHashed,
  STDoubleMateSeriesFilter,
  STCounterMateSeriesFilter,
  STSeriesMove,
  STSeriesMoveToGoal,
  STContinuationSolver, /* occurs in direct pser stipulations */
  STDefenseFork,        /* occurs in direct pser stipulations */
  STDefenseMove,        /* occurs in direct pser stipulations */
  STMaxTimeSeriesFilter,
  STMaxSolutionsSeriesFilter,
  STStopOnShortSolutionsSeriesFilter,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STCirceSteingewinnFilter,
  STGoalReachableGuardSeriesFilter,
  STPiecesParalysingMateFilter,
  STRestartGuardSeriesFilter,
  STSeriesMovePlayed,
  STSelfCheckGuard,
  STSeriesMoveLegalityChecked,
  STSeriesMoveDealtWith
};

enum
{
  nr_series_slice_rank_order_elmts = (sizeof series_slice_rank_order
                                      / sizeof series_slice_rank_order[0])
};

/* Determine the rank of a series slice type
 * @param type series slice type
 * @return rank of type; nr_series_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_series_slice_rank(SliceType type)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  if (type==STSeriesFork)
    result = 0;
  else
    for (result = 0; result!=nr_series_slice_rank_order_elmts; ++result)
      if (series_slice_rank_order[result]==type)
        break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the position where to insert a slice into an series branch.
 * @param si entry slice of series branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_series_slice_insertion_pos(slice_index si, SliceType type)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  {
    unsigned int const rank_type = get_series_slice_rank(type);
    assert(rank_type!=nr_series_slice_rank_order_elmts);
    while (true)
    {
      unsigned int const rank = get_series_slice_rank(slices[si].type);
      if (rank==nr_series_slice_rank_order_elmts)
        break;
      else if (rank>rank_type)
      {
        result = si;
        break;
      }
      else
        si = slices[si].u.pipe.next;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a series pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
void shorten_series_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  --slices[pipe].u.branch.length;
  --slices[pipe].u.branch.min_length;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a the appropriate proxy slices before each
 * STGoal*ReachedTester slice
 * @param si identifies slice
 * @param st address of structure representing the traversal
 */
static void instrument_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    Goal const goal = extractGoalFromTester(si);
    slice_index const ready = alloc_branch(STReadyForSeriesMove,
                                           slack_length_series+1,
                                           slack_length_series+1);
    slice_index const move_to_goal = alloc_series_move_to_goal_slice(goal);
    slice_index const played = alloc_branch(STSeriesMovePlayed,
                                            slack_length_series,
                                            slack_length_series);
    slice_index const checked = alloc_branch(STSeriesMoveLegalityChecked,
                                             slack_length_series,
                                             slack_length_series);
    slice_index const dealt = alloc_branch(STSeriesMoveDealtWith,
                                           slack_length_series,
                                           slack_length_series);
    pipe_append(slices[si].prev,ready);
    pipe_append(ready,move_to_goal);
    pipe_append(move_to_goal,played);

    pipe_append(si,checked);
    pipe_append(checked,dealt);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch leading to a goal to be a series goal branch
 * @param si identifies entry slice of branch
 */
void stip_make_series_goal_branch(slice_index si)
{
  stip_structure_traversal st;
  SliceType type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);

  for (type = first_goal_tester_slice_type;
       type<=last_goal_tester_slice_type;
       ++type)
    stip_structure_traversal_override_single(&st,type,&instrument_tester);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Allocate a series branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_series_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const checked2 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length,min_length);
    slice_index const dealt2 = alloc_branch(STSeriesMoveDealtWith,
                                            length,min_length);
    slice_index const ready = alloc_branch(STReadyForSeriesMove,
                                           length,min_length);
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const played1 = alloc_branch(STSeriesMovePlayed,
                                             length-1,min_length-1);
    slice_index const checked1 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length-1,min_length-1);
    slice_index const dealt1 = alloc_branch(STSeriesMoveDealtWith,
                                            length-1,min_length-1);
    slice_index const inverter = alloc_move_inverter_series_filter();
    slice_index const played2 = alloc_branch(STSeriesMovePlayed,
                                             length-1,min_length-1);

    pipe_link(checked2,dealt2);
    pipe_link(dealt2,ready);
    pipe_link(ready,move);
    pipe_link(move,played1);
    pipe_link(played1,checked1);
    pipe_link(checked1,dealt1);
    pipe_link(dealt1,inverter);
    pipe_link(inverter,played2);
    pipe_link(played2,checked2);

    result = checked2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a fork to the branch leading to the goal
 * @param si identifies the entry slice of a series branch
 * @param to_goal identifies the entry slice of the branch leading to
 *                the goal
 */
void series_branch_set_goal_slice(slice_index si, slice_index to_goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STSeriesMoveLegalityChecked);

  {
    slice_index const ready = branch_find_slice(STReadyForSeriesMove,si);
    assert(ready!=no_slice);
    pipe_append(ready,alloc_series_fork_slice(length,min_length,to_goal));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a series branch
 * @param next identifies the entry slice of the next branch
 */
void series_branch_set_next_slice(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STSeriesMoveLegalityChecked);

  {
    slice_index const dealt1 = branch_find_slice(STSeriesMoveDealtWith,si);
    slice_index const dealt2 = branch_find_slice(STSeriesMoveDealtWith,dealt1);
    stip_length_type const length = slices[dealt2].u.branch.length;
    stip_length_type const min_length = slices[dealt2].u.branch.min_length;

    assert(dealt1!=no_slice);
    assert(dealt2!=no_slice);
    pipe_append(dealt2,alloc_series_fork_slice(length,min_length,next));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
