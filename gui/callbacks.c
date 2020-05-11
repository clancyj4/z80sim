#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>

#include "callbacks.h"
#include "sim.h"
#include "simglb.h"
#include "guiglb.h"
#include "prototypes.h"

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  Get_Project_File();
}


void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  Save_Project(project_fn);
}


void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  Save_File();
}


void
on_File_Cancel_Button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Get_File_Cancel();
}


void
on_Project_File_Cancel_Button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Get_Project_File_Cancel();
}


void
on_save_save_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Save_File_Save();
}


void
on_save_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Save_File_Cancel();
}


void
on_File_Open_Button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Get_File_Open();
}


void
on_Project_File_Open_Button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  Get_Project_File_Open();
}


void
on_load1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  Get_File();
}

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gtk_main_quit ();
}


void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_paste1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_delete1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  show_about(GTK_WIDGET(menuitem));
}

void
on_Step_tool_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  void do_step(void);

  do_step();
  Show_Code(PC, FALSE);
  Show_Registers();
  Show_Flags();
  Dump_From_Register(Dump_Reg);
  Dump_IOPort(current_port);
}


void
on_test_tool_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
/*  gtk_dialog_run(GTK_DIALOG(Error_Dialog)); */
}


void
on_Update_REG_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Update_Registers();
  Dump_From_Register(Dump_Reg);
}

void
on_IX_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'X';
  Dump_From_Register(Dump_Reg);
}


void
on_BC_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'B';
  Dump_From_Register(Dump_Reg);
}


void
on_DE_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'D';
  Dump_From_Register(Dump_Reg);
}


void
on_HL_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'H';
  Dump_From_Register(Dump_Reg);
}


void
on_IY_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'Y';
  Dump_From_Register(Dump_Reg);
}


void
on_PC_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'P';
  Dump_From_Register(Dump_Reg);
}


void
on_SP_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'S';
  Dump_From_Register(Dump_Reg);
}


void
on_Mem_Button_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  Dump_Reg = 'M';
  Dump_From_Register(Dump_Reg);
}

void
on_buttonS_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(0);
}


void
on_buttonZ_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(1);
}


void
on_buttonY_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(2);
}


void
on_buttonH_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(3);
}


void
on_buttonX_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(4);
}


void
on_buttonP_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(5);
}


void
on_buttonN_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(6);
}


void
on_buttonC_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  Toggle_Flag(7);
}


void
on_RunButton_toggled                   (GtkToggleToolButton *toggletoolbutton,
                                        gpointer         user_data)
{
  void do_go(void);

  if (!gtk_toggle_tool_button_get_active(toggletoolbutton))
  {
    gtk_toggle_tool_button_set_active(toggletoolbutton, FALSE);
    if (cpu_state == CONTIN_RUN)
    {
      cpu_state = STOPPED;
    }
  }
  else
  {
    do_go();
    Show_All();
    gtk_toggle_tool_button_set_active(toggletoolbutton, FALSE);
  }
}


void
on_Trace_tool_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  void do_trace(char *);
  do_trace("");
  Show_All();
}


void
on_Break_tool_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  dump_code_vars();
  dump_memory_vars();
}


void
on_Error_OK_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_Break_Button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  Code_Break();
}


void
on_OpcodeButton_toggled                (GtkToggleToolButton *toggletoolbutton,
                                        gpointer         user_data)
{
  gboolean tog;

  tog = gtk_toggle_tool_button_get_active(toggletoolbutton);

  if (tog)
    show_opcodes = TRUE;
  else
    show_opcodes = FALSE;

  Show_Code(PC, TRUE);
}


void
on_Mem_Update_Button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Update_Memory();
  Show_Code(PC, TRUE);
}


void
on_Mem_Step_Back_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  Mem_Change_Addr(-1);
}

void
on_Mem_Step_Fwd_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  Mem_Change_Addr(1);
}

void
on_Mem_Row_Back_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  Mem_Change_Addr(-8);
}

void
on_Mem_Row_Fwd_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  Mem_Change_Addr(8);
}

void
on_Mem_Page_Back_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  Mem_Change_Addr(-128);
}

void
on_Mem_Page_Fwd_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  Mem_Change_Addr(128);
}

void
on_Synch_Memory_Button_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
  Synch_Mem_Reg();
}


void
on_Reset_Button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  PC = ram;
  Show_Code(PC, TRUE);
  Show_Registers();

}


void
on_Zero_Button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  Zero_Registers();
}


void
on_view1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_ioport1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  printf("on_ioport1_activate called with current_port=%d\n", current_port);
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
  {
    show_iowin(TRUE);
    Dump_IOPort(current_port);
  }
  else
    show_iowin(FALSE);
}


void
on_log1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
    show_log(TRUE);
  else
    show_log(FALSE);
}


void
on_IOASCII_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (IOPort[current_port] == NULL)	/* toggling a non-existent port? */
    Create_IOPort_Struct(current_port);	/* strange - but create it anyway */

printf("ishex set to 0.\n");
  IOPort[current_port]->ishex = 0;
  Dump_IOPort(current_port);
}


void
on_IOHex_toggled                       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (IOPort[current_port] == NULL)	/* toggling a non-existent port? */
    Create_IOPort_Struct(current_port);	/* strange - but create it anyway */

printf("ishex set to 1.\n");
  IOPort[current_port]->ishex = 1;
  Dump_IOPort(current_port);
}


void
on_PortSpin_value_changed              (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  printf("on_PortSpin_value_changed called.\n");
  current_port = (int)gtk_spin_button_get_value(spinbutton);
  printf("current port changed to %d which is ", current_port);
  if (IOPort[current_port] == NULL)
    printf("UNassigned.\n");
  else
  {
    printf("assigned.\n");
  }
  Dump_IOPort(current_port);
}


void
on_PC_Set_Button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  set_PC_to_selection();
}


void
on_ops1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_step1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_trace1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_go1_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_memory_fill_activated               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_Clear_Break_Button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  Code_UnBreak();
}


void
on_SlowRunButton_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (run_slow == 0)
    run_slow = run_slow_pref;		/* set the delay */
  else
    run_slow = 0;
}



gboolean
on_Code_Text_button_release_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  Code_Clicked();
  return FALSE;
}


gboolean
on_Mem_text_button_release_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  Mem_Clicked();
  return FALSE;
}


void
on_SlowRunSpin_value_changed           (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  run_slow_pref = gtk_spin_button_get_value_as_int(spinbutton) * 1000;
  if (run_slow > 0)
    run_slow = run_slow_pref;		/* set the delay */
}


void
on_In_Port_Submit_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  assign_in_port_to_buff(current_port);
}


void
on_Code_scroll_to_PC_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  Show_Code(PC, TRUE);
}


void
on_IOWIN_delete_event                  (GtkWidget       *object,
                                        gpointer         user_data)
{
  fprintf(stderr, "IOWIN destroy called.\n");
}


void
on_breaks_activated                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
    show_breaks(TRUE);
  else
    show_breaks(FALSE);
}


gboolean
on_Break_Text_button_release_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  Breaks_Clicked();
  return FALSE;
}


void
on_BreaksSpin_value_changed            (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
  int pv;

  printf("on_BreaksSpin_value_changed called.\n");
  pv = gtk_spin_button_get_value_as_int(spinbutton);
  pass_value_changed(pv);
}


void
on_Breakpoints_delete_event            (GtkWidget       *object,
                                        gpointer         user_data)
{
  printf("Breakpoints delete event called.\n");
  show_breaks(FALSE);
}


void
on_Log_delete_event            (GtkWidget       *object,
                                        gpointer         user_data)
{
  show_log(FALSE);
}

