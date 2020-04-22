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

static GtkTextBuffer *log_textbuffer;

/*
 * Add a string to the end of the log window then scroll to the bottom
 * of the buffer. We use a mark as the scroll point to avoid any nastiness
 * with line height (see GTK+ reference for gtk_text_view_scroll_to_iter
 * for details).
 */

void Add_to_Log(char *str)
{
  GtkTextIter end;
  GtkTextMark *mark;

  gtk_text_buffer_get_end_iter(log_textbuffer, &end);
  gtk_text_buffer_insert(log_textbuffer, &end, str, -1);

  gtk_text_buffer_get_end_iter(log_textbuffer, &end);
  mark = gtk_text_buffer_create_mark(log_textbuffer, NULL, &end, FALSE);
  gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(logtext),
			mark, 0.0, FALSE, 0.0, 0.0); 
}

void show_log(gboolean onoff)
{
  if (onoff)
    gtk_widget_show(Log);
  else
    gtk_widget_hide(Log);
}

void init_Log(GtkWidget *Main)
{
  PangoFontDescription *codefont;

//  Log = create_Log();
//  logtext = lookup_widget(Log, "Log_Text");
  log_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logtext));

  codefont = pango_font_description_from_string("Monospace");
  gtk_widget_modify_font(logtext, codefont);
}

