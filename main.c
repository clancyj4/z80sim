#include <gtk/gtk.h>

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
  GtkWidget       *window;

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "z80em2.glade", NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "Main"));
  gtk_builder_connect_signals(builder, NULL);

  g_object_unref(builder);

  gtk_widget_show(window);                

  Dump_Reg = 'P';
  init_Log(Main);
  if (do_getfile("./test/z80main.bin"))
//  if (do_getfile("./test/testops.bin"))
//  if (do_getfile("./test/testio.bin"))
//  if (do_getfile("./test/testscroll.bin"))
    return(1);

  init_Flags(Main);
  init_Registers(Main);
  init_Memory(Main);
  init_Code(Main);
  init_IOport();

  Dump_From_Register(Dump_Reg);
  Show_Flags();
  Show_Registers();
  Show_Code(PC, TRUE);

  gtk_main();

  return 0;
}

// called when window is closed
void on_window_main_destroy()
{
  gtk_main_quit();
}
