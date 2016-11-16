#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <debug_printf.h>
#include <time.h>
#include <unistd.h>

// third party library
#include "csv.h"

// headers for our own code
#include "util.h"

#include "vector.h"
#include "parameter.h"
#include "globals.h"


// String utility functions - definition
int ends_with(const char *str, const char *suffix)
{
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix >  lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int starts_with(const char *str, const char *prefix)
{
  if (!str || !prefix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lenprefix = strlen(prefix);
  if (lenprefix >  lenstr)
    return 0;
  return strncmp(str, prefix, lenprefix) == 0;
}

char *strcpy_deep(const char *str){
  size_t str_len = strlen(str) + 1;
  char *ret = malloc(sizeof(char)*str_len);
  strcpy(ret, str);
  return ret;
}

//

void rm_if_file_exists(const char *path){
  if( access( path, F_OK ) != -1 ) {
    remove(path);
  }
}

int is_csv_file(const char *path)
{
  struct stat path_stat;
  stat(path, &path_stat);

  /* debugging use: to print the type of the input path
  switch (path_stat.st_mode & S_IFMT) {
  case S_IFBLK:  printf("block device\n");            break;
  case S_IFCHR:  printf("character device\n");        break;
  case S_IFDIR:  printf("directory\n");               break;
  case S_IFIFO:  printf("FIFO/pipe\n");               break;
  case S_IFLNK:  printf("symlink\n");                 break;
  case S_IFSOCK: printf("socket\n");                  break;
  case S_IFREG:  printf("regular file\n");            break;
  default:       printf("unknown?\n");                break;
  }*/

  switch (path_stat.st_mode & S_IFMT) {
  case S_IFBLK:  
  case S_IFCHR:  
  case S_IFDIR:  
  case S_IFIFO:  
  case S_IFLNK:  
  case S_IFSOCK:
    return 0;
  case S_IFREG:
  default:
    if(ends_with(path, ".csv"))
      return 1;
    else
      return 0;
  }
}

int is_subfolder(const char *path){
  struct stat path_stat;
  stat(path, &path_stat);    
  if (S_ISDIR(path_stat.st_mode)){
    return 1;
  }
  return 0;
}

int is_headline(const char *line){
  int starts_with_ret = starts_with(line, "Function,") ||  starts_with(line, "Is Return Flag,");
  return starts_with_ret;
}

DIR *open_input_directory(const char *dir){
  // check whether the input directory exists
  struct stat st = {0};  
  if(stat(dir, &st) == -1){
    fprintf(stderr, "Cannot find the input directory: %s \n", dir);
    printf("%s", help);
    exit(EXIT_FAILURE);
  }
  
  // open the input directory
  DIR *d = opendir(dir);
  if (!d){
    fprintf(stderr, "Cannot open the input directory: %s \n", dir);
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    printf("%s", help);
    exit(EXIT_FAILURE);
  }
  return d;
}

int get_function_name_and_real_path(struct dirent *dir_ent, const char* parent_dir,
				char *output_function_name, char *output_real_path){
  if(strcmp(dir_ent->d_name, ".")== 0 || strcmp(dir_ent->d_name, "..")== 0)
     return 0;

  // Get the function name
  strcpy(output_function_name, dir_ent->d_name);
  
  // Get the file name and real path of the file
  char output_file_name[PATH_MAX+1];
  strcpy(output_file_name, parent_dir);
  if(!ends_with(parent_dir, "/"))
    strcat(output_file_name, "/");
  strcat(output_file_name, dir_ent->d_name);
  char *realpath_ret = realpath(output_file_name, output_real_path);
  if(realpath_ret == NULL){
    fprintf(stderr, "Cannot get the real path of: %s \n", output_file_name);
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    return 0; 
  }
  return 1;
}

int get_sub_name_and_real_path(struct dirent *dir_ent, const char* parent_dir,
				char *output_real_path){
  char file_name[PATH_MAX + 1];
  if(strcmp(dir_ent->d_name, ".")== 0 || strcmp(dir_ent->d_name, "..")== 0)
     return 0;
  
  // Get the file name and real path of the file
  strcpy(file_name, parent_dir);
  if(!ends_with(parent_dir, "/"))
    strcat(file_name, "/");
  strcat(file_name, dir_ent->d_name);
  char *realpath_ret = realpath(file_name, output_real_path);
  if(realpath_ret == NULL){
    fprintf(stderr, "Cannot get the real path of: %s \n", file_name);
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    return 0; 
  }
  return 1;
}

int try_open_file_and_get_first_line(const char *file_name, char **line_p, int *read_char_number, FILE **fpp){
  // try to open the file
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  *read_char_number = 0;
  
  fp = fopen(file_name, "r");
  if (fp == NULL){
    fprintf(stderr, "Cannot open the file: [%s] \n", file_name);
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    return -1;
  }
  
  // read the first line, check if there is a line
  if((*read_char_number = getline(&line, &len, fp)) == -1){
    fprintf(stderr, "No line in file: [%s]\n", file_name);
    if(line) free(line); // free the buffer that getline creates
    fclose(fp); // close the file
    return -1;
  }
  
  // if there is a line, check if the first line is a headline
  int is_headline_ret = is_headline(line);
  if(is_headline_ret == 1){
    // if this is a head line, then we skip this line, and read the second line
    if((*read_char_number = getline(&line, &len, fp)) == -1){
      fprintf(stderr, "No line except a headline in file: [%s]\n", file_name);
      if(line) free(line); // free the buffer that getline creates
      fclose(fp); // close the file
      return -1;
    }
  }
  
  *fpp = fp;
  *line_p = line;
  return len;
}

void printf_parameter_ids(llist parameter_list, FILE *output_file, int current_function_id, const char *current_function_name){
  // print the parameters
  printf("parameter list: %d\n", llist_size(parameter_list));

  int current_parameter_id = 0;
  _list_node * node = ( ( _llist * ) parameter_list )->head;
  while(node != NULL){
    
    struct parameter_entry* p = (struct parameter_entry *) node->node;
    if(parameter_can_be_null(*p))
      fprintf(output_file, "%d\t%d\t%s\t%s\t%s\t\%s\n", current_function_id, current_parameter_id, current_function_name, p->type, p->name, "can be null");
    else
      fprintf(output_file, "%d\t%d\t%s\t%s\t%s\t\%s\n", current_function_id, current_parameter_id, current_function_name, p->type, p->name, "do not know");

    current_parameter_id ++;
    node = node->next;
  }

  printf("finished printf parameter ids.\n");
}

void printf_io_examples(llist parameter_list, char *outputname_prefix/*, Vector *calls*/){
  if(llist_size(parameter_list) < 1) return;
  
  char output_parameter_i[PATH_MAX+1];
  char output_parameter_o[PATH_MAX+1];
  char output_return[PATH_MAX+1];

  strcpy(output_parameter_i, outputname_prefix);
  strcpy(output_parameter_o, outputname_prefix);
  strcpy(output_return, outputname_prefix);
  
  strcat(output_parameter_i, ".parameter.example.i");
  strcat(output_parameter_o, ".parameter.example.o");
  strcat(output_return, ".return.example");
  
  FILE *output_file_i = fopen(output_parameter_i, "w");
  FILE *output_file_o = fopen(output_parameter_o, "w");
  FILE *output_file_ret = fopen(output_return, "w");
  if (output_file_i == NULL || output_file_o == NULL || output_file_ret == NULL) {
    printf("Error opening one of the files: %s, %s, %s\n", output_parameter_i, output_parameter_o, output_return);
    exit(1);
  }

  char run_name[PATH_MAX+1] = "";
  int call_id = -1;

  _list_node * head =  ( ( _llist * ) parameter_list )->head;
  struct parameter_entry* p = (struct parameter_entry*) head->node;

  parameter_randomly_pick_a_call(p, run_name, &call_id);
  // randomly_pick_a_call(calls, &call_id); // to be debugged

  FILE *output_picked_fp = NULL;
  output_picked_fp = fopen (pickedIOids_outputfile, "a" );
  fprintf(output_picked_fp, "%s\t%d\n", run_name, call_id);
  fclose(output_picked_fp);

  _list_node * node = head;
  int current_parameter_id = 0;
  while(node != NULL){
    
    p = (struct parameter_entry *) node->node;

    char *tmp = move_beyond_deref_symbol(p->name);
    char *trim_param_name = strcpy_deep(p);
    char *dotPos = strchr(trim_param_name, '.');
    char *dashPos = strchr(trim_param_name, '-');
    
    int isReturnValue = 0;
    if(!dotPos && !dashPos && strcmp(trim_param_name,return_name)==0){
      isReturnValue = 1;
    }
    else if(dotPos || dashPos){
      if(dotPos)
	*dotPos = '\0';
      if(dashPos)
	*dashPos = '\0';

      if(strcmp(trim_param_name, return_name) == 0)
	isReturnValue = 1;
    }
    free(trim_param_name);

    if(isReturnValue){
      parameter_printf_value_o(p, output_file_ret, run_name, call_id);
    }
    else{
      parameter_printf_value_i(p, output_file_i, run_name, call_id);
      parameter_printf_value_o(p, output_file_o, run_name, call_id);
    }
    
    node = node->next;
    current_parameter_id ++;
  }
  fclose(output_file_i);
  fclose(output_file_o);
  fclose(output_file_ret);
  
  return;
}

void copy_head_from_file(const char *source_file, const char*dest_file, int end_line_number){
  printf("copy to line: %d\n", end_line_number);
  FILE *source_fp = NULL;
  char *line = NULL;
  int read_char_number = 0;
  int ret = try_open_file_and_get_first_line(source_file, &line, &read_char_number, &source_fp);
  if(ret == -1) return -1;

  FILE *dest_fp = NULL;
  dest_fp = fopen (dest_file, "w" );
  
  size_t len = strlen(line);
  int line_number = 0;
  do{
    if(len < 3) continue;
    char *new_line = malloc(sizeof(char) * (len + 2));
    strcpy(new_line, line);
    make_sure_new_line_ends(new_line);
    if(line_number > end_line_number)
      break;
    fprintf(dest_fp, "%s", new_line);
    free(new_line);
    line_number ++;
  } // end loop: read the file line by line
  while ((read_char_number = getline(&line, &len, source_fp)) != -1);
  if (line) // free the buffer that getline creates
    free(line);
  fclose(source_fp); // close the file
  fclose(dest_fp); // close the file
  return;
}

void make_sure_new_line_ends(char *line){
  // input line should have three more bytes than len
  size_t len = strlen(line);
  
  if(line[len] != '\0'){
    debug_printf("Error: %s\n", "the end of string should be a \\0");
  }else{
    if(line[len -1] != '\n'){
      line[len] = '\n';
      line[len + 1] = '\0';
    }
  }
}
