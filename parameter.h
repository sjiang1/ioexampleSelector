#ifndef _PARAMETER_H
#define _PARAMETER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "llist.h"

// Data structure - global call id
struct global_call_id{
  char *run_name;
  int call_id;
};

int global_call_id_compare(struct global_call_id id1, struct global_call_id id2);

// Data structure - value
struct value_list{
  // run_names: this is to identifiy the records with same call ids but in different program runs.
  Vector run_names; // Dynamic array of string 
  Vector call_ids; // Dynamic array of int
  Vector values; // Dynamic array of string
};

void value_list_init(struct value_list *p);
int find_matched_value_in_value_list(const char *run_name, int call_id, struct value_list values);
int value_list_need_one_value(struct value_list values);
void printf_value_list(struct value_list values);
int value_list_has_null(struct value_list values);

struct changed_value_list{
  // run_names: this is to identifiy the records with same call ids but in different program runs.
  Vector run_names; // Dynamic array of string 
  Vector call_ids; // Dynamic array of int
  Vector before_values; // Dynamic array of string
  Vector after_values; // Dynamic array of string
};

void changed_value_list_init(struct changed_value_list *p);
int find_matched_value_in_changed_value_list(const char *run_name, int call_id, struct changed_value_list values);
int changed_value_list_need_one_value_in_after(struct changed_value_list values);
void printf_changed_value_list(struct changed_value_list values);
int changed_value_list_has_input_null(struct changed_value_list values);

// Data structure - parameter
struct parameter_entry{
  char *name;
  char *type;
  struct value_list input_values;
  struct value_list output_values;
  struct changed_value_list changed_values;
};

void parameter_init(struct parameter_entry* p);
void parameter_inner_free(struct parameter_entry* p);
struct parameter_entry *find_matched_parameter_in_array(const char *name, llist parameter_arr);
int parameter_can_be_null(struct parameter_entry p);
void parameter_printf_value_i(struct parameter_entry* p, FILE* output_file, char *run_name, int call_id);
void parameter_printf_value_o(struct parameter_entry* p, FILE* output_file, char *run_name, int call_id);
void parameter_randomly_pick_a_call(struct parameter_entry* p, char *output_run_name, int *output_call_id);
#endif 

// Manipulating parameters names
char *move_beyond_deref_symbol(char *full_name);
int parameter_name_is_under(const char *name, const char *tmp_name);
