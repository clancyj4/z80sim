/*
        Z80 Emulator GUI

        Copyright (C) 2008 - 2020 Justin Clancy
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sim.h"
#include "simglb.h"
#include "guiglb.h"
#include "prototypes.h"

static GtkTextBuffer *breaks_textbuffer;
static GtkTextTag *redtag;
static int selected_breaks_line;
static gboolean Breaks_Selected = FALSE;
static int selected_bp;

void show_breaks(gboolean onoff)
{
  if (onoff)
  {
    gtk_widget_show(Breaks);
    show_breaks_content();
  }
  else
    gtk_widget_hide(Breaks);
}


void show_breaks_content()
{
  int i;
  char str[64];
  char whole_buffer[SBSIZE * 14];

  whole_buffer[0] = (char)0;
  for (i = 0; i < SBSIZE; i++)
  {
    if (soft[i].sb_pass > 0)
    {
      sprintf(str, "breakpoint %02d: address %04X count %03d\n", i, soft[i].sb_adr, soft[i].sb_pass);
      strcat(whole_buffer, str);
    }
  }

  gtk_text_buffer_set_text(                   /* show buffer in the win */
        breaks_textbuffer, whole_buffer, -1);
}


  /*
   * Invoked when the breaks window is clicked.
   * Sets the 'Breaks_Selected' flag, selected_breaks_line to the selected line and
   * highlights the new selection.
   */

void Breaks_Clicked()
{
  int newline;
  GtkTextIter aniter;
  GtkTextIter start, end;

  gtk_text_buffer_get_iter_at_mark(breaks_textbuffer, &aniter, gtk_text_buffer_get_insert(breaks_textbuffer));

  newline = gtk_text_iter_get_line(&aniter);
  selected_breaks_line = newline;
  Breaks_Selected = TRUE;

  /* get the breakpoint number from the selected line. */

  gtk_text_buffer_get_iter_at_line(breaks_textbuffer, &start, selected_breaks_line);
  gtk_text_iter_forward_chars(&start, 11);                      /* skip past "breakpoint " */
  end = start;
  gtk_text_iter_forward_chars(&end, 2);
  selected_bp = atoi(gtk_text_buffer_get_text(breaks_textbuffer, &start, &end, FALSE));

  darken_breaks(-1, redtag);                  /* remove old tag */
  highlight_breaks(newline);                  /* ..& redo it */
}


void init_Breaks()
{
  PangoFontDescription *codefont;

  breaks_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(breakstext));
  codefont = pango_font_description_from_string("Monospace");
  gtk_widget_override_font(codetext, codefont);

  redtag = gtk_text_buffer_create_tag(breaks_textbuffer, "redtag", "foreground", "red", NULL);

  show_breaks_content();
}


darken_breaks(int line, GtkTextTag *tag)
{
  GtkTextIter start, end;

  if (line == -1)
  {
    gtk_text_buffer_get_start_iter(breaks_textbuffer, &start);
    gtk_text_buffer_get_end_iter(breaks_textbuffer, &end);
  }
  else
  {
    gtk_text_buffer_get_iter_at_line(breaks_textbuffer, &start, line);
    end = start;
    gtk_text_iter_forward_to_line_end(&end);
  }

  gtk_text_buffer_remove_tag(breaks_textbuffer, tag, &start, &end);
}


highlight_breaks(int line)
{
  GtkTextIter start, end;

  gtk_text_buffer_get_iter_at_line(breaks_textbuffer, &start, line);
  end = start;
  gtk_text_iter_forward_to_line_end(&end);
  gtk_text_buffer_apply_tag(breaks_textbuffer, redtag, &start, &end);
}


void on_Break_Clear_clicked()
{
  if (!Breaks_Selected)
  {
    show_error("No breakpoint selected");
    return;
  }

  printf("breakpoint selected to clear is %d\n", selected_bp);
  soft[selected_bp].sb_pass = 0;

  show_breaks_content();
  Show_Code(PC, TRUE);
}


void pass_value_changed(int pv)
{
  if (!Breaks_Selected)
  {
    show_error("No breakpoint selected");
    return;
  }

  soft[selected_bp].sb_pass = pv;

  show_breaks_content();
}

