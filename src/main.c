#include "akpk/akpk.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>


void closeApp(GtkWidget *window, gpointer data)
{
    gtk_main_quit();
}

int main() {
  GtkWidget *window;
  GtkTreeStore *store;
  GtkWidget *view;


  struct AKPK* bank = akpk_open("test.pck");

  akpk_extract(bank);

  if (bank) akpk_close(bank);
  return 0;
}
