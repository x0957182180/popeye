#if !defined(CONDITIONS_OSCILLATING_KINGS_H)
#define CONDITIONS_OSCILLATING_KINGS_H

/* This module implements the condition Royal Dynasty */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

extern boolean OscillatingKingsTypeB[nr_sides];
extern boolean OscillatingKingsTypeC[nr_sides];
extern boolean oscillatedKs[toppile+1];

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type king_oscillator_attack(slice_index si, stip_length_type n);

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
stip_length_type king_oscillator_defend(slice_index si, stip_length_type n);

/* Instrument a stipulation for strict SAT
 * @param si identifies root slice of stipulation
 */
void stip_insert_king_oscillators(slice_index si);

#endif
