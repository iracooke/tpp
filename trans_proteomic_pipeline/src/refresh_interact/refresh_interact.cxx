/*---------------------------------------------------------------------------##
##  File:
##      @(#) refresh_interact.c
##  Author:
##      Robert M. Hubley   rhubley@systemsbiology.org
##  Description:
##      Interact files contain, among other things, the results
##      of many peptide to known protein searches.  These searches
##      are for exact matches and may produce multiple hits in 
##      a non-redundant protein database.  The non-redundant database
##      is updated frequently while the interact files are mainly static.
##      This utility refreshes the data associated with the protein
##      database search.  The relevant fields which are changed include
##      the ID of the primary protein match, the number of additional
##      proteins which were found but not listed, and the description
##      of the primary protein match.  Changes to the ID, and number
##      of additional hits are highlighted in GREEN while missing
##      database entries are highlighted in RED.
##
##      This program uses a fast dictionary search algorithm similar to
##      the Commentz-Walter algorithm (See "A String Matching Algorithm
##      Fast on the Average," Technical Report,IBM-Germany, Scientific
##      Center Heidelberg, Tiergartenstrasse 15, D-6900 Heidelberg, Germany.
##      See also, A.V., and M. Corasick, "Efficient String Matching:  An
##      Aid to Bibliographic Search," CACM June 1975, Vol. 18, No. 6, which 
##      describes the failure function.  The C-code specific to this
##      search was borrowed from the GNU fgrep project which is provided
##      under the GNU General Public License (Version 2).
##
#******************************************************************************
#* 
#* Changes
#*
#* Tue May  7 16:48:31 PDT 2002     v0.2    Fixed a problem with the
#*                                          additional references
#*                                          link.  It now includes
#*                                          all information in the
#*                                          link including the database
#*                                          name and MassType.
#*
#* Wed May  8 10:28:14 PDT 2002     v0.3    Simplified the print_ 
#*                                          interact_file code. No
#*                                          functional changes.
#*
#* Mon May 13 10:03:20 PDT 2002     v0.4    Added "-o" flag to specify
#*                                          a file to be used for output.
#*                                          If it is not specified the
#*                                          output is sent to stdout.
#*
#*/
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "kwset.h"
#include "refresh_interact.h"
#include <stdio.h>
#include <string.h>

#include "compbio.h"
#include "iub.h"
#include "sequence_iub.h"
#include "aminoacid.h"
#include "sequence_aminoacid.h"
#include "fasta_sequence_io.h"
#define PROGNAME "refresh_interact"

#include "common/TPPVersion.h" // contains version number, name, revision


//
// Function signatures
//
interact_data_line_t ** parse_interact_file(char * interact_file,long * num_lines);
char * long_to_string(long value);
char * make_substr(char * string, long start, long end);
int print_interact_file(char * file,interact_data_line_t ** interact_data,long interact_lines);
char * replace_substr(char * string, char * orig_substr, char * repl_substr );
int link_duplicates(interact_data_line_t ** interact_data,long interact_lines);
int compare_interact_lines(const void * line1, const void * line2);
interact_data_line_t ** build_uniq_kwlist(interact_data_line_t ** interact_data,long * interact_lines);
static void usage (int status);
int compare_match_records(const void * rec1, const void * rec2);


//
// Global Variables
//
char * program_name;
char * author = "Robert M. Hubley, Institute for Systems Biology";
int verbose = 0;
double version = 0.4;

//
// Main
//
int
main (int argc, char **argv)
{
  kwset_t kwset;
  size_t offset;
  struct kwsmatch * match;
  char * fasta_db_file = NULL;
  char * out_file = NULL;
  char * interact_file = NULL;
  char * seq_string = NULL;
  FILE *fp;
  fasta_sequence_t ** fasta_seqs = NULL;
  fasta_sequence_t * fasta_db_seq = NULL;
  interact_data_line_t ** interact_data = NULL;
  interact_data_line_t ** uniq_interact_data = NULL;
  interact_data_line_t * interact_line = NULL;
  db_ref_t * new_db_ref = NULL;
  db_ref_t ** ref_list = NULL;
  long num_interact_lines = 0L;
  long num_hits = 0L;
  long uniq_lines = 0L;
  int i = 0;
  int opt = 0;
  int prev_index;
  time_t start_time = 0;
  time_t incr_time = 0;


  // Setup the program name for prefixing output lines
  program_name = argv[0];
  if (program_name && strrchr (program_name, '/'))
    program_name = strrchr (program_name, '/') + 1;

  // Process the command line arguments
  while ((opt = getopt(argc,argv,"d:hv:o:")) != -1 ) 
    switch (opt)
    {
      case 'd':
        fasta_db_file = optarg;
        break;

      case 'o':
        out_file = optarg;
        break;

      case 'v':
        verbose = atoi(optarg);
        if ( verbose < 0 || verbose > 10 ) 
          usage(-1);
        break;

      case 'h':
        usage(0);

      default:
        usage(-1);
    }
  if ( fasta_db_file == NULL ) 
    usage(-1);

  if ( optind == argc ) 
    interact_file = "STDIN";
  else
    if ( optind == argc-1 ) 
     interact_file = argv[optind];
    else
      usage(-1);

  fprintf(stderr,"%s:\n",PROGNAME);
  start_time = time(NULL);

  fprintf(stderr,"  - Parsing the interact file...");
  interact_data = parse_interact_file(interact_file,&num_interact_lines);
  uniq_lines = num_interact_lines;
  uniq_interact_data = build_uniq_kwlist(interact_data,&uniq_lines);
  fprintf(stderr,"done.(%d sec)\n",time(NULL)-start_time);
  incr_time = time(NULL);

  if ( !(kwset = kwsalloc((char *) 0)) )
    printf("Error initializing the keyword set!\n");

  fprintf(stderr,"  - Building Commentz-Walter keyword tree...");
  for ( i = 0; i < uniq_lines; i++ ) {
    interact_line = (interact_data_line_t *)uniq_interact_data[i];
    if ( interact_line->sequence != NULL ) {
      if ( kwsincr(kwset, 
                   interact_line->sequence,
                   strlen(interact_line->sequence)) != 0 ) 
         printf("Error adding %s to the structure!\n",
                interact_line->sequence);
    }
  }

  /* Prep the keyword trie for searching */
  if ( kwsprep (kwset) != 0 )
    printf("Error preping the structure for a search!\n");

  fprintf(stderr,"done.(%d sec)\n",time(NULL)-incr_time);
  incr_time = time(NULL);

  /* Perform the search */
  fprintf(stderr,"  - Searching the tree...");
  if ( ! (fp = fopen(fasta_db_file,"r"))){
    printf("%s: could not open %s file!\n",PROGNAME,fasta_db_file);
    exit(-1);
  }

  while ( (fasta_db_seq = fasta_sequence_io_nextSequence( fp,
                                SEQUENCE_SUBTYPE_TYPE_AMINOACID)) != NULL ) {
    // Obtain the sequence from Paul's Fasta object
    seq_string = sequence_toString(fasta_db_seq->sequence);

    // Search the keyword set for matches to this sequence
    num_hits = kwsexec_multiple(kwset,seq_string,strlen(seq_string),&match);

    // Sort by pattern indexes: This will allow us to ignore hits patterns
    // which occur more than once in a database sequence.  Otherwise we
    // might double count hits.
    if ( num_hits > 0 ) 
      qsort(match,num_hits,sizeof(struct kwsmatch),compare_match_records);

    // Process the hits for this protein sequence
    prev_index = -1;
    for ( i=0; i<num_hits; i++){
      if ( match[i].index != prev_index ) {

        if ( (new_db_ref = (db_ref_t *)malloc(sizeof(db_ref_t))) == NULL )
          printf("Error: Could not allocate memory for db_ref_t!\n");
        if ( strchr(fasta_db_seq->description,' ') != NULL ) {
          new_db_ref->alias = make_substr(fasta_db_seq->description,
                                        0,
                                        strchr(fasta_db_seq->description,' ') - 
                                        fasta_db_seq->description - 1);
          new_db_ref->description = make_substr(fasta_db_seq->description,
                                        strchr(fasta_db_seq->description,' ') - 
                                        fasta_db_seq->description,
                                        strlen(fasta_db_seq->description));
        }else {
           new_db_ref->alias = strdup(fasta_db_seq->description);
        }
        //printf("added %s ---> %s\n", new_db_ref->alias, new_db_ref->description);
  
        ref_list = ((interact_data_line_t *)
                     uniq_interact_data[match[i].index])->updated_refs;
  
        if ( uniq_interact_data[match[i].index]->updated_hits == 0 ){
          //printf("allocing memory for a new reflist\n");
          if ( (ref_list = malloc( sizeof(db_ref_t *))) == NULL ) 
            printf("Error: Could not allocate memory for a new ref_list!\n");
        }else{
          //printf("reallocing memory for a reflist\n");
          if ( (ref_list = realloc(ref_list,
                        (((interact_data_line_t *)
                         uniq_interact_data[match[i].index])->updated_hits + 1) * 
                        sizeof(db_ref_t *))) == NULL ) 
            printf("Error: Could not reallocate memory for ref_list!\n");
        }
        ref_list[uniq_interact_data[match[i].index]->updated_hits] = new_db_ref;
        uniq_interact_data[match[i].index]->updated_hits += 1;
        ((interact_data_line_t *)uniq_interact_data[match[i].index])->updated_refs = ref_list;
        prev_index = match[i].index;
  
      } // end if
    } // end for
    free(seq_string);
    fasta_sequence_deallocate( fasta_db_seq );
  }// end while
  fclose( fp );
  fprintf(stderr,"done.(%d sec)\n",time(NULL)-incr_time);
  incr_time = time(NULL);
  fprintf(stderr,"  - Linking duplicate interact lines...");
  link_duplicates(interact_data,num_interact_lines);
  fprintf(stderr,"done.(%d sec)\n",time(NULL)-incr_time);
  incr_time = time(NULL);
  fprintf(stderr,"  - Printing results...");
  print_interact_file(out_file,interact_data,num_interact_lines);
  fprintf(stderr,"done.(%d sec)\n",time(NULL)-incr_time);
  incr_time = time(NULL);
}



// void usage(int status)
//
// Print the usage details for the command line
// options of this program.
static void
usage (int status)
{
  if (status != 0) 
  {
    fprintf (stderr, "Usage: %s [OPTION] -d DB_FILE [-o OUTFILE] [INTERACT_FILE]...\n",
             program_name);
    fprintf (stderr, "Try `%s -h' for more information.\n\n",
             program_name);
    fprintf (stderr, "  Version: %f (%s)\n", version, szTPPVersionInfo);
    fprintf (stderr, "  Author: %s\n\n", author);
  }
  else
  {
    printf("Usage: %s [OPTION] -d DB_FILE [-o OUTFILE] [INTERACT_FILE]...\n\n",
            program_name);
    printf("\
  Search for INTERACT_FILE sequences in DB_FILE fasta \n\
  database, update records in the INTERACT_FILE and \n\
  print on standard output.\n\n\
      Example: %s -d human.nci interact.htm\n\n",program_name);

    printf("  Version: %f (%s)\n", version, szTPPVersionInfo);
    printf("  Author: %s\n\n", author);

    printf ("\
Options:\n\
  -d <file>                 Database file.\n\
  -o <file>                 Optional.  File to store updated interact data.\n\
                              If not specified stdout is assumed.\n\
  -h                        This help information.\n\
  -v #                      Verbose (mostly debugging) output.\n");
    printf("\n\n\
  Interact files contain, among other things, the results\n\
  of many peptide-to-known-protein searches.  These searches\n\
  are for exact matches and may produce multiple hits in·\n\
  a non-redundant protein database.  The non-redundant database\n\
  is updated frequently while the interact files are mainly static.\n\
  This utility refreshes the data associated with the protein\n\
  database search.  The relevant fields which are changed include\n\
  the ID of the primary protein match, the number of additional\n\
  proteins which were found but not listed, and the description\n\
  of the primary protein match.  Changes to the ID, and number\n\
  of additional hits are highlighted in GREEN while missing\n\
  database entries are highlighted in RED.\n\
\n\
  This program uses a fast dictionary search algorithm similar to\n\
  the Commentz-Walter algorithm (See \"A String Matching Algorithm\n\
  Fast on the Average,\" Technical Report,IBM-Germany, Scientific\n\
  Center Heidelberg, Tiergartenstrasse 15, D-6900 Heidelberg, Germany.\n\
  See also, A.V., and M. Corasick, \"Efficient String Matching:  An\n\
  Aid to Bibliographic Search,\" CACM June 1975, Vol. 18, No. 6, which·\n\
  describes the failure function.  The C-code specific to this\n\
  search was borrowed from the GNU fgrep project which is provided\n\
  under the GNU Lesser General Public License (Version 2).\n\n");
  }
  exit (status);
}



// char * make_substr(char * string, long start, long end)
//
// Create a new string from the substr of an existing
// string.  This routine allocates memory for the new
// string which can be released using free().
char * 
make_substr(char * string, long start, long end) 
{
  char * substr;

  if ( verbose > 1 ) 
    printf("make_substr: called with s=%s start=%d end=%d\n",
            string, start, end);
  substr = (char *)malloc((end-start+2) * sizeof( char ));
  strncpy(substr,string+start,(end-start+1));
  // Ask paul about this why is this not equivalent to:
  //   *(substr+end-start+1) = '\0';
  substr[end-start+1] = '\0';
  return substr;
}




char * 
replace_substr(char * string, char * orig_substr, char * repl_substr ) 
{
  char * substr_ptr;
  char * suffix_ptr;
  char * search_ptr;
  char * new_string;
  long substr_len_diff = 0L;
  long offset = 0L;

  if ( verbose > 1 ) 
    printf("replace_substr: called with s=%s o=%s r=%s\n",
            string, orig_substr, repl_substr);

  substr_len_diff =  strlen(repl_substr) - strlen(orig_substr);
  search_ptr = string;

  while ( (substr_ptr = strstr(search_ptr,orig_substr)) != NULL ) {

    if ( substr_len_diff > 0 ) {
      offset = substr_ptr - string;
      string = realloc(string,
                       (strlen(string) + substr_len_diff + 1)*sizeof(char));
      substr_ptr = string + offset;
    }
    suffix_ptr = substr_ptr + strlen(orig_substr);
    memmove(suffix_ptr+substr_len_diff,
                             suffix_ptr,
                             strlen(suffix_ptr)+1);
    strncpy(substr_ptr,repl_substr,strlen(repl_substr));

    search_ptr = substr_ptr + strlen(repl_substr);
  }
  // TODO: could free some memory if we shrunk the string
  return string;
}



interact_data_line_t **
build_uniq_kwlist(interact_data_line_t ** interact_data,long * interact_lines)
{
  int i = 0;
  interact_data_line_t ** sorted_lines;
  interact_data_line_t ** uniq_lines;
  db_ref_t ** ref_ptr;
  char * sequence;
  long ref_hits;
  long u_index;
  char * prev_sequence;

  if ( verbose > 1 ) 
    printf("build_uniq_kwlist: called\n");

  sorted_lines = malloc(sizeof(interact_data_line_t *)* *interact_lines);
  uniq_lines = malloc(sizeof(interact_data_line_t *)* *interact_lines);

  for ( i = 0; i < *interact_lines; i++ )
    sorted_lines[i] = (interact_data_line_t *)interact_data[i];

  qsort(sorted_lines,*interact_lines,sizeof(interact_data_line_t *),
        compare_interact_lines);

  u_index = 0;
  prev_sequence = "";
  for ( i = 0; i < *interact_lines; i++ ) {
    if ( sorted_lines[i]->sequence != NULL ) 
      if ( strcmp(sorted_lines[i]->sequence,prev_sequence) != 0 ) {
        prev_sequence = sorted_lines[i]->sequence; 
        uniq_lines[u_index++] = sorted_lines[i];
      }
  }
  free(sorted_lines);
  *interact_lines = u_index;
  return(uniq_lines);

}



int
link_duplicates(interact_data_line_t ** interact_data,long interact_lines)
{
  int i = 0;
  interact_data_line_t ** sorted_lines;
  db_ref_t ** ref_ptr;
  char * sequence;
  long ref_hits;

  if ( verbose > 1 ) 
    printf("link_duplicates: called\n");

  //printf("Allocating memory for sort\n");
  sorted_lines = malloc(sizeof(interact_data_line_t *)*interact_lines);

  //printf("Creating array for sort\n");
  for ( i = 0; i < interact_lines; i++ )
    sorted_lines[i] = (interact_data_line_t *)interact_data[i];

  //printf("sorting...\n");
  qsort(sorted_lines,interact_lines,sizeof(interact_data_line_t *),
        compare_interact_lines);

  //printf("Printing sorted data\n");
  //for ( i = 0; i < interact_lines; i++ ) 
  //  printf("sequence = %s  pointer = %d\n", sorted_lines[i]->sequence,
  //         sorted_lines[i]->updated_refs);


  sequence = "start";
  for ( i = 0; i < interact_lines; i++ ) {
    if ( sorted_lines[i]->sequence != NULL ) {   // must be a data line
      if ( sorted_lines[i]->updated_refs != NULL ) {
        ref_ptr = sorted_lines[i]->updated_refs;
        ref_hits = sorted_lines[i]->updated_hits;
        sequence = sorted_lines[i]->sequence;
      }else {
        //printf("we are here with s=%s seq=%s\n",sequence ,sorted_lines[i]->sequence);
        if ( strcmp(sequence, sorted_lines[i]->sequence) == 0 ) {
          //printf("updating:\n%s\n",sorted_lines[i]->line);
          sorted_lines[i]->updated_refs = ref_ptr;
          sorted_lines[i]->updated_hits = ref_hits;
          //printf("now:\n%s\n",sorted_lines[i]->line);
        }
      }
    }
  }

}


// int compare_match_records(const void * rec1, const void * rec2)
//
int
compare_match_records(const void * rec1, const void * rec2)
{
  if ( verbose > 8 ) 
    printf("compare_match_records: called\n");
  return  ((struct kwsmatch *)rec1)->index -
          ((struct kwsmatch *)rec2)->index; 
}



// int compare_interact_lines(const void * line1, const void * line2)
//
int
compare_interact_lines(const void * line1, const void * line2)
{
  int return_val = 0;
  interact_data_line_t * i_line1;
  interact_data_line_t * i_line2;

  if ( verbose > 8 ) 
    printf("compare_interact_lines: called\n");
 
  i_line1 = *((interact_data_line_t **)line1);

  if ( (*((interact_data_line_t **)line1))->sequence == NULL && 
       (*((interact_data_line_t **)line2))->sequence == NULL  ) 
    return 0;
  if ( (*((interact_data_line_t **)line1))->sequence != NULL && 
       (*((interact_data_line_t **)line2))->sequence == NULL  ) 
    return 1;
  if ( (*((interact_data_line_t **)line1))->sequence == NULL && 
       (*((interact_data_line_t **)line2))->sequence != NULL  ) 
    return -1;

  return_val = strcmp((*((interact_data_line_t **)line1))->sequence, 
                      (*((interact_data_line_t **)line2))->sequence);

  if ( return_val == 0 ) {
    if ( (*((interact_data_line_t **)line1))->updated_refs == NULL && 
         (*((interact_data_line_t **)line2))->updated_refs == NULL  ) 
      return 0;
    if ( (*((interact_data_line_t **)line1))->updated_refs != NULL && 
         (*((interact_data_line_t **)line2))->updated_refs == NULL  ) 
      return -1;
    if ( (*((interact_data_line_t **)line1))->updated_refs == NULL && 
         (*((interact_data_line_t **)line2))->updated_refs != NULL  ) 
      return 1;
  }

  return return_val;

}


// char * long_to_string(long value)
//
char *
long_to_string(long value) 
{
  long size = 100;
  long conv_size = 0;
  char * string;

  if ( verbose > 8 ) 
    printf("long_to_string: called\n");
  string = malloc(sizeof(char) * size );
  conv_size = snprintf(string,size,"%ld",value);
  if ( conv_size > 0 )
   string = realloc(string,conv_size + 2);
  else
   return NULL;
  return string;
}


// int print_interact_file(char * filename, 
//                         interact_data_line_t ** interact_data,
//                         long interact_lines)
//
int
print_interact_file(char * filename, interact_data_line_t ** interact_data,long interact_lines)
{
  int i = 0;
  int j = 0;
  interact_data_line_t * interact_line;
  db_ref_t  **  db_ref;
  char * search_str;
  char * repl_str;
  char * hit_str_ptr;
  char * upd_hit_str_ptr;
  char * temp_ptr;
  char * desc_start_ptr;
  int desc_start_index;
  int found_hit = 0;
  FILE * outstream;

  if ( filename == NULL ) 
    outstream = stdout;
  else 
    if ( ! (outstream = fopen(filename,"w"))){
      fprintf(stderr,"\nError: could not open file: %s!\n",filename);
      exit(-1);
    }

  if ( verbose > 1 ) 
    printf("print_interact_file: called\n");

  for ( i = 0; i < interact_lines; i++ ) {
    interact_line = (interact_data_line_t *)interact_data[i];
    if ( verbose > 2 )
     fprintf(stderr,"print_interact_file:  Starting with a new line...index=%d\n",i);
    if ( interact_line->sequence != NULL ) {
       if ( verbose > 2 )
         fprintf(outstream,"%s",interact_line->line);
       if ( interact_line->updated_refs != NULL ) {
         db_ref = (db_ref_t **)interact_line->updated_refs;
         if ( interact_line->hits != (interact_line->updated_hits-1) ) {
           hit_str_ptr = long_to_string(interact_line->hits);
           upd_hit_str_ptr = long_to_string(interact_line->updated_hits-1);
           if ( interact_line->hits == 0 ){

             search_str = strdup("[DBREF]</A>      ");
             search_str = replace_substr(search_str,
                                         "[DBREF]",
                                         interact_line->db_ref);


             repl_str = strdup("[DBREF]</A>      <A TARGET=\"Win1\" HREF=\"/cgi-bin/blast_html4?Db=[DBNAME]&amp;Pep=[PEP]&amp;MassType=[MASSTYPE]\"><FONT COLOR=\"#00FF00\">+[AHITS]</FONT></A>");

             repl_str = replace_substr(repl_str,
                                       "[DBREF]",
                                       interact_line->db_ref);
             repl_str = replace_substr(repl_str,
                                       "[DBNAME]",
                                       interact_line->dbname);
             repl_str = replace_substr(repl_str,
                                       "[PEP]",
                                       interact_line->sequence);
             repl_str = replace_substr(repl_str,
                                       "[MASSTYPE]",
                                       interact_line->mtype);
             repl_str = replace_substr(repl_str,
                                       "[AHITS]",
                                       upd_hit_str_ptr);
           }else{
             search_str = strdup(">+[AHITS]<");
             search_str = replace_substr(search_str,
                                         "[AHITS]",
                                         hit_str_ptr);

             repl_str = strdup("><FONT COLOR=\"#00FF00\">+[AHITS]</FONT><");
             repl_str = replace_substr(repl_str,
                                       "[AHITS]",
                                       upd_hit_str_ptr);
           }

           //printf("----replacing %s with %s\n",search_str,repl_str);
           interact_line->line = replace_substr(interact_line->line,
                                                search_str,
                                                repl_str);

           free(search_str);
           free(repl_str);
           free(hit_str_ptr);
           free(upd_hit_str_ptr);
         }

         // The order of records in the database could have changed
         // in which case the original interact db_ref might not
         // be the first record in the new db_refs list.  So..before
         // we update the interact record we should first be sure
         // it doesn't exist somewhere in the db_refs list.
         found_hit = 0;
         for( j = 0; j < interact_line->updated_hits; j++ ){
           if ( verbose > 4 ) 
             fprintf(stderr,"print_interact_file: dbref=%s updrefs[%d]=%s\n",
                     interact_line->db_ref, j, ((db_ref_t *)db_ref[j])->alias);
           if ( strcmp(interact_line->db_ref,((db_ref_t *)db_ref[j])->alias)
                == 0 )
             found_hit = j;
         }

         // Update the db_ref if we need to
         if ( strcmp(interact_line->db_ref,
                     ((db_ref_t *)db_ref[found_hit])->alias) != 0 )
         {
           // Color green if we need to change the record
           search_str = strdup(">[DBREF]<");
           search_str = replace_substr(search_str,
                                       "[DBREF]",
                                        interact_line->db_ref);
           repl_str = strdup("><FONT COLOR=\"#00FF00\">[DBREF]</FONT><");
           repl_str = replace_substr(repl_str,
                                     "[DBREF]",
                                     interact_line->db_ref);

           //printf("----replacing %s with %s\n",search_str,repl_str);
           interact_line->line = replace_substr(interact_line->line,
                                              search_str,
                                              repl_str);

           free(search_str);
           free(repl_str);

           //printf("----replacing %s with %s\n",interact_line->db_ref,
           //               ((db_ref_t *)db_ref[found_hit])->alias);
           interact_line->line = replace_substr(interact_line->line,
                                      interact_line->db_ref,
                                      ((db_ref_t *)db_ref[found_hit])->alias);

           // Change the description
           desc_start_ptr = interact_line->line;
           while ( *desc_start_ptr != '\0' &&
                   (temp_ptr = strstr(desc_start_ptr,"</A>")) != NULL )
             desc_start_ptr = temp_ptr + 4;
           if ( (temp_ptr = strstr(desc_start_ptr," N/A ")) != NULL )
             desc_start_ptr = temp_ptr+6;
           while ( *desc_start_ptr != '\0' && *desc_start_ptr == ' ')
             desc_start_ptr++;
           desc_start_index = desc_start_ptr-interact_line->line;
           if ( ((db_ref_t *)db_ref[found_hit])->description != NULL ) {
             interact_line->line = realloc(interact_line->line,
                                      sizeof(char) * 
                                      (desc_start_index + 3
                                      + strlen(
                                      ((db_ref_t *)db_ref[found_hit])->description)));
             strcpy(interact_line->line+desc_start_index,
                    ((db_ref_t *)db_ref[found_hit])->description);
             strcat(interact_line->line,"\n");
           }else {
             interact_line->line = realloc(interact_line->line,
                                      sizeof(char) * 
                                      (desc_start_index + 3 + 24));
             strcpy(interact_line->line+desc_start_index,
                      "no description available\n");
           }

         }
         if ( verbose > 2 )
           fprintf(outstream,"==>");
         fprintf(outstream,"%s",interact_line->line);
       }else {
         // Color red if we can't find the record  
         search_str = strdup(">[DBREF]<");
         search_str = replace_substr(search_str,
                                     "[DBREF]",
                                     interact_line->db_ref);

         repl_str = strdup("><FONT COLOR=\"#FF0000\">[DBREF]</FONT><");
         repl_str = replace_substr(repl_str,
                                   "[DBREF]",
                                   interact_line->db_ref);

         interact_line->line = replace_substr(interact_line->line,
                                              search_str,
                                              repl_str);

         free(search_str);
         free(repl_str);
         if ( verbose > 2 )
           fprintf(outstream,"==>");
         fprintf(outstream,"%s",interact_line->line);
       }
    }else{
       fprintf(outstream,"%s",interact_line->line);
    }
  }
  fclose(outstream);

}



// interact_data_line_t ** parse_interact_file(char * interact_file,
//                                             long * num_interact_lines)
//
interact_data_line_t **
parse_interact_file(char * interact_file,long * num_interact_lines) 
{
  char  ch;
  long line_num = 0L;
  long buffer_incr = 80L;
  long buffer_size = 0L;
  long buffer_index = 0L;
  long token_len = 0L;
  char * line_buffer;
  char * buffer_ptr;
  char * prefix_ptr;
  char * db_ref_string;
  char * db_string;
  char * mt_string;
  char * pep_string;
  char * count_string;
  FILE * fp;
  interact_data_line_t * interact_data_line;
  interact_data_line_t ** interact_file_lines;
  int interact_lines_incr = 20;

  if ( verbose > 1 ) 
    printf("parse_interact_file: called\n");

  buffer_size = buffer_incr;
  line_buffer = (char *)malloc(buffer_size * sizeof( char ));

  buffer_ptr = line_buffer;

  if ( strcmp(interact_file,"STDIN") == 0 ) 
    fp = stdin;
  else
    if ( ! (fp = fopen(interact_file,"r"))){
      fprintf(stderr,"\nError: could not open file: %s!\n",interact_file);
      exit(-1);
    }

  *num_interact_lines = 0L;
  *num_interact_lines = interact_lines_incr;
  interact_file_lines = (interact_data_line_t **)malloc(
                       sizeof(interact_data_line_t *) * (*num_interact_lines));

  while ( (ch = (char)fgetc(fp)) != EOF ) {
    /* allocate more memory if needed */
    if ( (buffer_index + 1) >= buffer_size ){
      buffer_size += buffer_incr;
      line_buffer = (char *)realloc(line_buffer, buffer_size * sizeof( char ));
      buffer_ptr = ( line_buffer + buffer_index );
    }

    /* Process a line */
    if ( ch == '\n' ) {
      *buffer_ptr = '\n';
      buffer_ptr++;
      *buffer_ptr = '\0';

      interact_data_line = (interact_data_line_t * )malloc(
                                       sizeof(interact_data_line_t));

      interact_data_line->line = strcpy(
                            (char *)malloc(sizeof(char)*strlen(line_buffer)+1),
                            line_buffer);

      interact_data_line->hits = 0;
      interact_data_line->db_ref = NULL;
      interact_data_line->sequence = NULL;

      if ( (prefix_ptr = strstr(line_buffer,"html4?Ref=")) != NULL ) {
        prefix_ptr += strlen("html4?Ref=");
        token_len = strchr(prefix_ptr,'&') - prefix_ptr;
        db_ref_string = (char *)malloc((token_len+1) * sizeof( char ));
        strncpy(db_ref_string,prefix_ptr,token_len);
        *(db_ref_string+token_len) = '\0';
        interact_data_line->db_ref = db_ref_string;
        //printf("dbRef = %s\n", db_ref_string);
      }
      if ( (prefix_ptr = strstr(line_buffer,"\">+")) != NULL ) {
        prefix_ptr += strlen("\">+");
        token_len = strchr(prefix_ptr,'<') - prefix_ptr;
        count_string = (char *)malloc((token_len+1) * sizeof( char ));
        strncpy(count_string,prefix_ptr,token_len);
        *(count_string+token_len) = '\0';
        interact_data_line->hits = atol(count_string);
        //printf("Count = %s\n", count_string);
      }
      if ( (prefix_ptr = strstr(line_buffer,"&amp;Pep=")) != NULL ) {
        prefix_ptr += strlen("&amp;Pep=");
        token_len = strchr(prefix_ptr,'"') - prefix_ptr;
        pep_string = (char *)malloc((token_len+1) * sizeof( char ));
        strncpy(pep_string,prefix_ptr,token_len);
        *(pep_string+token_len) = '\0';
        interact_data_line->sequence = pep_string;
        //printf("Pep = %s\n", pep_string);
      }
      if ( (prefix_ptr = strstr(line_buffer,"Db=")) != NULL ) {
        prefix_ptr += strlen("Db=");
        token_len = strchr(prefix_ptr,'&') - prefix_ptr;
        db_string = (char *)malloc((token_len+1) * sizeof( char ));
        strncpy(db_string,prefix_ptr,token_len);
        *(db_string+token_len) = '\0';
        interact_data_line->dbname = db_string;
        //printf("Db = %s\n", db_string);
      }
      if ( (prefix_ptr = strstr(line_buffer,"MassType=")) != NULL ) {
        prefix_ptr += strlen("MassType=");
        token_len = strchr(prefix_ptr,'&') - prefix_ptr;
        mt_string = (char *)malloc((token_len+1) * sizeof( char ));
        strncpy(mt_string,prefix_ptr,token_len);
        *(mt_string+token_len) = '\0';
        interact_data_line->mtype = mt_string;
        //printf("MassType = %s\n", mt_string);
      }



      interact_data_line->updated_hits = 0L;
      interact_data_line->updated_refs = 0L;


      if ( line_num >= *num_interact_lines ) {
        *num_interact_lines += interact_lines_incr;
        interact_file_lines = (interact_data_line_t **)
                              realloc(interact_file_lines,
                              sizeof(interact_data_line_t *) * 
                              (*num_interact_lines));
      }
      interact_file_lines[line_num] = interact_data_line;
      buffer_ptr = line_buffer;
      buffer_index = 0L;
      line_num++;
    }else {
      *buffer_ptr = ch;
      buffer_ptr++;
      buffer_index++;
    }

  }
  fclose(fp);

  *num_interact_lines = line_num;
  interact_file_lines = (interact_data_line_t **)
                         realloc(interact_file_lines,
                         sizeof(interact_data_line_t *) *
                         (*num_interact_lines));

  if ( verbose > 1 ) 
    fprintf(stderr,"\nparse_interact_file: num of lines read = %d\n",line_num);

  return interact_file_lines;
}

