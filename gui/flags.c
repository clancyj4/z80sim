/*
        Z80 Emulator GUI

        Copyright (C) 2008 Justin Clancy
*/

#include <gtk/gtk.h>
#include <stdio.h>

#include "sim.h"
#include "simglb.h"

#define NFLAGS	8

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
  {
    flentry[i] = lookup_widget(Main, FlagEnt[i]);
    gtk_label_set_text(GTK_LABEL(flentry[i]), "0");
  }
}
