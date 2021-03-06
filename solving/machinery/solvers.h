#if !defined(SOLVING_SOLVERS_H)
#define SOLVING_SOLVERS_H

#include "stipulation/stipulation.h"

/* Instrument the slices representing the stipulation with solving slices
 * @param solving_machinery proxy slice into the solving machinery to be built
 */
void build_solvers1(slice_index solving_machinery);
void build_solvers2(slice_index solving_machinery);

#endif
