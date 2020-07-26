#ifndef CN_COMMAND_LINE_PARSE_H
#define CN_COMMAND_LINE_PARSE_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The current state of parsing command line arguments, without binding a
 * parsing algorithm directly to the state.
 */
typedef struct {
	int argc;
	char** argv;
	int numArgsParsed;
} CnCommandLineParse;

CnCommandLineParse cnCommandLineParse_Make(int argc, char** argv);
bool cnCommandLineParse_ShouldContinue(const CnCommandLineParse* parse);
bool cnCommandLineParse_IsComplete(const CnCommandLineParse* parse);
bool cnCommandLineParse_HasLookAhead(const CnCommandLineParse* parse, int amount);
const char* cnCommandLineParse_LookAhead(const CnCommandLineParse* parse, int amount);
int cnCommandLineParse_LookAheadIndex(const CnCommandLineParse* parse, int amount);
void cnCommandLineParse_Advance(CnCommandLineParse* parse, int argsParsed);

#ifdef __cplusplus
}
#endif

#endif /* CN_COMMAND_LINE_PARSE_H */
