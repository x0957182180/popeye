#include "pyleafh.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyint.h"
#include "pyleaf.h"
#include "pyoutput.h"
#include "pyhash.h"

#include <assert.h>
#include <stdlib.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean leaf_h_must_starter_resign(slice_index leaf)
{
  boolean result = false;
  Side const starter = slices[leaf].starter;

  assert(starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = OptFlag[keepmating] && !is_a_mating_piece_left(starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side has an end in 1.
 * @param side_at_move
 * @param leaf slice index
 * @return true iff side_at_move can end in 1 move
 */
static boolean is_end_in_1_possible(Side side_at_move, slice_index leaf)
{
  boolean end_found = false;

  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  generate_move_reaching_goal(leaf,side_at_move);

  --MovesLeft[side_at_move];

  while (encore() && !end_found)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && (!isIntelligentModeActive || isGoalReachable())
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      end_found = true;
      coupfort();
    }

    repcoup();

    if (periods_counter>=nr_periods)
      break;
  }

  ++MovesLeft[side_at_move];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",end_found);
  TraceFunctionResultEnd();
  return end_found;
}

/* Determine whether there is a solution in a help leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
boolean leaf_h_has_solution(slice_index leaf)
{
  boolean result = false;
  Side const starter = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (!(OptFlag[keepmating] && !is_a_mating_piece_left(starter))
      && is_end_in_1_possible(starter,leaf))
  {
    result = true;
    coupfort();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a leaf slice.has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_h_has_non_starter_solved(slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  assert(slices[leaf].starter!=no_side);

  TraceValue("%u\n",slices[leaf].starter);

  result = leaf_is_goal_reached(slices[leaf].starter,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Determine and find final moves of a help leaf
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
static boolean leaf_h_solve_final_move(slice_index leaf)
{
  boolean final_move_found = false;
  Side const side_at_move = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",side_at_move);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,side_at_move);

  --MovesLeft[side_at_move];

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !(isIntelligentModeActive && !isGoalReachable())
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      final_move_found = true;
      write_final_help_move(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  ++MovesLeft[side_at_move];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",final_move_found);
  TraceFunctionResultEnd();
  return final_move_found;
}

/* Solve the final move for a countermate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_cmate_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const just_moved = advers(slices[leaf].starter);
  Side const side_at_move = slices[leaf].starter;

  if (goal_checker_mate(just_moved))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,side_at_move);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(side_at_move,leaf))
      {
        found_solution = true;
        write_final_help_move(goal_countermate);
      }
      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Solve the final move for a doublemate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_dmate_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].starter;

  if (!immobile(side_at_move))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,side_at_move);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(side_at_move,leaf))
      {
        found_solution = true;
        write_final_help_move(goal_doublemate);
      }

      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Determine and write the solution of a help leaf slice without
 * consulting nor updating the hash table
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
static boolean leaf_h_solve_nohash(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = leaf_h_cmate_solve_final_move(leaf);
      break;

    case goal_doublemate:
      result = leaf_h_dmate_solve_final_move(leaf);
      break;

    default:
      result = leaf_h_solve_final_move(leaf);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution of a leaf slice at root level
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_h_root_solve(slice_index leaf)
{
  boolean const save_isIntelligentModeActive = isIntelligentModeActive;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  init_output(leaf);

  isIntelligentModeActive = false;
  result = leaf_h_solve_nohash(leaf);
  isIntelligentModeActive = save_isIntelligentModeActive;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution of a help leaf slice.
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_h_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (inhash(leaf,hash_help_insufficient_nr_half_moves,1))
    result = false;
  else if (leaf_h_solve_nohash(leaf))
    result = true;
  else
  {
    result = false;
    addtohash(leaf,hash_help_insufficient_nr_half_moves,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_h_has_starter_apriori_lost(slice_index leaf)
{
  boolean result = false;
  Side const final = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  assert(slices[leaf].starter!=no_side);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(final);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_h_has_starter_won(slice_index leaf)
{
  boolean result;
  Side const final = slices[leaf].starter;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  assert(slices[leaf].starter!=no_side);

  result = (!(OptFlag[keepmating] && !is_a_mating_piece_left(final))
            && is_end_in_1_possible(final,leaf));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param leaf slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_h_has_starter_reached_goal(slice_index leaf)
{
  boolean const result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  assert(slices[leaf].starter!=no_side);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param leaf slice index
 */
void leaf_h_solve_postkey(slice_index leaf)
{
  assert(slices[leaf].starter!=no_side);

  output_start_leaf_variation_level();
  leaf_h_solve_final_move(leaf);
  output_end_leaf_variation_level();
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_h_detect_starter(slice_index leaf,
                                             boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_proof:
      slices[leaf].starter = same_side_as_root ? White : Black;
      result = leaf_decides_on_starter;
      break;

    case goal_atob:
      slices[leaf].starter = same_side_as_root ? Black : White;
      result = leaf_decides_on_starter;
      break;

    default:
      if (slices[leaf].starter==no_side)
        slices[leaf].starter = White;
      break;
  }

  TraceValue("%u\n",slices[leaf].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
