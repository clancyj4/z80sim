/*
        Z80 Emulator GUI

        Copyright (C) 2008 Justin Clancy
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "guiglb.h"
#include "simglb.h"
#include "prototypes.h"

#define MEM_INITIAL_STRING "1234: 01 02 03 04 05 06 07 08 ABCDEFGH"
#define MEM_ENTRY_LENGTH sizeof(MEM_INITIAL_STRING)
#define MEM_NUMBYTES 8
#define DUMP_ROW_MASK (0xffff ^ (MEM_NUMBYTES - 1))
#define DUMP_ADDR_MASK 0xff80

extern WORD Mem_REG;

void dump_memory_vars(void)
{
//  logprint("++++ MEMORY ++++\n", NULL);
//  logprint("Memory_Selected=%d\n", Memory_Selected);
//  logprint("Selected_Memory_Addr=%04X Selected_Memory_Content=%02X\n",
//	Selected_Memory_Addr, Selected_Memory_Content);
//  logprint("---- MEMORY ----\n", NULL);
}

void Scroll_Memory(int row)
{
  GtkTextIter aniter;
  GtkTextMark *mark;

  gtk_text_buffer_get_start_iter(mem_textbuffer, &aniter); /* create iter */
  gtk_text_iter_set_line(&aniter, row);
  mark = gtk_text_buffer_create_mark(mem_textbuffer, NULL, &aniter, FALSE);
  gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(memtext), mark, 0, TRUE, 0, 0.6);
}


void Show_Addr_Content(void)
{
  if (Memory_Selected)
  {
    sprintf(tstr, "%04X", Selected_Memory_Addr);
    gtk_entry_set_text(GTK_ENTRY(Mem_Addr_Entry), tstr);
    sprintf(tstr, "%02X", Selected_Memory_Content);
    gtk_entry_set_text(GTK_ENTRY(Mem_Content_Entry), tstr);
  }
  else
  {
    sprintf(tstr, "    ");
    gtk_entry_set_text(GTK_ENTRY(Mem_Addr_Entry), tstr);
    sprintf(tstr, "  ");
    gtk_entry_set_text(GTK_ENTRY(Mem_Content_Entry), tstr);
  }
}

/*
 * Takes the address highlighted in the memory window and puts it
 * in the memory register.
 */

void Synch_Mem_Reg(void)
{
  if (! Memory_Selected)
  {
    show_error("Unable to synch memory to selection - no memory selected.\n");
    return;
  }

  Mem_REG = Selected_Memory_Addr;
  Show_Registers();
}


/* Called from various buttons to increment, change row */
/* or page around the memory window. */

void Mem_Change_Addr(WORD begin)
{
  int row;

  if (! Memory_Selected)
  {
//    show_error("Unable to jump to begin - no memory selected.\n");
    show_error("You haven't selected anything in the memory window.\n");
    return;
  }

printf("SMA=%04X begin=%d ", Selected_Memory_Addr, begin);
  Selected_Memory_Addr = (Selected_Memory_Addr + begin) & 0xffff;
  Selected_Memory_Content = *(Selected_Memory_Addr + ram);
//  Mem_REG = Selected_Memory_Addr;
printf("New_SMA=%04X\n", Selected_Memory_Addr);
  Show_Registers();
  Dump_From_Register(Dump_Reg);
  row = mem_addr_to_row(Selected_Memory_Addr & 0xfff8);
  if (row != -1)
    Scroll_Memory(row);
}

/*
 * Get the row in the memory textbuffer which contains the given address.
 * This relies on grepping or the masked (start of row) address.
 */

int mem_addr_to_row(WORD baddr)
{
  GtkTextIter start, match;
  gchar text[8];
  gboolean found;
  WORD addr;

  addr = baddr & 0xFFF8;		/* mask for 8 bit line */

/* DEBUG */
// printf("mem_addr_to_row: checking addr=%04X (%04X)\n", baddr, addr);

  /* Get the text of the address */

  sprintf(text, "%04X:", addr);

  /* search for this string in the mem buffer */

  gtk_text_buffer_get_start_iter(mem_textbuffer, &start);
  found = gtk_text_iter_forward_search (&start, text, 0, &match, NULL, NULL);

  /* If found return the line of the iter */

  if (found)
    return(gtk_text_iter_get_line(&match));
  else
  {
//    printf("mem_addr_to_row: Out of window - %s not matched.\n", text);
    return(-1);
  }
}


/* clear a type of tag from the memory window */

void darken_memory(GtkTextTag *hightag)
{
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter(mem_textbuffer, &start);
  gtk_text_buffer_get_end_iter(mem_textbuffer, &end);
  gtk_text_buffer_remove_tag(mem_textbuffer, hightag, &start, &end);
}


/* Apply a tag (colour) to selected position */

int highlight_memory(WORD begin, GtkTextTag *hightag)
{
  GtkTextIter start, end;
  int row;
  WORD addr;
gchar *teststr;

  addr = begin & DUMP_ROW_MASK;
  row = mem_addr_to_row(addr);
  if (row == -1)
    return row;

/* DEBUG */
  printf("highlight_memory: begin=%04X, addr=%04X, row=%d\n",
	begin, addr, row);

  /* Highlight the hex code */

  gtk_text_buffer_get_iter_at_line(mem_textbuffer, &start, row);
  gtk_text_iter_forward_chars(&start, 6 + ((begin - addr) * 3));
  end = start;
  gtk_text_iter_forward_chars(&end, 2);
teststr=gtk_text_iter_get_text(&start, &end);

  gtk_text_buffer_apply_tag(mem_textbuffer, hightag, &start, &end);

  /* Highlight the character */

  gtk_text_buffer_get_iter_at_line(mem_textbuffer, &start, row);
  gtk_text_iter_forward_chars(&start, 6 + (MEM_NUMBYTES * 3) + (begin - addr));
  end = start;
  gtk_text_iter_forward_chars(&end, 1);
teststr=gtk_text_iter_get_text(&start, &end);
  gtk_text_buffer_apply_tag(mem_textbuffer, hightag, &start, &end);

  return(row);
}


/* Invoked when the memory window is clicked. Sets the selected memory */
/* row, column and address. */

void Mem_Clicked(void)
{
  GtkTextIter iter, start, end;
  int newrow, newcol;
  char addr[8];

  gtk_text_buffer_get_iter_at_mark(mem_textbuffer,	/* get iter where clicked */
   &iter, gtk_text_buffer_get_insert(mem_textbuffer));

  newrow = gtk_text_iter_get_line(&iter);		/* get row selection */
  newcol = gtk_text_iter_get_line_offset(&iter);	/* ..& col */

  /* if same row AND column as last selection OR */
  /* not a content field (eg an address). Forget it */
  /* WARNING: hardwired values for relevant columns. */

  if (
      (
       (newrow == mem_sel_row) &&
       (newcol == mem_sel_col)
      ) ||
      (newcol < 5 || newcol > 28)
     )
    return;

  mem_sel_row = newrow;
  mem_sel_col = newcol;
  Memory_Selected = TRUE;

  /* get the base address of the row */

  gtk_text_buffer_get_iter_at_line(mem_textbuffer, &start, mem_sel_row); 
  end = start;
  gtk_text_iter_forward_chars(&end, 4);		/* move to address end */
  strcpy(addr, gtk_text_buffer_get_text(	/* ..get the address */
              mem_textbuffer,
              &start, &end, TRUE)
        );
  Selected_Memory_Addr = exatoi(addr + 1);	/* base address */
  Selected_Memory_Addr += (newcol - 5) / 3;	/* ..+ offset */
  Selected_Memory_Content = *(Selected_Memory_Addr + ram);

//  printf("MEM SELECT - row=%d, col=%d Addr=%04X Content=%02X\n",
//	newrow, newcol, Selected_Memory_Addr, Selected_Memory_Content);

  Show_Addr_Content();				/* Show the Editing fields */
  darken_memory(redtag);			/* clear any old highlight */
  highlight_memory(Selected_Memory_Addr, redtag);
  Scroll_Memory(newrow);			/* ensure select is visible */
}


/*
 * 
 */

void Show_Memory(WORD begin, int length, gboolean force)
{
  int cur_line, j;
  int nrows;
  int row;
  WORD mptr;
  char Line[MEM_ENTRY_LENGTH+3];
  char tstr[MEM_NUMBYTES + 1];
  char *cptr;
  char *whole_buffer;
  
  /* How many rows in the memory window are we talking about? */

//  if ((length % MEM_NUMBYTES) == 0)
//    nrows = length / MEM_NUMBYTES;
//  else
    nrows = (length / MEM_NUMBYTES) + 1;
    row = mem_addr_to_row(begin);		/* get row for the begin address */

  /* First we check to see if the begin address is within */
  /* the existing memory textbuffer - if not, rebuild the text buffer */

  if (row < mem_buff_row_start || row > mem_buff_row_start + nrows || force)
  {
    mem_buff_row_start = row;			/* new start row */

    whole_buffer = malloc(((nrows + 1) * MEM_ENTRY_LENGTH) + 1);
    whole_buffer[0] = '\0';

    mptr = begin & DUMP_ADDR_MASK;		/* code addr start */
    cptr = ram + mptr;				/* get real pointer start */

    for (cur_line = 0; cur_line < nrows; cur_line++)
    {
      for (j = 0; j < MEM_NUMBYTES; j++)		/* build the ASCII string */
      {
        if (*(cptr+j) < 0x20 || *(cptr+j) > 0x7e)	/* chuck out non printables */
          tstr[j] = '.';
        else
          tstr[j] = *(cptr+j);
      }
      tstr[MEM_NUMBYTES] = '\0';			/* terminate the ASCII string */

      sprintf(Line, "%04X: %02X %02X %02X %02X %02X %02X %02X %02X %s\n",
	(WORD)mptr,
	(BYTE)*(cptr+0), (BYTE)*(cptr+1), (BYTE)*(cptr+2), (BYTE)*(cptr+3),
	(BYTE)*(cptr+4), (BYTE)*(cptr+5), (BYTE)*(cptr+6), (BYTE)*(cptr+7),
	tstr);

      mptr = (mptr + MEM_NUMBYTES) & 0xffff;		/* wrap round 64 KBytes */
      cptr = ram + mptr;				/* update real pointer */

      strcat(whole_buffer, Line);			/* Append this line to buffer */
    }

    gtk_text_buffer_set_text(mem_textbuffer, whole_buffer, -1);
    free(whole_buffer);
  }

  highlight_memory(begin, greentag);

  if (Memory_Selected)				/* highlight selection? */
    if (highlight_memory(Selected_Memory_Addr, redtag) == -1)
      Memory_Selected = FALSE;			/* oops - not visible */

  Show_Addr_Content();				/* Show the Editing fields */

}


void Update_Memory()
{
  if (! Memory_Selected)
  {
    show_error("Unable to modify memory - no memory selected.\n");
    return;
  }

  Selected_Memory_Content = exatoi(
	(char *)gtk_entry_get_text(GTK_ENTRY(Mem_Content_Entry)));
  *(Selected_Memory_Addr + ram) = Selected_Memory_Content;

  Dump_From_Register(Dump_Reg);
//  Show_Code(PC);
}


void init_Memory(GtkWidget *Main)
{
  PangoFontDescription *codefont;

//  memtext = lookup_widget(Main, "Mem_text");

  mem_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(memtext));

  codefont = pango_font_description_from_string("Monospace");
  gtk_widget_modify_font(memtext, codefont);

  redtag = gtk_text_buffer_create_tag(mem_textbuffer, "redtag",
        "background", "red", NULL);
  greentag = gtk_text_buffer_create_tag(mem_textbuffer, "greentag",
        "background", "green", NULL);

  Memory_Selected = FALSE;

//  Mem_Addr_Entry = lookup_widget(Main, "Mem_Addr_Entry");
//  Mem_Content_Entry = lookup_widget(Main, "Mem_Content_Entry");

  mem_buff_row_start = 0;			/* new start row */
//  g_signal_connect(mem_textbuffer, "mark_set", Mem_clicked, NULL);
}
