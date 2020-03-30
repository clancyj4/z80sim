/* Stuff for the GTK+ GUI */

#define DEFAULT_MEMDUMP_BYTES	512		/* bytes for memdump */
#define CODE_LIST_LENGTH	1024		/* lines of disassembly */

/* default run slow time in milliseconds */
/* NB. The GUI widget is primed to accept between 10mS and 10 Secs */

#define RUN_SLOW_MSECS		10

/* how often the main GUI loop is called during a run slow wait */

#define RUN_SLOW_TICK		10000		/* 10 mS */

/* I/O port definitions */

#define NUMIOPORTS	256	/* number of ports to manage */
				/* rumor has it that the Z80 can address */
				/* 64K addresses - if so, change this */
#define IOOUTBUFLEN	2048	/* length of output buffer in bytes */
#define IOINBUFLEN	256	/* length of input buffer in bytes */

typedef struct IOPortStruct
{
  unsigned char obuffer[IOOUTBUFLEN];
  int out_ptr;
  unsigned char ibuffer[IOINBUFLEN];
  int in_ptr;
  int in_len;
  int ishex;
} IOPortStruct;

extern char	Disass_Str[];
extern char	Opcode_Str[];
extern int	show_opcodes;
extern int	run_slow;
extern int	run_slow_pref;

extern char	tstr[];
extern char	lstr[];
extern char	Dump_Reg;
extern struct	IOPortStruct *IOPort[];
extern int	current_port;
