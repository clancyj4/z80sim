#include <gtk/gtk.h>
#include "guiglb.h"

char Disass_Str[64];
char Opcode_Str[64];
int show_opcodes = 0;
int run_slow = 0;
int run_slow_pref;
char tstr[1024];		/* General temp string */
char lstr[1024];		/* temp string for log window ops */
char Dump_Reg;

char FlagEnt[NFLAGS][8] =
{
  "entry_S",
  "entry_Z",
  "entry_Y",
  "entry_H",
  "entry_X",
  "entry_P",
  "entry_N",
  "entry_C"
};

GtkWidget *   codetext;
GtkWidget *   optcodeb;
GtkWidget *   slowrunspin;
GtkWidget *   flentry[NFLAGS];
GtkWidget *   sflentry;
GtkWidget *   logtext;
GtkWidget *   a_entry;
GtkWidget *   bc_entry;
GtkWidget *   de_entry;
GtkWidget *   hl_entry;
GtkWidget *   ix_entry;
GtkWidget *   iy_entry;
GtkWidget *   pc_entry;
GtkWidget *   sp_entry;
GtkWidget *   i_entry;
GtkWidget *   r_entry;
GtkWidget *   aa_entry;
GtkWidget *   abc_entry;
GtkWidget *   ade_entry;
GtkWidget *   ahl_entry;
GtkWidget *   mem_entry;
GtkWidget *   memtext;
GtkWidget *   Mem_Addr_Entry;
GtkWidget *   Mem_Content_Entry;
GtkWidget *   about_win;
GtkWidget *   FCwin;
GtkWidget *   FSwin;
GtkWidget *   FPwin;

char *project_fn;
GtkTextBuffer *mem_textbuffer;
gboolean Memory_Selected;
int mem_sel_row, mem_sel_col;
int mem_buff_row_start;
BYTE Selected_Memory_Content;
WORD Selected_Memory_Addr;

GtkTextBuffer *code_textbuffer;
gint selected_code_line;      	         /* persistent record of last selected line */
WORD Selected_Code_Addr;
WORD codelines[K64K];
int buff_lines_start;           /* line at which the buffer starts */
gboolean Code_Selected;

struct IOPortStruct *IOPort[NUMIOPORTS];
GtkWidget *   inportprompt;
int current_port = 0;
GtkWidget *ioport_win;
GtkWidget *ioporttext;
GtkWidget *ioportintext;
GtkWidget *ioinqueuetext;

GtkWidget *logtext;
GtkWidget *Log;

