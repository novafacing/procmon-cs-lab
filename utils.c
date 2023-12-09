#ifndef UTILS_H
#define UTILS_H
#include "utils.h"
#endif

mem_info * get_mem_info(void) {
  mem_info * mem = (mem_info *)calloc(1, sizeof(mem_info));
  struct sysinfo info = {0};
  int err = sysinfo(&info);
  if (err != 0) {
    DPRINT("[Error]: could not retrieve system info.\n");
    return 0;
  }
  mem->ram_total = info.totalram;
  mem->ram_free = info.freeram;
  mem->swap_total = info.totalswap;
  mem->swap_free = info.freeswap;

  struct mntent * m;
  FILE * mnt_entry = setmntent(_PATH_MOUNTED, "r");
  while ((m = getmntent(mnt_entry))) {
    if (*m->mnt_fsname == '/') {
      mem->drive_count++;
    }
  }
  mem->drive_names = (char **)calloc(mem->drive_count, sizeof(char *));
  mem->drive_space = (unsigned long long *)calloc(mem->drive_count, sizeof(unsigned long long));
  mem->drive_size = (unsigned long long *)calloc(mem->drive_count, sizeof(unsigned long long));
  if (mem->drive_names == NULL) {
    DPRINT("[Error]: no mounted drives or unable to count drives.\n");
    mem->drive_space = 0;
    return mem;
  }
  endmntent(mnt_entry);
  mnt_entry = setmntent(_PATH_MOUNTED, "r");
  int drive_ctr = 0;
  while ((m = getmntent(mnt_entry))) {
    if (*m->mnt_fsname == '/') {
      mem->drive_names[drive_ctr] = (char *)strdup(m->mnt_fsname);
      struct statvfs vfs;
      int err = statvfs(m->mnt_dir, &vfs);
      if (err != 0) {
        DPRINT("[Error]: unable to stat drive %s - %s.\n", m->mnt_dir, strerror(errno));
        continue;
      }
      mem->drive_size[drive_ctr] = vfs.f_bsize * vfs.f_blocks;
      mem->drive_space[drive_ctr] = vfs.f_frsize * vfs.f_bfree;
      drive_ctr++;
    }
  }
  endmntent(mnt_entry);
  return mem;
}

void free_mem_info(mem_info * mem) {
  free(mem->drive_space);
  mem->drive_space = NULL;
  free(mem->drive_size);
  mem->drive_size = NULL;
  for (int i = 0; i < mem->drive_count; i++) {
    free(mem->drive_names[i]);
    mem->drive_names[i] = NULL;
  }
  free(mem->drive_names);
  mem->drive_names = NULL;
  free(mem);
  mem = NULL;
}
unsigned long get_cpu_uptime(void) {
  struct sysinfo info;
  int err = sysinfo(&info);
  if (err != 0) {
    DPRINT("[Error]: could not retrieve system info.\n");
    return 0;
  }
  return info.uptime;
}

unsigned long get_proc_ct(void) {
  struct sysinfo info;
  int err = sysinfo(&info);
  if (err != 0) {
    DPRINT("[Error]: could not retrieve system info.\n");
    return 0;
  }
  return info.procs;
}

cpu_loads * get_cpu_loads(void) {
  cpu_info * cpu = get_cpu_info();
  cpu_loads * cpu_util = (cpu_loads *)calloc(1, sizeof(cpu_loads));
  cpu_util->cores = cpu->physical_core_ct;
  free_cpu_info(cpu);
  cpu_util->loads = (long int *)calloc(cpu_util->cores, sizeof(long int));
  FILE * proc_stat = fopen("/proc/stat", "r");
  if (proc_stat == NULL) {
    DPRINT("[Error]: could not open proc/stat.\n");
    exit(1);
  }

  int err = fscanf(proc_stat, "%*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d\n");
  if (err == EOF) {
    DPRINT("[Error]: error reading /proc/stat.\n");
  }
  cpu_times * prior = (cpu_times *)calloc(cpu_util->cores, sizeof(cpu_times));
  for (unsigned int i = 0; i < cpu_util->cores; i++) {
    err = fscanf(proc_stat, "%*s %ld %ld %ld %ld %ld %ld %ld %ld %*d %*d\n", &(prior[i].user), &(prior[i].nice), &(prior[i].system), &(prior[i].idle), &(prior[i].iowait), &(prior[i].irq), &(prior[i].softirq), &(prior[i].steal));
    if (err != 8) {
      DPRINT("[Error]: could not read from proc/stat");
      exit(1);
    }
  }
  fclose(proc_stat);
  proc_stat = fopen("/proc/stat", "r");
  if (proc_stat == NULL) {
    DPRINT("[Error]: could not open proc/stat.\n");
    exit(1);
  }
  cpu_times * anterior = (cpu_times *)calloc(cpu_util->cores, sizeof(cpu_times));
  err = fscanf(proc_stat, "%*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d\n");
  if (err == EOF) {
    DPRINT("[Error]: error reading /proc/stat.\n");
  }

  for (unsigned int i = 0; i < cpu_util->cores; i++) {
    fscanf(proc_stat, "%*s %ld %ld %ld %ld %ld %ld %ld %ld %*d %*d\n", &(anterior[i].user), &(anterior[i].nice), &(anterior[i].system), &(anterior[i].idle), &(anterior[i].iowait), &(anterior[i].irq), &(anterior[i].softirq), &(anterior[i].steal));
    if (err != 8) {
      DPRINT("[Error]: could not read from proc/stat");
      exit(1);
    }
  }
  fclose(proc_stat);
  
  for (unsigned int i = 0; i < cpu_util->cores; i++) {
    long int idle_p = prior[i].idle + prior[i].iowait;
    long int idle = anterior[i].idle + anterior[i].iowait;
    long int nonidle_p = prior[i].user + prior[i].nice + prior[i].system + prior[i].irq + prior[i].softirq + prior[i].steal;
    long int nonidle = anterior[i].user + anterior[i].nice + anterior[i].system + anterior[i].irq + anterior[i].softirq + anterior[i].steal;
    long int total_p = idle_p + nonidle_p;
    long int total = idle + nonidle;

    long int total_d = total - total_p;
    long int idle_d = idle - idle_p;
    cpu_util->loads[i] = (total_d - idle_d) / total_d;
  }
  free(prior);
  free(anterior);
  return cpu_util;
}

void free_cpu_loads(cpu_loads * c) {
  free(c->loads);
  free(c);
}

cpu_info * get_cpu_info(void) {
  if (!cpuid_present()) {
    DPRINT("[Error]: cpuid instruction not present.\n");
    return NULL;
  }

  struct cpu_raw_data_t raw;
  struct cpu_id_t data;

  if (cpuid_get_raw_data(&raw) < 0) {
    DPRINT("[Error]: could not retrieve raw CPU info - %s.\n", cpuid_error());
    return NULL;
  }

  if (cpu_identify(&raw, &data) < 0) {
    DPRINT("[Error]: could not retrieve CPU identification info - %s.\n", cpuid_error());
    return NULL;
  }

  cpu_info * cpu = (cpu_info *)calloc(1, sizeof(cpu_info));
  cpu->physical_core_ct = data.num_cores;
  cpu->logical_core_ct = data.num_logical_cpus;
  cpu->model_string = strdup(data.cpu_codename);

DPRINT("[Error]: could not retrieve CPU temperature info - %s.\n", cpuid_error());
DPRINT("[Info]: setting CPU temperature to 1.\n");
cpu->temperature = 1;
;
  cpu->l1_cache_size = data.l1_data_cache;
  cpu->l2_cache_size = data.l2_cache;
  cpu->clock_speed = cpu_clock_by_os();
  cpu->uptime = get_cpu_uptime();
  cpu->proc_count = get_proc_ct();
  return cpu;
}

void free_cpu_info(cpu_info * cpu) {
  free(cpu->model_string);
  cpu->model_string = NULL;
  free(cpu);
  cpu = NULL;
}

int procsort_comparator(const void * p1, const void * p2, void * method) {
  proc_info * a = *(proc_info **)p1;
  proc_info * b = *(proc_info **)p2;
  if ((*(sort_method *)method).ord == ASCENDING) {
    switch((*(sort_method *)method).fld) {
      case PID:   return a->pid - b->pid;
      case PPID:  return a->ppid - b->ppid;
      case NAME:  return strcmp(basename(a->name), basename(b->name));
      case STATE: return a->state - b->state;
      case OWNER: return strcmp(a->euser, b->euser);
      default:    DPRINT("[Error]: sort method field invalid: %d.\n", (*(sort_method *)method).fld);
                  exit(1);
    }
  } else if ((*(sort_method *)method).ord == DESCENDING) {
    switch((*(sort_method *)method).fld) {
      case PID:   return b->pid - a->pid;
      case PPID:  return b->ppid - a->ppid;
      case NAME:  return strcmp(basename(b->name), basename(a->name));
      case STATE: return b->state - a->state;
      case OWNER: return strcmp(b->euser, a->euser);
      default:    DPRINT("[Error]: sort method field invalid: %d.\n", (*(sort_method *)method).fld);
                  exit(1);
    }
  } else {
    DPRINT("[Error]: sort method order invalid: %d.\n", (*(sort_method *)method).ord);
    exit(1);
  }
}

int isdigits(char * c) {
  while (*c) {
    if (!isdigit(*c++)) {
      return 0;
    }
  }
  return 1;
}

pid_t * getpids(void) {
  size_t pids_max = 128;
  size_t pids_sz = 0;
  pid_t * pids = (pid_t *)calloc(pids_max, sizeof(pid_t));
  DIR * proc_dir = opendir("/proc");
  struct dirent * proc = NULL;
  while ((proc = readdir(proc_dir)) != NULL) {
    if (isdigits(proc->d_name)) {
      pids[pids_sz++] = atoi(proc->d_name);
    }
    if (pids_sz > ((pids_max * 3) / 4)) {
      pids_max *= 2;
      pid_t * _pids = (pid_t *)calloc(pids_max, sizeof(pid_t));
      for (unsigned long i = 0; i < pids_sz; i++) {
        _pids[i] = pids[i];
      }
      free(pids);
      pids = _pids;
    }
  }
  pids[pids_sz] = -1;
  closedir(proc_dir);
  return pids;
}

char * makeprocfpath(const char * base, pid_t pid, const char * filename) {
  size_t base_len = strlen(base);
  size_t pid_len = snprintf(NULL, 0, "%d", pid);
  size_t filename_len = strlen(filename);
  char * procpath = (char *)calloc(base_len + pid_len + filename_len + 1, sizeof(char));
  sprintf(procpath, "%s", base);
  sprintf(&procpath[base_len], "%d", pid);
  sprintf(&procpath[base_len + pid_len], "%s", filename);
  procpath[base_len + pid_len + filename_len] = '\0';
  return procpath;
}

char * makeprocdpath(const char * base, pid_t pid) {
  size_t base_len = strlen(base);
  size_t pid_len = snprintf(NULL, 0, "%d", pid);
  char * procpath = (char *)calloc(base_len + pid_len + 1, sizeof(char));
  sprintf(procpath, "%s", base);
  sprintf(&procpath[base_len], "%d", pid);
  procpath[base_len + pid_len] = '\0';
  return procpath;
}

void free_procinfo(proc_info * p) {
  if (p->pid > 0 || p->pid < -4) {
    free(p->name);
    p->name = NULL;
  }
  if (p->pid > 0 || p->pid < -5) {
    free(p->euser);
    p->euser = NULL;
  }
  free(p);
}

void free_procinfo_arr(proc_info ** p) {
  while (*p != NULL) {
    free_procinfo(*p);
    p++;
  }
}

void free_proc_ctr(proc_ctr * p) {
  free_procinfo_arr(p->procs);
  free(p->procs);
  free(p);
}

void free_chararr(char ** arr) {
  while (*arr) {
    free(*arr);
    arr++;
  }
}

proc_ctr * getprocs(sort_method * method) {
  pid_t * proc_pids = getpids();
  pid_t * _proc_pids = proc_pids;
  /* Get process count */
  size_t proc_count = 0;
  while (*proc_pids != -1) {
    proc_count++;
    proc_pids++;
  }
  proc_pids = _proc_pids;
  proc_info ** procs = (proc_info **)calloc(proc_count + 1, sizeof(proc_info *));
  procs[proc_count] = NULL;
  for (unsigned long i = 0; i < proc_count; i++) {
    /* allocate a proc_info struct */
    procs[i] = (proc_info *)calloc(1, sizeof(proc_info));
    /* Create filenames we'll need */
    /* /proc/[pid]/stat - PID, PPID, STATE
     * /proc/[pid]/ - EUSER
     * /proc/[pid]/cmdline - NAME
     */
    char * statpath = makeprocfpath("/proc/", proc_pids[i], "/stat");
    FILE * statfile = fopen(statpath, "r");
    if (statfile == NULL) {
      DPRINT("[Error]: statfile failed to open.\n");
      procs[i]->pid = -1;
      free(statpath);
      continue;
    }
    int err = fscanf(statfile, "%d %*s %c %d", &procs[i]->pid, &procs[i]->state, &procs[i]->ppid);
    if (err != 3) {
      DPRINT("[Error]: fscanf read invalid.\n");
      procs[i]->pid = -2;
      free(statpath);
      continue;
    }
    fclose(statfile);
    free(statpath);

    char * pidirpath = makeprocdpath("/proc/", proc_pids[i]);
    struct stat pidirinfo;
    err = stat(pidirpath, &pidirinfo);
    if (err == -1) {
      DPRINT("[Error]: stat failed.\n");
      procs[i]->pid = -3;
      free(pidirpath);
      continue;
    }
    struct passwd * pw = getpwuid(pidirinfo.st_uid);
    procs[i]->euser = (char *)strdup(pw->pw_name);
    free(pidirpath);

    char * cmdlinepath = makeprocfpath("/proc/", proc_pids[i], "/cmdline");
    FILE * cmdlinefile = fopen(cmdlinepath, "r");
    if (cmdlinefile == NULL) {
      DPRINT("[Error]: cmdlinefile failed to open.\n");
      procs[i]->pid = -4;
      free(cmdlinepath);
      continue;
    }
    procs[i]->name = (char *)calloc(PATH_MAX, sizeof(char));
    err = fscanf(cmdlinefile, "%s", procs[i]->name);
    if (err != 1) {
      DPRINT("[Error]: could not scan cmdlinefile.\n");
      procs[i]->pid = -5;
      fclose(cmdlinefile);
      free(cmdlinepath);
      continue;
    }
    fclose(cmdlinefile);
    free(cmdlinepath);
  }
  qsort_r(procs, proc_count, sizeof(proc_info *), procsort_comparator, method);
  proc_ctr * proc_info_counter = (proc_ctr *)calloc(1, sizeof(proc_ctr));
  proc_info_counter->proc_count = proc_count;
  proc_info_counter->procs = procs;
  free(_proc_pids);
  return proc_info_counter;
}

bool kill_proc_by_pid(pid_t pid) {
  int err = kill(pid, SIGABRT);
  if (err < -1) {
    DPRINT("[Error]: SIGABRT error for pid %d - %s\n", pid, strerror(errno));
    return false;
  }
  return true;
}

size_t count_occur(const char * str, const char occur) {
  size_t count = 0;
  while (*str) {
    if (*str == occur) {
      count++;
    }
    str++;
  }
  return occur;
}

char ** split_delimiter(const char * str, const char delimiter) {
  char ** split = (char **)calloc(count_occur(str, delimiter) + 2, sizeof(char *));
  char ** _split = split;
  char * token = NULL;
  char * strc = (char *)strdup(str);
  while ((token = strsep(&strc, &delimiter))) {
    *split++ = strdup(token);
  }
  *split = NULL;
  return _split;
}

bool run_proc_by_str(char * cmd) {
  char ** args = split_delimiter(cmd, ' ');
  int pid = fork();
  if (pid == 0) {
    /* child, do execv */
    execvp(args[0], args);
  } else {
    /* parent, free the arglist */
    free_chararr(args);
    free(args);
  }
  return true;
}

bool stop_proc_by_pid(pid_t pid) {
  int err = kill(pid, SIGSTOP);
  if (err < -1) {
    DPRINT("[Error]: SIGSTOP error for pid %d - %s\n", pid, strerror(errno));
    return false;
  }
  return true;
}

bool cont_proc_by_pid(pid_t pid) {
  int err = kill(pid, SIGCONT);
  if (err < -1) {
    DPRINT("[Error]: SIGCONT error for pid %d - %s\n", pid, strerror(errno));
    return false;
  }
  return true;
}
