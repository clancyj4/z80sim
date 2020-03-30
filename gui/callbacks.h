#include <gtk/gtk.h>


void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_paste1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_delete1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_Step_tool_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_test_tool_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_Update_REG_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_A_button_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_I_Button_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_R_Button_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_IX_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_BC_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_DE_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_HL_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_IY_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_PC_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_SP_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_BC_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_DE_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_HL_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_IX_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_IY_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_PC_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_SP_Button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

gboolean
on_Code_Text_event                     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_buttonS_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonZ_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonY_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonH_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonX_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonP_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonN_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonC_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_RunButton_toggled                   (GtkToggleToolButton *toggletoolbutton,
                                        gpointer         user_data);

void
on_Trace_tool_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_Break_tool_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_Error_OK_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_Break_Button_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_OpcodeButton_clicked                (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_OpcodeButton_toggled                (GtkToggleToolButton *toggletoolbutton,
                                        gpointer         user_data);

void
on_Mem_Button_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_Mem_Update_Button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_Mem_Step_Back_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_Mem_Step_Fwd_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_Mem_Page_Back_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_Mem_Row_Fwd_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_Mem_Page_Fwd_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_Mem_Row_Back_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_Synch_Memory_Button_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_Reset_Button_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_Zero_Button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_view1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_ioport1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_log1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_IOASCII_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_IOHex_toggled                       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_PortSpin_value_changed              (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_PC_Set_Button_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_ops1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_step1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_trace1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_go1_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_memory_fill1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_Clear_Break_Button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_File_Cancel_Button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_File_Open_Button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_SlowRunButton_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

gboolean
on_Code_Text_button_press_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_Code_Text_button_release_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_Mem_text_button_release_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_SlowRunSpin_value_changed           (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

gboolean
on_Log_destroy_event                   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_Log_delete_event                    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_In_Port_Submit_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_Code_scroll_to_PC_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_IOWIN_destroy                       (GtkWidget       *object,
                                        gpointer         user_data);
