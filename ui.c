#ifndef UI_H
#define UI_H
#include "ui.h"
#endif
#ifndef UTILS_H
#define UTILS_H
#include "utils.h"
#endif

#define DEFAULT_WIDTH (400)
#define DEFAULT_HEIGHT (400)
#ifdef DEBUG
#define DPRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define DPRINT(...) do { } while ( false )
#endif

gint g_current_timeout_tag = 0;
pid_t g_current_selected_pid = -1;
search_reg g_current_search = {{0}, false};
int g_list_tree_mode = LIST;
sort_method * g_proclist_sort_method = NULL;

void run_process_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  DPRINT("Run process button clicked!\n");
  GtkBuilder * builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "procmon.glade", NULL);
  GtkWidget * window = GTK_WIDGET(gtk_builder_get_object(builder, "run_process_dialog"));
  gtk_builder_connect_signals(builder, NULL);
  gtk_widget_show(window);
}

void accept_run_process_button_clicked_cb(GtkButton * button, gpointer user_data) {
  gchar * term = (gchar *)gtk_entry_get_text(GTK_ENTRY(user_data));
  run_proc_by_str(term);
  GtkWidget * window = gtk_widget_get_toplevel(GTK_WIDGET(user_data));
  gtk_widget_destroy(window);
}

void cancel_run_process_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  GtkWidget * window = gtk_widget_get_toplevel(GTK_WIDGET(user_data));
  gtk_widget_destroy(window);
}

void kill_process_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  pid_t pid = g_current_selected_pid;
  DPRINT("Kill process button clicked! %d\n", pid);
  if (pid != -1 && !kill(pid, 0)) {
    kill(pid, SIGABRT);
  }
  g_current_selected_pid = -1;
}

void stop_process_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  DPRINT("Stop process button clicked!\n");
  pid_t pid = g_current_selected_pid;
  if (pid != -1 && !kill(pid, 0)) {
    kill(pid, SIGSTOP);
  }
  g_current_selected_pid = -1;
}

void continue_process_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  DPRINT("Continue process button clicked!\n");
  pid_t pid = g_current_selected_pid;
  if (pid != -1 && !kill(pid, 0)) {
    kill(pid, SIGCONT);
  }
  g_current_selected_pid = -1;
}

void list_tree_toggle_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  g_list_tree_mode ^= TREE;
}

void process_list_tree_list_row_activated_cb(GtkTreeView * tree_view, GtkTreePath * path, GtkTreeViewColumn * column, gpointer user_data) {
  pid_t pid;
  GtkTreeIter iter;
  GtkTreeModel * model = gtk_tree_view_get_model(tree_view);
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, 0, &pid, -1);
  g_object_unref(model);
  g_current_selected_pid = pid;
  return;
}

void process_search_box_activate_cb(GtkSearchEntry * search, GVariant * parameter, gpointer user_data) {
  DPRINT("Process search box searching!\n");
  const gchar * condition = gtk_entry_get_text(GTK_ENTRY(search));
  if (*condition == '\0') {
    g_current_search.active = false;
    return;
  }
  int err = regcomp(&g_current_search.reg, condition, REG_EXTENDED);
  if (err) {
    DPRINT("Could not compile search regex.\n");
    g_current_search.active = false;
    return;
  }
  g_current_search.active = true;
}

void run_menuitem_activate_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  DPRINT("Run menu item clicked!\n");
}

gchar * get_cpuinfo_string(void) {
  cpu_info * cpu = get_cpu_info();
  size_t text_len = 0;
  long int hrs = (cpu->uptime / 3600);
  long int min = (cpu->uptime - (3600 * hrs))/60;
  long int scs = (cpu->uptime - (3600 * hrs) - (min * 60));
  text_len += snprintf(NULL, 0, "%d", cpu->physical_core_ct);
  text_len += snprintf(NULL, 0, "%d", cpu->logical_core_ct);
  text_len += snprintf(NULL, 0, "%s", cpu->model_string);
  text_len += snprintf(NULL, 0, "%d", cpu->temperature);
  text_len += snprintf(NULL, 0, "%d", cpu->l1_cache_size);
  text_len += snprintf(NULL, 0, "%d", cpu->l2_cache_size);
  text_len += snprintf(NULL, 0, "%d", cpu->clock_speed);
  text_len += snprintf(NULL, 0, "%ld", cpu->proc_count);
  text_len += snprintf(NULL, 0, "%ld", hrs);
  text_len += snprintf(NULL, 0, "%ld", min);
  text_len += snprintf(NULL, 0, "%ld", scs);
  const char * fmt = "CPU Model: %16s\n"
                     "Physical Cores: %d\n"
                     "Temperature: %dC\n"
                     "Logical Cores: %d\n"
                     "L1 Cache: %dKB\n"
                     "L2 Cache: %dKB\n"
                     "Clock Speed: %dMHz\n"
                     "Uptime: %ldH %ldM %ldS\n"
                     "Process Count: %ld\n";
  gchar * cpuinfo_string = (char *)calloc(strlen(fmt) + text_len + 1, sizeof(char));
  snprintf(cpuinfo_string, strlen(fmt) + text_len, fmt, cpu->model_string, cpu->physical_core_ct, cpu->temperature, cpu->logical_core_ct, cpu->l1_cache_size, cpu->l2_cache_size, cpu->clock_speed, hrs, min, scs, cpu->proc_count);
  free_cpu_info(cpu);
  return cpuinfo_string;
}

void export_menuitem_activate_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  time_t current_time = time(NULL);
  char * filename = (char *)calloc(snprintf(NULL, 0, "%ld.log", current_time) + 2, sizeof(char));
  snprintf(filename, snprintf(NULL, 0, "%ld.log", current_time)+ 1, "%ld.log", current_time);
  FILE * logfile = fopen(filename, "w");
  DPRINT("Export menu item clicked!\n");

  proc_ctr * proc_wrapper = getprocs(g_proclist_sort_method);
  for (unsigned int i = 0; i < proc_wrapper->proc_count; i++) {
    if (proc_wrapper->procs[i]->pid != -1 && *proc_wrapper->procs[i]->name != '\0') {
      fprintf(logfile, "%d %d %s %c %s\n", proc_wrapper->procs[i]->pid, proc_wrapper->procs[i]->ppid, proc_wrapper->procs[i]->name, proc_wrapper->procs[i]->state, proc_wrapper->procs[i]->euser);
    }
  }
  free_proc_ctr(proc_wrapper);


  gchar * cpuinfo_string = get_cpuinfo_string();
  fprintf(logfile, "%s\n", cpuinfo_string);
  free(cpuinfo_string);

  gchar * mem_string = get_meminfo_string();
  fprintf(logfile, "%s\n", mem_string);
  free(mem_string);

  mem_info * mem = get_mem_info();
  const char * fmt_disk = "Disk: %s\n"
                          "Free: %ld\n"
                          "Total: %ld\n";
  size_t text_len = 0;
  for (int i = 0; i < mem->drive_count; i++) {
    text_len = 0; 
    text_len += snprintf(NULL, 0, "%s", mem->drive_names[i]);
    text_len += snprintf(NULL, 0, "%lld", mem->drive_space[i]);
    text_len += snprintf(NULL, 0, "%lld", mem->drive_size[i]);
    gchar * diskinfo_string = (char *)calloc(strlen(fmt_disk) + text_len + 1, sizeof(char));
    snprintf(diskinfo_string, strlen(fmt_disk) + text_len, fmt_disk, mem->drive_names[i], mem->drive_space[i], mem->drive_size[i]);
    fprintf(logfile, "%s\n", diskinfo_string);
    free(diskinfo_string);
  }
  free_mem_info(mem);

  fclose(logfile);
  free(filename);
}

void refresh_interval_switch_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  DPRINT("Refresh interval changed!\n");
}

void main_window_destroy_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data) {
  DPRINT("Goodbye!\n");
  gtk_main_quit();
}

int regmatch(proc_info * p) {
  int err = regexec(&g_current_search.reg, p->name, 0, NULL, 0);
  if (!err) {
    return 1;
  }
  err = regexec(&g_current_search.reg, p->euser, 0, NULL, 0);
  if (!err) {
    return 1;
  }
  char * pid_s = (char *)calloc(snprintf(NULL, 0, "%d", p->pid) + 1, sizeof(char));
  snprintf(pid_s, snprintf(NULL, 0, "%d", p->pid), "%d", p->pid);
  err = regexec(&g_current_search.reg, pid_s, 0, NULL, 0);
  free(pid_s);
  if (!err) {
    return 1;
  }
  char * ppid_s = (char *)calloc(snprintf(NULL, 0, "%d", p->ppid) + 1, sizeof(char));
  snprintf(ppid_s, snprintf(NULL, 0, "%d", p->ppid), "%d", p->ppid);
  err = regexec(&g_current_search.reg, ppid_s, 0, NULL, 0);
  free(ppid_s);
  if (!err) {
    return 1;
  }
  char * state_s  = (char *)calloc(snprintf(NULL, 0, "%c", p->state) + 1, sizeof(char));
  snprintf(state_s, snprintf(NULL, 0, "%c", p->state), "%c", p->state);
  err = regexec(&g_current_search.reg, state_s, 0, NULL, 0);
  free(state_s);
  if (!err) {
    return 1;
  }
  return 0;
}

static GtkTreeModel * process_list_model(void) {
  GtkListStore * store = gtk_list_store_new(NUM_COLS, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_CHAR, G_TYPE_STRING);
  GtkTreeIter iter = {0};

  /* TODO: Add sort method logic, but for speed this'll do. */
  proc_ctr * proc_wrapper = getprocs(g_proclist_sort_method);
  for (unsigned long i = 0; i < proc_wrapper->proc_count; i++) {
    if (proc_wrapper->procs[i]->pid != -1 && *proc_wrapper->procs[i]->name != '\0') {
      if (!g_current_search.active || (g_current_search.active && regmatch(proc_wrapper->procs[i]))) {

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 
                          COL_PID, proc_wrapper->procs[i]->pid,
                          COL_PPID, proc_wrapper->procs[i]->ppid,
                          COL_NAME, proc_wrapper->procs[i]->name,
                          COL_STATUS, proc_wrapper->procs[i]->state,
                          COL_EUSER, proc_wrapper->procs[i]->euser,
                          -1);
      }
    }
  }
  free_proc_ctr(proc_wrapper);

  return GTK_TREE_MODEL(store);
}

void update_cpu_info_view(GtkBuilder * builder) {
  GtkTextView * text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "cpu_info_box"));
  if (text_view == NULL) {
    DPRINT("[Error]: could not find cpu info widget.\n");
    exit(1);
  }
  GtkTextBuffer * buffer = gtk_text_view_get_buffer(text_view);
  gchar * cpuinfo_string = get_cpuinfo_string();
  gtk_text_buffer_set_text(buffer, cpuinfo_string, -1);
  free(cpuinfo_string);
  /*
  cpu_loads * loads = get_cpu_loads();
  text_len = 0;
  GtkTextIter insertion_iter;
  for (unsigned int i = 0; i < loads->cores; i++) {
    text_len += snprintf(NULL, 0, "CPU Core %d %ld\n", i, loads->loads[i]);
    gchar * coreinfo_string = (char *)calloc(text_len + 1, sizeof(char));
    snprintf(coreinfo_string, text_len, "CPU Core %d %ld\n", i, loads->loads[i]);
    gtk_text_buffer_get_end_iter(buffer, &insertion_iter);
    gtk_text_buffer_insert(buffer, &insertion_iter, coreinfo_string, -1);
  }
  free_cpu_loads(loads);
  */
}

gchar * get_meminfo_string(void) {
  mem_info * mem = get_mem_info();
  size_t text_len = 0;
  text_len += snprintf(NULL, 0, "%ld", mem->ram_total);
  text_len += snprintf(NULL, 0, "%ld", mem->ram_free);
  text_len += snprintf(NULL, 0, "%ld", mem->swap_total);
  text_len += snprintf(NULL, 0, "%ld", mem->swap_free);
  const char * fmt_ram = "Ram Total: %ld\n"
                         "Ram Free: %ld\n"
                         "Swap Total: %ld\n"
                         "Swap Free: %ld\n";
  gchar * meminfo_string = (char *)calloc(strlen(fmt_ram) + text_len + 1, sizeof(char));
  snprintf(meminfo_string, strlen(fmt_ram) + text_len, fmt_ram, mem->ram_total, mem->ram_free, mem->swap_total, mem->swap_free);
  return meminfo_string;
}

void update_mem_info_view(GtkBuilder * builder) {
  GtkTextView * text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "mem_info_box"));
  if (text_view == NULL) {
    DPRINT("[Error]: could not find mem info widget.\n");
    exit(1);
  }
  GtkTextBuffer * buffer = gtk_text_view_get_buffer(text_view);
  gchar * meminfo_string = (gchar *)get_meminfo_string();
  gtk_text_buffer_set_text(buffer, meminfo_string, -1);
  free(meminfo_string);

  const char * fmt_disk = "Disk: %s\n"
                          "Free: %ld\n"
                          "Total: %ld\n";
  mem_info * mem = get_mem_info();
  size_t text_len = 0;
  for (int i = 0; i < mem->drive_count; i++) {
    text_len = 0; 
    text_len += snprintf(NULL, 0, "%s", mem->drive_names[i]);
    text_len += snprintf(NULL, 0, "%lld", mem->drive_space[i]);
    text_len += snprintf(NULL, 0, "%lld", mem->drive_size[i]);
    gchar * diskinfo_string = (char *)calloc(strlen(fmt_disk) + text_len + 1, sizeof(char));
    snprintf(diskinfo_string, strlen(fmt_disk) + text_len, fmt_disk, mem->drive_names[i], mem->drive_space[i], mem->drive_size[i]);
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, diskinfo_string, -1);
    free(diskinfo_string);
  }
  free_mem_info(mem);
}
void cpu_info_view(GtkBuilder * builder) {
  GtkTextView * text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "cpu_info_box"));
  if (text_view == NULL) {
    DPRINT("[Error]: could not find cpu info widget.\n");
    exit(1);
  }
  GtkTextBuffer * buffer = gtk_text_buffer_new(NULL);
  gtk_text_view_set_buffer(text_view, buffer);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
  return;
}

void mem_info_view(GtkBuilder * builder) {
  GtkTextView * text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "cpu_info_box"));
  if (text_view == NULL) {
    DPRINT("[Error]: could not find cpu info widget.\n");
    exit(1);
  }
  GtkTextBuffer * buffer = gtk_text_buffer_new(NULL);
  gtk_text_view_set_buffer(text_view, buffer);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
  return;
}

void sort_proclist_pid(void) {
  g_proclist_sort_method->fld = PID;
  g_proclist_sort_method->ord = (g_proclist_sort_method->ord == ASCENDING) ? DESCENDING : ASCENDING;
}

void sort_proclist_ppid(void) {
  g_proclist_sort_method->fld = PPID;
  g_proclist_sort_method->ord = (g_proclist_sort_method->ord == ASCENDING) ? DESCENDING : ASCENDING;
}

void sort_proclist_name(void) {
  g_proclist_sort_method->fld = NAME;
  g_proclist_sort_method->ord = (g_proclist_sort_method->ord == ASCENDING) ? DESCENDING : ASCENDING;
}

void sort_proclist_state(void) {
  g_proclist_sort_method->fld = STATE;
  g_proclist_sort_method->ord = (g_proclist_sort_method->ord == ASCENDING) ? DESCENDING : ASCENDING;
}

void sort_proclist_euser(void) {
  g_proclist_sort_method->fld = OWNER;
  g_proclist_sort_method->ord = (g_proclist_sort_method->ord == ASCENDING) ? DESCENDING : ASCENDING;
}

void process_list_view(GtkBuilder * builder) {
  GtkWidget * process_list = GTK_WIDGET(gtk_builder_get_object(builder, "process_list_tree_list"));
  if (process_list == NULL) {
    DPRINT("[Error]: could not find process list widget\n");
    exit(1);
  }
  GtkTreeViewColumn * col = gtk_tree_view_column_new();
  GtkCellRenderer * renderer = gtk_cell_renderer_text_new();
  GtkTreeModel * model = process_list_model();
  gtk_tree_view_column_set_title(col, "PID");
  gtk_tree_view_append_column(GTK_TREE_VIEW(process_list), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_set_attributes(col, renderer, "text", COL_PID, NULL);
  gtk_tree_view_column_set_clickable(col, TRUE);
  g_signal_connect(col, "clicked", sort_proclist_pid, col);
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "PPID");
  gtk_tree_view_append_column(GTK_TREE_VIEW(process_list), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_set_attributes(col, renderer, "text", COL_PPID, NULL);
  gtk_tree_view_column_set_clickable(col, TRUE);
  g_signal_connect(col, "clicked", sort_proclist_ppid, col);
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Name");
  gtk_tree_view_append_column(GTK_TREE_VIEW(process_list), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_set_attributes(col, renderer, "text", COL_NAME, NULL);
  gtk_tree_view_column_set_clickable(col, TRUE);
  g_signal_connect(col, "clicked", sort_proclist_name, col);
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Status");
  gtk_tree_view_append_column(GTK_TREE_VIEW(process_list), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_set_attributes(col, renderer, "text", COL_STATUS, NULL);
  gtk_tree_view_column_set_clickable(col, TRUE);
  g_signal_connect(col, "clicked", sort_proclist_state, col);
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Owner");
  gtk_tree_view_append_column(GTK_TREE_VIEW(process_list), col);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(col, renderer, TRUE);
  gtk_tree_view_column_set_attributes(col, renderer, "text", COL_EUSER, NULL);
  gtk_tree_view_column_set_clickable(col, TRUE);
  g_signal_connect(col, "clicked", sort_proclist_euser, col);
  gtk_tree_view_set_model(GTK_TREE_VIEW(process_list), model);
  g_object_unref(model);
  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(process_list)), GTK_SELECTION_SINGLE);
  return;
}

void update_process_list_view(GtkBuilder * builder) {
  GtkWidget * process_list = GTK_WIDGET(gtk_builder_get_object(builder, "process_list_tree_list"));
  if (process_list == NULL) {
    fprintf(stderr, "[Error]: could not find process list widget\n");
    exit(1);
  }
  GtkTreeModel * model = process_list_model();
  gtk_tree_view_set_model(GTK_TREE_VIEW(process_list), model);
  g_object_unref(model);
  return;
}

int timer_callback(gpointer data) {
  /*
     GtkWidget * main_window = (GtkWidget *)data;
     GtkWidget * process_list_tree_list = get_child_by_name(main_window, "process_list_tree_list");
     */
  update_process_list_view(data);
  update_cpu_info_view(data);
  update_mem_info_view(data);
  return 1;
}

int init_app(GtkApplication * app, int argc, char ** argv) {
  gtk_init(&argc, &argv);
  g_proclist_sort_method = (sort_method *)calloc(1, sizeof(sort_method));
  g_proclist_sort_method->fld = NAME;
  g_proclist_sort_method->ord = ASCENDING;
  GtkBuilder * builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "procmon.glade", NULL);
  GtkWidget * window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
  gtk_builder_connect_signals(builder, NULL);
  guint32 initial_timer = 500; /* timer every 500ms */
  gtk_widget_show(window);
  process_list_view(builder);
  cpu_info_view(builder);
  mem_info_view(builder);
  g_current_timeout_tag = g_timeout_add(initial_timer, timer_callback, builder);
  gtk_main();
  free(g_proclist_sort_method);
  return 0;
}

