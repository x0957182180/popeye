#include "pystip.h"
#include "pydata.h"
#include "trace.h"
#include "pyleaf.h"
#include "pycompos.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pysequen.h"

#include <assert.h>
#include <stdlib.h>

Slice slices[max_nr_slices];

Side regular_starter;

static slice_index next_slice;

/* Allocate a composite slice.
 * Initializes type to STSequence and composite fields to null values
 * @return index of allocated slice
 */
slice_index alloc_composite_slice(SliceType type, Play play)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",type);
  TraceFunctionParam("%d\n",play);

  slices[result].type = type; 
  slices[result].u.composite.play = play;
  slices[result].u.composite.length = 0;
  slices[result].u.composite.is_exact = false;
  slices[result].u.composite.op1 = no_slice;
  slices[result].u.composite.op2 = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Allocate a target leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(End end, square s)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result].type = STLeaf; 
  slices[result].u.leaf.end = end;
  slices[result].u.leaf.goal = goal_target;
  slices[result].u.leaf.target = s;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Allocate a (non-target) leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_leaf_slice(End end, Goal goal)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",end);
  TraceFunctionParam("%d\n",goal);

  slices[result].type = STLeaf; 
  slices[result].u.leaf.end = end;
  slices[result].u.leaf.goal = goal;
  slices[result].u.leaf.target = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result] = slices[original];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Release all slices
 */
void release_slices(void)
{
  next_slice = 0;
}

static void transform_to_quodlibet_recursive(slice_index *hook)
{
  slice_index const index = *hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",*hook);

  TraceValue("%d\n",slices[index].type);
  switch (slices[index].type)
  {
    case STLeaf:
      if (slices[index].u.leaf.end==ESelf
          || slices[index].u.leaf.end==EReflex
          || slices[index].u.leaf.end==ESemireflex)
      {
        /* Insert a new quodlibet node at *hook's current position.
         * Move *hook to positon op2 of the new quodlibet node, and
         * add a new direct leaf at op1 of that node.
         * op1 is tested before op2, so it is more efficient to make
         * op1 the new direct leaf.
         */
        Goal const goal = slices[index].u.leaf.goal;
        *hook = alloc_composite_slice(STQuodlibet,PDirect);
        TraceValue("allocated quodlibet slice %d\n",*hook);
        slices[*hook].u.composite.op1 = alloc_leaf_slice(EDirect,goal);
        slices[*hook].u.composite.op2 = index;
        slices[*hook].u.composite.is_exact = false;
        slices[*hook].u.composite.length = 1;
      }
      break;

    case STQuodlibet:
    case STReciprocal:
      transform_to_quodlibet_recursive(&slices[index].u.composite.op1);
      transform_to_quodlibet_recursive(&slices[index].u.composite.op2);
      break;

    case STSequence:
      transform_to_quodlibet_recursive(&slices[index].u.composite.op1);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

void transform_to_quodlibet(void)
{
  slice_index start = 0;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  transform_to_quodlibet_recursive(&start);
  assert(start==0);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Does a leaf have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si leaf slice identifier
 * @return true iff the leaf has as goal one of the elements of goals.
 */
static boolean leaf_ends_in_one_of(Goal const  goals[],
                                   unsigned int nrGoals,
                                   slice_index si)
{
  Goal const goal = slices[si].u.leaf.goal;

  unsigned int i;
  for (i = 0; i<nrGoals; ++i)
    if (goal==goals[i])
      return true;

  return false;
}

/* Do all leaves of a slice and its descendants have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si slice identifier
 * @return true iff all leaves have as goal one of the elements of goals.
 */
static boolean slice_ends_only_in(Goal const goals[],
                                  unsigned int nrGoals,
                                  slice_index si)
{
  switch (slices[si].type)
  {
    case STLeaf:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;
      return (slice_ends_only_in(goals,nrGoals,op1)
              && slice_ends_only_in(goals,nrGoals,op2));
    }

    case STSequence:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      return slice_ends_only_in(goals,nrGoals,op1);
    }

    default:
      assert(0);
      exit(1);
  }
}

/* Do all leaves of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff all leaves have as goal one of the elements of goals.
 */
boolean stip_ends_only_in(Goal const  goals[], unsigned int nrGoals)
{
  return slice_ends_only_in(goals,nrGoals,0);
}


/* Does >= leaf of a slice and its descendants have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si slice identifier
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
static boolean slice_ends_in(Goal const goals[],
                             unsigned int nrGoals,
                             slice_index si)
{
  switch (slices[si].type)
  {
    case STLeaf:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;
      return (slice_ends_in(goals,nrGoals,op1)
              || slice_ends_in(goals,nrGoals,op2));
    }

    case STSequence:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      return slice_ends_in(goals,nrGoals,op1);
    }

    default:
      assert(0);
      exit(1);
  }
}

/* Does >= 1 leaf of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
boolean stip_ends_in(Goal const goals[], unsigned int nrGoals)
{
  return slice_ends_in(goals,nrGoals,0);
}

/* Continue search for a goal. Cf. find_next_goal() */
static slice_index find_goal_recursive(Goal goal,
                                       slice_index start,
                                       boolean *active,
                                       slice_index si)
{
  slice_index result = no_slice;

  switch (slices[si].type)
  {
    case STLeaf:
      if (*active)
      {
        if (slices[si].u.leaf.goal==goal)
          result = si;
      }
      else
        *active = si==start;
      break;

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;
      result = find_goal_recursive(goal,start,active,op1);
      if (result==no_slice)
        result = find_goal_recursive(goal,start,active,op2);
      break;
    }

    case STSequence:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      result = find_goal_recursive(goal,start,active,op1);
      break;
    }

    default:
      assert(0);
      exit(1);
  }

  return result;
}

/* Traversal of the stipulation tree up to the next slice with a
 * specific goal. Repeated calls, with start set to the result of the
 * previous call, result in a complete traversal.
 * @param goal defines where to stop traversal
 * @param start traversal starts (continues) at the identified slice
 *              (excluding it, i.e. the result will be different from
 *              start); must be 0 or the result of a previous call
 * @return if found, index of the next slice with the requested goal;
 *         no_slice otherwise
 */
slice_index find_next_goal(Goal goal, slice_index start)
{
  boolean active = start==0;

  assert(start<next_slice);

  /* Either this is the first run (-> si==0) or we start from the
   * previous result, which must have been a leaf. */
  assert(start==0 || slices[start].type==STLeaf);

  return find_goal_recursive(goal,start,&active,0);
}

static boolean are_goals_equal(slice_index si1, slice_index si2)
{
  return ((slices[si1].u.leaf.goal ==slices[si2].u.leaf.goal)
          && (slices[si1].u.leaf.goal!=goal_target
              || (slices[si1].u.leaf.target==slices[si2].u.leaf.target)));
}

static boolean find_unique_goal_recursive(slice_index current_slice,
                                          slice_index *found_so_far)
{
  switch (slices[current_slice].type)
  {
    case STLeaf:
      if (*found_so_far==no_slice)
      {
        *found_so_far = current_slice;
        return true;
      }
      else
        return are_goals_equal(*found_so_far,current_slice);

    case STReciprocal:
    case STQuodlibet:
    {
      slice_index const op1 = slices[current_slice].u.composite.op1;
      slice_index const op2 = slices[current_slice].u.composite.op2;
      return (find_unique_goal_recursive(op1,found_so_far)
              && find_unique_goal_recursive(op2,found_so_far));
    }
    
    case STSequence:
    {
      slice_index const op1 = slices[current_slice].u.composite.op1;
      return find_unique_goal_recursive(op1,found_so_far);
    }

    default:
      assert(0);
      return false;
  }
}

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @return no_goal if goal is not unique; unique goal otherwise
 */
slice_index find_unique_goal(void)
{
  slice_index found_so_far = no_slice;
  return (find_unique_goal_recursive(0,&found_so_far)
          ? found_so_far
          : no_slice);
}

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean slice_is_unsolvable(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_is_unsolvable(si);
      break;

    case STReciprocal:
      result = reci_end_is_unsolvable(si);
      break;
      
    case STQuodlibet:
      result = quodlibet_end_is_unsolvable(si);
      break;

    case STSequence:
      result = sequence_end_is_unsolvable(si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether a slice has >=1 solution
 * @param si slice index
 * @return true iff slice has >=1 solution(s)
 */
boolean slice_is_solvable(slice_index si)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_is_solvable(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Solve a slice (i.e. find and write all solutions)
 * @param side_at_move side doing the first move
 * @param si slice index
 * @return true iff slice has >=1 solution(s)
 */
boolean slice_solve(slice_index si)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of sequence slice
 */
void d_slice_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      d_leaf_solve_continuations(table,si);
      break;
    
    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_solve_continuations(slices[si].u.composite.length,
                                      table,
                                      si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write defender's set play
 * @param si slice index
 */
void d_slice_solve_setplay(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      d_leaf_solve_setplay(si);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_solve_setplay(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean d_slice_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = d_leaf_solve_complete_set(si);
      break;

    case STSequence:
      result = d_sequence_end_solve_complete_set(si);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_solve_complete_set(si);
      break;

    case STReciprocal:
      /* not really meaningful */
      break;
      
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write the solutions and tries in the current position
 * in direct play.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_slice_solve(boolean restartenabled, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      d_leaf_solve(si);
      break;
      
    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_solve(restartenabled,si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the key just played, then continue solving in the slice
 * to find and write the post key play (threats, variations) and
 * write the refutations (if any)
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param type type of attack
 */
void d_slice_write_key_solve_postkey(int refutations,
                                     slice_index si,
                                     attack_type type)
{
  switch (slices[si].type)
  {
    case STLeaf:
      d_leaf_write_key_solve_postkey(refutations,si,type);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_write_key_solve_postkey(refutations,si,type);
      break;

    default:
      assert(0);
      break;
  }
}

/* Solve a slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_slice_solve(boolean restartenabled, slice_index si)
{
  switch (slices[si].type)
  {
    case STLeaf:
      return h_leaf_solve(si);

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      return h_composite_solve(restartenabled,
                               si,
                               slices[si].u.composite.length);

    default:
      assert(0);
      return false;
  }
}

/* Solve a slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean ser_slice_solve(boolean restartenabled, slice_index si)
{
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      solution_found = ser_leaf_solve(si);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      solution_found = ser_composite_solve(restartenabled,
                                           si,
                                           slices[si].u.composite.length);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
}

/* Determine whether the attacking side wins
 * @param si slice identifier
 * @return true iff attacker wins
 */
boolean d_slice_does_attacker_win(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = d_leaf_does_attacker_win(si);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      result = d_composite_does_attacker_win(slices[si].u.composite.length,
                                             si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Find and write variations
 * @param len_threat length of threat (shorter variations are suppressed)
 * @param threats table containing threats (variations not defending
 *                against all threats are suppressed)
 * @param refutations table containing refutations (written at end)
 * @param si slice index
 */
void d_slice_solve_variations(int len_threat,
                              int threats,
                              int refutations,
                              slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",len_threat);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      d_leaf_solve_variations(si);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_solve_variations(slices[si].u.composite.length,
                                   len_threat,
                                   threats,
                                   refutations,
                                   si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether the defender has lost in direct play with his move
 * just played.
 * Assumes that there is no short win for the defending side.
 * @param si slice identifier
 * @return true iff there is a short win or loss
 */
boolean d_slice_has_defender_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = d_leaf_has_defender_lost(si);
      break;

    case STSequence:
      result = d_sequence_end_has_defender_lost(si);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_defender_lost(si);
      break;

    case STReciprocal:
      result = d_reci_end_has_defender_lost(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_slice_has_defender_won(slice_index si)
{
  boolean result = false;

  switch (slices[si].type)
  {
    case STLeaf:
      result = d_leaf_has_defender_won(si);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_defender_won(si);
      break;

    case STSequence:
      result = d_sequence_end_has_defender_won(si);
      break;

    case STReciprocal:
      result = d_reci_end_has_defender_won(si);
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Determine whether the attacker has lost with his last move in
 * direct play. 
 * Assumes that he has not won with his last move.
 * @param si slice identifier
 * @return true iff attacker has lost
 */
boolean d_slice_has_attacker_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      result = d_leaf_has_attacker_lost(si);
      break;

    case STSequence:
      result = d_sequence_end_has_attacker_lost(si);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_attacker_lost(si);
      break;

    case STReciprocal:
      result = d_reci_end_has_attacker_lost(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacker has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_slice_has_attacker_won(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      result = d_leaf_has_attacker_won(si);
      break;

    case STSequence:
      result = d_sequence_end_has_attacker_won(si);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_attacker_won(si);
      break;

    case STReciprocal:
      result = d_reci_end_has_attacker_won(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice in direct play
 * (e.g. forced reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void d_slice_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      d_leaf_write_unsolvability(si);
      break;

    case STQuodlibet:
      d_quodlibet_write_unsolvability(si);
      break;

    case STSequence:
      d_sequence_write_unsolvability(si);
      break;

    case STReciprocal:
      d_reci_write_unsolvability(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_slice_is_threat_refuted(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = !d_leaf_is_solved(si);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      result = d_composite_is_threat_refuted(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void slice_detect_starter(slice_index si, boolean is_duplex)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      leaf_detect_starter(si,is_duplex);
      break;

    case STSequence:
      sequence_detect_starter(si,is_duplex);
      break;

    case STReciprocal:
      reci_detect_starter(si,is_duplex);
      break;

    case STQuodlibet:
      quodlibet_detect_starter(si,is_duplex);
      break;

    default:
      assert(0);
      break;
  }

  if (si==0)
  {
    regular_starter = slices[0].starter;
    TraceValue("%d\n",regular_starter);
  }

  TraceValue("%d\n",slices[si].u.composite.length);
  TraceValue("%d\n",slices[si].u.composite.play);
  if (slices[si].type!=STLeaf
      && slices[si].u.composite.play==PHelp
      && slices[si].u.composite.length%2 == 1)
  {
    if (slices[si].starter==no_side)
      slices[si].starter = no_side;
    else
      slices[si].starter = advers(slices[si].starter);
  }

  TraceValue("%d\n",slices[si].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies sequence
 * @param s starting side of leaf
 */
void slice_impose_starter(slice_index si, Side s)
{
  switch (slices[si].type)
  {
    case STLeaf:
      leaf_impose_starter(si,s);
      break;

    case STSequence:
      sequence_impose_starter(si,s);
      break;

    case STReciprocal:
      reci_impose_starter(si,s);
      break;

    case STQuodlibet:
      quodlibet_impose_starter(si,s);
      break;

    default:
      assert(0);
      break;
  }
}
