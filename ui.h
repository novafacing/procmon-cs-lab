#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#define DEBUG 0
#define DEFAULT_WIDTH (400)
#define DEFAULT_HEIGHT (400)

enum {
	COL_PID,
	COL_PPID,
	COL_NAME,
	COL_STATUS,
	COL_EUSER,
	NUM_COLS
};

enum {
	SORTID_PID,
	SORTID_PPID,
	SORTID_NAME,
	SORTID_STATUS,
	SORTID_EUSER,
	NUM_SORTIDS
};

enum {
	LIST,
	TREE
};

typedef struct search_reg {
	regex_t reg;
	bool active;
} search_reg;

typedef void(*cb_fptr)(GSimpleAction * simple, GVariant * parameter, gpointer user_data);

/* Function Declarations */
int init_app(GtkApplication * app, int argc, char ** argv);

/* Callback Action Functions */
gchar * get_meminfo_string(void);
void run_callback(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void export_callback(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void quit_callback(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void refresh_callback_hs(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void refresh_callback_1s(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void refresh_callback_5s(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void refresh_callback_10s(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void procs_page_update_callback(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void cpu_page_update_callback(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void ram_page_update_callback(GSimpleAction * simple, GVariant * parameter, gpointer user_data);

/* GLADE UI Callbacks */
void run_process_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void kill_process_button_clicked_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void list_tree_toggle_button_clicked_cb(GSimpleAction * simple, GVariant * paremeter, gpointer user_data);
void run_menuitem_activate_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void export_menuitem_activate_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void refresh_interval_switch_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
void process_search_box_activate_cb(GtkSearchEntry * search, GVariant * parameter, gpointer user_data);

void main_window_destroy_cb(GSimpleAction * simple, GVariant * parameter, gpointer user_data);
