#ifndef UTILS_H
#define UTILS_H
#include "utils.h"
#endif
#include <stdio.h>


void test_proc_list() {
  sort_method * m = (sort_method *)calloc(1, sizeof(sort_method));
  m->fld = NAME;
  m->ord = ASCENDING;

  proc_ctr * proc_counter  = getprocs(m);
  for (unsigned long i = 0; i < proc_counter->proc_count; i++) {
    if (*proc_counter->procs[i]->name != '\0') {
      printf("%d %d %s %c %s\n", proc_counter->procs[i]->pid, proc_counter->procs[i]->ppid, proc_counter->procs[i]->name, proc_counter->procs[i]->state, proc_counter->procs[i]->euser);
    }
  }
  free_proc_ctr(proc_counter);
  free(m);
  m = NULL;
}

void test_cpu_info() {
  cpu_info * cpu = get_cpu_info();
  printf("%d %d %s %d %d %d %d %ld %ld\n", cpu->physical_core_ct, cpu->logical_core_ct, cpu->model_string, cpu->temperature, cpu->l1_cache_size, cpu->l2_cache_size, cpu->clock_speed, cpu->uptime, cpu->proc_count);
  free_cpu_info(cpu);
}

void test_mem_info() {
  mem_info * mem = get_mem_info();
  printf("%ld %ld %ld %ld\n", mem->ram_total, mem->ram_free, mem->swap_total, mem->swap_free);
  for (int i = 0; i < mem->drive_count; i++) {
    printf("%lld %lld %s\n", mem->drive_space[i], mem->drive_size[i], mem->drive_names[i]);
  }
  free_mem_info(mem);
}

void test_runproc() {
  run_proc_by_str("sleep 30");
}

int main() {
  test_proc_list();
  test_cpu_info();
  test_mem_info();
  test_runproc();
}

