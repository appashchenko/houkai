#include "akpk/akpk.h"
#include <config.h>
#include <gtk/gtk.h>
#include <stdio.h>

enum { LABEL_COLUMN = 0, ID_COLUMN, OFFSET_COLUMN, N_COLUMN };

GtkTreeModel* fill_store(void) {
  GtkTreeStore* store =
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

  memset(&store, 1, 20);

  return GTK_TREE_MODEL(store);
}

static void on_activate(GtkApplication* app, gpointer user_data) {
  gint res;

  GtkWidget* dialog = gtk_file_chooser_dialog_new(
      "Open File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel",
      GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

  GtkFileFilter* filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Wwise AKPK Storage (PCK)");
  gtk_file_filter_add_pattern(filter, "*.pck");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  res = gtk_dialog_run(GTK_DIALOG(dialog));
  if (res == GTK_RESPONSE_ACCEPT) {
    char* filename;
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
    filename = gtk_file_chooser_get_filename(chooser);
    g_free(filename);
    gtk_widget_destroy(dialog);
  } else {
    gtk_widget_destroy(dialog);
    return;
  }

  GtkWidget* window;
  GtkWidget* tree;
  GtkTreeModel* model;
  GtkCellRenderer* renderer;
  GtkTreeViewColumn* column;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  model = fill_store();

  tree = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
  g_object_unref(G_OBJECT(model));

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "text",
                                                    LABEL_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  g_signal_connect(tree, "realize", G_CALLBACK(gtk_tree_view_expand_all),
  NULL);
  gtk_container_add(GTK_CONTAINER(window), tree);

  gtk_window_present(GTK_WINDOW(window));
  gtk_widget_show_all(window);
}

int main(int argc, char* argv[]) {

  struct AKPK* bank = akpk_open("test.pck");

  akpk_extract(bank);

  if (bank) akpk_close(bank);
  return 0;
}
