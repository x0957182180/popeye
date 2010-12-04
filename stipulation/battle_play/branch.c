#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_find_shortest.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/ready_for_attack.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/defense_move_played.h"
#include "stipulation/battle_play/defense_move_legality_checked.h"
#include "stipulation/battle_play/ready_for_defense.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types dealing with defense moves appear
 * STDefenseFork and STRootDefenseFork are not mentioned because they have
 * variable ranks.
 */
static slice_index const defense_slice_rank_order[] =
{
  STReflexDefenderFilter,
  STReadyForDefense,
  STThreatSolver,
  STDefenseMove,
  STDefenseMovePlayed,
  STSeriesMovePlayed,
  STMaxNrNonTrivialCounter,
  STRefutationsCollector,
  STDefenseMoveShoeHorningDone,
  STKillerMoveCollector,
  STSelfDefense,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STCirceSteingewinnFilter,
  STSelfCheckGuard,
  STDefenseMoveLegalityChecked,
  STSeriesMoveLegalityChecked,
  STNoShortVariations,
  STAttackHashed,
  STThreatEnforcer,
  STDefenseMoveFiltered,
  STVariationWriter,
  STRefutingVariationWriter,
  STOutputPlaintextTreeCheckWriterAttackerFilter,
  STDefenseDealtWith,
  STReflexAttackerFilter
};

enum
{
  nr_defense_slice_rank_order_elmts = (sizeof defense_slice_rank_order
                                       / sizeof defense_slice_rank_order[0])
};

/* Determine the rank of a defense slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_defense_slice_rank(SliceType type)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  if (type==STDefenseFork)
    result = 0;
  else
    for (result = 0; result!=nr_defense_slice_rank_order_elmts; ++result)
      if (defense_slice_rank_order[result]==type)
        break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the position where to insert a slice into an defense branch.
 * @param si entry slice of defense branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_defense_slice_insertion_pos(slice_index si, SliceType type)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  {
    unsigned int const rank_type = get_defense_slice_rank(type);
    assert(rank_type!=nr_defense_slice_rank_order_elmts);
    while (true)
    {
      unsigned int const rank = get_defense_slice_rank(slices[si].type);
      if (rank==nr_defense_slice_rank_order_elmts)
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

/* Order in which the slice types dealing with attack moves appear
 * STAttackFork and STRootAttackFork are not mentioned because they have
 * variable ranks.
 */
static slice_index const attack_slice_rank_order[] =
{
  STReflexAttackerFilter,
  STReadyForAttack,
  STDegenerateTree,
  STAttackFindShortest,
  STAttackRoot,
  STAttackMove,
  STRestartGuardRootDefenderFilter,
  STAttackMovePlayed,
  STEndOfSolutionWriter,
  STThreatCollector,
  STAttackMoveShoeHorningDone,
  STKillerMoveCollector,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STCirceSteingewinnFilter,
  STSelfCheckGuard,
  STAttackMoveLegalityChecked,
  STMaxNrNonTrivial,
  STAttackMoveFiltered,
  STKeepMatingGuardDefenderFilter,
  STSolutionSolver,
  STKeyWriter,
  STTrySolver,
  STContinuationSolver,
  STContinuationWriter,
  STCheckDetector,
  STAttackDealtWith,
  STOutputPlaintextTreeCheckWriterDefenderFilter,
  STMaxThreatLength,
  STReflexDefenderFilter
};

enum
{
  nr_attack_slice_rank_order_elmts = (sizeof attack_slice_rank_order
                                      / sizeof attack_slice_rank_order[0])
};

/* Determine the rank of a attack slice type
 * @param type attack slice type
 * @return rank of type; nr_attack_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_attack_slice_rank(SliceType type)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  if (type==STRootAttackFork || type==STAttackFork)
    result = 0;
  else
    for (result = 0; result!=nr_attack_slice_rank_order_elmts; ++result)
      if (attack_slice_rank_order[result]==type)
        break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the position where to insert a slice into an attack branch.
 * @param si entry slice of attack branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_attack_slice_insertion_pos(slice_index si, SliceType type)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  {
    unsigned int const rank_type = get_attack_slice_rank(type);
    assert(rank_type!=nr_attack_slice_rank_order_elmts);
    do
    {
      unsigned int const rank = get_attack_slice_rank(slices[si].type);
      if (rank==nr_attack_slice_rank_order_elmts)
        break;
      else if (rank>rank_type)
      {
        result = si;
        break;
      }
      else
        si = slices[si].u.pipe.next;
    }
    while (si!=no_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch consisting mainly of an attack move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_attack_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;
  slice_index shortest;
  slice_index attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_ready_for_attack_slice(length,min_length);
  shortest = alloc_attack_find_shortest_slice(length,min_length);
  attack = alloc_attack_move_slice(length,min_length);
  pipe_link(result,shortest);
  pipe_link(shortest,attack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch consisting mainly of an defense move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies slice where the defense branch leads to
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  {
    slice_index const dready = alloc_ready_for_defense_slice(length,
                                                             min_length);
    slice_index const defense = alloc_defense_move_slice(length,min_length);
    slice_index const dplayed = alloc_defense_move_played_slice(length-1,
                                                                min_length-1);
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length-1,
                                                           min_length-1);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length-1,min_length-1);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length-1,min_length-1);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);
    pipe_link(ddealt,next);

    result = dready;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert(length>slack_length_battle);
  assert(min_length>slack_length_battle);

  {
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length,min_length);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length,
                                                           min_length);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length,min_length);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length,min_length);
    slice_index const aready = alloc_ready_for_attack_slice(length,min_length);
    slice_index const shortest = alloc_attack_find_shortest_slice(length,
                                                                  min_length);

    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const aplayed = alloc_branch(STAttackMovePlayed,
                                             length-1,min_length-1);
    slice_index const ashoehorned = alloc_branch(STAttackMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const checked = alloc_branch(STAttackMoveLegalityChecked,
                                             length-1,min_length-1);
    slice_index const afiltered = alloc_branch(STAttackMoveFiltered,
                                               length-1,min_length-1);
    slice_index const adealt = alloc_branch(STAttackDealtWith,
                                            length-1,min_length-1);
    slice_index const dready = alloc_ready_for_defense_slice(length-1,
                                                             min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const dplayed = alloc_defense_move_played_slice(length-2,
                                                                min_length-2);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);
    pipe_link(ddealt,aready);
    pipe_link(aready,shortest);
    pipe_link(shortest,attack);
    pipe_link(attack,aplayed);
    pipe_link(aplayed,ashoehorned);
    pipe_link(ashoehorned,checked);
    pipe_link(checked,afiltered);
    pipe_link(afiltered,adealt);
    pipe_link(adealt,dready);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);

    result = dchecked;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a battle slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void battle_branch_shorten_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.branch.length -= 2;
  slices[si].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
