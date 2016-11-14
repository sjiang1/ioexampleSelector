#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parameter.h"
#include "vector.h"
#include "llist.h"

// functions - value
void value_list_init(struct value_list *p){
  vector_init(&(p->run_names));
  vector_init(&(p->call_ids));
  vector_init(&(p->values));
}

void value_list_inner_free(struct value_list *p){
  vector_inner_free(&(p->run_names));
  vector_inner_free(&(p->call_ids));
  vector_inner_free(&(p->values));
}

int find_matched_value_in_value_list(const char *run_name, int call_id, struct value_list values){
  int matched_index = -1;
  for(int i=0; i<values.call_ids.size; i++){
    int *tmp_call_id = vector_get(&values.call_ids,i);
    char *tmp_run_name = vector_get(&values.run_names, i);
    if(*tmp_call_id == call_id
       && strcmp(tmp_run_name, run_name)==0){
      matched_index = i;
      break;
    }
  }
  return matched_index;
}

int find_matched_value_in_changed_value_list(const char *run_name, int call_id, struct changed_value_list values){
  int matched_index = -1;
  for(int i=0; i<values.call_ids.size; i++){
    int *tmp_call_id = vector_get(&values.call_ids,i);
    char *tmp_run_name = vector_get(&values.run_names, i);
    if(*tmp_call_id == call_id
       && strcmp(tmp_run_name, run_name)==0){
      matched_index = i;
      break;
    }
  }
  return matched_index;
}

int value_list_need_one_value(struct value_list values){
  int call_id_count = values.call_ids.size;
  int value_count = values.values.size;
  return call_id_count == value_count + 1;
}

void printf_value_list(struct value_list values){
  for(int i=0; i<values.values.size;i++){
    char *value = vector_get(&values.values, i);
    int *call_id = vector_get(&values.call_ids,i);
    char *run_name = vector_get(&values.run_names,i);
    printf("\t%s\t%d\t%s\n", run_name, *call_id, value);
  }
}

int value_list_has_null(struct value_list values){
  for(int i=0; i<values.values.size;i++){
    char *value = vector_get(&values.values, i);
    if(strcmp(value, "NULL") == 0
       || strcmp(value, "null") == 0
       || strcmp(value, "0") == 0)
      return 1;
  }
  return 0;
}

void changed_value_list_init(struct changed_value_list *p){
  vector_init(&(p->run_names));
  vector_init(&(p->call_ids));
  vector_init(&(p->before_values));
  vector_init(&(p->after_values));
}

void changed_value_list_inner_free(struct changed_value_list *p){
  vector_inner_free(&(p->run_names));
  vector_inner_free(&(p->call_ids));
  vector_inner_free(&(p->before_values));
  vector_inner_free(&(p->after_values));
}

int changed_value_list_need_one_value_in_after(struct changed_value_list values){
  int call_id_count = values.call_ids.size;
  int value_count = values.after_values.size;
  return call_id_count == value_count + 1;
}

void printf_changed_value_list(struct changed_value_list values){
  for(int i=0; i<values.after_values.size;i++){
    char *before_value = vector_get(&values.before_values, i);
    char *after_value = vector_get(&values.after_values, i);
    int *call_id = vector_get(&values.call_ids,i);
    char *run_name = vector_get(&values.run_names,i);
    printf("\t%s\t%d\t%s\t%s\n", run_name, *call_id, before_value, after_value);
  }
}

int changed_value_list_has_input_null(struct changed_value_list values){
  for(int i=0; i<values.before_values.size;i++){
    char *value = vector_get(&values.before_values, i);
    if(strcmp(value, "NULL") == 0
       || strcmp(value, "null") == 0
       || strcmp(value, "0") == 0)
      return 1;
  }
  return 0;
}

// functions - parameter
void parameter_init(struct parameter_entry *p){
  p->name = 0;
  p->type = 0;

  value_list_init(& (p->input_values));
  value_list_init(& (p->output_values));
  changed_value_list_init(& (p->changed_values));
  
}

void parameter_inner_free(struct parameter_entry *p){
  if(p->name)
    free(p->name);
  if(p->type)
    free(p->type);

  value_list_inner_free(& (p->input_values));
  value_list_inner_free(& (p->output_values));
  changed_value_list_inner_free(& (p->changed_values));

}

char *move_beyond_deref_symbol(char *full_name){
  char *p = full_name;
  int remaining_len = strlen(full_name);
  while(p[0] == '*' && remaining_len){
    p++;
    remaining_len --;
  }
  return p;
}


struct parameter_entry *find_matched_parameter_in_array(const char *name, llist parameter_list){
  struct parameter_entry *matched_parameter = NULL;

  _list_node * node = ( ( _llist * ) parameter_list )->head;

  int index = 0;
  while(node != NULL){
    struct parameter_entry *tmp_parameter = (struct parameter_entry *) node->node;
    char *tmp_name = tmp_parameter->name;
    
    if(strcmp(name, tmp_name) == 0){
      matched_parameter = tmp_parameter;
      break;
    }

    node = node->next;
    index ++;
  }
  return matched_parameter;
}

int parameter_can_be_null(struct parameter_entry p){
  if(value_list_has_null(p.input_values))
    return 1;
  if(changed_value_list_has_input_null(p.changed_values))
    return 1;
  return 0;
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
  char *result; // the return string
  char *ins;    // the next insert point
  char *tmp;    // varies
  int len_rep;  // length of rep (the string to remove)
  int len_with; // length of with (the string to replace rep with)
  int len_front; // distance between rep and end of last rep
  int count;    // number of replacements

  // sanity checks and initialization
  if (!orig && !rep)
    return NULL;
  len_rep = strlen(rep);
  if (len_rep == 0)
    return NULL; // empty rep causes infinite loop during count
  if (!with)
    with = "";
  len_with = strlen(with);

  // count the number of replacements needed
  ins = orig;
  for (count = 0; tmp = strstr(ins, rep); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep; // move to next "end of rep"
  }
  strcpy(tmp, orig);
  return result;
}

char *replace_new_lines(char *value){
  size_t len = strlen(value);
  int newline_count = 0;
  char *new_value = str_replace(value, "\r\n", "[fw-newline]");
  char *new_value1 = str_replace(new_value, "\r", "[fw-newline]");
  free(new_value);
  new_value = str_replace(new_value1, "\n", "[fw-newline]");
  free(new_value1);  
  return new_value;
}

void parameter_printf_value_i(struct parameter_entry* p, FILE* output_file, char *run_name, int call_id){
  int index0 = find_matched_value_in_value_list(run_name, call_id, p->input_values);
  int index2 = find_matched_value_in_changed_value_list(run_name, call_id, p->changed_values);

  char * value = NULL;
  if (index0 > -1){
    value = vector_get(&(p->input_values.values), index0); 
  }
  else if (index2 > -1){
    value = vector_get(&(p->changed_values.before_values), index2);
  }

  
  if(value != NULL){
    char *new_value = replace_new_lines(value);
    if (new_value[0] == '\0'){
      fprintf(output_file, "%s\t%s\n", p->name, "[empty string]");
    }else{
      fprintf(output_file, "%s\t%s\n", p->name, new_value);
    }
    free(new_value);
  }

  return;
}

void parameter_printf_value_o(struct parameter_entry* p, FILE* output_file, char *run_name, int call_id){
  int index1 = find_matched_value_in_value_list(run_name, call_id, p->output_values);
  int index2 = find_matched_value_in_changed_value_list(run_name, call_id, p->changed_values);

  char * value = NULL;
  if (index1 > -1){
    value = vector_get(&(p->output_values.values), index1); 
  }
  else if (index2 > -1){
    value = vector_get(&(p->changed_values.after_values), index2);
  }

  if(value != NULL){
    char *new_value = replace_new_lines(value);
    if (new_value[0] == '\0'){
      fprintf(output_file, "%s\t%s\n", p->name, "[empty string]");
    }else{
      fprintf(output_file, "%s\t%s\n", p->name, new_value);
    }
    free(new_value);
  }

  return;
}

int vector_find_global_call_id(Vector *vector, struct global_call_id target_id){
  int length = vector->size;
  for(int i =0; i<length; i++){
    void *data = vector_get(vector, i);
    struct global_call_id* current_id = (struct global_call_id *)data;
    if(global_call_id_compare(*current_id, target_id) == 0)
      return 1;
  }
  return 0;
}

void collect_global_call_ids(struct value_list values, Vector *global_call_ids){
  char *run_name;
  int *call_id;
  int length=values.run_names.size;
  for(int i=0; i<length; i++){
    void *data=vector_get(&(values.run_names), i);
    run_name = (char *)data;
    data = vector_get(&(values.call_ids), i);
    call_id = (int *)data;
    struct global_call_id current_id;
    current_id.run_name=strcpy_deep(run_name);
    current_id.call_id = *call_id;
    int hasId = vector_find_global_call_id(global_call_ids, current_id);
    if(!hasId){
      struct global_call_id *new_id = malloc(sizeof(struct global_call_id));
      new_id->run_name = current_id.run_name;
      new_id->call_id = current_id.call_id;
      vector_append(global_call_ids, new_id);
    }else{
      free(current_id.run_name);
    }
  }
}

void collect_global_call_ids_from_changed_values(struct changed_value_list values, Vector *global_call_ids){
  char *run_name;
  int *call_id;
  int length=values.run_names.size;
  for(int i=0; i<length; i++){
    void *data=vector_get(&(values.run_names), i);
    run_name = (char *)data;
    data = vector_get(&(values.call_ids), i);
    call_id = (int *)data;
    struct global_call_id current_id;
    current_id.run_name=strcpy_deep(run_name);
    current_id.call_id = *call_id;
    int hasId = vector_find_global_call_id(global_call_ids, current_id);
    if(!hasId){
      struct global_call_id *new_id = malloc(sizeof(struct global_call_id));
      new_id->run_name = current_id.run_name;
      new_id->call_id = current_id.call_id;
      vector_append(global_call_ids, new_id);
    }
    else{
      free(current_id.run_name);
    }
  }
}

void parameter_randomly_pick_a_call(struct parameter_entry* p, char *output_run_name, int *output_call_id){
  Vector global_call_ids;
  vector_init(&global_call_ids);
  collect_global_call_ids(p->input_values, &global_call_ids);
  collect_global_call_ids(p->output_values, &global_call_ids);
  collect_global_call_ids_from_changed_values(p->changed_values, &global_call_ids);

  // srand is in main function
  int length = global_call_ids.size;
  int randomindex = rand() % length;
  struct global_call_id* pickedId = vector_get(&global_call_ids, randomindex);
  strcpy(output_run_name, pickedId->run_name);
  *output_call_id = pickedId->call_id;

  // manually free global_call_ids
  for(int i =0; i<global_call_ids.size; i++){
    void *data=vector_get(&global_call_ids, i);
    struct global_call_id *current_id = (struct global_call_id *)data;
    free(current_id->run_name);
    free(current_id);
  }
  free(global_call_ids.data);
}

int global_call_id_compare(struct global_call_id id1, struct global_call_id id2){
  if(strcmp(id1.run_name, id2.run_name) == 0){
    if(id1.call_id == id2.call_id)
      return 0;
  }
  return -1;
}
