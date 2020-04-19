/*
        Z80 Emulator GUI

        Copyright (C) 2008 Justin Clancy
*/

#include <gtk/gtk.h>
#include <stdio.h>

#include "sim.h"
#include "simglb.h"
#include "guiglb.h"

GtkWidget *flentry[NFLAGS];

void Show_Flags(void)
{
  int i, m;

  m = 1 << (NFLAGS - 1);

  for (i = 0; i < NFLAGS; i++)
  {
    if (F & m)
      gtk_label_set_text(GTK_LABEL(flentry[i]), "1");
    else
      gtk_label_set_text(GTK_LABEL(flentry[i]), "0");

    m = m >> 1;
  }
}

void Toggle_Flag(int flag)
{
  int m;
  
  m = 1 << (NFLAGS - 1);
  m = m >> flag;

  F = F ^ m;

  Show_Flags();
}


void init_Flags(GtkWidget *Main)
{
  int i;

  for (i = 0; i < NFLAGS; i++)
    gtk_label_set_text(GTK_LABEL(flentry[i]), "0");
}
