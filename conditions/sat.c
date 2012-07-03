#include "conditions/sat.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/temporary_hacks.h"
#include "solving/legal_move_counter.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean SATCheck;
boolean StrictSAT[nr_sides][maxply+1];
int SATFlights[nr_sides];
boolean satXY;

static ply sat_check_ply_id;

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type sat_flight_moves_generator_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  boolean const save_mummer = flagmummer[starter];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(SATCheck);

  nextply(sat_check_ply_id);

  current_killer_state= null_killer_state;
  trait[nbply]= starter;

  flagmummer[starter] = false;
  dont_generate_castling= true;
  if (starter==White)
    gen_wh_piece(king_square[starter],abs(e[king_square[starter]]));
  else
    gen_bl_piece(king_square[starter],-abs(e[king_square[starter]]));
  dont_generate_castling = false;
  flagmummer[starter] = save_mummer;
  SATCheck = false;

  result = attack(slices[si].next1,n);

  SATCheck = true;
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void substitute_generator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const generator = branch_find_slice(STMoveGenerator,slices[si].next2,st->context);
    assert(generator!=no_slice);
    pipe_substitute(generator,alloc_pipe(STSATFlightMoveGenerator));
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation with SAT specific king flight move generators
 * @param root_slice root slice of stipulation
 */
void stip_substitute_sat_king_flight_generators(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STSATFlightsCounterFork,
                                           &substitute_generator);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side is in SAT check
 * @param side side for which to test check
 * @return true iff side is in check
 */
boolean echecc_SAT(ply ply_id, Side side)
{
  boolean result;
  int nr_flights = SATFlights[side];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (satXY || (CondFlag[strictSAT] && StrictSAT[side][parent_ply[ply_id]]))
  {
    SATCheck = false;
    if (!echecc(ply_id,side))
      --nr_flights;
    SATCheck = true;
  }

  if (nr_flights==0)
    result = true;
  else
  {
    sat_check_ply_id = ply_id;

    legal_move_counter_interesting[ply_id+1] = nr_flights-1;
    assert(legal_move_counter_count[ply_id+1]==0);

    result = (attack(slices[temporary_hack_sat_flights_counter[side]].next2,
                     length_unspecified+1)
              == length_unspecified+1);

    assert(result
           ==(legal_move_counter_count[ply_id+1]
              >legal_move_counter_interesting[ply_id+1]));
    legal_move_counter_count[ply_id+1] = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
