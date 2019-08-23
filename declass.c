// AUTHOR: JORDAN RANDLEMAN - DECLASSIFIER TO PRE-PREPROCESS .C FILES USING CLASSES
/**
 * compile: $ gcc -o declass declass.c
 *          $ ./declass yourFile.c 
 *     (OR) $ ./declass -l yourFile.c // optional '-l' shows class details 
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
// scrape file's contents, put contents into file, & add new filename extension
#define FSCRAPE(BUFF,FNAME)({\
  BUFF[0]='\0';FILE*fptr;if((fptr=fopen(FNAME,"r"))==NULL){exit(EXIT_FAILURE);}\
  while(fgets(&BUFF[strlen(BUFF)],500,fptr)!=NULL);fclose(fptr);\
})
#define FPUT(BUFF,FNAME)\
  ({FILE*fptr;if((fptr=fopen(FNAME,"w"))==NULL){exit(EXIT_FAILURE);}fprintf(fptr,"%s",BUFF);fclose(fptr);})
#define NEW_EXTENSION(FNAME,EXT) ({char*p=&FNAME[strlen(FNAME)];while(*(--p)!='.');strcpy(p,EXT);})
// checks if char is whitespace, or alphanumeric/underscore
#define IS_WHITESPACE(c) (c == ' ' || c == '\t' || c == '\n')
#define VARCHAR(CH) (((CH)>='A' && (CH)<='Z') || ((CH)>='a' && (CH)<='z') || (((CH)>='0' && (CH)<='9')) || (CH) == '_')
// append string to NEW_FILE buffer when splicing in large-scale changes
#define APPEND_BUFF_OR_STR_TO_NEW_FILE(appending) \
  for(int m_i = 0, len = strlen(appending); m_i < len; ++m_i, *j += 1) NEW_FILE[*j] = appending[m_i];
// initialize array with zero's (wipes garbage memory)
#define FLOOD_ZEROS(arr, len) ({for(int arr_i = 0; arr_i < len; ++arr_i) arr[arr_i] = 0;})
// file, class, & object limitations => all self imposed for memory's sake, increment as needed
#define MAX_FILESIZE 1000001 // 1 gigabyte + '\0'
#define MAX_OBJECTS 1000
#define MAX_CLASSES 1000
#define MAX_MEMBERS_PER_CLASS 100
#define MAX_METHODS_PER_CLASS 100
#define MAX_MEMBER_BYTES_PER_CLASS 5001
#define MAX_METHOD_BYTES_PER_CLASS 5001
#define MAX_WORDS_PER_METHOD 1000
#define MAX_DEFAULT_VALUE_LENGTH 251

/*****************************************************************************
 *                       -:- DECLASS.C 8 CAVEATS -:-                        *
 *   (1) 'DECLASS_' PREFIX & 'this' POINTER ARE RESERVED                    *
 *   (2) DECLARE CLASSES GLOBALLY & OBJECTS LOCALLY (NEVER IN STRUCT/UNION) *
 *   (3) DECLARE MEMBERS/METHODS USED IN A METHOD ABOVE ITS DECLARATION     *
 *   (4) DECLARE CLASS MEMBERS, METHODS, & OBJECTS INDIVIDUALLY:            *
 *       (*) IE NOT:      'className c, e;'                                 *
 *       (*) ALTERNATIVE: 'className c; <press enter> className e;'         *
 *   (5) NO NESTED CLASS DECLARATIONS NOR METHOD INVOCATIONS:               *
 *       (*) IE NOT:      'someObj.method1(someObj.method2());'             *
 *       (*) ALTERNATIVE: 'int x = someObj.method2(); someObj.method1(x);'  *
 *   (6) CLASS ARRAYS RECIEVED AS ARGS MUST BE DENOTED WITH '[]' NOT '*':   *
 *       (*) IE NOT:      'func(className *classArr){...}'                  *
 *       (*) ALTERNATIVE: 'func(className classArr[]){...}'                 *
 *   (7) NO POINTER TO ARRAY OF OBJECTS:                                    *
 *       (*) IE NOT:      className (*ptrToArrObj)[10];                     *
 *       (*) ALTERNATIVE: pointer to an object w/ array of objects member   *
 *   (8) CONTAINMENT, NOT INHERITANCE: CLASSES CAN ONLY ACCESS MEMBERS &    *
 *       METHODS OF THEIR OWN IMMEDIATE MEMBER CLASS OBJECTS:               *
 *       (*) IE: suppose classes c1, c2, & c3, with c1 in c2 & c2 in c3.    *
 *               c3 can access c2 members and c2 ca access c1 members,      *
 *               but c3 CANNOT access c1 members                            *
 *       (*) ALTERNATIVES: (1) simply include a c1 object as a member in c3 *
 *                         (2) create methods in c2 invoking c1 methods as  *
 *                             an interface for c3                          *
 *****************************************************************************
 *                         -:- DECLASS.C NOTES -:-                          *
 *   (1) W/O "#define DECLASS_NSMRTPTR", THE SMRTPTR.H LIBRARY IS INCLUDED, *
 *       W/ IMPROVED MALLOC/CALLOC/REALLOC/FREE FCNS & GARBAGE COLLECTION   *
 *       (*) "smrtptr.h"'s fcns same as stdlib's all prefixed with "smrt"   *
 *   (2) DENOTE CONSTRUCTORS AS TYPLESS METHODS W/ SAME NAME AS ITS CLASS   *
 *   (3) DELCARING OBJECTS => CONSTRUCTORS (CTORS) & DEFAULT (DFLT) VALUES: *
 *       (*) only dflt values: "className objectName;"                      *
 *       (*) dflts & ctor(if defined): "className objectName(ctor_args);"   *
 *       (*) dflts & ctor for an array: "className objectName[size](args);" *
 *****************************************************************************/

// stores class names, & their associated methods
typedef struct class_info { 
  char class_name[75], method_names[MAX_METHODS_PER_CLASS][75]; 
  char member_names[MAX_MEMBERS_PER_CLASS][75], member_values[MAX_MEMBERS_PER_CLASS][MAX_DEFAULT_VALUE_LENGTH];
  bool class_has_alloc;                                     // class has 1+ member of: malloc/calloc/smrtmalloc/smrtcalloc
  bool class_has_ctor, class_has_ctor_args;                 // class has user-defined ctor to invoke when assigning default
  bool member_is_array[MAX_MEMBERS_PER_CLASS];              // init empty arrays as {0}
  bool member_is_pointer[MAX_MEMBERS_PER_CLASS];            // init pointers as 0 (same as NULL)
  bool member_value_is_alloc[MAX_MEMBERS_PER_CLASS];        // track alloc'd members to be freed
  char member_value_user_ctor[MAX_MEMBERS_PER_CLASS][350];  // track user-define ctor arr vals (spliced out for DFLT fcn)
  char member_object_class_name[MAX_MEMBERS_PER_CLASS][75]; // used to intialize contained class objects
  int total_methods, total_members; 
} CLASS_INFO;
CLASS_INFO classes[MAX_CLASSES];
int total_classes = 0;

// stores object names, & their associated class
typedef struct objNames { 
  char class_name[75], object_name[75]; 
  bool is_class_pointer, is_class_array;
} OBJ_INFO;
OBJ_INFO objects[MAX_OBJECTS];
int total_objects = 0;

// basic c type keywords:
#define TOTAL_TYPES 14
char basic_c_types[TOTAL_TYPES][11] = {
  "char ","short ","int ","unsigned ","signed ","struct ","union ",
  "long ","float ","double ","bool ","enum ","typedef ","void "
};
// brace-additions
#define TOTAL_BRACE_KEYWORDS 5
char brace_keywords[TOTAL_BRACE_KEYWORDS][8] = {"else if", "if", "else", "for", "while"};
// "classless" default methods available for all classes

/* BRACE FUNCTIONS & SMRTPTR.H */
bool SMRT_PTRS;
void add_braces(char []);
/* MESSAGE FUNCTIONS */
void confirm_valid_file(char *);
void declass_missing_Cfile_alert();
void declass_DECLASSIFIED_ascii_art();
void show_l_flag_data();
/* COMMENT & BLANK LINE SKIPPING FUNCTIONS */
void whitespace_all_comments(char *);
void trim_sequential_spaces(char []);
int remove_blank_lines(char *);
/* STRING HELPER FUNCTIONS */
bool no_overlap(char, char *);
bool is_at_substring(char *, char *);
/* OBJECT CONSTRUCTION FUNCTIONS */
void mk_initialization_brace(char [], int);
void mk_ctor_macros(char [], char *);
void mk_class_global_initializer(char *, char *, char *);
/* USER-DEFINED OBJECT CONSTRUCTOR PARSING FUNCTIONS */
bool get_user_ctor(char *, char *, char *, bool *);
char *check_for_ctor_obj(char *, char *, char *, int *, int *, bool *);
/* OBJECT METHOD PARSER */
int parse_method_invocation(char *, char *, int *, bool, char[][75]);
void splice_in_prepended_method_name(char *, char *, int, char *, int *, int *, char[][75]);
void splice_in_prepended_NESTED_method_name(char *, char *, int, char *, int *, char[][75]);
void rmv_excess_buffer_objectChain(char *, char *, char *, char *, char *);
/* OBJECT METHOD PARSER HELPER FUNCTIONS */
int is_method_invocation(char *);
bool invoked_member_is_method(char *, char *, bool);
void get_invoked_member_name(char *, char *);
void get_object_name(char *, char *, char *, int, char [][75], bool, bool *);
int prefix_local_members_and_cpy_method_args(char *, char *, char [][75], int *, char);
/* STORE OBJECT INFORMATION */
bool store_object_info(char *);
/* PARSE CLASS HELPER FUNCTIONS */
bool is_struct_definition(char *);
void get_class_name(char *, char *);
void confirm_only_one_ctor(char *);
bool get_prepended_method_name(char *, char *, char *);
int get_initialized_member_value(char *);
void register_member_class_objects(char *);
void check_for_alloc_sizeof_arg();
int get_class_member(char *, bool);
void add_method_word(char [][75], int *, char *, char *);
/* CONFIRM WHETHER METHOD'S WORD IS A LOCAL CLASS MEMBER */
bool not_local_var_declaration(char *);
bool not_in_method_words(char [][75], int, char *);
bool not_an_external_invocation(char *);
void splice_in_this_arrowPtr(char *);
int parse_local_nested_method(char *, char *, char *, char [][75]);
bool valid_member(char *, char *, char, char, char [][75], int);
/* PARSE CLASS */
int parse_class(char *, char [], int *);

// declassed program contact header
#define DECLASS_SUPPORT_CONTACT "Email jrandleman@scu.edu or see https://github.com/jrandleman for support */"
// smrtptr.h to implement stdlib.h's memory handling functions w/ garbage collection
#define DECLASS_SMART_POINTER_H_ "\
/****************************** SMRTPTR.H START ******************************/\n\
// Source: https://github.com/jrandleman/C-Libraries/tree/master/Smart-Pointer\n\
#ifndef SMRTPTR_H_\n\
#define SMRTPTR_H_\n\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <string.h>\n\
// garbage collector & smart pointer storage struct\n\
static struct SMRTPTR_GARBAGE_COLLECTOR {\n\
  long len, max; // current # of ptrs && max capacity\n\
  void **ptrs;   // unique ptr set to free all smrt ptrs\n\
} SMRTPTR_GC = {-1};\n\
// invoked by atexit to free all ctor-alloc'd memory\n\
static void smrtptr_free_all() {\n\
  int i = 0;\n\
  for(; i < SMRTPTR_GC.len; ++i) free(SMRTPTR_GC.ptrs[i]);\n\
  if(SMRTPTR_GC.len > 0) free(SMRTPTR_GC.ptrs);\n\
  // if(SMRTPTR_GC.len > 0) printf(\"FREED %ld SMART POINTERS!\\n\", SMRTPTR_GC.len); // optional\n\
  SMRTPTR_GC.len = 0;\n\
}\n\
// throws invalid allocation errors\n\
static void smrtptr_throw_bad_alloc(char *alloc_type, char *smrtptr_h_fcn) {\n\
  fprintf(stderr, \"-:- ERROR: COULDN'T %s MEMORY FOR SMRTPTR.H'S %s -:-\\n\\n\", alloc_type, smrtptr_h_fcn);\n\
  fprintf(stderr, \"-:- FREEING ALLOCATED MEMORY THUS FAR AND TERMINATING PROGRAM -:-\\n\\n\");\n\
  exit(EXIT_FAILURE); // still frees any ptrs allocated thus far\n\
}\n\
// smrtptr stores ptr passed as arg to be freed atexit\n\
void smrtptr(void *ptr) {\n\
  // free ptrs atexit\n\
  atexit(smrtptr_free_all);\n\
  // malloc garbage collector\n\
  if(SMRTPTR_GC.len == -1) {\n\
    SMRTPTR_GC.ptrs = malloc(sizeof(void *) * 10);\n\
    if(!SMRTPTR_GC.ptrs) {\n\
      fprintf(stderr, \"-:- ERROR: COULDN'T MALLOC MEMORY TO INITIALIZE SMRTPTR.H'S GARBAGE COLLECTOR -:-\\n\\n\");\n\
      exit(EXIT_FAILURE);\n\
    }\n\
    SMRTPTR_GC.max = 10, SMRTPTR_GC.len = 0;\n\
  }\n\
  // reallocate if \"max\" ptrs already added\n\
  if(SMRTPTR_GC.len == SMRTPTR_GC.max) {\n\
    SMRTPTR_GC.max *= SMRTPTR_GC.max;\n\
    void **SMRTPTR_TEMP = realloc(SMRTPTR_GC.ptrs, sizeof(void *) * SMRTPTR_GC.max);\n\
    if(!SMRTPTR_TEMP) smrtptr_throw_bad_alloc(\"REALLOC\", \"GARBAGE COLLECTOR\");\n\
    SMRTPTR_GC.ptrs = SMRTPTR_TEMP;\n\
  }\n\
  // add ptr to SMRTPTR_GC if not already present (ensures no double-freeing)\n\
  int i = 0;\n\
  for(; i < SMRTPTR_GC.len; ++i) if(SMRTPTR_GC.ptrs[i] == ptr) return;\n\
  SMRTPTR_GC.ptrs[SMRTPTR_GC.len++] = ptr;\n\
}\n\
// malloc's a pointer, stores it in the garbage collector, then returns ptr\n\
void *smrtmalloc(size_t alloc_size) {\n\
  void *smtr_malloced_ptr = malloc(alloc_size);\n\
  if(smtr_malloced_ptr == NULL) smrtptr_throw_bad_alloc(\"MALLOC\", \"SMRTMALLOC FUNCTION\");\n\
  smrtptr(smtr_malloced_ptr);\n\
  return smtr_malloced_ptr;\n\
}\n\
// calloc's a pointer, stores it in the garbage collector, then returns ptr\n\
void *smrtcalloc(size_t alloc_num, size_t alloc_size) {\n\
  void *smtr_calloced_ptr = calloc(alloc_num, alloc_size);\n\
  if(smtr_calloced_ptr == NULL) smrtptr_throw_bad_alloc(\"CALLOC\", \"SMRTCALLOC FUNCTION\");\n\
  smrtptr(smtr_calloced_ptr);\n\
  return smtr_calloced_ptr;\n\
}\n\
// realloc's a pointer, stores it anew in the garbage collector, then returns ptr\n\
// compatible both \"smart\" & \"dumb\" ptrs!\n\
void *smrtrealloc(void *ptr, size_t realloc_size) {\n\
  int i = 0;\n\
  void *smtr_realloced_ptr;\n\
  // realloc a \"smart\" ptr already in garbage collector\n\
  for(; i < SMRTPTR_GC.len; ++i)\n\
    if(SMRTPTR_GC.ptrs[i] == ptr) {\n\
      smtr_realloced_ptr = realloc(ptr, realloc_size); // frees ptr in garbage collector\n\
      if(smtr_realloced_ptr == NULL) smrtptr_throw_bad_alloc(\"REALLOC\", \"SMRTREALLOC FUNCTION\");\n\
      SMRTPTR_GC.ptrs[i] = smtr_realloced_ptr; // point freed ptr at realloced address\n\
      return smtr_realloced_ptr;\n\
    }\n\
  // realloc a \"dumb\" ptr then add it to garbage collector\n\
  smtr_realloced_ptr = realloc(ptr, realloc_size);\n\
  if(smtr_realloced_ptr == NULL) smrtptr_throw_bad_alloc(\"REALLOC\", \"SMRTREALLOC FUNCTION\");\n\
  smrtptr(smtr_realloced_ptr);\n\
  return smtr_realloced_ptr;\n\
}\n\
// prematurely frees ptr arg prior to atexit (if exists)\n\
void smrtfree(void *ptr) {\n\
  int i = 0, j;\n\
  for(; i < SMRTPTR_GC.len; ++i) // find ptr in garbage collector\n\
    if(SMRTPTR_GC.ptrs[i] == ptr) {\n\
      free(ptr);\n\
      for(j = i; j < SMRTPTR_GC.len - 1; ++j) // shift ptrs down\n\
        SMRTPTR_GC.ptrs[j] = SMRTPTR_GC.ptrs[j + 1];\n\
      SMRTPTR_GC.len--;\n\
      return;\n\
    }\n\
}\n\
#endif\n\
/******************************* SMRTPTR.H END *******************************/"

/******************************************************************************
* MAIN EXECUTION
******************************************************************************/

int main(int argc, char *argv[]) {
  // confirm passed .c file in cmd line arg to declass
  if(argc<2 || argc>3 || (argv[argc-1][strlen(argv[argc-1])-2] != '.' && argv[argc-1][strlen(argv[argc-1])-1] != 'c')) 
    declass_missing_Cfile_alert();
  // determine if displaying class info at exit as per '-l' argv[1]
  bool show_class_info = false;
  if(argc == 3) {
    if(strcmp(argv[1], "-l") != 0) declass_missing_Cfile_alert(); // info flag != '-l'
    show_class_info = true;
  }

  // old & new file buffers, as well as filename
  char file_contents[MAX_FILESIZE], NEW_FILE[MAX_FILESIZE];
  char filename[100];
  FLOOD_ZEROS(file_contents, MAX_FILESIZE); FLOOD_ZEROS(NEW_FILE, MAX_FILESIZE);
  FLOOD_ZEROS(filename, 100);
  strcpy(filename, argv[argc-1]);
  confirm_valid_file(filename);

  FSCRAPE(file_contents, filename);
  char filler_array_argument[MAX_WORDS_PER_METHOD][75];
  int i = 0, j = 0;
  bool in_a_string = false;

  // wrap braces around single-line "braceless" if, else if, else, while, & for loops
  add_braces(file_contents);

  while(file_contents[i] != '\0') {
    // don't modify anything in strings
    if(file_contents[i] == '"' && file_contents[i-1] != '\\') in_a_string = !in_a_string;

    // store class info & convert to structs calling external fcns
    if(!in_a_string && is_at_substring(&file_contents[i], "class ")) 
      i += parse_class(&file_contents[i], NEW_FILE, &j);

    // store declared class object info
    for(int k = 0; !in_a_string && k < total_classes; ++k)
      if(is_at_substring(&file_contents[i], classes[k].class_name) 
        && !VARCHAR(file_contents[i+strlen(classes[k].class_name)]) && !VARCHAR(file_contents[i-1]))
        if(store_object_info(&file_contents[i])) { // assign default values

          // check if class is already initialized by user
          int already_assigned = i;
          while(file_contents[already_assigned] != '\0' && no_overlap(file_contents[already_assigned], "\n;,=")) 
            ++already_assigned;
          if(file_contents[already_assigned] == '=') break;

          // determine if object is invoking it's user-defined constructor
          char *user_ctor_finder = &file_contents[i];
          char user_ctor[500]; FLOOD_ZEROS(user_ctor, 500);
          bool is_fcn_returning_obj = false;
          bool user_ctor_invoked = get_user_ctor(user_ctor_finder, user_ctor, classes[k].class_name, &is_fcn_returning_obj);

          // don't splice in any constructors if "object" is actually a fcn returning an object
          if(is_fcn_returning_obj) break;

          // initialization undefined -- use default initial values
          while(file_contents[i] != '\0' && file_contents[i-1] != ';') NEW_FILE[j++] = file_contents[i++];
          if(objects[total_objects-1].is_class_array) // object = array, use macro init
            sprintf(&NEW_FILE[j], " DECLASS__%s_ARR(%s);", classes[k].class_name, objects[total_objects-1].object_name);
          else // object != array, so init via its class' global object
            sprintf(&NEW_FILE[j], " DECLASS__%s_CTOR(%s);", classes[k].class_name, objects[total_objects-1].object_name);
          j = strlen(NEW_FILE);

          // add user-defined ctor invocation w/ initialization values (if present)
          if(user_ctor_invoked) { sprintf(&NEW_FILE[j], " %s", user_ctor); j = strlen(NEW_FILE); }
          break;
        }

    // modify object invoking method to fcn call w/ a prepended class-converted-struct name
    if(!in_a_string && total_classes > 0) 
      i += parse_method_invocation(&file_contents[i], NEW_FILE, &j, false, filler_array_argument);

    // save non-class data to file
    NEW_FILE[j++] = file_contents[i++]; 
  }
  NEW_FILE[j] = '\0';

  // notify user declassification conversion completed
  declass_DECLASSIFIED_ascii_art();
  printf("%s ==DECLASSIFIED=> ", filename);
  NEW_EXTENSION(filename, "_DECLASS.c");
  printf("%s", filename);
  printf("\n=================================================================================\n");
  trim_sequential_spaces(NEW_FILE);
  
  char HEADED_NEW_FILE[MAX_FILESIZE]; FLOOD_ZEROS(HEADED_NEW_FILE, MAX_FILESIZE);

  // determine if ought to include smrtptr.h at top of file - as per whether
  // "#define DECLASS_NSMRTPTR" wasn't/was found
  if(!SMRT_PTRS) sprintf(HEADED_NEW_FILE,"/* DECLASSIFIED: %s\n * %s\n\n%s", 
    filename, DECLASS_SUPPORT_CONTACT, NEW_FILE);
  else sprintf(HEADED_NEW_FILE,"/* DECLASSIFIED: %s\n * %s\n%s\n\n%s", 
    filename, DECLASS_SUPPORT_CONTACT, DECLASS_SMART_POINTER_H_, NEW_FILE);
  
  // write newly converted/declassified file & notify success to user
  FPUT(HEADED_NEW_FILE,filename);
  if(show_class_info) show_l_flag_data();
  printf(" >> Terminating Declassifier.\n");
  printf("=============================\n\n");

  return 0;
}

/******************************************************************************
* BRACE FUNCTIONS
******************************************************************************/

// add braces around any "braceless" single-line conditionals & while/for loops
// ALSO ASSIGNS GLOBAL "SMRT_PTRS" flags for on/off smrtptr.h
void add_braces(char file_contents[]) {
  char BRACED_FILE[MAX_FILESIZE], *blanker;
  int k, i = 0, j = 0, in_brace_args = 0;
  FLOOD_ZEROS(BRACED_FILE, MAX_FILESIZE);
  BRACED_FILE[j++] = file_contents[i++]; // so "file_contents[i-1]" won't throw error
  bool in_a_string = false;
  SMRT_PTRS = true;

  // remove commments from program, & the initial '/' if "file_contents" starts with a comment:
  blanker = file_contents;
  if(file_contents[0] == '/' && (file_contents[1] == '/' || file_contents[1] == '*')) BRACED_FILE[0] = ' ';
  whitespace_all_comments(blanker); // ensures braces & DECLASS_NSMRTPTR flag applied correctly

  while(file_contents[i] != '\0') {
    if(file_contents[i] == '"' && file_contents[i-1] != '\\') in_a_string = !in_a_string;
    // check whether user disabled smrtptr.h: "#define DECLASS_NSMRTPTR"
    if(!in_a_string && file_contents[i] == '\n' && SMRT_PTRS) {
      int l = i;
      while(IS_WHITESPACE(file_contents[l])) ++l;
      if(file_contents[l] == '#') {
        ++l;
        while(IS_WHITESPACE(file_contents[l])) ++l;
        if(is_at_substring(&file_contents[l], "define")) {
          l += strlen("define");
          while(IS_WHITESPACE(file_contents[l])) ++l;
          if(is_at_substring(&file_contents[l], "DECLASS_NSMRTPTR") 
            && !VARCHAR(file_contents[l+strlen("DECLASS_NSMRTPTR")])) { 
            SMRT_PTRS = false, i = l + strlen("DECLASS_NSMRTPTR");
            sprintf(&BRACED_FILE[j], "\n#define DECLASS_NSMRTPTR"); 
            j = strlen(BRACED_FILE); continue;
          }
        }
      }
    }

    // check for else if, if, else, while, & for
    for(k = 0; !in_a_string && k < TOTAL_BRACE_KEYWORDS; ++k)
      if(is_at_substring(&file_contents[i],brace_keywords[k]) && !VARCHAR(file_contents[i-1]) 
        && !VARCHAR(file_contents[i+strlen(brace_keywords[k])])) {                    // at a "brace keyword"
        for(int l = 0, len = strlen(brace_keywords[k]); l < len; ++l)                 // skip brace keyword
          BRACED_FILE[j++] = file_contents[i++]; 
        while(IS_WHITESPACE(file_contents[i]))                      // skip optional space btwn keyword & '('
          BRACED_FILE[j++] = file_contents[i++];
        if(file_contents[i] == '(' || strcmp(brace_keywords[k], "else") == 0) {       // actual brace keyword
          if(file_contents[i] == '(') {                                               // not else
            BRACED_FILE[j++] = file_contents[i++];                                    // move past '('
            in_brace_args = 1;
            while(file_contents[i] != '\0' && in_brace_args > 0) {                    // copy brace keywords args
              if(file_contents[i] == '"' && file_contents[i-1] != '\\') in_a_string = !in_a_string;
              if(file_contents[i] == '(')      ++in_brace_args;
              else if(file_contents[i] == ')') --in_brace_args;
              BRACED_FILE[j++] = file_contents[i++]; 
            }
          }
          while(file_contents[i] != '\0' && IS_WHITESPACE(file_contents[i])) { // skip to 1st st8ment after brace keyword
            if(file_contents[i] == '"' && file_contents[i-1] != '\\') in_a_string = !in_a_string;
            BRACED_FILE[j++] = file_contents[i++]; 
          }
          if(file_contents[i] == ';' || file_contents[i] == '{'                // braced keyword or do-while loop
          ||(file_contents[i]=='d'&&file_contents[i+1]=='o'&&!VARCHAR(file_contents[i+2]))) break;
          BRACED_FILE[j++] = '{';                                              // add brace
          while(file_contents[i] != '\0' && file_contents[i-1] != ';') {       // copy single-line conditional
            if(file_contents[i] == '"' && file_contents[i-1] != '\\') in_a_string = !in_a_string;
            BRACED_FILE[j++] = file_contents[i++]; 
          }
          BRACED_FILE[j++] = '}';                                              // add brace
        }
        break;                                                                 // found brace keyword, don't check others
      }
    BRACED_FILE[j++] = file_contents[i++];
  }
  BRACED_FILE[j] = '\0';
  FLOOD_ZEROS(file_contents, MAX_FILESIZE);
  strcpy(file_contents, BRACED_FILE); // use "braced" file variant as official file
}

/******************************************************************************
* MESSAGE FUNCTIONS
******************************************************************************/

// confirms file exists, non-empty, & takes less memory than MAX_FILESIZE
void confirm_valid_file(char *filename) {
  struct stat buf;
  if(stat(filename, &buf)) {
    fprintf(stderr, "-:- FILE \"%s\" DOES NOT EXIST! -:-\n\n", filename);
    exit(EXIT_FAILURE);
  }
  if(buf.st_size > MAX_FILESIZE || buf.st_size == 0) {
    if(buf.st_size > MAX_FILESIZE) {
      fprintf(stderr, "-:- FILE \"%s\" SIZE %lld BYTES EXCEEDS %d BYTE CAP! -:- \n",filename,buf.st_size,MAX_FILESIZE); 
      fprintf(stderr, "-:- RAISE 'MAX_FILESIZE' MACRO LIMIT! -:- \n\n");
    } else fprintf(stderr, "-:- CAN'T DECLASSIFY AN EMPTY FILE! -:- \n\n"); 
    exit(EXIT_FAILURE);
  }
}

// 'declassified' in ascii
void declass_DECLASSIFIED_ascii_art() {
  printf("\n=================================================================================\n");
  printf("||^\\\\  /|===\\ //==\\ ||     //^\\\\    //==/ //==/ ==== |===\\ ==== /|===\\ ||^\\\\   //\n");
  printf("||  )) ||==   ||    ||    |/===\\|   \\\\    \\\\     ||  |==    ||  ||==   ||  )) //\n");
  printf("||_//  \\|===/ \\\\==/ |===/ ||   || /==// /==//   ==== ||    ==== \\|===/ ||_// <*>");
  printf("\n=================================================================================\n");
}

// error & how-to-execute message
void declass_missing_Cfile_alert() {
  printf("\n========================================\n");
  printf("  /|===\\ ||^\\\\ ||^\\\\ //==\\\\ ||^\\\\   //\n");
  printf("  ||==   ||_// ||_// ||  || ||_//  //\n");
  printf("  \\|===/ || \\\\ || \\\\ \\\\==// || \\\\ <*>");
  printf("\n========================================");
  printf("\n** Missing .c File Cmd Line Argument! **\n");
  printf("Execution:  $ gcc -o declass declass.c\n            $ ./declass yourCFile.c");
  printf("\n========================================");
  printf("\n*** Or Else: Misused '-l' Info Flag! ***\n");
  printf("Execution:  $ ./declass -l yourCFile.c");
  printf("\n========================================");
  printf("\n********* Filename Conversion: *********\n"); 
  printf("yourCFile.c ==> yourCFile_DECLASSIFIED.c");
  printf("\n========================================\n");
  printf(">> Terminating Declassifier.");
  printf("\n============================\n\n");
  exit(EXIT_FAILURE);
}

// output class data if argv[1] == '-l' flag
void show_l_flag_data() {
  if(total_classes > 0) printf("\n--=[ TOTAL CLASSES: %d ]=--", total_classes);
  (total_objects > 0) ? printf("=[ TOTAL OBJECTS: %d ]=--\n", total_objects) : printf("\n");
  for(int i = 0; i < total_classes; ++i) {
    int class_objects_sum = 0;
    for(int j = 0; j < total_objects; ++j) 
      if(strcmp(classes[i].class_name, objects[j].class_name) == 0) class_objects_sum++;
    printf("\nCLASS No%d, %s:\n", i + 1, classes[i].class_name);

    int total_members = classes[i].total_members; // differentiate between class struct member members & class members
    for(int j = 0; j < classes[i].total_members; ++j) if(classes[i].member_names[j][0] == 0) --total_members;
    if(total_members > 0) {
      printf(" L_ MEMBERS: %d\n", total_members);
      for(int j = 0; j < classes[i].total_members; ++j) {
        if(classes[i].member_names[j][0] == 0) continue;
        char bar = (classes[i].total_methods > 0) ? '|' : ' ';
        if(classes[i].member_is_pointer[j])    printf(" %c  L_ *%s", bar, classes[i].member_names[j]);
        else if(classes[i].member_is_array[j]) printf(" %c  L_ %s[]", bar, classes[i].member_names[j]);
        else printf(" %c  L_ %s", bar, classes[i].member_names[j]);
        if(classes[i].member_value_is_alloc[j]) printf(" (( ALLOCATED MEMORY ))");
        printf("\n");
      }
    }

    char method_name[150];
    if(classes[i].total_methods > 0) {
        printf(" L_ METHODS: %d\n", classes[i].total_methods);
      for(int j = 0; j < classes[i].total_methods; ++j) {
        FLOOD_ZEROS(method_name, 150);
        if(strcmp(classes[i].method_names[j], "DECLASS__constructor") == 0)
          sprintf(method_name, "%s() (( CONSTRUCTOR ))", classes[i].class_name);
        else sprintf(method_name, "%s()", classes[i].method_names[j]);
        (class_objects_sum > 0) ? printf(" | L_ %s\n", method_name) : printf("   L_ %s\n", method_name);
      }
    }

    if(class_objects_sum > 0) {
      printf(" L_ OBJECTS: %d\n", class_objects_sum);
      for(int j = 0; j < total_objects; ++j) 
        if(strcmp(classes[i].class_name, objects[j].class_name) == 0) {
          if(objects[j].is_class_pointer)    printf("   L_ *%s\n", objects[j].object_name);
          else if(objects[j].is_class_array) printf("   L_ %s[]\n", objects[j].object_name);
          else printf("   L_ %s\n", objects[j].object_name);
        }
    }
  }
  printf("\n=============================\n");
}

/******************************************************************************
* COMMENT & BLANK LINE SKIPPING FUNCTIONS
******************************************************************************/

// replaces all comments inside of a class instance with spaces
void whitespace_all_comments(char *end) { // end = 1 past 1st '{'
  int in_class_scope = 1;
  bool in_a_string = false;
  while(*end != '\0' && in_class_scope > 0) {
    // confirm in class' scope
    if(*end == '{') in_class_scope++;
    else if(*end == '}') in_class_scope--;
    else if(*end == '"' && *(end-1) != '\\') in_a_string = !in_a_string;
    if(in_class_scope < 0) break;
    if(!in_a_string && *end == '/' && *(end + 1) == '/') {
      while(*end != '\0' && *end != '\n') *end++ = ' ';
    } else if(!in_a_string && *end == '/' && *(end + 1) == '*') {
      *end++ = ' ', *end++ = ' ';
      while(*end != '\0' && (*end != '*' || *(end + 1) != '/')) *end++ = ' ';
      *end++ = ' ', *end++ = ' ';
    }
    end++;
  }
}

// trims any sequences of spaces ended by ('\n' || ';') to just ('\n' || ';') in "OLD_BUFFER"
void trim_sequential_spaces(char OLD_BUFFER[]) {
  char *read = OLD_BUFFER, NEW_BUFFER[MAX_FILESIZE], *scout, *write;
  FLOOD_ZEROS(NEW_BUFFER, MAX_FILESIZE);
  bool in_a_string = false;
  write = NEW_BUFFER;
  *write++ = *read++;                                   // so first string check doesn't check garbage memory
  while(*read != '\0') {
    if(*read == '"' && *(read-1) != '\\') in_a_string = !in_a_string;
    if(!in_a_string && *read == ' ') {                  // trims space(s) + ('\n' || ';') sequence
      scout = read;
      while(*scout != '\0' && *scout == ' ') ++scout;   // skip over spaces
      if(*scout == '\n' || *scout == ';') read = scout; // if correct format, passover spaces
    }
    *write++ = *read++;
  }
  *write = '\0';
  FLOOD_ZEROS(OLD_BUFFER, MAX_FILESIZE);
  strcpy(OLD_BUFFER, NEW_BUFFER);
}

// gives size to skip of lines composed solely of whitespaces with '\n's at either end
int remove_blank_lines(char *end) {
  char *blank_char = end + 1;               // 'end' starts at '\n'
  int total_blank_chars = 0;
  while(true) {                             // while still more blank lines
    int blank_line_size = 1;
    while(*blank_char != '\0' && IS_WHITESPACE(*blank_char) && *blank_char != '\n')
      blank_char++, blank_line_size++;      // traverse up to next '\n' or non-whitespace
    if(*blank_char == '\n') {               // if an entire line of only whitespaces
      total_blank_chars += blank_line_size; // increment skipping-size by blank line's length
      blank_char++;                         // move to next line/char
    } else return total_blank_chars;
  }
}

/******************************************************************************
* STRING HELPER FUNCTIONS
******************************************************************************/

// returns whether c exists in '*bad_chars'
bool no_overlap(char c, char *bad_chars) {
  while(*bad_chars != '\0') if(*bad_chars++ == c) return false;
  return true;
}

// returns if 'p' points to the particular substring 'substr'
bool is_at_substring(char *p, char *substr) {
  for(int i = 0, len = strlen(substr); i < len; ++i) if(p[i] != substr[i]) return false;
  return true;
}

/******************************************************************************
* OBJECT CONSTRUCTION FUNCTIONS
******************************************************************************/

// given a class index, returns an initialization brace for it's member values
void mk_initialization_brace(char brace[], int class_index) {
  char *p = brace;
  *p++ = '{';
  for(int j = 0; j < classes[class_index].total_members; ++j) {
    if(classes[class_index].member_values[j][0] == 0) { // empty value
      // if struct's member (struct inner members' name = value = 0), skip
      if(classes[class_index].member_names[j][0] == 0) continue;
      else if(classes[class_index].member_is_array[j] || (j > 0 && classes[class_index].member_names[j-1][0] == 0)
        || (classes[class_index].member_object_class_name[j][0] != 0 && !classes[class_index].member_is_pointer[j]))
          sprintf(p, "{0},"); // wrap empty (non-ptr) array/object/struct value in braces
      else if((j > 0 && classes[class_index].member_names[j-1][0] != 0) || j == 0) 
        sprintf(p, "0,"); 
    } else sprintf(p, "%s,", classes[class_index].member_values[j]); // non-empty value
    p += strlen(p);
  }
  *p++ = '}';
  *p = '\0';
}

// fills 'ctor_macros' string w/ macros for both single & array object constructions/initializations
void mk_ctor_macros(char ctor_macros[], char *class_name) {
  // add macro for a single object construction instance
  sprintf(ctor_macros, "#define DECLASS__%s_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__%s_DFLT();", 
    class_name, class_name);
  int macro_idx = strlen(ctor_macros);
  // search for members that are also other class objects
  for(int l = 0; l < classes[total_classes].total_members; ++l) {
    if(classes[total_classes].member_object_class_name[l][0] != 0) { // member = class object
      // append macros to initialize any members that are class objects
      if(classes[total_classes].member_is_array[l] && !classes[total_classes].member_is_pointer[l]) { // append arr ctor
        sprintf(&ctor_macros[macro_idx], "\\\n\tDECLASS__%s_ARR(DECLASS_THIS.%s);", 
          classes[total_classes].member_object_class_name[l], classes[total_classes].member_names[l]);
      } else if(!classes[total_classes].member_is_pointer[l]) {                                       // append 1 obj ctor
        sprintf(&ctor_macros[macro_idx], "\\\n\tDECLASS__%s_CTOR(DECLASS_THIS.%s);", 
          classes[total_classes].member_object_class_name[l], classes[total_classes].member_names[l]);
      }
      macro_idx = strlen(ctor_macros);
      // check for any user-defined array ctors (all macros by default) that ought to be invoked
      // to be invoked outside of the brace initialization
      if(classes[total_classes].member_value_user_ctor[l][0] != 0) {
        sprintf(&ctor_macros[macro_idx], "\\\n\t%s;", classes[total_classes].member_value_user_ctor[l]);
        macro_idx = strlen(ctor_macros);
      }
    }
  }
  sprintf(&ctor_macros[macro_idx], "})");
  macro_idx = strlen(ctor_macros);

  // add macro for a an array of object constructions
  sprintf(&ctor_macros[macro_idx], "\n#define DECLASS__%s_ARR(DECLASS_ARR) ({\\\n\
  for(int DECLASS__%s_IDX=0;DECLASS__%s_IDX<(sizeof(DECLASS_ARR)/sizeof(DECLASS_ARR[0]));++DECLASS__%s_IDX)\\\n\
    DECLASS__%s_CTOR(DECLASS_ARR[DECLASS__%s_IDX]);\\\n})", 
  class_name, class_name, class_name, class_name, class_name, class_name);

  // add a macro to invoke user-defined ctors for object arrays
  if(classes[total_classes].class_has_ctor) {
    macro_idx = strlen(ctor_macros);
    sprintf(&ctor_macros[macro_idx], "\n#define DECLASS__%s_USERCTOR_ARR(DECLASS_ARR", class_name);
    macro_idx = strlen(ctor_macros);
    // if has args, sprintf arg for "__VA_ARGS__"
    if(classes[total_classes].class_has_ctor_args) {
      sprintf(&ctor_macros[macro_idx], ", ..."); macro_idx = strlen(ctor_macros);
    }
    // sprintf loop to iterate over object array's individual objects
    sprintf(&ctor_macros[macro_idx], ") ({\\\n\
  for(int DECLASS__%s_USERCTOR_IDX=0;DECLASS__%s_USERCTOR_IDX<(sizeof(DECLASS_ARR)/sizeof(DECLASS_ARR[0]));\
++DECLASS__%s_USERCTOR_IDX)\\\n\
    DECLASS_%s_(", class_name, class_name, class_name, class_name);
    macro_idx = strlen(ctor_macros);
    // if has args, sprintf arg for "__VA_ARGS__"
    if(classes[total_classes].class_has_ctor_args) { 
      sprintf(&ctor_macros[macro_idx], "__VA_ARGS__, "); macro_idx = strlen(ctor_macros);
    }
    // pass object in object array to user-defined ctor
    sprintf(&ctor_macros[macro_idx], "&DECLASS_ARR[DECLASS__%s_USERCTOR_IDX]);\\\n})", class_name); 
  }
}

// make global initializing function to assign default values
void mk_class_global_initializer(char *class_global_initializer, char *class_name, char *initial_values_brace) {
  sprintf(class_global_initializer, "\n%s DECLASS__%s_DFLT(){\n\t%s this=%s;\n\treturn this;\n}", 
    class_name, class_name, class_name, initial_values_brace);
}

/******************************************************************************
* USER-DEFINED OBJECT CONSTRUCTOR PARSING FUNCTIONS
******************************************************************************/

// returns whether object declaration is invoking a user-defined constructor
// & fills the "user_ctor" string w/ the invocation if so
bool get_user_ctor(char *p, char *user_ctor, char *class_name, bool *is_fcn_returning_obj) {
  while(*p != '\0' && no_overlap(*p, "\n(;,=")) ++p;
  if(*p != '(') return false; // object not being constructed via user-defined ctor
  char ctor_vals[MAX_DEFAULT_VALUE_LENGTH]; FLOOD_ZEROS(ctor_vals, MAX_DEFAULT_VALUE_LENGTH);
  int val_idx = 0, in_ctor_args = 1;
  bool in_a_string = false;

  // copy inner ctor args to "ctor_vals" & white-out "p" buffer's "(<args>)" w/ spaces
  char *whiteout_start = p;
  ++p;                                                          // move past first '('
  while(*p != '\0' && in_ctor_args > 0) {
    if(*p == '"' && *(p-1) != '\\') in_a_string = !in_a_string; // account for whether in string
    if(!in_a_string && *p == '(')      ++in_ctor_args;          // confirm still in ctor arg scope
    else if(!in_a_string && *p == ')') --in_ctor_args;
    if(in_ctor_args == 0) break;                                // if out of scope
    ctor_vals[val_idx++] = *p++;                                // copy ctor args
  }
  ctor_vals[val_idx] = '\0';
  if(*p == ')') ++p;                                            // move past last ')'

  // check whether p is actually pointing to a fcn that returns an obj
  char *q = p;
  while(*q != '\0' && no_overlap(*q, ";{")) ++q;
  if(*q == '{') {
    *is_fcn_returning_obj = true;
    return false;
  }

  // not a fcn returning an obj, thus whiteout user's ctor args that're being spliced out
  while(whiteout_start != p) *whiteout_start++ = ' ';
  // create user-defined ctor invocation
  if(objects[total_objects-1].is_class_array) { // invoke ctor for each object in object array declaration
    if(strlen(ctor_vals) > 0)
      sprintf(user_ctor, "DECLASS__%s_USERCTOR_ARR(%s, %s);",class_name,objects[total_objects-1].object_name,ctor_vals);
    else sprintf(user_ctor, "DECLASS__%s_USERCTOR_ARR(%s);", class_name, objects[total_objects-1].object_name);
  } else {                                      // invoke ctor for single object declaration
    if(strlen(ctor_vals) > 0)
      sprintf(user_ctor, "DECLASS_%s_(%s, &%s);", class_name, ctor_vals, objects[total_objects-1].object_name);
    else sprintf(user_ctor, "DECLASS_%s_(&%s);", class_name, objects[total_objects-1].object_name);
  }
  return true;
}

// check for a user-defined constructor invocation
char *check_for_ctor_obj(char*end,char*struct_buff_idx,char*class_instance,int*class_size,int*struct_inc,bool*found_ctor){
  char *is_ctor = end;
  bool in_a_string = false;
  *found_ctor = false;
  *struct_inc = 0;
  // from initial '(', traverse the rest of the line & confirm ends w/ ';' not '{'
  while(*is_ctor != '\0' && *is_ctor != '\n') { 
    if(*end == '"' && *(end-1) != '\\') in_a_string = !in_a_string;
    if(!in_a_string && (*is_ctor == ';' || *is_ctor == '{')) break; // either method or ctor
    ++is_ctor;
  }
  
  // obj ctor as default val for an obj member, NOT a method declaration
  if(*is_ctor == ';') { 
    *found_ctor = true;
    char ctored_class[75], ctored_obj[75], appended_ctor_object[100];
    char *ctored_class_ptr = ctored_class, *ctored_obj_ptr = ctored_obj;
    char *ctoring_invokers = end - 1, *check_no_ctor_args = is_ctor;
    FLOOD_ZEROS(ctored_obj, 75); FLOOD_ZEROS(ctored_class, 75); 
    FLOOD_ZEROS(appended_ctor_object, 100);
    while(no_overlap(*ctoring_invokers, "\n;")) --ctoring_invokers;               // move to the end of the line
    while(!VARCHAR(*ctoring_invokers)) ++ctoring_invokers;                        // move to class name "type"
    while(VARCHAR(*ctoring_invokers))  *ctored_class_ptr++ = *ctoring_invokers++; // copy class of object being ctor'd
    while(!VARCHAR(*ctoring_invokers)) ++ctoring_invokers;                        // move to object name
    while(VARCHAR(*ctoring_invokers))  *ctored_obj_ptr++ = *ctoring_invokers++;   // copy object name
    *ctored_class_ptr = '\0', *ctored_obj_ptr = '\0';

    // check whether object ctor is for a single or array of objects
    char *check_array = end - 1, ctor_fcn_assignment[300];
    FLOOD_ZEROS(ctor_fcn_assignment, 300);
    while(IS_WHITESPACE(*check_array)) --check_array;
    bool array_ctor = (*check_array == ']');
    if(array_ctor) 
      sprintf(ctor_fcn_assignment, " = DECLASS__%s_USERCTOR_ARR", ctored_class); 
    else sprintf(ctor_fcn_assignment, " = DECLASS_%s_", ctored_class); 
    
    // check whether ctor default value has args
    while(*check_no_ctor_args != ')') --check_no_ctor_args;         // find end of ctor
    --check_no_ctor_args;                                           // move past last ')'
    while(IS_WHITESPACE(*check_no_ctor_args)) --check_no_ctor_args; // if directly preceded by '(' ctor = empty
    if(*check_no_ctor_args == '(') {                                // splice "&DECLASS_THIS.objName" to end of ctor args
      if(array_ctor)      sprintf(appended_ctor_object, "DECLASS_THIS.%s", ctored_obj);
      else                sprintf(appended_ctor_object, "&DECLASS_THIS.%s", ctored_obj);
    } else if(array_ctor) sprintf(appended_ctor_object, "DECLASS_THIS.%s, ", ctored_obj);
    else                  sprintf(appended_ctor_object, ", &DECLASS_THIS.%s", ctored_obj);
    char *LAST_ARG = check_no_ctor_args, *FIRST_ARG = end;
    
    // confirm class of object with default ctor value exists
    int is_a_defined_class = 0;
    for(; is_a_defined_class < total_classes + 1; ++is_a_defined_class)
      if(strcmp(ctored_class, classes[is_a_defined_class].class_name) == 0) break;
    if(is_a_defined_class == total_classes + 1) {
      fprintf(stderr, "-:- WARNING: UNDEFINED BEHAVIOR IN DECLASS.C FCN: \"%s\", LINE: %d -:-\n", __func__, __LINE__);
      fprintf(stderr, "-:- EXPECTED OBJECT CTOR FOR MEMBER: \"%s\" IN CLASS: \"%s\" -:-\n", ctored_obj, ctored_class);
      fprintf(stderr, "-:- CHARACTER NUMBER IN FILE BEING PARSED: %ld -:-\n", end - class_instance + 1);
      fprintf(stderr, "-:- CONTINUING DECLASSIFICATION PROCESS - I HOPE YOU KNOW WHAT YOU'RE DOING -:-\n\n");
      return end;
    }

    // shift & splice in last "&DECLASS_THIS.objName" arg && the ctor functional invocation by assignment
    char *ctor_fcn_assign_ptr = ctor_fcn_assignment, *ctor_append_ptr = appended_ctor_object;
    int ctor_fcn_assign_len   = strlen(ctor_fcn_assignment);
    int ctor_appened_arg_len  = strlen(appended_ctor_object);
    char *endOfFile = end + strlen(end) + ctor_fcn_assign_len; // end of the file + fcnal ctor assign length
    *(endOfFile + 1) = '\0';

    // shift file up then splice in functional ctor invocation by assignment
    // "+ 1" to also shift over the 1st '(' preceding ctor's args
    while(endOfFile - ctor_fcn_assign_len + 1 != end) { 
      *endOfFile = *(endOfFile - ctor_fcn_assign_len); --endOfFile;
    }
    while(*ctor_fcn_assign_ptr != '\0') { // splice in ctor invocation
      *end++ = *ctor_fcn_assign_ptr, *class_size += 1;
      *struct_buff_idx++ = *ctor_fcn_assign_ptr++, *struct_inc += 1;
    }

    // skip to where ought to splice in last "&DECLASS_THIS.objName" ctor arg
    // "+ctor_fcn_assign_len+1" accounts for shifted distance & to move "end" 1 past 1st/last "arg_position"
    // as whether is/isn't an object array ctor
    char *arg_position = (array_ctor) ? FIRST_ARG : LAST_ARG;
    while(end != arg_position + ctor_fcn_assign_len + 1) 
      *struct_buff_idx++ = *end++, *class_size += 1, *struct_inc += 1;
    endOfFile = end + strlen(end) + ctor_appened_arg_len; // end of the file + appended last arg

    // shift file up then splice in last "&DECLASS_THIS.objName" ctor arg
    // + 1 to also shift over char currently in last arg's position to make room for splicing in "&DECLASS_THIS.objName"
    while(endOfFile - ctor_appened_arg_len + 1 != end) { 
      *endOfFile = *(endOfFile - ctor_appened_arg_len); --endOfFile;
    }
    while(*ctor_append_ptr != '\0') { // splice in ctor's last "&DECLASS_THIS.objName" arg
      *end++ = *ctor_append_ptr, *class_size += 1;
      *struct_buff_idx++ = *ctor_append_ptr++, *struct_inc += 1;
    }
  }
  return end;
}

/******************************************************************************
* OBJECT METHOD PARSER
******************************************************************************/

// parse object method invocations -- 'method_words' only meaningful when 'is_nested_method' == true
int parse_method_invocation(char *s, char *NEW_FILE, int *j, bool is_nested_method, char method_words[][75]) {
  char *p = s, first_char = *s, new_fcn_call[75];
  FLOOD_ZEROS(new_fcn_call, 75);
  int method_name_size = 0;
  if(!(VARCHAR(*p)) && (VARCHAR(*(p + 1)))) {                                  // may be object
    p++;
    for(int i = 0; i < total_objects; ++i) {                                   // check for each object
      int len = strlen(objects[i].object_name);
      if(strlen(p) > len && is_at_substring(p, objects[i].object_name)) {      // found object
        int invoker_size = is_method_invocation(p + len);
        if(invoker_size == 0) continue;                                        // no invocation notation (no '.' nor '->')
        char invoked_member_name[75];
        get_invoked_member_name(p + len + invoker_size, invoked_member_name);  // get member name
        if(invoked_member_is_method(invoked_member_name, objects[i].class_name, is_nested_method)) { // member = method
          while((invoker_size = is_method_invocation(p)) > 0 || VARCHAR(*p)) { // move p to after object & method names
            if(invoker_size == 0) invoker_size = 1; // VARCHAR
            p += invoker_size, method_name_size += invoker_size;
          }
          method_name_size++;                                                  // for 1st char ('%c' in sprintf below)
          sprintf(new_fcn_call, "%cDECLASS_%s_%s", first_char, objects[i].class_name, invoked_member_name);

          // whether method is invoked within another method, but splice in either way
          if(is_nested_method)
            splice_in_prepended_NESTED_method_name(new_fcn_call, p, i, NEW_FILE, &method_name_size, method_words);
          else
            splice_in_prepended_method_name(new_fcn_call, p, i, NEW_FILE, j, &method_name_size, method_words);
          break;
        }
      }
    }
  } else method_name_size = 0;
  return method_name_size;
}

// splice in prepended method name to buffer via an index subscript
void splice_in_prepended_method_name(char*new_fcn_call,char*p,int i,char*NEW_FILE,int*j,
                                     int*method_name_size,char method_words[][75]) {
  char objectName[200], objectChain[200]; // 'objectName' refers to outermost object in 'objectChain
  bool objectName_is_pointer = false;
  FLOOD_ZEROS(objectName, 200); FLOOD_ZEROS(objectChain, 200); 
  get_object_name(objectName, objectChain, p, i, method_words, false, &objectName_is_pointer);

  // remove invoking object's container object chain-prefix from 'NEWFILE' (if present)
  char *rmv_buffer_objectChain = &NEW_FILE[*j];
  rmv_excess_buffer_objectChain(objectName, objects[i].object_name, objectChain, rmv_buffer_objectChain, new_fcn_call);
  *j = strlen(NEW_FILE);

  // splice in prefixed method name & add invoker's address to end of arguments
  APPEND_BUFF_OR_STR_TO_NEW_FILE(new_fcn_call);
  while(*p != '\0' && *p != ')') NEW_FILE[(*j)++] = *p++, (*method_name_size)++;
  if(*(p - 1) != '(') NEW_FILE[(*j)++] = ',', NEW_FILE[(*j)++] = ' ';
  if(objectName_is_pointer) sprintf(&NEW_FILE[*j], "%s", objectChain);  // splice in ptr obj address
  else                      sprintf(&NEW_FILE[*j], "&%s", objectChain); // splice in val obj address
  *j = strlen(NEW_FILE);
}

// splice in prepended method name to buffer via a pointer
void splice_in_prepended_NESTED_method_name(char*new_fcn_call,char*p,int i,char*method_buff_idx,
                                            int*method_name_size,char method_words[][75]){
  char objectName[200], objectChain[200]; // 'objectName' refers to outermost object in 'objectChain
  bool objectName_is_pointer = false;
  FLOOD_ZEROS(objectName, 200); FLOOD_ZEROS(objectChain, 200);
  get_object_name(objectName, objectChain, p, i, method_words, true, &objectName_is_pointer);

  // remove invoking object's container object chain-prefix from 'method_buff_idx' (if present)
  rmv_excess_buffer_objectChain(objectName, objects[i].object_name, objectChain, method_buff_idx, new_fcn_call);
  while(*method_buff_idx == '\0') --method_buff_idx; // move back to 1st-'\0' sans-chain position
  ++method_buff_idx;                                 // move up to '\0'
  // if still need to remove 'this->' prefix
  if(is_at_substring(method_buff_idx - 6, "this->")) 
    for(int k = 0; k < 6; ++k) *method_buff_idx-- = '\0';

  // if the outermost 'objectName' object in 'objectChain' is itself also a member of the current class
  for(int idx = 0; idx < classes[total_classes].total_members; ++idx) {
    if(classes[total_classes].member_names[idx][0] == 0) continue; // struct member -- disregard
    if(strcmp(classes[total_classes].member_names[idx], objectName) == 0) {
      FLOOD_ZEROS(objectName, 200);                   // empty out object name
      sprintf(objectName, "(this->%s)", objectChain); // write prefixed-objectChain to objectName 
      FLOOD_ZEROS(objectChain, 200);                  // empty objectChain
      strcpy(objectChain, objectName);                // refill object with it's prefixed self
      break;
    }
  }

  // add prefixed method name
  sprintf(method_buff_idx, "%s", new_fcn_call); 
  method_buff_idx += strlen(method_buff_idx);
  // copy method argument & prefix 'this->' for any local members w/in
  p += prefix_local_members_and_cpy_method_args(p, method_buff_idx, method_words, method_name_size, ')');
  method_buff_idx += strlen(method_buff_idx);
  if(*(p - 1) != '(') *method_buff_idx++ = ',', *method_buff_idx++ = ' ';
  if(objectName_is_pointer) sprintf(method_buff_idx, "%s", objectChain);  // splice in ptr obj address
  else                      sprintf(method_buff_idx, "&%s", objectChain); // splice in val obj address
}

// removes the excess front of the objectChain from 'buffer', occuring if 
// the object invoking method is also invoked by another container object
void rmv_excess_buffer_objectChain(char*objectName,char*invokingObject,char*objectChain,char*buffer,char*new_fcn_call){
  if(strcmp(objectName, invokingObject) != 0) { // again, 'objectName' refers to outermost object in 'objectChain'
    // to get rid of "first_char" (possible method invoker) left in from "parse_method_invocation()"
    char *temp = new_fcn_call;
    while(*temp != '\0') *temp = *(temp + 1), ++temp;
    // removes excess/left-over objectChain from 'buffer'
    char objectChain_clone[200]; // used to get length values to rmv from 'buffer' w/o the altering original objectChain
    FLOOD_ZEROS(objectChain_clone, 200);
    strcpy(objectChain_clone, objectChain);
    char *excess_chain = &objectChain_clone[strlen(objectChain_clone) - 1];    // traverse chain up to first invoker
    if(*excess_chain == ']') while(*(excess_chain + 1) != '[') --excess_chain; // skip first subscript
    while(VARCHAR(*excess_chain)) --excess_chain;                              // skip 'invokingObject' method invoker
    *excess_chain = '\0';
    int chain_length = strlen(objectChain_clone);                              // how many chars to rmv from 'buffer'
    for(int k = 0; k < chain_length; ++k) *buffer-- = '\0';                    // rmv excess chained objects
    *buffer = '\0';
  }
}

/******************************************************************************
* OBJECT METHOD PARSER HELPER FUNCTIONS
******************************************************************************/

// returns length of '.' or '->' invocation (0 if none) + length of array 
// subscript (if present) for invocation by a class w/in a class array
int is_method_invocation(char *p) {
  #define after_arr(p) (p + array_arg_size)
  int invoker_size = 0, array_arg_size = 1;
  // has (or is in) an array subscript
  char *open_bracket = p, *close_bracket = p; 
  while(*open_bracket != '\0' && no_overlap(*open_bracket, "[;\n")) --open_bracket;
  while(*close_bracket != '\0' && no_overlap(*close_bracket, "];\n")) ++close_bracket, ++array_arg_size;
  if(*open_bracket != '[' || *close_bracket != ']') array_arg_size = 0;

  // period member invocation
  if(*p == '.' || *after_arr(p) == '.') return array_arg_size + 1;      
  while(*after_arr(p) != '\0' && *after_arr(p) == ' ') invoker_size++, p++;

  // arrow (may have whitespace on either side)
  if((*after_arr(p) == '-' && *(after_arr(p)+1) == '>') || (*after_arr(p) == '>' && *(after_arr(p)-1) == '-')) {
    p += 2, invoker_size += 2;
    while(*after_arr(p) != '\0' && *after_arr(p) == ' ') invoker_size++, p++;
  } else invoker_size = array_arg_size = 0; // none
  return invoker_size + array_arg_size;
  #undef after_arr
}

// return whether object's invoked member is a class method
bool invoked_member_is_method(char *invoked_member_name, char *class_name, bool is_nested_method) {
  // '+ is_nested_method' b/c nested methods processed prior to total_classes++
  for(int i = 0; i < total_classes + is_nested_method; ++i) 
    if(strcmp(classes[i].class_name, class_name) == 0)
      for(int j = 0; j < classes[i].total_methods; ++j)
        if(strcmp(classes[i].method_names[j], invoked_member_name) == 0) return true;
  return false;
}

 // return object's invoked 'class' member
void get_invoked_member_name(char *member_ptr, char *invoked_member_name) {
  int i = 0;
  while(*member_ptr != '\0' && VARCHAR(*member_ptr)) 
    invoked_member_name[i++] = *member_ptr++;
  invoked_member_name[i] = '\0';
}

// returns outmost object's name, the entire object invocation chain (w/ prefixed array
// subscripts as needed) as well as whether the outermost object is a pointer
// 'is_nested_object' indicates parsing a nested method-within-a-method invocation
void get_object_name(char*outerMost_objectName,char*objectChain,char*buffer,int i,
                     char method_words[][75],bool is_nested_object,bool*objectName_is_pointer) {
  // * outerMost_objectName is the name - w/o subscripts - of the outermost object leading the object chain of invocation
  // * the objectChain consists of an optional outer container object that contains the method-invoking
  //   object, as well as any potential array subscripts for either the container &/or invoking object.

  // find the chain's start, going past array subscripts, invocation notations, & object names.
  // assumes array subscripts are correct & user follows caveat (8) (will splice 'this->' as needed in subscript though)
  char *chain_head = buffer - 1, *chain_tail = buffer - 1;
  int in_a_subscript = 0;
  while(no_overlap(*chain_head, "\n;{}") && (VARCHAR(*chain_head) || !no_overlap(*chain_head, "[] .>"))) {
    if(*chain_head == ']') {                                         // don't parse subscripts yet - skip over
      ++in_a_subscript, --chain_head;                                // move past current subscript brace
      while(in_a_subscript > 0) {
        if(*chain_head == ']')       ++in_a_subscript;
        else if(*chain_head == '[')  --in_a_subscript;
        --chain_head;  
      } 
    }
    if(*chain_head == '>' && *(chain_head - 1) == '-') --chain_head; // move past arrow notation
    --chain_head;
  }
  while(!VARCHAR(*chain_head)) ++chain_head;                         // move up to the first object name

  // move chain tail past the method name ('buffer' starts at '(' after method name) to end of invocation
  char invoc_punc[75];      // stores invocation punctuation IN REVERSE (ie '->' stored as '>-')
  FLOOD_ZEROS(invoc_punc, 75);
  int index = 0;
  while(VARCHAR(*chain_tail)) --chain_tail;                         // skip method name
  while(!VARCHAR(*chain_tail) && no_overlap(*chain_tail, "[]"))     // cpy/skip method invocation punct
    invoc_punc[index++] = *chain_tail--;
  ++chain_tail;                                                     // move to 1 char past the chain's end
  invoc_punc[index] = '\0'; // determines 'chainless'/single-object invocation ptr status

  // save object method invocation chain
  index = 0;
  while(chain_head != chain_tail) objectChain[index++] = *chain_head++;
  objectChain[index] = '\0';

  // save the outermost object's name - w/o (if present) array subscript
  char *outermost = objectChain;
  index = 0;
  while(VARCHAR(*outermost)) outerMost_objectName[index++] = *outermost++;
  outerMost_objectName[index] = '\0';

  // if nested, find any local members w/in array subscript to prefix 'this->'
  if(is_nested_object) {
    char prefixed_objectChain[200];
    FLOOD_ZEROS(prefixed_objectChain, 200);
    prefixed_objectChain[0] = '\0';
    char *read_chain = objectChain, *write_chain = prefixed_objectChain;
    while(*read_chain != '\0') {
      if(*read_chain == '[') {
        int filler_int_arg = 0;
        // repurpose method argument parser/copier to handle array subscript instead
        read_chain += prefix_local_members_and_cpy_method_args(read_chain,write_chain,method_words,&filler_int_arg,']');
        write_chain += strlen(write_chain);
      }
      if(*read_chain != '\0') *write_chain++ = *read_chain++;
    }
    *write_chain = '\0';
    FLOOD_ZEROS(objectChain, 200); // save prefixed object chain as the only object chain
    strcpy(objectChain, prefixed_objectChain);
  }

  // determine whether outermost object is a ptr via its punction of method invocation
  int tries = 0;
  char *ptr_status;
  while(tries < 2) { // try searching 'objectChain' then 'invoc_punc' for pointer indicators
    ptr_status = (tries == 0) ? objectChain : invoc_punc;
    while(*ptr_status != '\0' && *ptr_status != '.') {
      if(*ptr_status == '[') while(*ptr_status != '\0' && *ptr_status != ']') ++ptr_status; // skip array subscript
      if(*ptr_status == '>' && *(ptr_status + 1) == '-') { // was stored backwards!
        *objectName_is_pointer = true;
        return;
      }
      ++ptr_status;
    }
    ++tries;
  }
  *objectName_is_pointer = false;
}

// prefixes local members with 'this->' & cpys entire method arg to 'write_to_buffer'
int prefix_local_members_and_cpy_method_args(char*end,char*write_to_buffer,char method_words[][75],
                                             int*buffer_length,char delimiter) {
  int end_increment = 0;
  char *word_start = end + 1, *findArrow;
  while(*end != '\0' && *end != delimiter) { // copy method arguments
    // search the invoked local method's args for any local members
    if(!VARCHAR(*end) && *end != '.' && (*end != '>' || *(end-1) != '-') && VARCHAR(*(end+1))) {
      if(IS_WHITESPACE(*end)) {                       // don't prefixed an invoked member with 'this'
        findArrow = end;
        while(IS_WHITESPACE(*findArrow)) --findArrow; // screen for arrow invocation
        if(*findArrow == '>' && *(findArrow-1) == '-') {
          *write_to_buffer++ = *end++, *buffer_length += 1, ++end_increment;
          continue;
        }
      }
      word_start = end + 1; 
    } else if(VARCHAR(*end) && !VARCHAR(*(end+1))) {                          // at argument word's end
      char argument[75];
      FLOOD_ZEROS(argument, 75);
      int idx = 0;
      while(word_start != end + 1) argument[idx++] = *word_start++;           // copy argument word
      argument[idx++] = '\0';
      if(not_in_method_words(method_words, idx, argument))                    // argument != redefined variable
        for(int i = 0; i < classes[total_classes].total_members; ++i)
          if(strcmp(classes[total_classes].member_names[i], argument) == 0) { // argument = local member
            splice_in_this_arrowPtr(write_to_buffer);
            write_to_buffer += strlen(write_to_buffer);
            break; 
          }
    }
    *write_to_buffer++ = *end++, *buffer_length += 1, ++end_increment;        // copy-current & traverse-next char in arg
  }
  return end_increment;
}

/******************************************************************************
* STORE OBJECT INFORMATION
******************************************************************************/

// stores an object's name & associated class name/type in global struct
bool store_object_info(char *s) {
  bool not_an_arg = true, is_fcn_assignment = false;
  char *q = s, *p = s, object_name[75], class_type_name[75];
  FLOOD_ZEROS(object_name, 75); 
  FLOOD_ZEROS(class_type_name, 75);
  int i = 0, j = 0;
  while(*q != '\0' && *q != ';') if(*q++ == ')') { not_an_arg = false; break; } // determine if arg
  // determine whether object is assigned a value by a fcn 
  // OR initialized by user-defined ctor
  if(!not_an_arg) {
    q = s;
    while(*q != '\0' && no_overlap(*q, ";,)")) if(*q++ == '(') { is_fcn_assignment = true; break; }
    if(is_fcn_assignment) {
      q = s;
      while(*q != '\0' && no_overlap(*q, ";,)")) if(*q++ == '(' || *q++ == '=') { is_fcn_assignment = true; break; }
      not_an_arg = is_fcn_assignment;
    }
  }
  while(!IS_WHITESPACE(*p)) class_type_name[i++] = *p++; p++; // skip class_type declaration
  bool is_class_pointer = false, is_class_array = false;
  if(*p == '*') is_class_pointer = true, p++;                 // check if object == class pointer
  while(VARCHAR(*p)) object_name[j++] = *p++;                 // store object's name, class, & ptr status
  if(*p == '[') is_class_array = true;                        // check if object == class array
  object_name[j] = '\0', class_type_name[i] = '\0';
  if(strlen(object_name) == 0 || class_type_name[strlen(class_type_name)-1] == ',') return false; // prototype fcn arg
  strcpy(objects[total_objects].object_name, object_name);
  strcpy(objects[total_objects].class_name, class_type_name);
  objects[total_objects].is_class_pointer = is_class_pointer;
  objects[total_objects].is_class_array = is_class_array;
  total_objects++;
  return (!is_class_pointer && not_an_arg); // don't pre-init pointers nor arguments
}

/******************************************************************************
* PARSE CLASS HELPER FUNCTIONS
******************************************************************************/

// returns whether struct is definition (true) or variable declaration (false)
bool is_struct_definition(char *end) {
  char *brace = end, *newline = end;
  while(*newline != '\0' && *newline != '\n') ++newline;
  while(*brace != '\0' && *brace != '{') if(brace++ == newline) return false;
  return true;
}

// get class (now struct) name
void get_class_name(char *s, char *class_name) {
  char *p = s, *name = class_name;
  while(*p != '\0' && *p++ != ' ');                                         // skip 'class '
  while(*p != '\0' && *p != ' ' && *p != '{' && *p != '\n') *name++ = *p++; // copy class (becoming struct) name 
  *name = '\0';
}

// checks whether or not class already has a user-defined ctor, & throws error if so
void confirm_only_one_ctor(char *class_name) {
  for(int i = 0; i < classes[total_classes].total_members; ++i)
    if(strcmp(classes[total_classes].member_names[i], "DECLASS__constructor") == 0) {
      fprintf(stderr, "\n-:- ERROR: MORE THAN 1 CONSTRUCTOR FOR CLASSNAME \"%s\" FOUND! -:-", class_name);
      fprintf(stderr, "\n-:-        NO FUNCTION OVERLOADING, TERMINATING PROGRAM        -:-\n\n");
      exit(EXIT_FAILURE);
    }
}

// parse & prepend function name w/ class (now struct) name, & return 
// whether method is user-defined class constructor
bool get_prepended_method_name(char *s, char *class_name, char *prepended_method_name) {
  char method_name[75], ctor_name[75];
  FLOOD_ZEROS(method_name, 75); FLOOD_ZEROS(ctor_name, 75);
  char *p = s, *q, *name = method_name, *ctor = ctor_name;
  bool method_is_ctor = false;
  while(*p != '\0' && IS_WHITESPACE(*p)) p++; p++; // skip tab

  // check for whether method is user-defined class ctor (typeless fcn w/ same name as class)
  q = p - 1;
  while(*q != ' ' && *q != '(') *ctor++ = *q++;         // "type" if *q == ' ' && "class_name" if == '('
  *ctor = '\0';
  if(*q == '(' && strcmp(ctor_name, class_name) == 0) { // no type & fcn name == class_name: method = ctor
    confirm_only_one_ctor(class_name);
    sprintf(prepended_method_name, "DECLASS_%s_", class_name);
    strcpy(classes[total_classes].method_names[classes[total_classes].total_methods], "DECLASS__constructor");
    method_is_ctor = classes[total_classes].class_has_ctor = true;
    if(*(q + 1) != ')') classes[total_classes].class_has_ctor_args = true;

  // is not ctor -- continue cpying method name
  } else { 
    while(*p != '\0' && *p++ != ' ');                                         // skip type
    while(*p != '\0' && VARCHAR(*p)) *name++ = *p++;                          // copy name
    *name = '\0';
    sprintf(prepended_method_name, "DECLASS_%s_%s", class_name, method_name); // className_'function name'
    // store method info in global class struct's instance of the current class
    strcpy(classes[total_classes].method_names[classes[total_classes].total_methods], method_name);
  }
  classes[total_classes].total_methods += 1;
  return method_is_ctor;
}

// returns a member's initialized value (0 by default) & returns how far back name is after value initialization
int get_initialized_member_value(char *member_end) {
  char *start_of_val;
  int i = 0, distance_back = 0;
  int len = classes[total_classes].total_members;
  --member_end, ++distance_back;
  while(*member_end != '\0' && no_overlap(*member_end, "=;\n")) --member_end, ++distance_back; // find '='
  if(*member_end == '=') {                                                                     // is initialized value
    start_of_val = member_end + 1;
    while(IS_WHITESPACE(*start_of_val)) start_of_val++;                                        // find start of value
    for(; *start_of_val != ';'; i++, ++start_of_val)                                           // copy initialized value
      classes[total_classes].member_values[len][i] = *start_of_val;
    classes[total_classes].member_values[len][i] = '\0';
    
    // determine whether initialized member value was a form of memory allocation
    classes[total_classes].member_value_is_alloc[len] = false;
    char *is_alloc = classes[total_classes].member_values[len];
    char *value_front = is_alloc;
    while(*is_alloc != '\0') {
      if(  // at malloc/calloc
        (((is_at_substring(is_alloc, "malloc") || is_at_substring(is_alloc, "calloc")) 
        && !VARCHAR(*(is_alloc+6)) && strlen(is_alloc) > 5)
        || // at smrtmalloc/smrtcalloc
        ((is_at_substring(is_alloc, "smrtmalloc") || is_at_substring(is_alloc, "smrtcalloc")) 
        && !VARCHAR(*(is_alloc+10)) && strlen(is_alloc) > 9))
        && (is_alloc == value_front || !VARCHAR(*(is_alloc-1)))) {

        classes[total_classes].member_value_is_alloc[len] = true;
        classes[total_classes].class_has_alloc = true;
        break;
      }
      ++is_alloc;
    }

    /* 
     * determine whether initialized member value was a user-defined ctor array invocation
     * wherease single-object ctors use a funciton that can return an object, arrays rely on
     * invoking a macro (thus doesn't work for brace initialization) so splice out the macro 
     * in the DFLT fcn for the class && invoke the user-defined ctor outside of the object array's
     * brace initialization
    */
    char obj_arr_ctor_val[350]; FLOOD_ZEROS(obj_arr_ctor_val, 350);
    char obj_single_ctor_val[350]; FLOOD_ZEROS(obj_single_ctor_val, 350);
    sprintf(obj_arr_ctor_val, "DECLASS__%s_USERCTOR_ARR", objects[total_objects-1].class_name);
    sprintf(obj_single_ctor_val, "DECLASS_%s_", objects[total_objects-1].class_name);
    if(is_at_substring(classes[total_classes].member_values[len],obj_arr_ctor_val)
      || is_at_substring(classes[total_classes].member_values[len],obj_single_ctor_val)){
      strcpy(classes[total_classes].member_value_user_ctor[len], classes[total_classes].member_values[len]);
      FLOOD_ZEROS(classes[total_classes].member_values[len], MAX_DEFAULT_VALUE_LENGTH);
      sprintf(classes[total_classes].member_values[len], "{0}");
    } else {
      classes[total_classes].member_value_user_ctor[len][0] = 0;
      classes[total_classes].member_value_user_ctor[len][1] = '\0';
    }

    while(IS_WHITESPACE(*member_end) || *member_end == '=') // move ptr to end of member name to copy
      --member_end, ++distance_back; 
    --distance_back;                                        // [start, end) so move end ptr right after name
  } else {                                                  // no initialized value: set to 0
    classes[total_classes].member_values[len][0] = 0;
    classes[total_classes].member_values[len][1] = '\0';
    distance_back = 0;
  }
  return distance_back;
}

// accounts for whether member is itself a class object, & adds it to the 'objects' struct array if so
void register_member_class_objects(char *member_end) {
  // move 'member_end' back from line's end to line's front (to get member data type)
  while(*member_end != '\0' && *member_end != '\n') --member_end;
  while(IS_WHITESPACE(*member_end)) ++member_end;

  // store member data-type/name & pointer/array status
  char member_type[100], member_name[100]; 
  int idx = 0;
  bool is_class_pointer = false, is_class_array = false;
  FLOOD_ZEROS(member_type, 100); FLOOD_ZEROS(member_name, 100);

  // find the member's data type & determine if type is a class name (thus member = class object)
  while(VARCHAR(*member_end)) member_type[idx++] = *member_end++; 
  member_type[idx] = '\0';
  for(int i = 0; i < total_classes; ++i) {
    if(strcmp(classes[i].class_name, member_type) == 0) {

      // get member/object name & pointer/array status
      while(IS_WHITESPACE(*member_end)) ++member_end;
      if(*member_end == '*') is_class_pointer = true, member_end++;   // check if member is class pointer
      idx = 0;
      while(VARCHAR(*member_end)) member_name[idx++] = *member_end++; // copy member/class-object name
      if(*member_end == '[') is_class_array = true;                   // check if member is class array
      member_name[idx] = '\0';

      // record whether member class object has a alloc'd member to destroy/free
      if(classes[i].class_has_alloc) classes[total_classes].class_has_alloc = true;

      // record member class object's class name
      strcpy(classes[total_classes].member_object_class_name[classes[total_classes].total_members], member_type);
      
      // register class object member as one of its class' objects
      strcpy(objects[total_objects].object_name, member_name);
      strcpy(objects[total_objects].class_name, member_type);
      objects[total_objects].is_class_pointer = is_class_pointer;
      objects[total_objects].is_class_array = is_class_array;
      total_objects++;
      return;
    }
  }
  // if not a class, class name is 0
  classes[total_classes].member_object_class_name[classes[total_classes].total_members][0] = 0;
  classes[total_classes].member_object_class_name[classes[total_classes].total_members][1] = '\0';
}

// check if sizeof() arg is the member just created (ie *node = malloc(sizeof(node));) and if so, 
// prepend sizeof arg with "this" (ie sizeof(this.node)) for global initializer fcn
void check_for_alloc_sizeof_arg() {
  int len = classes[total_classes].total_members;
  if(classes[total_classes].member_values[len][0] == 0 || !classes[total_classes].member_value_is_alloc[len]) return;
  char *p=classes[total_classes].member_values[len], *name=classes[total_classes].member_names[len], *size, *prep, *q;
  char member_name[MAX_DEFAULT_VALUE_LENGTH], prepended_sizeof_arg[260];
  FLOOD_ZEROS(member_name, MAX_DEFAULT_VALUE_LENGTH); FLOOD_ZEROS(prepended_sizeof_arg, 260);
  size = member_name, prep = prepended_sizeof_arg;
  while(*p != '\0' && strlen(p) > 7) {
    if(is_at_substring(p, "sizeof(")) {
      p += 7;                                          // skip past sizeof
      while(*p != '\0' && !VARCHAR(*p)) ++p;           // skip past potential '*'
      char *startOfSizeofArg = p;
      while(*p != '\0' && VARCHAR(*p)) *size++ = *p++; // copy name
      *size = '\0';
      if(strcmp(member_name, name) == 0) { // check if sizeof() arg = newest member (ie *node = malloc(sizeof(node));)
        q = classes[total_classes].member_values[len];
        while(*q != '\0' && q != startOfSizeofArg) *prep++ = *q++; // copy up to sizeof arg
        sprintf(prep, "this.");                                    // prepend size of arg with "this"
        prep += strlen(prep);
        while(*q != '\0') *prep++ = *q++;                          // copy rest of sizeof arg
        *prep = '\0';
        FLOOD_ZEROS(classes[total_classes].member_values[len], MAX_DEFAULT_VALUE_LENGTH);
        strcpy(classes[total_classes].member_values[len], prepended_sizeof_arg);
        break;
      }
    }
    ++p;
  }
}

// check for & store member in class - returns 0 = no member, 1 = member, 2 = member initialized with values
int get_class_member(char *end, bool is_fcn_ptr) {
  char *member_start, *member_end = end;
  while(IS_WHITESPACE(*member_end)) --member_end;
  if(*member_end == ';' || is_fcn_ptr) {                             // is member
    register_member_class_objects(member_end);                       // account for whether member is itself a class obj
    if(is_fcn_ptr) 
      while(*member_end != '\0' && *member_end != ';') ++member_end; // to check for initialization value
    int len = classes[total_classes].total_members;                  // new member index
    member_end -= get_initialized_member_value(member_end);          // get initialized value (0 by default)
    
    if(*(member_end - 1) == ']') {                                   // is array member
      while(*member_end != '[') --member_end; 
      classes[total_classes].member_is_array[len] = true;
    } else classes[total_classes].member_is_array[len] = false;
    
    member_start = member_end;                                       // [member_start, member_end)
    while(!IS_WHITESPACE(*(member_start - 1))) --member_start;

    char *find_asterisk = member_start;                              // determine if pointer member
    while(no_overlap(*find_asterisk, "*\n;") && !VARCHAR(*find_asterisk)) --find_asterisk;
    classes[total_classes].member_is_pointer[len] = (*find_asterisk == '*');

    int i = 0;                                                       // copy member to classes struct
    for(; member_start != member_end; ++member_start) 
      if(VARCHAR(*member_start)) classes[total_classes].member_names[len][i++] = *member_start;
    classes[total_classes].member_names[len][i] = '\0';
    check_for_alloc_sizeof_arg(); // prepend alloc sizeof() arg w/ "this." if arg = newest member
    classes[total_classes].total_members += 1;
    return 1 + (classes[total_classes].member_values[len][0] != 0);
  }
  return 0;
}

// adds a word from method into 'method_words[][]'
void add_method_word(char method_words[][75], int *word_size, char *word_start, char *word_end) {
  char *p = word_start, method_word[75];
  FLOOD_ZEROS(method_word, 75);
  int i = 0, j;
  while(p - 1 != word_end) method_word[i++] = *p++;
  method_word[i] = '\0';
  for(j = 0; j < *word_size; ++j) if(strcmp(method_words[j], method_word) == 0) break;
  if(j == *word_size) {
    strcpy(method_words[*word_size], method_word);
    *word_size += 1;
  }
}

/******************************************************************************
* CONFIRM WHETHER METHOD'S WORD IS A LOCAL CLASS MEMBER
******************************************************************************/

// returns whether var is a redefinition/declaration (false) or member (true)
bool not_local_var_declaration(char *word_start) {
  // if 'basic_c_types' found before ';' or ') {', & isn't an array declaration elt nor array 
  // subscript nor assigned value, var is presumed to be a type redefinition/declaration (ie NOT a member)
  char *scout = word_start, *findParenthesis;
  bool found_close_bracket = false, found_close_brace = false;
  while(*scout != ';') {
    if(*scout == '}') found_close_brace   = true; // not an array declaration elt
    if(*scout == ']') found_close_bracket = true; // not in array subscript
    if(*scout == '[' && !found_close_bracket) return true;
    if(*scout == '{') { // if '{' preceded by 'whitespace' & ')', not a declaration
      findParenthesis = scout - 1;
      while(IS_WHITESPACE(*findParenthesis)) --findParenthesis;
      if(*findParenthesis == ')' || !found_close_brace) return true;
    }
    if(*scout == '=') return true;       // elt is used as an assigned value
    for(int i = 0; i < TOTAL_TYPES; ++i) // check for basic type declaration
      if(is_at_substring(scout, basic_c_types[i]) && !VARCHAR(*(scout - 1))) 
        return false;
    --scout;
  }
  return true;
}

// returns whether method doesn't already have word (if false, var is then NOT a member)
bool not_in_method_words(char method_words[][75], int word_size, char *word) {
  for(int j = 0; j < word_size; ++j) if(strcmp(method_words[j], word) == 0) return false;
  return true;
}

// returns whether word isn't an invocation (true)
// otherwise, being invoked by some external struct or class (false)
bool not_an_external_invocation(char *method_buff_idx) {
  char *p = method_buff_idx;
  while(VARCHAR(*p)) --p;
  while(IS_WHITESPACE(*p)) --p;
  if(*p == '.' || (*p == '>' && *(p - 1) == '-')) return false;
  return true;
}

// given a member's index, splices in 'this->' after member's name
void splice_in_this_arrowPtr(char *method_buff_idx) {
  char members_name[75];
  FLOOD_ZEROS(members_name, 75);
  method_buff_idx--;                                  // move ptr back to a char (at '\0' right now)
  while(VARCHAR(*method_buff_idx)) --method_buff_idx; // move ptr back to front of member name
  method_buff_idx++;                                  // increment ptr to right B4 member name
  strcpy(members_name, method_buff_idx);
  sprintf(method_buff_idx, "this->%s", members_name); // splice in 'this->members_name'
}

// finds methods invoked w/in methods of their same class such that they
// should be prepended 'className'_ & take 'this' as their last arg
int parse_local_nested_method(char *end, char *method_buff_idx, char *class_name, char method_words[][75]) {
  int prepended_size = 0;
  if(!VARCHAR(*end) && VARCHAR(*(end + 1))) {
    *method_buff_idx++ = *end++, prepended_size++;                    // move to first letter
    char method_name[75];
    FLOOD_ZEROS(method_name, 75);
    int i = 0;
    while(VARCHAR(*end)) method_name[i++] = *end++, prepended_size++; // copy method name
    if(*end != '(') return 0;                                         // if no method
    method_name[i] = '\0';
    for(int j = 0; j < classes[total_classes].total_methods; ++j)     // find if class has method name
      if(strcmp(classes[total_classes].method_names[j], method_name) == 0) {
        sprintf(method_buff_idx, "DECLASS_%s_%s", class_name, method_name); // prepend method name w/ 'className'_
        method_buff_idx += strlen(method_buff_idx);
        // copy method arguments & prefix any local members w/in w/ 'this->'
        end += prefix_local_members_and_cpy_method_args(end, method_buff_idx, method_words, &prepended_size, ')');
        method_buff_idx += strlen(method_buff_idx);
        (*(end - 1) == '(') ? sprintf(method_buff_idx, "this") : sprintf(method_buff_idx, ", this");
        return prepended_size;
      }
  }
  return 0;
}

// umbrella fcn evaluating all conditions for a word to be a local class member
bool valid_member(char*word_start,char*memberName,char nextChar,char period,char method_words[][75],int word_size) {
  return (is_at_substring((word_start+1), memberName) && !VARCHAR(*word_start) // at potential member
    && (!VARCHAR(nextChar) || period == '.')                                   // member = word or struct
    && not_local_var_declaration(word_start+1)                                 // not redeclared
    && not_in_method_words(method_words, word_size, memberName)                // not redefined
    && not_an_external_invocation(word_start+1));                              // not external invocation
}

/******************************************************************************
* PARSE CLASS
******************************************************************************/

// parses class instance
int parse_class(char *class_instance, char *NEW_FILE, int *j) {
  #define skip_over_blank_lines(first_newline) \
    blank_line_size = remove_blank_lines(first_newline); \
    if(blank_line_size > 0) { \
      class_size += blank_line_size; \
      end += blank_line_size; \
      start_of_line = end; \
    }

  // method_buff stores functions in class to be spliced out
  char method_buff[MAX_METHOD_BYTES_PER_CLASS], *method_buff_idx;
  FLOOD_ZEROS(method_buff, MAX_METHOD_BYTES_PER_CLASS);
  
  // struct_buff stores non-method elts from class to be put into struct
  char struct_buff[MAX_MEMBER_BYTES_PER_CLASS], class_name[75], *struct_buff_idx;
  FLOOD_ZEROS(struct_buff, MAX_MEMBER_BYTES_PER_CLASS);
  FLOOD_ZEROS(class_name, 75);
  get_class_name(class_instance, class_name);
  sprintf(struct_buff, "typedef struct DECLASS_%s {", class_name);
  struct_buff_idx = &struct_buff[strlen(struct_buff)]; // points to '\0'
  method_buff_idx = method_buff;

  // store class info in global struct
  strcpy(classes[total_classes].class_name, class_name);
  classes[total_classes].total_methods = 0, classes[total_classes].total_members = 0;
  classes[total_classes].class_has_alloc = false;
  classes[total_classes].class_has_ctor = false, classes[total_classes].class_has_ctor_args = false;

  // # of class or comment chars
  int class_size = 0, class_comment_size, blank_line_size;

  // class scope btwn 'start' & 'end'
  char *start = class_instance, *end, *start_of_line;
  int in_class_scope = 1, in_method_scope;
  bool in_a_string;
  while(*start++ != '{') class_size++;
  end = start;

  // copy members to struct_buff & methods to method_buff
  while(*end != '\0' && in_class_scope > 0) {
    // confirm in class' scope
    if(*end == '{') in_class_scope++;
    else if(*end == '}') in_class_scope--;
    if(in_class_scope < 0) break;

    // skip over struct/union members - the struct's variables are considered 
    // within the struct's scope and thus NOT members of the outer class
    // NOTE: UNIONS & STRUCTS HANDLED IDENTICALLY!
    if((is_at_substring(end, "struct") || is_at_substring(end, "union")) && is_struct_definition(end)) {
      while(*end != '\0' && *(end - 1) != '{') *struct_buff_idx++ = *end++, class_size++;
      int in_struct_scope = 1;
      while(*end != '\0' && in_struct_scope > 0) {
        if(*end == '{') in_struct_scope++;
        else if(*end == '}') in_struct_scope--;
        if(*end == ';') { // store struct members to use in CTOR brace initialization
          #define init_struct_members(attrib) ({\
            classes[total_classes].attrib[classes[total_classes].total_members][0] = 0; \
            classes[total_classes].attrib[classes[total_classes].total_members][1] = '\0'; \
          })
          init_struct_members(member_names); 
          init_struct_members(member_values);
          #undef init_struct_members
          classes[total_classes].total_members += 1;
        }
        *struct_buff_idx++ = *end++, class_size++;
      }
    }

    // check for newline - methods & members
    if(*end == '\n') {
      start_of_line = end;
      if(get_class_member(end, false) == 2) { // check for member & rmv any init vals from struct
        *struct_buff_idx-- = '\0';
        while(*struct_buff_idx != '\0' && *struct_buff_idx != '\n' && *struct_buff_idx != '=') *struct_buff_idx-- = '\0';
        *struct_buff_idx++ = ';';
      }
      skip_over_blank_lines(end);             // don't copy any blank lines to struct_buff
    }

    // check for function method
    if(*end == '(') { 
      // check for function assignment -- especially for 'malloc'/'calloc'
      char *findEq = end;
      while(no_overlap(*findEq, "\n=;")) --findEq; // move to either beginning of line or assignment
      if(*findEq == '=') {                         // if function is being assigned
        while(*end != '\0' && *end != '\n')        // move to the end of the line
          *struct_buff_idx++ = *end++, ++class_size;
        continue;                                  // invoke next loop iteration to trigger "member" detection logic above
      }

      // check for fcn ptr => treated as member
      if(*(end + 1) == '*') { 
        while(*end != '\0' && *end != ')') *struct_buff_idx++ = *end++, class_size++;
        if(get_class_member(end, true) == 2) { // copy fcn ptr args BUT NOT INITIALIZING VALUE to struct buff
          while(*end != '\0' && *end != '=' && *(end - 1) != '\n') *struct_buff_idx++ = *end++, class_size++;
          *struct_buff_idx++ = ';';
          *struct_buff_idx++ = '\n';
          while(*end != '\0' && *(end - 1) != '\n') end++, class_size++;
        } else {                              // copy fcn ptr args to struct buff
          while(*end != '\0' && *(end - 1) != '\n') *struct_buff_idx++ = *end++, class_size++;
        }
        start_of_line = end - 1;
        skip_over_blank_lines(end - 1);       // don't copy any blank lines to struct_buff
        continue;
      }

      // only 3 cases result in a class member having the structure: "word" "word" '('
      // (1) fcn ptr, but this is handled directly above
      // (2) a method, in which case the last char in the line ought to be either '\n' of '{'
      // (3) a member object being declared with a user-defined constructor, in which case the line ends w/ ';'
      // handle case (3): check for user-defined constructor invocation
      bool found_ctor = false;
      int struct_increment = 0;
      end = check_for_ctor_obj(end, struct_buff_idx, class_instance, &class_size, &struct_increment, &found_ctor);
      if(found_ctor) {
        struct_buff_idx += struct_increment;
        // move line's end & invoke next loop iteration to trigger "member" detection logic above method detection
        while(*end != '\0' && *end != '\n') *struct_buff_idx++ = *end++, ++class_size; 
        continue;
      }

      // store method variables and keywords to single out local members to make 'this' point to
      char method_words[MAX_WORDS_PER_METHOD][75], *word_start;
      for(int i = 0; i < MAX_WORDS_PER_METHOD; ++i) FLOOD_ZEROS(method_words[i], 75);
      int word_size = 0;

      // confirm whether still w/in method scope or w/in a string
      in_method_scope = 1;
      in_a_string = false;
      
      // get className-prepended method name
      char prepended_method_name[150];
      FLOOD_ZEROS(prepended_method_name, 150);
      bool method_is_ctor = get_prepended_method_name(start_of_line, class_name, prepended_method_name);

      // remove method from struct_buff (only members)
      char *wipe_method_line = start_of_line;
      while(wipe_method_line++ != end) *struct_buff_idx-- = '\0';

      // copy method to 'method_buff' & move 'end' forward
      while(IS_WHITESPACE(*start_of_line) && start_of_line != end)  *method_buff_idx++ = *start_of_line++; // copy indent
      while(!IS_WHITESPACE(*start_of_line) && start_of_line != end) *method_buff_idx++ = *start_of_line++; // copy type
      sprintf(method_buff_idx, " %s", prepended_method_name);                              // copy appended method name
      method_buff_idx += strlen(method_buff_idx);                                          // move method_buff_idx to '\0'

      // store method's arg words in 'method_words[][]' to discern from local class member vars
      while(*(end + 1) != '\0' && *end != ')') {
        // Check for class objects in method argument
        int k; 
        for(k = 0; k < total_classes + 1; ++k)
          if(is_at_substring(end, classes[k].class_name) 
            && !VARCHAR(*(end + strlen(classes[k].class_name))) && !VARCHAR(*(end-1))) {
            store_object_info(end); // register object arg as a class object instance
            break; 
          }
        // Save method's words in argument
        if(k == total_classes + 1){
          if(!VARCHAR(*end) && *end != ')' && VARCHAR(*(end + 1))) word_start = end + 1;
          else if(VARCHAR(*end) && !VARCHAR(*(end + 1)))
            add_method_word(method_words, &word_size, word_start, end);         // add arg word to 'method_words[][]'
        }
        *method_buff_idx++ = *end++, class_size++;                              // copy method up to ')'
      }

      // splice in 'this' class ptr as last arg in method
      if(*(end - 1) == '(') sprintf(method_buff_idx, "%s *this", class_name);   // spliced class ptr is single method arg                      
      else                  sprintf(method_buff_idx, ", %s *this", class_name); // spliced class ptr is poly method arg
      method_buff_idx += strlen(method_buff_idx); 
      while(*end != '\0' && *(end - 1) != '{') *method_buff_idx++ = *end++, class_size++; // copy method up to '{'

      // copy the rest of the method into 'method_buff'
      in_class_scope++;                                                                   // skip first '{'
      while(*end != '\0' && in_method_scope > 0 && in_class_scope > 0) {                  // copy method body
        // account for current scope & cpy method
        if(*end == '{') in_method_scope++, in_class_scope++;
        else if(*end == '}') in_method_scope--, in_class_scope--;
        else if(*end == '"' && *(end-1) != '\\') in_a_string = !in_a_string; // don't prefix '->' to member names in strs
        if(in_method_scope < 0 || in_class_scope < 0) break;

        // check for class object declaration
        for(int k = 0; !in_a_string && k < total_classes + 1; ++k)
          if(is_at_substring(end, classes[k].class_name) 
            && !VARCHAR(*(end-1)) && !VARCHAR(*(end+strlen(classes[k].class_name)))) {
            if(store_object_info(end)) {
              char *already_assigned = end;
              while(*already_assigned != '\0' && no_overlap(*already_assigned, "\n;,=")) ++already_assigned;
              if(*already_assigned == '=') break;
              // determine if object is invoking it's user-defined constructor
              char *user_ctor_finder = end;
              char user_ctor[500]; FLOOD_ZEROS(user_ctor, 500);
              bool is_fcn_returning_obj = false;
              bool user_ctor_invoked=get_user_ctor(user_ctor_finder,user_ctor,classes[k].class_name,&is_fcn_returning_obj);
              // don't splice in any constructors if "object" is actually a fcn returning an object
              if(is_fcn_returning_obj) break;
              // implement macro ctor
              while(*end != '\0' && *(end-1) != ';') *method_buff_idx++ = *end++, ++class_size;
              if(objects[total_objects-1].is_class_array) // object = array, use array macro init
                sprintf(method_buff_idx, " DECLASS__%s_ARR(%s);", 
                  classes[k].class_name, objects[total_objects-1].object_name);
              else                                        // object != array, so use single-object macro init
                sprintf(method_buff_idx, " DECLASS__%s_CTOR(%s);", 
                  classes[k].class_name, objects[total_objects-1].object_name);
              method_buff_idx += strlen(method_buff_idx);
              // add user-defined ctor invocation w/ initialization values (if present)
              if(user_ctor_invoked) { 
                sprintf(method_buff_idx, " %s", user_ctor); 
                method_buff_idx += strlen(method_buff_idx); 
              }
            }
            break;
          }

        // either add method word to 'method_words[][]' or prepend 'this->' to member
        if(!in_a_string) {
          if(!VARCHAR(*(end-1)) && *(end-1) != '\'' && VARCHAR(*end)) word_start = end; // beginning of word
          if(VARCHAR(*end) && !VARCHAR(*(end + 1))) {                                // end of word - member or var
            // check if a member of the current/local class
            int i = 0;
            for(; i < classes[total_classes].total_members; ++i) {
              if(classes[total_classes].member_names[i][0] == 0) continue; // struct member -- disregard
              char *endOfMember = word_start + strlen(classes[total_classes].member_names[i]);
              char nextChar = *endOfMember, period = *(endOfMember + 1);   // either a word or class' struct member invoked
              // word is a member of the local class -- prefix 'this->'
              if(valid_member(word_start - 1, classes[total_classes].member_names[i], 
                nextChar, period, method_words, word_size)) {

                splice_in_this_arrowPtr(method_buff_idx);
                method_buff_idx += strlen(method_buff_idx);
                break;
              }
            }
            // not a member - thus add arg word to 'method_words[][]'
            if(i == classes[total_classes].total_members) add_method_word(method_words, &word_size, word_start, end); 
          }
        }

        // prepend methods invoked within methods with the appropriate 'className'_
        int method_buff_idx_position = strlen(method_buff) + 1;
        int nested_method_size = (!in_a_string)
          ? parse_method_invocation(end, method_buff_idx, &method_buff_idx_position, true, method_words)
          : 0;
        end += nested_method_size, class_size += nested_method_size;

        // check whether nested method or not
        if(nested_method_size > 0) {
          method_buff_idx += strlen(method_buff_idx);
        } else {

          // check whether nested method, but for local class (ie NOT for another object)
          int local_nested_method_size = (!in_a_string) 
            ? parse_local_nested_method(end, method_buff_idx, class_name, method_words) 
            : 0;
          if(local_nested_method_size > 0) {
            end += local_nested_method_size, class_size += local_nested_method_size;
            method_buff_idx += strlen(method_buff_idx);
          }
          *method_buff_idx++ = *end++, class_size++;
        }
      }
      if(method_is_ctor) { // return object at the end of user-defined ctors for default assignment
        sprintf(method_buff_idx - 1, "\treturn *this;\n\t}");
        method_buff_idx += strlen(method_buff_idx);
      }
      start_of_line = end; // start next line after method
    } 
    *struct_buff_idx++ = *end;
    end++, class_size++;
  }

  // clean-up formatting of struct & method buffers
  sprintf(struct_buff_idx, " %s;", class_name);
  *method_buff_idx = '\0';

  end++, class_size++; // skip '};'
  if(*(end - 1) == ';') end++, class_size++;

  // copy the constructor macros, class-converted-to-struct, & spliced-out methods to 'NEW_FILE'
  char macro_ctor_comment[100], struct_comment[100], method_comment[100];
  FLOOD_ZEROS(macro_ctor_comment, 100); FLOOD_ZEROS(struct_comment, 100); FLOOD_ZEROS(method_comment, 100);
  sprintf(macro_ctor_comment, "/* %s CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */\n", class_name);
  sprintf(struct_comment, "\n\n/* %s CLASS CONVERTED TO STRUCT: */\n", class_name);
  sprintf(method_comment, "\n\n/* %s CLASS METHODS SPLICED OUT: */", class_name);

  // make a global class object with default values to initialize client's unassigned class objects with
  char initial_values_brace[500];
  FLOOD_ZEROS(initial_values_brace, 500);
  mk_initialization_brace(initial_values_brace, total_classes);
  char class_global_initializer[2500];
  FLOOD_ZEROS(class_global_initializer, 2500);
  mk_class_global_initializer(class_global_initializer, class_name, initial_values_brace);

  // make macro ctors to assign any objects of this class its default values, as well as
  // initialize any contained class object members too - both for single & array instances of objects
  char ctor_macros[3500];
  FLOOD_ZEROS(ctor_macros, 3500); mk_ctor_macros(ctor_macros, class_name); 

  // struct before methods to use class/struct type for method's 'this' ptr args
  if(strlen(struct_buff) > 0) {
    APPEND_BUFF_OR_STR_TO_NEW_FILE("/******************************** CLASS START ********************************/\n");
    APPEND_BUFF_OR_STR_TO_NEW_FILE(macro_ctor_comment); APPEND_BUFF_OR_STR_TO_NEW_FILE(ctor_macros);
    APPEND_BUFF_OR_STR_TO_NEW_FILE(struct_comment); APPEND_BUFF_OR_STR_TO_NEW_FILE(struct_buff);
    APPEND_BUFF_OR_STR_TO_NEW_FILE(class_global_initializer);
    if(strlen(method_buff)>0)APPEND_BUFF_OR_STR_TO_NEW_FILE(method_comment);APPEND_BUFF_OR_STR_TO_NEW_FILE(method_buff);
    APPEND_BUFF_OR_STR_TO_NEW_FILE("\n/********************************* CLASS END *********************************/");
  }

  total_classes++;
  return class_size;
  #undef skip_over_blank_lines // nothing below, but just for the sake of keeping it local to the function
}
