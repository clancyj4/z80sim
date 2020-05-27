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

WORD Mem_REG;

void set_reg_button(char dreg)
{
  switch (dreg)
  {
    case 'B':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bc_button), TRUE);
      break;
    case 'D':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(de_button), TRUE);
      break;
    case 'H':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl_button), TRUE);
      break;
    case 'X':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ix_button), TRUE);
      break;
    case 'Y':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(iy_button), TRUE);
      break;
    case 'P':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pc_button), TRUE);
      break;
    case 'S':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sp_button), TRUE);
      break;
    case 'M':
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_button), TRUE);
      break;
    default:
      printf("Illegal dump register %c\n", dreg);
      break;
  }
}


void Dump_From_Register(char dreg)
{
  WORD temp;

  switch (dreg)
  {
    case 'B':
      temp = B * 256 + C;
      break;
    case 'D':
      temp = D * 256 + E;
      break;
    case 'H':
      temp = H * 256 + L;
      break;
    case 'X':
      temp = IX;
      break;
    case 'Y':
      temp = IY;
      break;
    case 'P':
      temp = PC - ram;
      break;
    case 'S':
      temp = STACK - ram;
      break;
    case 'M':
      temp = Mem_REG;
      break;
    default:
      temp = 0;
      break;
    }

  Show_Memory(temp, DEFAULT_MEMDUMP_BYTES, TRUE);
}


void Show_Registers(void)
{
  sprintf(tstr, "%02X", A & 0xff);
  gtk_entry_set_text (GTK_ENTRY(a_entry), tstr);

  sprintf(tstr, "%04X", (B*256+C) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(bc_entry), tstr);

  sprintf(tstr, "%04X", (D*256+E) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(de_entry), tstr);

  sprintf(tstr, "%04X", (H*256+L) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(hl_entry), tstr);

  sprintf(tstr, "%04X", IX & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(ix_entry), tstr);

  sprintf(tstr, "%04X", IY & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(iy_entry), tstr);

  sprintf(tstr, "%04X", ((WORD)(PC - ram)) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(pc_entry), tstr);

  sprintf(tstr, "%04X", ((WORD)(STACK - ram)) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(sp_entry), tstr);

  sprintf(tstr, "%02X", I & 0xff);
  gtk_entry_set_text (GTK_ENTRY(i_entry), tstr);

  sprintf(tstr, "%02X", (WORD)(R & 0xff));
  gtk_entry_set_text (GTK_ENTRY(r_entry), tstr);

  sprintf(tstr, "%04X", (A_*256+F_) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(aa_entry), tstr);

  sprintf(tstr, "%04X", (B_*256+C_) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(abc_entry), tstr);

  sprintf(tstr, "%04X", (D_*256+E_) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY(ade_entry), tstr);

  sprintf(tstr, "%04X", (H_*256+L_) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY( ahl_entry), tstr);

  sprintf(tstr, "%04X", (Mem_REG) & 0xffff);
  gtk_entry_set_text (GTK_ENTRY( mem_entry), tstr);
}


void Update_Registers(void)
{
  WORD temp;

  A = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(a_entry)));

  temp = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(bc_entry)));
  B = temp / 256;
  C = temp % 256;

  temp = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(de_entry)));
  D = temp / 256;
  E = temp % 256;
  
  temp = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(hl_entry)));
  H = temp / 256;
  L = temp % 256;
  
  IX = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(ix_entry)));
  IY = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(iy_entry)));
  STACK = ram + (exatoi((char *)gtk_entry_get_text(GTK_ENTRY(sp_entry))) & 0xffff);
  PC = ram + (exatoi((char *)gtk_entry_get_text(GTK_ENTRY(pc_entry))) & 0xffff);
  I = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(i_entry)));
  R = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(r_entry)));

  temp = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(aa_entry)));
  A_ = temp / 256;
  F_ = temp % 256;

  temp = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(abc_entry)));
  B_ = temp / 256;
  C_ = temp % 256;

  temp = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(ade_entry)));
  D_ = temp / 256;
  E_ = temp % 256;

  temp = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(ahl_entry)));
  H_ = temp / 256;
  L_ = temp % 256;

  Mem_REG = exatoi((char *)gtk_entry_get_text(GTK_ENTRY(mem_entry)));

  Show_Registers();
  Show_Code(PC, FALSE);
}


void Zero_Registers(void)
{
  A = 0;
  F = 0;
  B = 0;
  C = 0;
  D = 0;
  E = 0;
  H = 0;
  L = 0;
  IX = 0;
  IY = 0;

  A_ = 0;
  F_ = 0;
  B_ = 0;
  C_ = 0;
  D_ = 0;
  E_ = 0;
  H_ = 0;
  L_ = 0;

  STACK = ram;

  Show_Registers();
  Dump_From_Register(Dump_Reg);
  Show_Flags();
}


void init_Registers(GtkWidget *Main)
{
//  a_entry = lookup_widget(Main, "A_REG_Entry");
//  bc_entry = lookup_widget(Main, "BC_REG_Entry");
//  de_entry = lookup_widget(Main, "DE_REG_Entry");
//  hl_entry = lookup_widget(Main, "HL_REG_Entry");
//  ix_entry = lookup_widget(Main, "IX_REG_Entry");
//  iy_entry = lookup_widget(Main, "IY_REG_Entry");
//  pc_entry = lookup_widget(Main, "PC_REG_Entry");
//  sp_entry = lookup_widget(Main, "SP_REG_Entry");
//  i_entry = lookup_widget(Main, "I_REG_Entry");
//  r_entry = lookup_widget(Main, "R_REG_Entry");
//  aa_entry = lookup_widget(Main, "AA_REG_Entry");
//  abc_entry = lookup_widget(Main, "ABC_REG_Entry");
//  ade_entry = lookup_widget(Main, "ADE_REG_Entry");
//  ahl_entry = lookup_widget(Main, "AHL_REG_Entry");
//  mem_entry = lookup_widget(Main, "Mem_REG_Entry");
}
