typedef struct
{
  char * alias;
  char * description;
} db_ref_t;

typedef struct
{
  char      * line;
  char      * db_ref;
  db_ref_t ** updated_refs;
  char      * sequence;
  long      hits;
  long      updated_hits;
  char      * mtype;
  char      * dbname;
} interact_data_line_t;


typedef struct
{
  char * file_prefix;
  interact_data_line_t ** data;
  char * file_suffix;
} interact_file_t;

