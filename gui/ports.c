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

static GtkTextBuffer *ioport_textbuffer;
static GtkTextBuffer *ioportin_textbuffer;
static GtkTextBuffer *ioinqueue_textbuffer;

/* Create an IO Port struct for a given port */
/* In other words allocate memory for it and assign the IOPort[] */
/* pointer.  Note that these memory allocations are never released. */

void Create_IOPort_Struct(int port)
{

  /* Does a struct already exist? If so, quietly ignore the call */
  /* NOTE: Or should we complain loudly - cos this shouldn't happen */

  if (IOPort[port] != NULL)
  {
    fprintf(stderr, "DEBUG: Structure already exists for port %d\n", port);
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

printf("Assigned struct for port %d\n", port);

  IOPort[port]->obuffer[0] = '\0';			/* clear the out buffer */
  IOPort[port]->out_ptr = 0;				/* ..set ptr to start */
  IOPort[port]->ishex = 0;				/* default to ASCII */
  IOPort[port]->in_len = 0;				/* clear input buffer */

  current_port = port;
  Dump_IOPort(port);
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
  int i, ascptr, hcount;
  GtkTextIter end;

  whole_buffer[0] = 0;				/* start with a clean buff */

  if (IOPort[port] == NULL)			/* struct exists? */
    return;

  /* set the ASCII and Hex drop entry according to ishex */

  gtk_combo_box_set_active(GTK_COMBO_BOX(ioasciihex), IOPort[port]->ishex);

  gtk_spin_button_set_value(GTK_SPIN_BUTTON(ioportspin), port);

  if (IOPort[port]->ishex)			        /* dump in hex? */
  {
    hcount = 0;
    for (i = 0; i < IOPort[port]->out_ptr; i++)
    {
      sprintf(tstr, "%02X ", IOPort[port]->obuffer[i]);
      strcat(whole_buffer, tstr);
      hcount++;
      if (hcount > PORT_HEX_LEN - 1)
      {
        hcount = 0;
        strcat(whole_buffer, "\n");
      }
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
	 || (bchar == 0x0a) || (bchar == 0x0d)
	 )
        whole_buffer[ascptr++] = bchar;
      else
        whole_buffer[ascptr++] = '.';
    }

    whole_buffer[ascptr] = (char)0;		/* terminate string */
  }
 
  gtk_text_buffer_set_text(ioport_textbuffer, whole_buffer, -1);
  gtk_text_buffer_get_end_iter(ioport_textbuffer, &end);
  gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(ioporttext), &end, 0, FALSE, 0, 0);
}


/* Show/hide the IO Port window */

void show_iowin(gboolean onoff)
{
  if (onoff)
    gtk_widget_show(ioport_win);
  else
    gtk_widget_hide(ioport_win);

  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(iomenuitem), onoff);
}


/* get a byte from the ibuffer or prompt for new input */

BYTE IOPort_IN(int port)
{
  BYTE c;

  if (IOPort[port] == NULL)			/* struct exists? */
    Create_IOPort_Struct(port);

  printf("IOPort_IN: port=%d in_len=%d in_ptr=%d\n",
	port, IOPort[port]->in_len, IOPort[port]->in_ptr);

  if (IOPort[port]->in_len == 0)
  {
    sprintf(tstr, "Port %d requires input - IN opcode will return 0x00 until it is supplied.\n", port);
    Add_to_Log(tstr);
    show_log(TRUE);

    return((BYTE)0);
  }

  c = IOPort[port]->ibuffer[IOPort[port]->in_ptr++];    /* get the next byte from the queue */
  IOPort[port]->in_len--;

  set_in_queue_buffer(port);                            /* update the IN queue */

  return(c);
}


/* called when the io port in submit is called. */
/* assigns the content of the ioportin_textbuffer to the IOPort struct. */

void assign_in_port_to_buff(int port)
{
  GtkTextIter start, end;
  char * whole_buffer;
  int i;
  char str[8];
  int strptr = 0;
  int strln;

  if (IOPort[port] == NULL)			/* struct exists? */
    Create_IOPort_Struct(port);

  IOPort[port]->in_ptr = 0;			/* start from scratch */
  IOPort[port]->in_len = 0;
  
  gtk_text_buffer_get_start_iter(ioportin_textbuffer, &start);
  gtk_text_buffer_get_end_iter(ioportin_textbuffer, &end);

  whole_buffer = gtk_text_iter_get_text(&start, &end);
  printf("in text=%s\n", whole_buffer);

  strln = gtk_text_buffer_get_char_count(ioportin_textbuffer);
  printf("in port buff len is %d.\n", strln);

  if (IOPort[port]->ishex)			/* scan in hex? */
  {
    for (i = 0; i < strln + 1; i++)
      if (*(whole_buffer + i) == ' ' || i == strln)
      {
        str[strptr] = 0;
        IOPort[port]->ibuffer[IOPort[port]->in_ptr++] = (char)strtol(str, NULL, 16);
        strptr = 0;
        IOPort[port]->in_len++;
      }
      else
        str[strptr++] = *(whole_buffer + i);

    IOPort[port]->ibuffer[IOPort[port]->in_ptr] = (char)0;
  }
  else
  {
    strcpy(IOPort[port]->ibuffer, whole_buffer);	/* just ASCII */
    IOPort[port]->in_len = strlen(IOPort[port]->ibuffer);
  }

  printf("Converted string is \"%s\"\n", IOPort[port]->ibuffer);
  IOPort[port]->in_ptr = 0;

  set_in_queue_buffer(port);

  gtk_text_buffer_delete(ioportin_textbuffer, &start, &end);
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

  ioport_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ioporttext));
  ioportin_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ioportintext));
  ioinqueue_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ioinqueuetext));

  iofont = pango_font_description_from_string("Monospace");
  gtk_widget_override_font(ioporttext, iofont);

  /* clear all buffer pointers - they will be allocated as necessary */

  for (i = 0; i < NUMIOPORTS; i++)                       /* clear up any old allocations */
  {
    if (IOPort[i] != NULL)
      free(IOPort[i]);
    IOPort[i] = NULL;                                    /* and reset the IOPort pointer */
  }

  Create_IOPort_Struct(0);                               /* always create port 0 (for project saving) */
}


void set_in_queue_buffer(int port)
{
  char buffer[IOINBUFLEN * 3];
  char tstr[4];
  int i;
  int ptr;

  ptr = IOPort[port]->in_ptr;
  buffer[0] = (char)0;

  if (IOPort[port]->ishex)
    for (i = 0; i < IOPort[port]->in_len; i++)
    {
      sprintf(tstr, "%02X ", IOPort[port]->ibuffer[ptr++]);
      strcat(buffer, tstr);
    }
  else
    strcat(buffer, IOPort[port]->ibuffer + ptr);

  gtk_text_buffer_set_text(ioinqueue_textbuffer, buffer, -1);
}

