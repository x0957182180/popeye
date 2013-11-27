#include "pieces/walks/sting.h"
#include "pieces/walks/hoppers.h"
#include "pieces/walks/leapers.h"
#include "solving/find_square_observer_tracking_back_from_target.h"

/* Generate moves for a Sting
 */
void sting_generate_moves(void)
{
  rider_hoppers_generate_moves(vec_queen_start,vec_queen_end);
  leaper_generate_moves(vec_queen_start,vec_queen_end);
}

boolean sting_check(validator_id evaluate)
{
  return ((*checkfunctions[King])(evaluate)
          || (*checkfunctions[Grasshopper])(evaluate));
}
