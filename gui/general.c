/*
	Z80 Emulator GUI

	Copyright (C) 2008 Justin Clancy
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "sim.h"
#include "simglb.h"
#include "guiglb.h"
#include "prototypes.h"

static GtkWidget *FCwin;


/* Called when we are running in continuous CPU mode */
/* If the run_slow var is non-zero we wait for run_slow usecs, checking */
/* every RUN_SLOW_TICK usecs for a GUI event */
/* We do it this way so that GUI response times are not restricted to */
/* every 'run_slow' usecs (which could be several seconds) */

int check_gui_break(void)
{
  int i;

  if (cpu_state != CONTIN_RUN)
  {
//    fprintf(stderr, "check_gui_break called when cpu_state is not CONTIN\n");
    return(0);
  }

  if (run_slow)
  {
    for (i = 0; i < run_slow / RUN_SLOW_TICK; i++)
    {
      usleep(RUN_SLOW_TICK);
      while (gtk_events_pending())
        gtk_main_iteration();
    }
    Show_All();
  }

  return(0);
}


void show_about(GtkWidget *widget)
{
  GtkWidget *about_win;

//  about_win = create_About_Dialog();
  gtk_widget_show(about_win);
  gtk_dialog_run(GTK_DIALOG(about_win));
  gtk_widget_destroy(about_win);
}

void Get_File(void)
{

//  FCwin = create_File_Chooser();
  gtk_widget_show(FCwin);
  gtk_dialog_run(GTK_DIALOG(FCwin));
  
  gtk_widget_destroy(FCwin);
}

void Get_File_Cancel(void)
{
  gtk_widget_destroy(FCwin);
}


void Get_File_Open(void)
{
  char *fn;
  fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FCwin));

  if (fn != NULL)
  {
    sprintf(lstr, "GETFILE: %s\n", fn);
//    Add_to_Log(lstr);
    do_getfile(fn);
    Show_Code(PC, TRUE);
    Do_Code_Reload();
    Show_Registers();
    Dump_From_Register(Dump_Reg);
  }
  gtk_widget_destroy(FCwin);
}


void show_error(char *str)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            str);
  gtk_window_set_title(GTK_WINDOW(dialog), "Error");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}
