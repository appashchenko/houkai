#include <gtk/gtk.h>

GtkWidget *main_window;

enum { LABEL_COLUMN = 0, ID_COLUMN, OFFSET_COLUMN, N_COLUMN };

static void app_quit_activated(GSimpleAction *action, GVariant *parameter,
                               gpointer app) {
  g_application_quit(G_APPLICATION(app));
}

static void app_activate(GtkApplication *app, gpointer user_data);
static void open_file_dialog_response(GtkDialog *, GtkResponseType);
static void open_file_dialog_activated(GSimpleAction *, GVariant *, gpointer);
GtkTreeModel *fill_store(void);

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.houkai.explorer", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(app_activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}

GtkTreeModel *fill_store(void) {
  GtkTreeStore *store =
      gtk_tree_store_new(N_COLUMN, G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER);
  GtkTreeIter iter = {0}, parent = {0};

  gtk_tree_store_append(store, &parent, FALSE);
  gtk_tree_store_set(store, &parent, LABEL_COLUMN, "HEADER", ID_COLUMN, 0,
                     OFFSET_COLUMN, 0, -1);

  gtk_tree_store_append(store, &iter, &parent);
  gtk_tree_store_set(store, &iter, LABEL_COLUMN, "BKHD", ID_COLUMN, 1,
                     OFFSET_COLUMN, 0, -1);

  gtk_tree_store_append(store, &iter, &parent);
  gtk_tree_store_set(store, &iter, LABEL_COLUMN, "BKHD", ID_COLUMN, 2,
                     OFFSET_COLUMN, 0, -1);

  return GTK_TREE_MODEL(store);
}

static void app_activate(GtkApplication *app, gpointer user_data) {
  // GtkWidget *main_window;
  GtkWidget *main_box;
  GtkWidget *tool_box;
  GtkWidget *nav;
  GtkWidget *items;
  GtkTreeModel *model;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  main_window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(main_window), "Houkai Explorer");
  gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 200);

  GSimpleAction *act_quit = g_simple_action_new("quit", NULL);
  g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act_quit));
  g_signal_connect(act_quit, "activate", G_CALLBACK(app_quit_activated), app);

  GSimpleAction *act_open = g_simple_action_new("open", NULL);
  g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act_open));
  g_signal_connect(act_open, "activate", G_CALLBACK(open_file_dialog_activated),
                   app);

  GMenu *menubar = g_menu_new();
  GMenuItem *menu_item_file = g_menu_item_new("File", NULL);
  GMenu *menu_file = g_menu_new();
  GMenuItem *menu_file_open = g_menu_item_new("Open", "app.open");
  g_menu_append_item(menu_file, menu_file_open);
  g_object_unref(menu_file_open);
  GMenuItem *menu_file_exit = g_menu_item_new("Exit", "app.quit");
  g_menu_append_item(menu_file, menu_file_exit);
  g_object_unref(menu_file_exit);

  g_menu_item_set_submenu(menu_item_file, G_MENU_MODEL(menu_file));
  g_menu_append_item(menubar, menu_item_file);
  g_object_unref(menu_file);
  gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menubar));
  gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(main_window),
                                          TRUE);

  tool_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *button = gtk_button_new_with_label("Play");
  gtk_box_append(GTK_BOX(tool_box), button);
  GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append(GTK_BOX(tool_box), sep1);
  gtk_window_set_child(GTK_WINDOW(main_window), tool_box);

  main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_child(GTK_WINDOW(main_window), main_box);

  model = fill_store();

  nav = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(nav), FALSE);
  g_object_unref(G_OBJECT(model));
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "text",
                                                    LABEL_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(nav), column);
  g_signal_connect(nav, "realize", G_CALLBACK(gtk_tree_view_expand_all), NULL);

  items = gtk_tree_view_new();
  gtk_widget_set_hexpand(items, TRUE);

  gtk_box_append(GTK_BOX(main_box), nav);
  gtk_box_append(GTK_BOX(main_box), items);

  gtk_window_present(GTK_WINDOW(main_window));
}

static void open_file_dialog_response(GtkDialog *dialog,
                                      GtkResponseType response) {
  GListModel *files = NULL;

  if (response == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    files = gtk_file_chooser_get_files(chooser);

    for (guint i = 0;; i++) {
      char *filename = NULL;
      GFile *file = NULL;

      file = g_list_model_get_item(files, i);
      if (file == NULL)
        break;

      filename = g_file_get_path(file);
      g_message("%s\n", filename);
      g_free(filename);
      g_object_unref(file);
    }

    g_object_unref(files);
  }

  gtk_window_destroy(GTK_WINDOW(dialog));
}

static void open_file_dialog_activated(GSimpleAction *action,
                                       GVariant *parameter, gpointer app) {
  GtkWidget *dialog;

  GtkFileChooserAction action_type = GTK_FILE_CHOOSER_ACTION_OPEN;
  GtkFileFilter *filter;

  dialog = gtk_file_chooser_dialog_new(
      "Open File", GTK_WINDOW(main_window), action_type, "Cancel",
      GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

  filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Wwise AKPK Storage (PCK)");
  gtk_file_filter_add_pattern(filter, "*.pck");

  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

  gtk_widget_show(dialog);
  g_signal_connect(dialog, "response", G_CALLBACK(open_file_dialog_response),
                   NULL);
}

/*
#include "akpk/akpk.h"
#include <stdio.h>

int main(int argv, char* argc[]) {
  if (argv > 1) {
    akpk_open(argc[1]);
  } else {
    printf("Usage: houkai file.pck\n");
    return 0;
  }
  return 0;
}*/

// # vim: ts=2 sw=2 expandtab
