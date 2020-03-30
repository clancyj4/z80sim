#include "guiglb.h"

// char Disass_Str[64];
// char Opcode_Str[64];
int show_opcodes = 0;
int run_slow = 0;
int run_slow_pref;
char tstr[1024];		/* General temp string */
char lstr[1024];		/* temp string for log window ops */
char Dump_Reg;

struct IOPortStruct *IOPort[NUMIOPORTS];
int current_port = 0;
