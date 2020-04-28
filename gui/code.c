/*
        Z80 Emulator GUI

        Copyright (C) 2008 Justin Clancy
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sim.h"
#include "simglb.h"
#include "guiglb.h"
#include "prototypes.h"

static GtkTextTag *redtag, *greentag, *bluetag, *boldtag;

/*
 * CONCEPTS
 *
 * LINES are lines of disassembly - every byte in the address space has an
 * associated line; for example: a 3 byte opcode will only take up one line as
 * will a 1 byte opcode.
 * The idea is that one line relates to one line of disassembled code. So while
 * there is 64K of address space, the number of lines can vary from 65,536 (all
 * NOPs) to something rather less (a real program). To emphasise that these are
 * lines which cover the entire address space they are sometimes referred to
 * as CODE LINES.
 *
 * BUFFERED LINES result from the fact that disassembly is done in chunks.
 * The program keeps track of where the PC is and keeps a buffered disassembly
 * centered around that. Think of it as a large (but invisible) window on the
 * entire disassembly. All buffered lines are viewable in the code window, but
 * one might need to use the scrollbar to see them. They are held
 * within a gtk_text_buffer.
 *
 * VISIBLE LINES are a subset of buffered lines. These are the parts of
 * the text buffer which are immediately visible in the code window without
 * having to use the scrollbar. If one uses the scrollbar, the set of
 * visible lines changes.
 *
 */


void dump_code_vars()
{
//  int i;
  GtkTextIter aniter;
//  GtkTextMark *mark;
  GdkRectangle coderect;
  int vbuf_start, vbuf_end;

  printf("+++ CODE +++\n");
  printf("selected_code_line=%d, Selected_Code_Addr=%04X, Code_Selected=%d\n",
         selected_code_line, Selected_Code_Addr, Code_Selected);
  printf("ram=%04X, SP=%04X, PC=%04X\n",
	(WORD)ram, (WORD)(STACK - ram),
	(WORD)(PC - ram));
  do_break("\n");
//  printf("Codeline table\n"); 
//  for (i = 0; i < 4; i++) 
//    printf("addr=%04X line=%d\n", i, codelines[i]);
  gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(codetext), &coderect);
  gtk_text_view_get_line_at_y(GTK_TEXT_VIEW(codetext),
		&aniter, coderect.y, NULL);
  vbuf_start = gtk_text_iter_get_line(&aniter);
  gtk_text_view_get_line_at_y(GTK_TEXT_VIEW(codetext),
		&aniter, coderect.y+coderect.height, NULL);
  vbuf_end = gtk_text_iter_get_line(&aniter);

  printf("coderect: x=%d y=%d width=%d height=%d\n",
	coderect.x, coderect.y, coderect.width, coderect.height);
  printf("visible: start=%d end=%d\n", vbuf_start, vbuf_end);
  printf("--- CODE ---\n");
}


/* Do all the stuff necessary if a new binary is loaded */

void Do_Code_Reload()
{
  int i;

  Code_Selected = FALSE;			/* old selections now irrelevant */
  build_code_cache();				/* rebuild the cache */
  darken_code(-1, redtag);			/* remove all old tags */
  darken_code(-1, bluetag);
  darken_code(-1, greentag);
  for (i = 0; i < SBSIZE; i++)			/* clear all BPs */
    soft[i].sb_pass = 0;
}

  
/* Get the address for the given line */
/* Yes - I know this could be hashed or whatever */

WORD line2addr(unsigned int line)
{
  WORD code_ptr;

  code_ptr = 0;					/* scan the codelines table */
  while (codelines[code_ptr] != line)		/*..for the addr */
    code_ptr++;

  return(code_ptr);
}


  /*
   * Disassemble the entire address space and for each byte in the address
   * space record the line of disassembly. This assumes that *all* the
   * address space is code, but it shouldn't matter unless someone sets the
   * PC to point to data (which could be interesting, anyway).
   * Obviously this makes debugging self modifying code impossible.
   */

void build_code_cache(void)
{
  int i;
  unsigned int line;
  BYTE *p, *oldp;

  line = 0;
  p = PC;
  while (p < (PC + K64K))			/* scan entire Z80 mem space */
  {
    Disass_Str[0] = 0;    			/* reset the disass string */
    oldp = p;					/* get the start point */
    disass(&p, p - ram);			/* get the operation length */
    for (i = 0; i < (int)(p - oldp); i++)	/* set all addrs to this line */
    {
      codelines[(oldp - ram) + i] = line;
    }
    line++;					/* next line */
  }
}


  /* Take a code window selected address and set the PC to it */

void set_PC_to_selection(void)
{
  if (!Code_Selected)
  {
    show_error("Unable to set PC to selected address - no code selected.\n");
    return;
  }

  PC = Selected_Code_Addr + ram;

  Show_Registers();
  Dump_From_Register(Dump_Reg);
  Show_Code(PC, FALSE);
  Code_Selected = FALSE;
  do_code_highlights();
}


  /*
   * Remove a tag from the code window. If line is -1 does the
   * whole buffer otherwise just that buffered line.
   */

void darken_code(int line, GtkTextTag *tag)
{
  GtkTextIter start, end;

  if (line == -1)
  {
    gtk_text_buffer_get_start_iter(code_textbuffer, &start);
    gtk_text_buffer_get_end_iter(code_textbuffer, &end);
  }
  else
  {
    gtk_text_buffer_get_iter_at_line(code_textbuffer, &start, line);
    end = start;
    gtk_text_iter_forward_to_line_end(&end);
  }

  gtk_text_buffer_remove_tag(code_textbuffer, tag, &start, &end);
}


  /* Apply a tag to a buffered line */

void highlight_code(int line, GtkTextTag *hightag)
{
  GtkTextIter start, end;

  gtk_text_buffer_get_iter_at_line(code_textbuffer, &start, line);
  end = start;
  gtk_text_iter_forward_to_line_end(&end);
  gtk_text_buffer_apply_tag(code_textbuffer, hightag, &start, &end);
}


  /*
   * Invoked when the code window is clicked.
   * Sets the 'Code_Selected' flag, selected_code_line to the selected line and
   * highlights the new selection.
   */

void Code_Clicked(void)
{
  gint newline;
  GtkTextIter aniter;

  gtk_text_buffer_get_iter_at_mark(code_textbuffer, &aniter, gtk_text_buffer_get_insert(code_textbuffer));

  newline = gtk_text_iter_get_line(&aniter);

  if (newline != selected_code_line || Code_Selected == FALSE)
  {
    selected_code_line = newline + buff_lines_start;
    Selected_Code_Addr = line2addr(selected_code_line);
    Code_Selected = TRUE;
//printf("code: newline=%d selected_code_line=%d Code_Selected=%d\n", newline, selected_code_line, Code_Selected);

    darken_code(-1, redtag);			/* remove all old tag */
    do_code_highlights();			/* ..& redo them */
  }
}


  /* highlight PC, breakpoints and selection in the buffered lines */

void do_code_highlights(void)
{
  int i, line;

  /* highlight all breakpoints */

  for (i = 0; i < SBSIZE; i++)
  {
    if (soft[i].sb_pass)			/* BP here? */
    {
      line = codelines[soft[i].sb_adr];		/* get line from address */
      if (line >= buff_lines_start &&
	line < buff_lines_start + CODE_LIST_LENGTH)	/* in buffer? */
        highlight_code(line - buff_lines_start, bluetag);/* yes - highlight it */
    }
  }

  line = codelines[PC - ram];			/* highlight next opcode */
  if (line >= buff_lines_start &&
	line <= buff_lines_start + CODE_LIST_LENGTH)
  {
    darken_code(-1, greentag);
    highlight_code(line - buff_lines_start, greentag);	/* highlight the LINE */
  }
  else
    printf("EEK! Code line %d not within %ld and %ld\n",
		line, buff_lines_start, buff_lines_start + CODE_LIST_LENGTH);

  /* highlight any selected code */

  if (Code_Selected == TRUE)
  {
    line = codelines[Selected_Code_Addr];		/* highlight selection */
    if (line >= buff_lines_start &&
	line < buff_lines_start + CODE_LIST_LENGTH)	/* ..if in buffer */
    {
      darken_code(line - buff_lines_start, bluetag);	/* remove blue */
      darken_code(line - buff_lines_start, greentag);	/* ..& green */
      highlight_code(line - buff_lines_start, redtag);
    }
    else
    {
      Code_Selected = FALSE;			/* not in buffer - lose it */
      darken_code(-1, redtag);			/* remove all old tag */
    }
  }
}


/*
 * 'disas_addr' is the *nominal* start address. But, because we don't want to have
 * each new disassembly start at the top of the code window, we back up a few
 * opcodes (ie. lines) to force the line associated with disas_addr to be somewhere
 * in the middle of the code window.
 *
 * 'force' is a flag to *force* a new disassembly and display.
 *
 * NOTE: 'disas_addr' will almost always be 'PC'.
 * -----------------------------------------------------------------------
 * 'disas_addr_line' is the absolute line associated with 'disas_addr', but it may be
 * tweaked backwards as described above.
 *
 * The aim is to keep the PC highlighted line within the visible lines with
 * as little disruption as possible (we don't want the window jumping around
 * unnecessarily) using the following rules:
 *
 * 1 If 'disas_addr_line' is outside the buffered lines - make a new lines buffer,
 *   (involving a new disassembly).
 *
 * 2 If 'disas_addr_line' is not within the visible lines (because of (1) we *know*
 *   it *is* within the buffered lines - scroll the window so that 'disas_addr_line'
 *   is in the middle of the visible lines. Remember that visible lines
 *   change if the scrollbar is moved.
 *
 */

void Show_Code(BYTE *disas_addr, gboolean force)
{
  BYTE *p, *isabreak;
  int line, i, pc_line, pc_row, scrollto, win_offset;
  unsigned int disas_addr_line;
  char whole_buffer[CODE_LIST_LENGTH*48];
  char br_char, pc_char;
  WORD code_ptr;
  BYTE *Start_Code_List;
  GtkTextIter aniter;
  GtkTextMark *mark;
  GdkRectangle coderect;
  gint winx, winy;
  int vbuf_start, vbuf_end, disas_buff_line;

  disas_addr_line = codelines[disas_addr - ram];	/* get line from disas addr */

printf("P1: disas_addr_line=%d force=%d buff_lines_start=%ld buff_lines_end=%ld\n", disas_addr_line, force, buff_lines_start, buff_lines_start + CODE_LIST_LENGTH);

  /*
   * (1) If 'disas_addr_line' is not in the text buffer rebuild the text buffer.
   *     If 'force' is set, do it anyway.
   */
 
  if (disas_addr_line < buff_lines_start
	|| disas_addr_line > buff_lines_start + CODE_LIST_LENGTH
	|| force)
  {
    buff_lines_start = disas_addr_line - BACKUP_LINES;
    if (buff_lines_start < 0)
      buff_lines_start = 0;

    code_ptr = 0;				/* scan the codelines table */
    while (codelines[code_ptr] != buff_lines_start)	/*..for the new start addr */
      code_ptr++;
    Start_Code_List = code_ptr + ram;		/* set disass start addr */

    p = Start_Code_List;			/* tmp pointer for disass */
    whole_buffer[0] = 0;			/* rewind to start */

    printf("Starting disassembly from %04X (line=%d) PC=%04X (line=%d)\n",
	(p - ram), disas_addr_line, (PC - ram), codelines[PC - ram]);

    for (line = 0; line < CODE_LIST_LENGTH; line++)
    {
      Disass_Str[0] = 0;
      isabreak = 0;				/* zero if no break */

      if ((((BYTE)(*p)) & 0xff) == BREAK_OP)	/* possible breakpoint? */
        for (i = 0; i < SBSIZE; i++)		/* scan all BPs */
          if (soft[i].sb_adr == (p - ram))	/* BP here? */
          {
            isabreak = p;			/* mark the BP address */
	    *p = soft[i].sb_oldopc;		/* restore the opcode */
          }

      sprintf(tstr, "%04X: ",			/* put addr etc @ SOL */
        (WORD)(p - ram));
      strcat(whole_buffer, tstr);

      disass(&p, p - ram);			/* get Opcode and Disass */

/* DEBUG: show buffered line number and code line number */

//sprintf(tstr, "%02d %03d  ", line, disas_addr_line + line);
//strcat(whole_buffer, tstr);

      if (show_opcodes)				/* machine code display? */
      {
        strcat(whole_buffer, Opcode_Str);	/* yes - add it */
        strcat(whole_buffer, "  ");		/* and a separator */
      }

      strcat(whole_buffer, Disass_Str);		/* add the disas string */

      if (isabreak)				/* breakpoint to restore? */
        *isabreak = BREAK_OP;
    }

    gtk_text_buffer_set_text(			/* show buffer in the win */
	code_textbuffer, whole_buffer, -1);

  }						/* END OF (1) */

 /*
  * At this point we know that disas_addr_line is within the text buffer.
  *
  * Now we check to see if it is within the visible lines.
  *
  * If not we need to scroll the window so that disas_addr_line is in
  * the middle of the visible lines.
  */

  /* Find the limits of the visible lines, upper and lower. */

  gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(codetext), &coderect);
//  printf("coderect1: x=%d y=%d width=%d height=%d\n",
//	coderect.x, coderect.y, coderect.width, coderect.height);
  gtk_text_view_get_line_at_y(GTK_TEXT_VIEW(codetext),
		&aniter, coderect.y, NULL);
  vbuf_start = gtk_text_iter_get_line(&aniter);
  gtk_text_view_get_line_at_y(GTK_TEXT_VIEW(codetext),
		&aniter, coderect.y+coderect.height, NULL);
  vbuf_end = gtk_text_iter_get_line(&aniter) - 1;

//  printf("visible (modified): start=%d end=%d\n", vbuf_start, vbuf_end);

  /* Is the target line NOT in the visible window? */

  disas_buff_line = disas_addr_line - buff_lines_start;

//  printf("Checking for %d between %d and %d\n",
//		disas_buff_line, vbuf_start, vbuf_end);
  if ( !(disas_buff_line >= vbuf_start && disas_buff_line <= vbuf_end ) )
  {
    if (disas_buff_line <= vbuf_start)			/* backing up or down? */
    {
      scrollto = disas_buff_line - BACKUP_LINES;
      if (scrollto < 0)
        scrollto = 0;
    }
    else
      scrollto = disas_buff_line + BACKUP_LINES;

//    printf("scrolling to %d\n", scrollto);
    gtk_text_buffer_get_end_iter(code_textbuffer, &aniter);
    gtk_text_iter_set_line(&aniter, scrollto);
    mark = gtk_text_buffer_create_mark(code_textbuffer,
			NULL, &aniter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(codetext),
			mark, 0.0, FALSE, 0.0, 0.0);
  }

// printf("p1: dch\n");
  do_code_highlights();
}


/* Set a breakpoint at the selected code */

void Code_Break(void)
{
  int i, line;

printf("Code_Break\n");
  if (!Code_Selected)
  {
    show_error("Unable to set break point - no code selected.\n");
    return;
  }

  for (i = 0; i < SBSIZE; i++)			/* check BPs */
    if (soft[i].sb_pass == 0)			/* available? */
      break;
  if (i == SBSIZE)
  {
    sprintf(tstr, "No breakpoints available.\n(Max %d)\n",
	SBSIZE);
    show_error(tstr);
    return;
  }

  soft[i].sb_adr = Selected_Code_Addr;		/* save addr */
  soft[i].sb_oldopc = 
	*(Selected_Code_Addr + ram);		/* ..and opcode */	
  *(Selected_Code_Addr + ram) = BREAK_OP;	/* set the BP */
  soft[i].sb_pass = 1;				/* activate the BP */
  soft[i].sb_passcount = 0;

  line = codelines[Selected_Code_Addr];		/* highlight the new BP */
  if (line >= buff_lines_start &&
	line < buff_lines_start + CODE_LIST_LENGTH)	/* visible? */
  {
    Code_Selected = FALSE;
    Show_Code(PC, FALSE);
    sprintf(lstr, "Breakpoint %d set at 0x%04X\n", i, Selected_Code_Addr);
    Add_to_Log(lstr);
  }
  else
  {
    sprintf(lstr, "Impossible! 0x%04X is not visible!\n", Selected_Code_Addr);
    Add_to_Log(lstr);
  }
}


/* Clear a breakpoint at the selected code */
/* DEBUG: change logic here to same as setting BPs */

void Code_UnBreak(void)
{
  int bpc, i;

  if (!Code_Selected)
  {
    show_error("Unable to clear break point - no code selected.\n");
    return;
  }

  bpc = -1;					/* no bp */
  for (i = 0; i < SBSIZE; i++)
    if (soft[i].sb_adr == Selected_Code_Addr)	/* search for BP adr */
      bpc = i;

  if (bpc != -1)				/* found it ? */
  {
    *(ram + Selected_Code_Addr) =
	 soft[bpc].sb_oldopc;			/* restore opcode */
    soft[bpc].sb_adr = (WORD)0;			/* clear BP entry */
    soft[bpc].sb_pass = 0;
    Code_Selected = FALSE;			/* clear selection */
    sprintf(lstr, "Breakpoint %d cleared from 0x%04X\n",
		bpc, Selected_Code_Addr);
    Add_to_Log(lstr);

    Show_Code(PC, TRUE);
  }
  else
    show_error("Selected code is not a breakpoint.\n");
}


/*
 * Set up all the stuff for the code frame.
 *
 * Create the code_textbuffer.
 * Set the frame font (monospace).
 * Set up the tags for highlighting (should be renamed to break, select etc).
 * Set showing opcodes to off.
 * Set up the runslow stuff.
 * Build the initial code/line cache.
 *
 */

void init_Code(GtkWidget *Main)
{
  PangoFontDescription *codefont;

  buff_lines_start = 0;

  code_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codetext));

  codefont = pango_font_description_from_string("Monospace");
  gtk_widget_override_font(codetext, codefont);

  redtag = gtk_text_buffer_create_tag(code_textbuffer, "redtag", 
	"foreground", "red", NULL); 
  greentag = gtk_text_buffer_create_tag(code_textbuffer, "greentag", 
	"foreground", "green", NULL); 
  bluetag = gtk_text_buffer_create_tag(code_textbuffer, "bluetag", 
	"foreground", "blue", NULL); 
  boldtag = gtk_text_buffer_create_tag(code_textbuffer, "boldtag", 
	"weight", PANGO_WEIGHT_BOLD, NULL); 

/* default the machine code button to FALSE */

  gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(optcodeb), FALSE);

/* Set up the runslow stuff. Note that the widget holds values in mS */
/* but the internal values are in usecs */

  run_slow_pref = RUN_SLOW_MSECS * 1000;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(slowrunspin), RUN_SLOW_MSECS);

  Code_Selected = FALSE;
  build_code_cache();
}
