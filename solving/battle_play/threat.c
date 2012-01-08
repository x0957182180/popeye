#include "solving/battle_play/threat.h"
#include "pytable.h"
#include "pydata.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/attack_play.h"
#include "solving/battle_play/check_detector.h"
#include "trace.h"

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

threat_activity threat_activities[maxply+1];

/* count threats not defeated by a defense while we are
 * threat_enforcing
 */
static unsigned int nr_threats_to_be_confirmed;

/* Allocate a STThreatEnforcer slice.
 * @param threat_start identifies the slice where threat play starts
 * @return index of allocated slice
 */
static slice_index alloc_threat_enforcer_slice(slice_index threat_start)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",threat_start);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STThreatEnforcer,threat_start);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
threat_enforcer_can_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_attack(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type threat_enforcer_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.fork.next;
  slice_index const threat_start = slices[si].u.fork.fork;
  ply const threats_ply = nbply+1;
  stip_length_type const len_threat = threat_lengths[threats_ply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",len_threat);

  if (len_threat<=slack_length_battle)
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

    threat_activities[threats_ply] = threat_enforcing;
    len_test_threats = can_attack(threat_start,len_threat);
    threat_activities[threats_ply] = threat_idle;

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

/* Allocate a STThreatCollector slice.
 * @return index of allocated slice
 */
static slice_index alloc_threat_collector_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STThreatCollector);

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
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type threat_collector_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(next,n);

  TraceValue("%u\n",nbply);
  if (threat_activities[nbply]==threat_solving
      && slack_length_battle<=result && result<=n)
    append_to_top_table();

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=acceptable number of refutations found
           n+4 refuted - >acceptable number of refutations found
 */
stip_length_type threat_collector_can_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_defend(next,n);

  if (threat_activities[nbply]==threat_enforcing
      && n>=threat_lengths[nbply]-2)
  {
    if (is_current_move_in_table(threats[nbply]))
    {
      if (slack_length_battle<=result && result<=n)
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

/* Allocate a STThreatSolver defender slice.
 * @param threat_start identifies the slice where threat play starts
 * @return index of allocated slice
 */
static slice_index alloc_threat_solver_slice(slice_index threat_start)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",threat_start);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STThreatSolver,threat_start);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_check_threat_solver(slice_index si,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't depend on generic order of traversal */
  stip_traverse_structure_pipe(si,st);
  stip_traverse_structure(slices[si].u.fork.fork,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve threats after an attacker's move
 * @param si slice index
 * @return length of threats
 *         <slack_length_battle if the attacker has something stronger
 *             than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
static stip_length_type solve_threats(slice_index si, stip_length_type n)
{
  slice_index const enforcer = slices[si].u.fork.fork;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* insert an empty ply for the virtual defense played before the threat */
  nextply(nbply);
  max_unsolvable[nbply] = slack_length_battle;
  result = attack(enforcer,n);
  finply();

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
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type threat_solver_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  ply const threats_ply = nbply+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==max_unsolvable[nbply])
    result = defend(next,n);
  else
  {
    TraceValue("%u\n",threats_ply);
    threats[threats_ply] = allocate_table();

    if (!attack_gives_check[nbply])
    {
      threat_activities[threats_ply] = threat_solving;
      threat_lengths[threats_ply] = solve_threats(si,n-1);
      threat_activities[threats_ply] = threat_idle;
    }

    result = defend(next,n);

    assert(get_top_table()==threats[threats_ply]);
    free_table();
    threat_lengths[threats_ply] = no_threats_found;
    threats[threats_ply] = table_nil;
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type threat_solver_can_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_defend(next,n);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Prepend a threat writer slice to a defense move slice
 * @param si identifies slice around which to insert threat handlers
 * @param st address of structure defining traversal
 */
static void append_threat_solver(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.branch.length>slack_length_battle+1)
  {
    {
      slice_index const prototype = alloc_pipe(STThreatStart);
      battle_branch_insert_slices(si,&prototype,1);
    }

    {
      slice_index const start = branch_find_slice(STThreatStart,si);
      slice_index const prototypes[] =
      {
        alloc_threat_solver_slice(start),
        alloc_threat_enforcer_slice(start),
        alloc_threat_collector_slice()
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      assert(start!=no_slice);
      battle_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation representation so that it can deal with
 * threats
 * @param si identifies slice where to start
 */
void stip_insert_threat_handlers(slice_index si)
{
  stip_structure_traversal st;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STReadyForDefense,
                                           &append_threat_solver);
  stip_traverse_structure(si,&st);

  for (i = 0; i<=maxply; ++i)
  {
    threat_lengths[i] = no_threats_found;
    threats[i] = table_nil;
    threat_activities[i] = threat_idle;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
