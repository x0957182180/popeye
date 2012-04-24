#include "solving/battle_play/threat.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/dummy_move.h"
#include "stipulation/move_played.h"
#include "stipulation/battle_play/branch.h"
#include "solving/solving.h"
#include "solving/battle_play/check_detector.h"
#include "solving/avoid_unsolvable.h"
#include "utilities/table.h"
#include "debugging/trace.h"

#include <assert.h>

/* Table where threats of the various move levels are collected
 */
static table threats[maxply+1];

/* Lengths of threats of the various move levels
 */
static stip_length_type threat_lengths[maxply+1];

/* value of threat_lengths[N] that signals that no threats were found
 * in ply N (and that threats[N] may not even refer to a table)
 */
static stip_length_type const no_threats_found = UINT_MAX;

/* count threats not defeated by a defense while we are
 * threat_enforcing
 */
static unsigned int nr_threats_to_be_confirmed;

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type threat_defeated_tester_defend(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(next,n);

  if (n>=threat_lengths[nbply]-2)
  {
    if (is_current_move_in_table(threats[nbply]))
    {
      if (slack_length<=result && result<=n)
      {
        --nr_threats_to_be_confirmed;
        if (nr_threats_to_be_confirmed>0)
          /* threats tried so far still work (perhaps shorter than
           * before the current defense), but we haven't tried all
           * threats yet -> don't stop the iteration over the
           * attacking moves
           */
          result = n+2;
      }
      else if (n==threat_lengths[nbply]-1)
        /* we have found a defeated threat -> stop the iteration */
        result = n;
    }
    else
      /* not a threat -> don't stop the iteration */
      result = n+2;
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type threat_collector_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(next,n);

  if (slack_length<=result && result<=n)
    append_to_top_table();

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type threat_solver_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  ply const threats_ply = nbply+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",threats_ply);
  threats[threats_ply] = allocate_table();

  if (!attack_gives_check[nbply])
    threat_lengths[threats_ply] = defend(slices[si].next2,n)-1;

  result = defend(next,n);

  assert(get_top_table()==threats[threats_ply]);
  free_table();
  threat_lengths[threats_ply] = no_threats_found;
  threats[threats_ply] = table_nil;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type threat_enforcer_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  slice_index const threat_start = slices[si].next2;
  ply const threats_ply = nbply+1;
  stip_length_type const len_threat = threat_lengths[threats_ply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",len_threat);

  if (len_threat<=slack_length)
    /* the attack has something stronger than threats (typically, it
     * delivers check)
     */
    result = attack(next,n);
  else if (len_threat<=n)
  {
    /* there are >=1 threats - don't report variations shorter than
     * the threats or variations that don't refute any threat
     */
    table const threats_table = threats[threats_ply];
    stip_length_type len_test_threats;

    nr_threats_to_be_confirmed = table_length(threats_table);

    len_test_threats = attack(threat_start,len_threat);

    if (len_test_threats>len_threat)
      /* variation is longer than threat */
      result = attack(next,n);
    else if (len_test_threats>len_threat-2 && nr_threats_to_be_confirmed>0)
      /* variation has same length as the threat(s), but it has
       * defeated at least one threat
       */
      result = attack(next,n);
    else
      /* variation is shorter than threat */
      result = len_test_threats;
  }
  else
    /* zugzwang, or we haven't looked for threats yet */
    result = attack(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a threat enforcer slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_threat_enforcer(slice_index si,
                                           stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
    stip_spin_off_testers_pipe_skip(si,st);
  else
  {
    /* trust in our descendants to start spinning off before the traversal
     * reaches our tester */
    stip_traverse_structure_children(si,st);
    assert(state->spun_off[slices[si].next2]!=no_slice);
  }

  slices[si].next2 = state->spun_off[slices[si].next2];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* End copying on the visited slice, by moving it to the copy and linking it
 * to a proxy slice that takes its original place
 * @param si visited slice
 * @param st structure representing the copying traversal
 */
static void stop_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = copy_slice(si);
  pipe_substitute(si,alloc_proxy_slice());
  link_to_branch((*copies)[si],si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Create a shallow copy of the visited fork slice
 * @param si visited slice
 * @param st structure representing the copying traversal
 */
static void copy_shallow(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = copy_slice(si);

  stip_traverse_structure_children_pipe(si,st);

  if (slices[si].next1!=no_slice)
    link_to_branch((*copies)[si],(*copies)[slices[si].next1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off the sequence of slices that enforce threats
 * @param si threat enforcer slice
 * @param st structure representing the traversal looking for entry points
 */
static void spin_off_from_threat_enforcer(slice_index si,
                                          stip_structure_traversal *st)
{
  stip_deep_copies_type copies;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STThreatDefeatedTester);
    attack_branch_insert_slices(slices[si].next2,&prototype,1);
  }

  init_deep_copy(&st_nested,&copies);
  st_nested.context = stip_traversal_context_attack;
  stip_structure_traversal_override_single(&st_nested,
                                           STThreatDefeatedTester,
                                           &stop_copying);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_conditional_pipe,
                                                &copy_shallow);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_testing_pipe,
                                                &copy_shallow);
  stip_traverse_structure(slices[si].next2,&st_nested);

  slices[si].next2 = copies[slices[si].next2];

  {
    /* if the threats are short, max_unsolvable might interfere with enforcing
     * them */
    slice_index const prototype = alloc_reset_unsolvable_slice();
    attack_branch_insert_slices(slices[si].next2,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void serve_as_end_of_copy(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off sequences of slices that solve and enforce threats
 * @param si threat solver slice
 */
static void spin_off_enforcers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STThreatEnforcer,
                                           &spin_off_from_threat_enforcer);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_solver(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>slack_length+1)
  {
    slice_index const prototype = alloc_testing_pipe(STThreatSolver);
    defense_branch_insert_slices(si,&prototype,1);

    stip_traverse_structure_children(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void connect_solver_to_threat_start(slice_index si,
                                           stip_structure_traversal *st)
{
  slice_index const * const threat_start = st->param;
  stip_deep_copies_type copies;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  init_deep_copy(&st_nested,&copies);
  st_nested.context = stip_traversal_context_defense;
  stip_structure_traversal_override_single(&st_nested,
                                           STThreatEnd,
                                           &serve_as_end_of_copy);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_conditional_pipe,
                                                &copy_shallow);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_testing_pipe,
                                                &copy_shallow);
  stip_traverse_structure(*threat_start,&st_nested);

  slices[si].next2 = alloc_proxy_slice();
  link_to_branch(slices[si].next2,copies[*threat_start]);

  {
    slice_index const prototypes[] = {
        alloc_dummy_move_slice(),
        alloc_move_played_slice(),
        alloc_pipe(STThreatCollector)
    };
    defense_branch_insert_slices_behind_proxy(slices[si].next2,prototypes,3,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_threat_start(slice_index si, stip_structure_traversal *st)
{
  slice_index * const threat_start = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *threat_start = si;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_threat_start(slice_index si, stip_structure_traversal *st)
{
  slice_index * const threat_start = st->param;
  slice_index const save_threat_start = *threat_start;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *threat_start = save_threat_start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void filter_output_mode(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_tree)
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const threat_solver_inserters[] =
{
  { STOutputModeSelector, &filter_output_mode                    },
  { STSetplayFork,        &stip_traverse_structure_children_pipe },
  { STReadyForDefense,    &insert_solver                         },
  { STThreatSolver,       &connect_solver_to_threat_start        },
  { STRefutationsSolver,  &stip_traverse_structure_children_pipe },
  { STThreatStart,        &remember_threat_start                 },
  { STAttackAdapter,      &forget_threat_start                   },
  { STDefenseAdapter,     &forget_threat_start                   }
};

enum
{
  nr_threat_solver_inserters = (sizeof threat_solver_inserters
                                / sizeof threat_solver_inserters[0])
};

static void insert_solvers(slice_index si)
{
  stip_structure_traversal st;
  slice_index threat_start = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&threat_start);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    threat_solver_inserters,
                                    nr_threat_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation representation so that it can deal with
 * threats
 * @param si identifies slice where to start
 */
void stip_insert_threat_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_solvers(si);
  spin_off_enforcers(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void end_insertion_if_too_short(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>slack_length+1)
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_enforcer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_attack)
  {
    slice_index const prototypes[] =
    {
      alloc_testing_pipe(STThreatEnforcer),
      alloc_pipe(STThreatStart)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    attack_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_end(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STThreatEnd);
    attack_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void connect_enforcer_to_threat_start(slice_index si,
                                             stip_structure_traversal *st)
{
  slice_index const * const threat_start = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  assert(*threat_start!=no_slice);
  slices[si].next2 = *threat_start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const threat_enforcer_inserters[] =
{
  { STOutputModeSelector, &filter_output_mode                    },
  { STSetplayFork,        &stip_traverse_structure_children_pipe },
  { STReadyForDefense,    &end_insertion_if_too_short            },
  { STNotEndOfBranchGoal, &insert_enforcer                       },
  { STReadyForAttack,     &insert_end                            },
  { STThreatEnforcer,     &connect_enforcer_to_threat_start      },
  { STRefutationsSolver,  &stip_traverse_structure_children_pipe },
  { STThreatStart,        &remember_threat_start                 },
  { STAttackAdapter,      &forget_threat_start                   },
  { STDefenseAdapter,     &forget_threat_start                   }
};

enum
{
  nr_threat_enforcer_inserters = (sizeof threat_enforcer_inserters
                                  / sizeof threat_enforcer_inserters[0])
};

/* Instrument the stipulation representation so that it can deal with
 * threats
 * @param si identifies slice where to start
 */
void stip_insert_threat_enforcers(slice_index si)
{
  stip_structure_traversal st;
  slice_index threat_start = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&threat_start);
  stip_structure_traversal_override(&st,
                                    threat_enforcer_inserters,
                                    nr_threat_enforcer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
