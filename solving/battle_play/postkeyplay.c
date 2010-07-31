#include "stipulation/battle_play/postkeyplay.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_root.h"
#include "trace.h"

/* Allocate a STPostKeyPlaySuppressor defender slice.
 * @return index of allocated slice
 */
static slice_index alloc_postkeyplay_suppressor_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPostKeyPlaySuppressor);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
postkeyplay_suppressor_defend_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
postkeyplay_suppressor_can_defend_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable,
                                       unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(next,n,n_max_unsolvable,max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Remove the STContinuationSolver slice not used in postkey play
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void substitute_defense_root(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_replace(si,alloc_defense_root_slice(length,min_length));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors postkey_handler_inserters[] =
{
  { STContinuationSolver,         &substitute_defense_root     },
  { STHelpRoot,                   &stip_structure_visitor_noop },
  { STSeriesRoot,                 &stip_structure_visitor_noop }
};

enum
{
  nr_postkey_handler_inserters = (sizeof postkey_handler_inserters
                                  / sizeof postkey_handler_inserters[0])
};

/* Instrument the stipulation representation so that it can write
 * variations
 */
void stip_insert_postkey_handlers(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,
                                postkey_handler_inserters,
                                nr_postkey_handler_inserters,
                                0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void append_postkeyplay_suppressor(slice_index si,
                                          stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length>slack_length_battle)
    pipe_append(si,alloc_postkeyplay_suppressor_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors postkey_suppressor_inserters[] =
{
  { STContinuationSolver,         &append_postkeyplay_suppressor },
  { STHelpRoot,                   &stip_structure_visitor_noop   },
  { STSeriesRoot,                 &stip_structure_visitor_noop   }
};

enum
{
  nr_postkey_suppressor_inserters = (sizeof postkey_suppressor_inserters
                                     / sizeof postkey_suppressor_inserters[0])
};

/* Instrument the stipulation representation so that post key play is
 * suppressed from output
 */
void stip_insert_postkeyplay_suppressors(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,
                                postkey_suppressor_inserters,
                                nr_postkey_suppressor_inserters,
                                0);
  stip_traverse_structure(root_slice,&st);

  TraceStipulation(root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
