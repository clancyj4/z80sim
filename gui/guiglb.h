/* Stuff for the GTK+ GUI */

#ifndef GUIGLB_H

#define GUIGLB_H

#include <gtk/gtk.h>
#include "sim.h"

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
#define PORT_HEX_LEN    16      /* Length of entries in a port hex dump */

#define NFLAGS  8

typedef struct IOPortStruct
{
  char obuffer[IOOUTBUFLEN];
  int out_ptr;
  char ibuffer[IOINBUFLEN];
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
extern char	FlagEnt[NFLAGS][8];

extern GtkWidget *   codetext;
extern GtkWidget *   optcodeb;
extern GtkWidget *   slowrunspin;
extern GtkWidget *   flentry[NFLAGS];
extern GtkWidget *   sflentry;
extern GtkWidget *   logtext;
extern GtkWidget *   a_entry;
extern GtkWidget *   bc_entry;
extern GtkWidget *   de_entry;
extern GtkWidget *   hl_entry;
extern GtkWidget *   ix_entry;
extern GtkWidget *   iy_entry;
extern GtkWidget *   pc_entry;
extern GtkWidget *   sp_entry;
extern GtkWidget *   i_entry;
extern GtkWidget *   r_entry;
extern GtkWidget *   aa_entry;
extern GtkWidget *   abc_entry;
extern GtkWidget *   ade_entry;
extern GtkWidget *   ahl_entry;
extern GtkWidget *   mem_entry;
extern GtkWidget *   bc_button;
extern GtkWidget *   de_button;
extern GtkWidget *   hl_button;
extern GtkWidget *   ix_button;
extern GtkWidget *   iy_button;
extern GtkWidget *   pc_button;
extern GtkWidget *   sp_button;
extern GtkWidget *   m_button;
extern GtkWidget *   ioporttext;
extern GtkWidget *   ioportintext;
extern GtkWidget *   ioinqueuetext;
extern GtkWidget *   ioportspin;
extern GtkWidget *   ioasciihex;
extern GtkWidget *   memtext;
extern GtkWidget *   Mem_Addr_Entry;
extern GtkWidget *   Mem_Content_Entry;
extern GtkWidget *   about_win;
extern GtkWidget *   FCwin;
extern GtkWidget *   FSwin;
extern GtkWidget *   FPwin;
extern GtkWidget *   logmenuitem;
extern GtkWidget *   iomenuitem;
extern GtkWidget *   breaksmenuitem;

extern char *project_fn;
extern GtkTextBuffer *mem_textbuffer;
extern gboolean Memory_Selected;
extern int mem_sel_row, mem_sel_col;
extern int mem_buff_row_start;
extern BYTE Selected_Memory_Content;
extern WORD Selected_Memory_Addr;

extern GtkWidget *   inportprompt;

#define BREAK_OP 0x76
#define K64K (64 * 1024)
#define BACKUP_LINES 24         /* was 12 */

extern GtkTextBuffer *code_textbuffer;
extern gint selected_code_line;         /* persistent record of last selected line */
extern WORD Selected_Code_Addr;
extern WORD codelines[K64K];
extern int buff_lines_start;       /* line at which the buffer starts */
extern gboolean Code_Selected;

extern GtkWidget *ioport_win;

extern GtkWidget *logtext;
extern GtkWidget *Log;

extern GtkWidget *breakstext;
extern GtkWidget *breakspassspin;
extern GtkWidget *Breaks;
#endif
