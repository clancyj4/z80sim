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

static GtkTextBuffer *ioport_textbuffer;

/* Create an IO Port struct for a given port */
/* In other words allocate memory for it and assign the IOPort[] */
/* pointer.  Note that these memory allocations are never released. */

void Create_IOPort_Struct(int port)
{

  /* Does a struct already exist? If so, quietly ignore the call */
  /* NOTE: Or should we complain loudly - cos this shouldn't happen */

  if (IOPort[port] != NULL)
  {
    fprintf(stderr, "DEBUG: Structure already exists for port %d\n",
		port);
    return;
  }

  /* Allocate a chunk of memory for the new port struct */
  /* and initialise it. */

  IOPort[port] = malloc(sizeof(IOPortStruct));
  if (IOPort[port] == NULL)
  {
    fprintf(stderr, "Memory allocation for IOPort[%d] failed.", port);
    exit(99);						/* Time to BAIL OUT */
  }

//printf("Assigned struct for port %d\n", port);

  IOPort[port]->obuffer[0] = '\0';			/* clear the out buffer */
  IOPort[port]->out_ptr = 0;				/* ..set ptr to start */
  IOPort[port]->ishex = 0;				/* default to ASCII */
  IOPort[port]->in_len = 0;				/* clear input buffer */
}


/*
 * Dump the contents of the port buffer to the IOPort window.
 * Called when IO port window stuff changes and (inefficiently) whenever
 * an OUT instruction is executed.
 *
 * Check the ishex flag and display the data in either ASCII or HEX.
 */

void Dump_IOPort(int port)
{
  char whole_buffer[IOOUTBUFLEN * 4];		/* 4x in case of Hex */
  char bchar;
  int i, ascptr;
  GtkTextIter end;

  whole_buffer[0] = 0;				/* start with a clean buff */

  if (IOPort[port] == NULL)			/* struct exists? */
  {
//    fprintf(stderr, "Attempting to dump from non existing port %d\n", port);
    return;
  }

  if (IOPort[port]->ishex)			/* dump in hex? */
  {
    for (i = 0; i < IOPort[port]->out_ptr; i++)
    {
      sprintf(tstr, "%02X ", IOPort[port]->obuffer[i]);
      strcat(whole_buffer, tstr);
    }
  }
  else
  {
    ascptr = 0;
    for (i = 0; i < IOPort[port]->out_ptr; i++)
    {
      bchar = IOPort[port]->obuffer[i];
      if (
	    (bchar >= 0x20 && bchar < 0x7f)	/* printable? */
	 || (bchar == 0x0a)
	 )
        whole_buffer[ascptr++] = bchar;
    }

    whole_buffer[ascptr] = (char)0;		/* terminate string */
  }
 
  gtk_text_buffer_set_text(ioport_textbuffer, whole_buffer, -1);
  gtk_text_buffer_get_end_iter(ioport_textbuffer, &end);
  gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(ioporttext), &end, 0, FALSE, 0, 0);
}


/* Show/hide the IO Port window */

void Show_IOport(GtkWidget *widget)
{
  gboolean porton;

  porton = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
  if (porton)
    gtk_widget_show(ioport_win);
  else
    gtk_widget_hide(ioport_win);
}


/* get a byte (char?) from the input part of the Port window */
/* Just a stub at the mo' - returns '7' */
/* What we really want to do is have a queue */

BYTE IOPort_IN(int port)
{
  if (IOPort[port] == NULL)				/* struct exists? */
    Create_IOPort_Struct(port);

  printf("IOPort_IN: port=%d in_len=%d in_ptr=%d\n",
	port, IOPort[port]->in_len, IOPort[port]->in_ptr);

  sprintf(tstr, "Port %d requires input", port);
  gtk_entry_set_text(GTK_ENTRY(in_port_prompt), tstr);

  return('7');
}


/* Add a byte to an output IOPort buffer */

void IOPort_OUT(int port, BYTE data)
{
  if (IOPort[port] == NULL)				/* struct exists? */
    Create_IOPort_Struct(port);				/* no - create it */

  IOPort[port]->obuffer[IOPort[port]->out_ptr] = data;
  IOPort[port]->out_ptr++;
  if (IOPort[port]->out_ptr >= IOOUTBUFLEN)		/* wrap around? */
  {
    IOPort[port]->out_ptr = 0;				/* just wrap to 0 */
    printf("Port %d wrapped around (Please fix me)\n", port);
  }
  Dump_IOPort(port);					/* VERY inefficient */
}


/* Set up all the stuff for the IOPort structs and the Port window */

void init_IOport(void)
{
  int i;

  PangoFontDescription *iofont;

//  ioport_win = create_IOWIN();

//  ioporttext = lookup_widget(ioport_win, "IOPortText");
  ioport_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ioporttext));

//  in_port_prompt = lookup_widget(ioport_win, "In_Port_Prompt");
//  in_port_data = lookup_widget(ioport_win, "In_Port_Data");

  iofont = pango_font_description_from_string("Monospace");
  gtk_widget_modify_font(ioporttext, iofont);

  /* clear all buffer pointers - they will be allocated as necessary */

  for (i = 0; i < NUMIOPORTS; i++)
    IOPort[i] = NULL;
}
