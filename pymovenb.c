#include "pymovenb.h"
#include "pypipe.h"
#include "pyhelp.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/* TODO rename variables to more meaningful names */
/* TODO make variables static */

/* number of current move at root level
 */
unsigned int MoveNbr;

/* number of first move at root level to be considered
 */
unsigned int RestartNbr;

/* Reset the restart number setting.
 */
void reset_restart_number(void)
{
  RestartNbr = 0;
  MoveNbr = 1;
}

unsigned int get_restart_number(void)
{
  return RestartNbr;
}

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
boolean read_restart_number(char const *optionValue)
{
  boolean result = false;

  char *end;
  unsigned long const restartNbrRequested = strtoul(optionValue,&end,10);
  if (optionValue!=end && restartNbrRequested<=UINT_MAX)
  {
    RestartNbr = (unsigned int)restartNbrRequested;
    result = true;
  }

  return result;
}

void IncrementMoveNbr(void)
{
  if (MoveNbr>=RestartNbr)
  {
    sprintf(GlobalStr,"%3u  (", MoveNbr);
    StdString(GlobalStr);
    ecritcoup(nbply,no_goal);

    if (!flag_regression)
    {
      StdString("   ");
      PrintTime();
    }

    StdString(")\n");
  }

  ++MoveNbr;
}

/* Initialise a STRestartGuard slice into an allocated and wired
 * pipe slice 
 * @param si identifies slice
 */
static void init_restart_guard_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STRestartGuard;
  slices[si].starter = slices[slices[si].u.pipe.next].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean restart_guard_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (MoveNbr<RestartNbr)
    result = false;
  else
    result = help_solve_in_n(slices[si].u.pipe.next,n);

  IncrementMoveNbr();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean restart_guards_inserter_help_root(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_insert_after(si);
  init_restart_guard_slice(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const restart_guards_inserters[] =
{
  &slice_operation_noop,              /* STBranchDirect */
  &slice_operation_noop,              /* STBranchDirectDefender */
  &slice_operation_noop,              /* STBranchHelp */
  &slice_operation_noop,              /* STBranchSeries */
  &slice_operation_noop,              /* STBranchFork */
  &slice_operation_noop,              /* STLeafDirect */
  &slice_operation_noop,              /* STLeafHelp */
  &slice_operation_noop,              /* STLeafSelf */
  &slice_operation_noop,              /* STLeafForced */
  &slice_traverse_children,           /* STReciprocal */
  &slice_traverse_children,           /* STQuodlibet */
  &slice_traverse_children,           /* STNot */
  &slice_traverse_children,           /* STMoveInverter */
  &restart_guards_inserter_help_root, /* STHelpRoot */
  &slice_traverse_children,           /* STHelpAdapter */
  &slice_traverse_children,           /* STHelpHashed */
  &slice_traverse_children,           /* STSelfCheckGuard */
  &slice_traverse_children,           /* STReflexGuard */
  0,                                  /* STRestartGuard */
  0,                                  /* STGoalReachableGuard */
  0                                   /* STKeepMatingGuard */
};

/* Instrument stipulation with STRestartGuard slices
 */
void stip_insert_restart_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&restart_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
