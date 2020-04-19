#include <gtk/gtk.h>
#include <stdlib.h>

#include "z80/sim.h"
#include "z80/simglb.h"
#include "guiglb.h"
#include "prototypes.h"

void Show_All(void)
{
  Show_Flags();
  Show_Registers();
  Dump_From_Register(Dump_Reg);
  Show_Code(PC, FALSE);
}

int main(int argc, char *argv[])
{
  GtkBuilder      *builder; 
  GtkWidget       *Main;
  int i;

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "z80em.glade", NULL);

  Main = GTK_WIDGET(gtk_builder_get_object(builder, "Main"));
  gtk_builder_connect_signals(builder, NULL);

  codetext = GTK_WIDGET(gtk_builder_get_object(builder, "Code_Text"));
  optcodeb = GTK_WIDGET(gtk_builder_get_object(builder, "OpcodeButton"));
  slowrunspin = GTK_WIDGET(gtk_builder_get_object(builder, "SlowRunSpin"));
  for (i = 0; i < NFLAGS; i++)
  {
    sflentry = GTK_WIDGET(gtk_builder_get_object(builder, FlagEnt[i]));
    flentry[i] = sflentry;
  }
  logtext = GTK_WIDGET(gtk_builder_get_object(builder, "Log_Text"));
  a_entry = GTK_WIDGET(gtk_builder_get_object(builder, "A_REG_Entry"));
  bc_entry = GTK_WIDGET(gtk_builder_get_object(builder, "BC_REG_Entry"));
  de_entry = GTK_WIDGET(gtk_builder_get_object(builder, "DE_REG_Entry"));
  hl_entry = GTK_WIDGET(gtk_builder_get_object(builder, "HL_REG_Entry"));
  ix_entry = GTK_WIDGET(gtk_builder_get_object(builder, "IX_REG_Entry"));
  iy_entry = GTK_WIDGET(gtk_builder_get_object(builder, "IY_REG_Entry"));
  pc_entry = GTK_WIDGET(gtk_builder_get_object(builder, "PC_REG_Entry"));
  sp_entry = GTK_WIDGET(gtk_builder_get_object(builder, "SP_REG_Entry"));
  i_entry = GTK_WIDGET(gtk_builder_get_object(builder, "I_REG_Entry"));
  r_entry = GTK_WIDGET(gtk_builder_get_object(builder, "R_REG_Entry"));
  aa_entry = GTK_WIDGET(gtk_builder_get_object(builder, "AA_REG_Entry"));
  abc_entry = GTK_WIDGET(gtk_builder_get_object(builder, "ABC_REG_Entry"));
  ade_entry = GTK_WIDGET(gtk_builder_get_object(builder, "ADE_REG_Entry"));
  ahl_entry = GTK_WIDGET(gtk_builder_get_object(builder, "AHL_REG_Entry"));
  mem_entry = GTK_WIDGET(gtk_builder_get_object(builder, "Mem_REG_Entry"));
  ioporttext = GTK_WIDGET(gtk_builder_get_object(builder, "IOPortText"));
  in_port_prompt = GTK_WIDGET(gtk_builder_get_object(builder, "In_Port_Prompt"));
  in_port_data = GTK_WIDGET(gtk_builder_get_object(builder, "In_Port_Data"));
  memtext = GTK_WIDGET(gtk_builder_get_object(builder, "Mem_text"));
  Mem_Addr_Entry = GTK_WIDGET(gtk_builder_get_object(builder, "Mem_Addr_Entry"));
  Mem_Content_Entry = GTK_WIDGET(gtk_builder_get_object(builder, "Mem_Content_Entry"));

  g_object_unref(builder);

  Dump_Reg = 'P';
  if (do_getfile("./test/z80main.bin"))
//  if (do_getfile("./test/testops.bin"))
//  if (do_getfile("./test/testio.bin"))
//  if (do_getfile("./test/testscroll.bin"))
    return(1);

  init_Log(Main);
  init_Flags(Main);
  init_Registers(Main);
  init_Memory(Main);
  init_Code(Main);
  init_IOport();

  Dump_From_Register(Dump_Reg);
  Show_Flags();
  Show_Registers();
  Show_Code(PC, TRUE);

  gtk_widget_show(Main);                

  gtk_main();

  return 0;
}
