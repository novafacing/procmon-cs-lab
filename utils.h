#include <sys/sysinfo.h>
#include <ctype.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/sysinfo.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include <dirent.h>

#include <pwd.h>
#include <limits.h>
#include <libcpuid/libcpuid.h>
#include <bsd/string.h>

#define PATH_MAX 4096
#ifdef DEBUG
#define DPRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define DPRINT(...) do { } while ( false )
#endif

enum field {
  PID,
  PPID,
  NAME,
  STATE,
  OWNER
};

enum order {
  ASCENDING,
  DESCENDING
};

typedef struct sort_method {
        enum field fld;
        enum order ord;
} sort_method;

typedef struct cpu_info {
        int physical_core_ct;
        int logical_core_ct;
        char * model_string;
        int temperature;
        int l1_cache_size;
        int l2_cache_size;
        int clock_speed;
        unsigned long uptime;
        unsigned long proc_count;
} cpu_info;

typedef struct mem_info {
        unsigned long ram_total;
        unsigned long ram_free;
        unsigned long swap_total;
        unsigned long swap_free;
        unsigned long long * drive_space;
        unsigned long long * drive_size;
        char ** drive_names;
        int drive_count;
} mem_info;

typedef struct proc_info {
        pid_t pid;
        pid_t ppid;
        char * name;
        char state;
        char * euser;
} proc_info;

typedef struct proc_ctr {
        size_t proc_count;
        proc_info ** procs;
} proc_ctr;

typedef struct cpu_loads {
        size_t cores;
        long int * loads;
} cpu_loads;

typedef struct cpu_times {
    long int user;
    long int nice;
    long int system;
    long int idle;
    long int iowait;
    long int irq;
    long int softirq;
    long int steal;
} cpu_times;



/* Function Declarations */
cpu_loads * get_cpu_loads();
void free_cpu_loads(cpu_loads * c);
bool run_proc_by_str(char * cmd);
void free_procinfo_arr(proc_info ** procs);
void free_procinfo(proc_info * p);
void free_proc_ctr(proc_ctr * p);
proc_ctr * getprocs(sort_method * m);
pid_t * getpids(void);
mem_info * get_mem_info(void);
void free_mem_info(mem_info * mem);
cpu_info * get_cpu_info(void);
void free_cpu_info(cpu_info * mem);
unsigned long get_cpu_uptime(void);
unsigned long get_proc_ct(void);
bool kill_proc_by_pid(pid_t pid);
bool stop_proc_by_pid(pid_t pid); 
bool cont_proc_by_pid(pid_t pid);
bool new_proc_by_str(const char * command);
int procsort_comparator(const void * p1, const void * p2, void * method) __attribute__((unused));

