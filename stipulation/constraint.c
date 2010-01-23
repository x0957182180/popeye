#include "pyreflxg.h"
#include "pydirect.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pyoutput.h"
#include "pydata.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STReflexGuard slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_guard(stip_length_type length,
                                      stip_length_type min_length,
                                      slice_index proxy_to_goal,
                                      slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  result = alloc_branch(STReflexGuard,
                        length,min_length,
                        proxy_to_goal);
  slices[result].u.pipe.u.reflex_guard.avoided = proxy_to_avoided;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Direct ***************
 */

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type reflex_guard_direct_has_solution_in_n(slice_index si,
                                                       stip_length_type n,
                                                       stip_length_type n_min)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      result = 0;
      break;

    case has_solution:
      result = n+2;
      break;

    case has_no_solution:
      result = direct_has_solution_in_n(slices[si].u.pipe.next,n,n_min);
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void reflex_guard_direct_solve_continuations_in_n(slice_index si,
                                                  stip_length_type n,
                                                  stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  direct_solve_continuations_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type reflex_guard_direct_solve_threats_in_n(table threats,
                                                        slice_index si,
                                                        stip_length_type n,
                                                        stip_length_type n_min)
{
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case has_solution:
      /* no threats to be found because of reflex obligations;
       * cf. issue 2843251 */
      result = n+2;
      break;

    case has_no_solution:
    {
      slice_index const next = slices[si].u.pipe.next;
      result = direct_solve_threats_in_n(threats,next,n,n_min);
      break;
    }

    case defender_self_check:
      /* must already have been dealt with in an earlier slice */
    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean reflex_guard_are_threats_refuted_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(slice_has_solution(slices[si].u.pipe.u.reflex_guard.avoided)
         ==has_no_solution);
  result = direct_are_threats_refuted_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean reflex_guard_root_defend(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.pipe.u.reflex_guard.min_length==slack_length_direct
      && slice_solve(slices[si].u.pipe.u.reflex_guard.avoided))
  {
    write_end_of_solution();
    result = false;
  }
  else if (slices[si].u.pipe.u.reflex_guard.length>slack_length_direct)
    result = direct_defender_root_defend(slices[si].u.pipe.next);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean reflex_guard_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==slack_length_direct)
    result = !slice_solve(avoided);
  else
    result = direct_defender_defend_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int reflex_guard_can_defend_in_n(slice_index si,
                                          stip_length_type n,
                                          unsigned int max_result)
{
  unsigned int result = max_result+1;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const
      min_length = slices[si].u.pipe.u.reflex_guard.min_length;
  stip_length_type const max_n_for_avoided = (length-min_length
                                              +slack_length_direct);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n<=max_n_for_avoided)
    switch (slice_has_solution(avoided))
    {
      case has_solution:
        result = 0;
        break;

      case has_no_solution:
        if (n>slack_length_direct)
          result = direct_defender_can_defend_in_n(next,n,max_result);
        break;

      default:
        assert(0);
        break;
    }
  else
    result = direct_defender_can_defend_in_n(next,n,max_result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Help ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (n==slack_length_help)
    result = slice_solve(avoided);
  else if (slice_has_solution(avoided)==has_solution)
    result = false;
  else 
    result = help_solve_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_help_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
   result = false;
  else
    result = help_has_solution_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_guard_help_solve_threats_in_n(table threats,
                                          slice_index si,
                                          stip_length_type n)
{
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_no_solution)
    help_solve_threats_in_n(threats,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Series ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = false;
  else
    result = series_solve_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_series_has_solution_in_n(slice_index si,
                                              stip_length_type n)
{
  boolean result = false;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = false;
  else
    result = series_has_solution_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_guard_series_solve_threats_in_n(table threats,
                                            slice_index si,
                                            stip_length_type n)
{
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_no_solution)
    series_solve_threats_in_n(threats,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Slice ***************
 */

/* Solve slice that is to be avoided
 * @param avoided slice to be avoided
 * @return true iff >=1 solution was found
 */
static boolean solve_avoided(slice_index avoided)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",avoided);
  TraceFunctionParamListEnd();

  output_start_unsolvability_mode();
  result = slice_solve(avoided);
  output_end_unsolvability_mode();

  if (result)
    write_end_of_solution();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_root_solve(slice_index si)
{
  boolean result;
  slice_index const length = slices[si].u.pipe.u.reflex_guard.length;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_avoided(avoided) || length==slack_length_direct)
    result = false;
  else
    result = slice_root_solve(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type reflex_guard_solve_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case defender_self_check:
      result = 0;
      break;

    case has_solution:
      result = n+2;
      break;

    case has_no_solution:
      result = direct_solve_in_n(next,n,n_min);
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean reflex_guard_root_make_setplay_slice(slice_index si,
                                             struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* TODO remove this check once STReflexGuard has been split up */
  if ((slices[si].u.pipe.u.branch.length-slack_length_direct)%2==0)
  {
    /* root_defender_filter */
    if (prod->sibling==no_slice)
    {
      /* semi-reflex stipulation */
      prod->sibling = slices[si].prev;
      assert(slices[prod->sibling].type==STDirectRoot);
    }

    if (slices[si].u.pipe.u.branch.length==slack_length_direct)
    {
      slice_index const proxy_to_goal = slices[si].u.pipe.u.branch.towards_goal;
      assert(slices[proxy_to_goal].type==STProxy);
      prod->setplay_slice = slices[proxy_to_goal].u.pipe.next;
    }
    else
      traverse_slices(slices[si].u.pipe.next,st);
  }
  else
  {
    /* root_attacker_filter */
    prod->sibling = si;
    traverse_slices(slices[si].u.pipe.next,st);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean reflex_guard_root_reduce_to_postkey_play(slice_index si,
                                                 struct slice_traversal *st)
{
  boolean result;
  slice_index *postkey_slice = st->param;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.pipe.u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (next==no_slice)
  {
    assert(avoided!=no_slice);
    *postkey_slice = avoided;
    dealloc_slice(si);
    result = true;
  }
  else
  {
    result = traverse_slices(next,st);

    if (*postkey_slice!=no_slice)
    {
      slice_index const length = slices[si].u.pipe.u.reflex_guard.length;

      if ((length-slack_length_direct)%2==1
          && length<=slack_length_direct+2)
        dealloc_slice(branch_deallocate_to_fork(avoided));

      dealloc_slice(si);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean reflex_guard_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

typedef struct
{
    slice_index to_be_avoided[2];
} init_param;

/* In alternate play, insert a STReflexGuard slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static boolean reflex_guards_inserter_branch(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  slice_index const proxy_to_goal = slices[si].u.pipe.u.branch.towards_goal;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (length==slack_length_direct+1)
  {
    /* insert an STReflexGuard slice that switches to the next branch
     */
    slice_index const proxy_to_avoided = param->to_be_avoided[1-length%2];
    slice_index const guard = alloc_reflex_guard(length-1,min_length-1,
                                                 proxy_to_goal,proxy_to_avoided);
    branch_link(si,guard);
  }
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const next_prev = slices[next].prev;
    if (next_prev!=si)
    {
      /* we are attached to a loop - attach to a newly created
       * STReflexGuard
       */
      if (slices[next_prev].type==STReflexGuard)
        /* attach to the one just created in the loop
         */
        pipe_set_successor(si,next_prev);
      else
      {
        /* create our own
         */
        slice_index const proxy_to_avoided = param->to_be_avoided[1-length%2];
        slice_index const guard = alloc_reflex_guard(length-1,min_length-1,
                                                     proxy_to_goal,proxy_to_avoided);
        pipe_set_successor(guard,next);
        branch_link(si,guard);
      }
    }
  }

  {
    slice_index const prev = slices[si].prev;
    slice_index const proxy_to_avoided = param->to_be_avoided[length%2];
    slice_index const guard = alloc_reflex_guard(length,min_length,
                                                 proxy_to_goal,proxy_to_avoided);
    branch_link(prev,guard);
    branch_link(guard,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* In series play, insert a STReflexGuard slice before a slice where
 * the reflex stipulation might force the side at the move to reach
 * the goal
 */
static boolean reflex_guards_inserter_series(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    init_param const * const param = st->param;
    slice_index const proxy_to_avoided = param->to_be_avoided[1];
    slice_index const proxy_to_goal = slices[si].u.pipe.u.branch.towards_goal;
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
    slice_index const prev = slices[si].prev;
    slice_index const guard = alloc_reflex_guard(length,min_length,
                                                 proxy_to_goal,proxy_to_avoided);
    branch_link(prev,guard);
    branch_link(guard,si);
    slice_traverse_children(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Prevent STReflexGuard slice insertion from recursing into the
 * following branch
 */
static boolean reflex_guards_inserter_branch_fork(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't traverse .towards_goal! */
  traverse_slices(slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static slice_operation const reflex_guards_inserters[] =
{
  &slice_traverse_children,            /* STProxy */
  &reflex_guards_inserter_branch,      /* STBranchDirect */
  &reflex_guards_inserter_branch,      /* STBranchDirectDefender */
  &reflex_guards_inserter_branch,      /* STBranchHelp */
  &reflex_guards_inserter_branch_fork, /* STHelpFork */
  &reflex_guards_inserter_series,      /* STBranchSeries */
  &reflex_guards_inserter_branch_fork, /* STSeriesFork */
  &slice_operation_noop,               /* STLeafDirect */
  &slice_operation_noop,               /* STLeafHelp */
  &slice_operation_noop,               /* STLeafForced */
  &slice_traverse_children,            /* STReciprocal */
  &slice_traverse_children,            /* STQuodlibet */
  &slice_traverse_children,            /* STNot */
  &slice_traverse_children,            /* STMoveInverter */
  &reflex_guards_inserter_branch,      /* STDirectRoot */
  &reflex_guards_inserter_branch,      /* STDirectDefenderRoot */
  0,                                   /* STDirectHashed */
  &slice_traverse_children,            /* STHelpRoot */
  0,                                   /* STHelpHashed */
  &slice_traverse_children,            /* STSeriesRoot */
  0,                                   /* STParryFork */
  0,                                   /* STSeriesHashed */
  0,                                   /* STSelfCheckGuard */
  0,                                   /* STDirectDefense */
  &slice_traverse_children,            /* STReflexGuard */
  0,                                   /* STSelfAttack */
  0,                                   /* STSelfDefense */
  0,                                   /* STRestartGuard */
  0,                                   /* STGoalReachableGuard */
  0,                                   /* STKeepMatingGuard */
  0,                                   /* STMaxFlightsquares */
  0,                                   /* STDegenerateTree */
  0,                                   /* STMaxNrNonTrivial */
  0                                    /* STMaxThreatLength */
};

/* Instrument a branch with STReflexGuard slices for a (non-semi)
 * reflex stipulation 
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided identifies proxy leading what the branch needs
 *                         to be guarded from
 */
void slice_insert_reflex_guards(slice_index si, slice_index proxy_to_avoided)
{
  slice_traversal st;
  init_param param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(slices[proxy_to_avoided].type==STProxy);

  {
    slice_index const avoided = slices[proxy_to_avoided].u.pipe.next;
    Goal const avoided_goal = slices[avoided].u.leaf.goal;
    slice_index const direct_avoided = alloc_leaf_slice(STLeafDirect,
                                                        avoided_goal);

    param.to_be_avoided[0] = proxy_to_avoided;
    param.to_be_avoided[1] = alloc_proxy_slice();
    branch_link(param.to_be_avoided[1],direct_avoided);

    slice_traversal_init(&st,&reflex_guards_inserters,&param);
    traverse_slices(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In battle play, insert a STReflexGuard slice after a defense slice
 * @param si identifies defense slice
 * @param address of structure representing the traversal
 */
static boolean reflex_guards_inserter_defense_semi(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    init_param const * const param = st->param;
    slice_index const proxy_to_goal = slices[si].u.pipe.u.branch.towards_goal;
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
    slice_index const prev = slices[si].prev;
    slice_index const proxy_to_avoided = param->to_be_avoided[length%2];
    slice_index const guard = alloc_reflex_guard(length,min_length,
                                                 proxy_to_goal,proxy_to_avoided);
    branch_link(prev,guard);
    branch_link(guard,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* In battle play, insert a STReflexGuard if we are at the end of the
 * branch
 * @param si identifies slice potentially at the end of the branch
 * @param st address of structure representing the traversal
 */
static boolean reflex_guards_inserter_attack_semi(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  init_param const * const param = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (length==slack_length_direct+1)
  {
    /* insert an STReflexGuard slice that switches to the next branch
     */
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
    slice_index const proxy_to_goal = slices[si].u.pipe.u.branch.towards_goal;
    slice_index const proxy_to_avoided = param->to_be_avoided[1-length%2];
    slice_index const guard = alloc_reflex_guard(length-1,min_length-1,
                                                 proxy_to_goal,proxy_to_avoided);
    branch_link(si,guard);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const reflex_guards_inserters_semi[] =
{
  &slice_traverse_children,             /* STProxy */
  &reflex_guards_inserter_attack_semi,  /* STBranchDirect */
  &reflex_guards_inserter_defense_semi, /* STBranchDirectDefender */
  &reflex_guards_inserter_branch,       /* STBranchHelp */
  &reflex_guards_inserter_branch_fork,  /* STHelpFork */
  &slice_traverse_children,             /* STBranchSeries */
  &reflex_guards_inserter_branch_fork,  /* STSeriesFork */
  &slice_operation_noop,                /* STLeafDirect */
  &slice_operation_noop,                /* STLeafHelp */
  &slice_operation_noop,                /* STLeafForced */
  &slice_traverse_children,             /* STReciprocal */
  &slice_traverse_children,             /* STQuodlibet */
  &slice_traverse_children,             /* STNot */
  &slice_traverse_children,             /* STMoveInverter */
  &reflex_guards_inserter_attack_semi,  /* STDirectRoot */
  &reflex_guards_inserter_defense_semi, /* STDirectDefenderRoot */
  0,                                    /* STDirectHashed */
  &slice_traverse_children,             /* STHelpRoot */
  0,                                    /* STHelpHashed */
  &slice_traverse_children,             /* STSeriesRoot */
  0,                                    /* STParryFork */
  0,                                    /* STSeriesHashed */
  0,                                    /* STSelfCheckGuard */
  0,                                    /* STDirectDefense */
  0,                                    /* STReflexGuard */
  0,                                    /* STSelfAttack */
  0,                                    /* STSelfDefense */
  0,                                    /* STRestartGuard */
  0,                                    /* STGoalReachableGuard */
  0,                                    /* STKeepMatingGuard */
  0,                                    /* STMaxFlightsquares */
  0,                                    /* STDegenerateTree */
  0,                                    /* STMaxNrNonTrivial */
  0                                     /* STMaxThreatLength */
};

/* Instrument a branch with STReflexGuard slices for a semi-reflex
 * stipulation 
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided identifies proxy leading what the branch needs
 *                         to be guarded from
 */
void slice_insert_reflex_guards_semi(slice_index si,
                                     slice_index proxy_to_avoided)
{
  slice_traversal st;
  init_param param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_avoided].type==STProxy);

  param.to_be_avoided[0] = proxy_to_avoided;
  param.to_be_avoided[1] = no_slice;

  slice_traversal_init(&st,&reflex_guards_inserters_semi,&param);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
