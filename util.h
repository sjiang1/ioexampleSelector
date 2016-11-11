#ifndef _UTIL_H
#define _UTIL_H

#include "vector.h"
#include "llist.h"

// String utility functions - declaration
int ends_with(const char *str, const char *suffix);
int starts_with(const char *str, const char *prefix);
char *strcpy_deep(const char *str);

// Utility functions
void rm_if_file_exists(const char *path);
int is_csv_file(const char *path);
int is_subfolder(const char *path);
int is_headline(const char *line);
DIR *open_input_directory(const char*dir);
int get_sub_name_and_real_path(struct dirent *dir_ent, const char* parent_dir, char *output_sub_real_path);
int get_function_name_and_real_path(struct dirent *dir_ent, const char* parent_dir, char *output_function_name, char *output_real_path);
int try_open_file_and_get_first_line(const char *file_name, char **line_p, int *read_char_number,FILE **fpp);
void copy_head_from_file(const char *source_file, const char*dest_file, int end_line_number);
void make_sure_new_line_ends(char *line);
// Utility functions - output results
void printf_parameter_ids(llist parameter_arr, FILE* output_file,int current_function_id, const char *current_function_name);
void printf_io_examples(llist parameter_arr, char *outputfile_prefix);

#endif
