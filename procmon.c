#ifndef UI_H
#define UI_H
#include "ui.h"
#endif

#include <sys/sysinfo.h>

int main(int argc, char ** argv) {
  GtkApplication * app = NULL;
  init_app(app, argc, argv);
}
