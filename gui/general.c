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

/* Called when we are running in continuous CPU mode */
/* If the run_slow var is non-zero we wait for run_slow usecs, checking */
/* every RUN_SLOW_TICK usecs for a GUI event */
/* We do it this way so that GUI response times are not restricted to */
/* every 'run_slow' usecs (which could be several seconds) */

int check_gui_break(void)
{
  int i;

  if (cpu_state != CONTIN_RUN)					/* no action unless cpu is running continuously */
    return(0);

  if (run_slow)
  {
    for (i = 0; i < run_slow / RUN_SLOW_TICK; i++)
    {
      usleep(RUN_SLOW_TICK);					/* sleep for one slice */
      while (gtk_events_pending())				/* check for events... */
        gtk_main_iteration();					/* and act on any pending */
    }
    Show_All();
  }

  return(0);
}


void show_about(GtkWidget *widget)
{
  gtk_widget_show(about_win);
  gtk_dialog_run(GTK_DIALOG(about_win));
  gtk_widget_hide(about_win);
}


void Get_File(void)
{
  gtk_widget_show(FCwin);
  gtk_dialog_run(GTK_DIALOG(FCwin));
  
  gtk_widget_hide(FCwin);
}


void Get_File_Cancel(void)
{
  gtk_widget_hide(FCwin);
}


void Get_File_Open(void)
{
  char *fn;
  fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FCwin));

  gtk_widget_hide(FCwin);

  load_z80_code(fn);
}


void Get_Project_File(void)
{
  gtk_widget_show(FPwin);
  gtk_dialog_run(GTK_DIALOG(FPwin));
  
  gtk_widget_hide(FPwin);
}


void Get_Project_File_Cancel(void)
{
  gtk_widget_hide(FPwin);
}


void Get_Project_File_Open(void)
{
  char *fn;
  fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FPwin));

  gtk_widget_hide(FPwin);

  Read_Project(fn);
}


void load_z80_code(char *fn)
{
  int i;

  if (fn != NULL)
  {
    sprintf(lstr, "GETFILE: %s\n", fn);
    Add_to_Log(lstr);
    do_getfile(fn);
    for (i = 0; i < SBSIZE; i++)                 /* clear old breakpoints because they aren't relevant any more */
    {
      soft[i].sb_adr = 0;
      soft[i].sb_pass = 0;
    }
    init_IOport();			  	 /* clear up the IO ports first */
    Zero_Registers();
    Show_Code(PC, TRUE);
    Do_Code_Reload();
    Show_Registers();
    Dump_From_Register(Dump_Reg);
  }
}


void Save_File(void)
{
  gtk_widget_show(FSwin);
  gtk_dialog_run(GTK_DIALOG(FSwin));
  
  gtk_widget_hide(FSwin);
}


void Save_File_Save(void)
{
  char *fn;
  fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FSwin));

  if (fn != NULL)
  {
    sprintf(lstr, "Saving project to %s\n", fn);
    Add_to_Log(lstr);
    Save_Project(fn);
  }
  strcpy(project_fn, fn);

  gtk_widget_hide(FSwin);
}


void Save_File_Cancel(void)
{
  gtk_widget_hide(FSwin);
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


void Save_Project(char *fn)
{
  FILE *fp;
  int i;
  WORD temp;
  char str[64];

  printf("Saving project in %s.\n", fn);

  strcpy(project_fn, fn);

  fp = fopen(fn, "w");
  if (fp == NULL)
  {
    sprintf(lstr, "Unable to save project to %s", fn);
    show_error(lstr);
    return;
  }

  fprintf(fp, "#Z80sim\n");
  fprintf(fp, "%c\n", Dump_Reg);
  fprintf(fp, "%d\n", run_slow_pref);
  fprintf(fp, "%s\n", project_fn);
  fwrite(&A, sizeof(BYTE), 1, fp);
  fwrite(&F, sizeof(BYTE), 1, fp);
  fwrite(&B, sizeof(BYTE), 1, fp);
  fwrite(&C, sizeof(BYTE), 1, fp);
  fwrite(&D, sizeof(BYTE), 1, fp);
  fwrite(&E, sizeof(BYTE), 1, fp);
  fwrite(&H, sizeof(BYTE), 1, fp);
  fwrite(&L, sizeof(BYTE), 1, fp);
  fwrite(&IX, sizeof(WORD), 1, fp);
  fwrite(&IY, sizeof(WORD), 1, fp);
  fwrite(&A_, sizeof(BYTE), 1, fp);
  fwrite(&F_, sizeof(BYTE), 1, fp);
  fwrite(&B_, sizeof(BYTE), 1, fp);
  fwrite(&C_, sizeof(BYTE), 1, fp);
  fwrite(&D_, sizeof(BYTE), 1, fp);
  fwrite(&E_, sizeof(BYTE), 1, fp);
  fwrite(&H_, sizeof(BYTE), 1, fp);
  fwrite(&L_, sizeof(BYTE), 1, fp);
  fwrite(&R, sizeof(BYTE), 1, fp);
  fwrite(&I, sizeof(BYTE), 1, fp);
  temp = PC - ram;
  fwrite(&temp, sizeof(WORD), 1, fp);
  temp = STACK - ram;
  fwrite(&temp, sizeof(WORD), 1, fp);

  for (i = 0; i < SBSIZE; i++)
    fwrite(&soft[i], sizeof(soft[0]), 1, fp);

  fprintf(fp, "%d\n", current_port);

  for (i = 0; i < NUMIOPORTS; i++)
    if (IOPort[i] != NULL)				/* if the port is active */
    {
      fprintf(fp, "%d\n", i);
      fwrite(IOPort[i]->obuffer, sizeof(IOPort[0]->obuffer), 1, fp);
      fprintf(fp, "%d\n", IOPort[i]->out_ptr);
      fwrite(IOPort[i]->ibuffer, sizeof(IOPort[0]->ibuffer), 1, fp);
      fprintf(fp, "%d\n", IOPort[i]->in_ptr);
      fprintf(fp, "%d\n", IOPort[i]->in_len);
      fprintf(fp, "%d\n", IOPort[i]->ishex);
    }

  fprintf(fp, "%d\n", NUMIOPORTS + 1);			/* terminate the list of io ports */

  fwrite(&ram, K64K, 1, fp);

  sprintf(str, "Saved project in %s.\n", fn);
  Add_to_Log(str);

  fclose(fp);
}


void Read_Project(char *fn)
{
  FILE *fp;
  char str[64];
  int i;
  WORD temp;
  int dbg;

  init_IOport();		/* clear up the IO ports first */

  fp = fopen(fn, "r");
  if (fp == NULL)
  {
    sprintf(lstr, "Unable to load project from %s", fn);
    show_error(lstr);
    return;
  }

  fgets(str, sizeof(str), fp);
  if (strcmp(str, "#Z80sim\n") != 0)
  {
    sprintf(lstr, "%s is not a Z80sim project file.", fn);
    show_error(lstr);
  }

  fscanf(fp, "%c\n", &Dump_Reg);
  fscanf(fp, "%d\n", &run_slow_pref);
  fgets(project_fn, 1024, fp);
  fread(&A, sizeof(BYTE), 1, fp);
  fread(&F, sizeof(BYTE), 1, fp);
  fread(&B, sizeof(BYTE), 1, fp);
  fread(&C, sizeof(BYTE), 1, fp);
  fread(&D, sizeof(BYTE), 1, fp);
  fread(&E, sizeof(BYTE), 1, fp);
  fread(&H, sizeof(BYTE), 1, fp);
  fread(&L, sizeof(BYTE), 1, fp);
  fread(&IX, sizeof(WORD), 1, fp);
  fread(&IY, sizeof(WORD), 1, fp);
  fread(&A_, sizeof(BYTE), 1, fp);
  fread(&F_, sizeof(BYTE), 1, fp);
  fread(&B_, sizeof(BYTE), 1, fp);
  fread(&C_, sizeof(BYTE), 1, fp);
  fread(&D_, sizeof(BYTE), 1, fp);
  fread(&E_, sizeof(BYTE), 1, fp);
  fread(&H_, sizeof(BYTE), 1, fp);
  fread(&L_, sizeof(BYTE), 1, fp);
  fread(&R, sizeof(BYTE), 1, fp);
  fread(&I, sizeof(BYTE), 1, fp);
  fread(&temp, sizeof(WORD), 1, fp);
  PC = temp + ram;
  fread(&temp, sizeof(WORD), 1, fp);
  STACK = temp + ram;

  for (i = 0; i < SBSIZE; i++)
    fread(&soft[i], sizeof(soft[0]), 1, fp);

printf("%04X\n", ftell(fp));

  fscanf(fp, "%d\n", &current_port);

dbg =  fscanf(fp, "%d\n", &i);
  if (i == SBSIZE)
  {
    printf("failed to read io port list dbg=%d\n", dbg);
    return;
  }

  while(i != (NUMIOPORTS + 1))
  {
    Create_IOPort_Struct(i);
    fread(&IOPort[i]->obuffer, sizeof(IOPort[0]->obuffer), 1, fp);
    fscanf(fp, "%d\n", &IOPort[i]->out_ptr);
    fread(&IOPort[i]->ibuffer, sizeof(IOPort[0]->ibuffer), 1, fp);
    fscanf(fp, "%d\n", &IOPort[i]->in_ptr);
    fscanf(fp, "%d\n", &IOPort[i]->in_len);
    fscanf(fp, "%d\n", &IOPort[i]->ishex);
    fscanf(fp, "%d\n", &i);
  }

  fread(&ram, K64K, 1, fp);

  sprintf(str, "Loaded project from %s.\n", fn);
  Add_to_Log(str);

  build_code_cache();
  Show_Code(PC, TRUE);
  Dump_IOPort(current_port);
  set_reg_button(Dump_Reg);
  Show_All();

  fclose(fp);
}
