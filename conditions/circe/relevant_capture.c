#include "conditions/circe/relevant_capture.h"
#include "conditions/circe/circe.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static slice_type determine_slice_type(circe_relevant_capture type)
{
  switch (type)
  {
    case circe_relevant_capture_thismove:
      return STCirceInitialiseFromCurrentMove;

    case circe_relevant_capture_lastmove:
      return STCirceInitialiseFromLastMove;

    case circe_relevant_capture_nocapture:
      return no_slice_type;

    default:
      assert(0);
      return no_slice_type;
  }
}

/* Instrument the solving machinery with the logic for determining which capture
 * is the basis of the rebirth in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_relevant_capture(slice_index si,
                                               circe_variant_type const *variant,
                                               slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",interval_start);
  TraceFunctionParamListEnd();

  {
    slice_type const type = determine_slice_type(variant->relevant_capture);
    if (type!=no_slice_type)
      circe_instrument_solving(si,interval_start,interval_start,alloc_pipe(type));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void circe_make_current_move_relevant_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->relevant_ply = nbply;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void circe_make_last_move_relevant_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->relevant_ply = parent_ply[nbply];

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
