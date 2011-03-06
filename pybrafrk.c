#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param to_goal identifies proxy slice that leads towards goal
 *                from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch_fork(SliceType type, slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.branch_fork.towards_goal = to_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void branch_fork_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  if (slices[si].u.branch_fork.towards_goal!=no_slice)
  {
    stip_traverse_structure(slices[si].u.branch_fork.towards_goal,st);
    proxy_slice_resolve(&slices[si].u.branch_fork.towards_goal,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void branch_fork_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(towards_goal,st);

  if (slices[si].starter==no_side)
  {
    if (slices[towards_goal].starter==no_side)
    {
      stip_traverse_structure_pipe(si,st);
      slices[si].starter = slices[slices[si].u.pipe.next].starter;
    }
    else
      slices[si].starter = slices[towards_goal].starter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
