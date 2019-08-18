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
#define FSCRAPE(BUFF,FNAME) \
  ({BUFF[0]='\0';FILE*fptr;if((fptr=fopen(FNAME,"r"))==NULL){exit(0);}while(fgets(&BUFF[strlen(BUFF)],500,fptr)!=NULL);fclose(fptr);})
#define FPUT(BUFF,FNAME) ({FILE*fptr;if((fptr=fopen(FNAME,"w"))==NULL){exit(0);}fprintf(fptr,"%s",BUFF);fclose(fptr);})
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
#define MAX_OBJECTS 100
#define MAX_CLASSES 100
#define MAX_MEMBERS_PER_CLASS 100
#define MAX_METHODS_PER_CLASS 100
#define MAX_MEMBER_BYTES_PER_CLASS 5001
#define MAX_METHOD_BYTES_PER_CLASS 5001
#define MAX_WORDS_PER_METHOD 1000
#define MAX_INIT_VALUE_LENGTH 251
// declassed program contact header
#define DECLASS_SUPPORT_CONTACT "Email jrandleman@scu.edu or see https://github.com/jrandleman for support */"

/*****************************************************************************
 *                      -:- DECLASS.C 8 CAVEATS -:-                         *
 *   (1) 'DECLASS_' PREFIX & 'this' POINTER ARE RESERVED                    *
 *   (2) DECLARE CLASSES GLOBALLY & OBJECTS LOCALLY (NEITHER IN STRUCTS)    *
 *   (3) DECLARE MEMBERS/METHODS USED IN A METHOD ABOVE ITS DECLARATION     *
 *   (4) NO POINTER TO ARRAY OF OBJECTS:                                    *
 *       * IE NOT:      className (*ptrToArrObj)[10];                       *
 *       * ALTERNATIVE: pointer to an object w/ an array of objects member  *
 *   (5) NO NESTED CLASS DECLARATIONS NOR METHOD INVOCATIONS:               *
 *       * IE NOT:      'someObj.method1(someObj.method2());'               *
 *       * ALTERNATIVE: 'int x = someObj.method2(); someObj.method1(x);'    *
 *   (6) CLASS ARRAYS RECIEVED AS ARGS MUST BE DENOTED WITH '[]' NOT '*':   *
 *       * IE NOT:     'func(className *classArr){...}'                     *
 *       * ALTERNATIVE:'func(className classArr[]){...}'                    *
 *   (7) DECLARE CLASS MEMBERS, METHODS & OBJECTS INDIVIDUALLY:             *
 *       * IE NOT:      'myClassName c, e;'                                 *
 *       * ALTERNATIVE: 'myClassName c; myClassName e;'                     *
 *   (8) CONTAINMENT, NOT INHERITANCE: CLASSES CAN ONLY ACCESS MEMBERS &    *
 *       METHODS OF THEIR OWN IMMEDIATE MEMBER CLASS OBJECTS:               *
 *       * IE: suppose classes c1, c2, & c3, with c1 in c2 & c2 in c3.      *
 *             c3 can access c2 members and c2 ca access c1 members,        *
 *             but c3 CANNOT access c1 members                              *
 *       * ALTERNATIVES: (1) simply include a c1 object as a member in c3   *
 *                       (2) create methods in c2 invoking c1 methods as    *
 *                           an interface for c3                            *
 *****************************************************************************/

// stores class names, & their associated methods
typedef struct class_info{ 
  char class_name[75], method_names[MAX_METHODS_PER_CLASS][75]; 
  char member_names[MAX_MEMBERS_PER_CLASS][75], member_values[MAX_MEMBERS_PER_CLASS][MAX_INIT_VALUE_LENGTH];
  bool member_is_array[MAX_MEMBERS_PER_CLASS];              // init empty arrays as {0}
  bool member_is_pointer[MAX_MEMBERS_PER_CLASS];            // init pointers as 0 (same as NULL)
  char member_object_class_name[MAX_MEMBERS_PER_CLASS][75]; // used to intialize contained class objects
  int total_methods, total_members; 
} CLASS_INFO;
CLASS_INFO classes[MAX_CLASSES];
int total_classes = 0;

// stores object names, & their associated class
typedef struct objNames { char class_name[75], object_name[75]; bool is_class_pointer, is_class_array; } OBJ_INFO;
OBJ_INFO objects[MAX_OBJECTS];
int total_objects = 0;

// basic c type keywords:
#define TOTAL_TYPES 14
char basic_c_types[TOTAL_TYPES][11] = {
  "char ","short ","int ","unsigned ","signed ","struct ","union ","long ","float ","double ","bool ","enum ","typedef ","void "
};

/* MESSAGE FUNCTIONS */
void confirm_valid_file(char *);
void declass_missing_Cfile_alert();
void declass_DECLASSIFIED_ascii_art();
void show_l_flag_data();
/* COMMENT & BLANK LINE SKIPPING FUNCTIONS */
void skip_comment(char [], int *, char [], int *);
void whitespace_all_class_comments(char *);
int remove_blank_lines(char *);
/* STRING HELPER FUNCTIONS */
bool no_overlap(char, char *);
bool is_at_substring(char *, char *);
/* OBJECT INITIALIZATION FUNCTIONS */
void mk_initialization_brace(char [], int);
void mk_ctor_macros(char [], char *);
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
void get_prepended_method_name(char *, char *, char *);
int get_initialized_member_value(char *);
void register_member_class_objects(char *);
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

  sprintf(NEW_FILE,"/* DECLASSIFIED: %s\n * %s\n\n", filename, DECLASS_SUPPORT_CONTACT);
  FSCRAPE(file_contents, filename);
  char filler_array_argument[MAX_WORDS_PER_METHOD][75];
  int i = 0, j = strlen(NEW_FILE);
  bool in_a_string = false;
  while(file_contents[i] != '\0') {
    // don't modify anything in strings
    if(file_contents[i] == '"') in_a_string = !in_a_string;

    // skip over comments w/o modifying
    if(!in_a_string) skip_comment(file_contents, &i, NEW_FILE, &j);

    // store class info & convert to structs calling external fcns
    if(!in_a_string && is_at_substring(&file_contents[i], "class ")) 
      i += parse_class(&file_contents[i], NEW_FILE, &j);

    // store declared class object info
    for(int k = 0; !in_a_string && k < total_classes; ++k)
      if(is_at_substring(&file_contents[i], classes[k].class_name))
        if(store_object_info(&file_contents[i])) { // assign default values

          // check if class is already initialized by user
          int already_assigned = i;
          while(file_contents[already_assigned] != '\0' && no_overlap(file_contents[already_assigned], "\n;,=")) ++already_assigned;
          if(file_contents[already_assigned] == '=') break; 

          // initialization undefined -- use default initial values
          while(file_contents[i] != '\0' && file_contents[i-1] != ';') NEW_FILE[j++] = file_contents[i++];
          if(objects[total_objects-1].is_class_array) // object = array, use macro init
            sprintf(&NEW_FILE[j], " DECLASS__%s_ARR(%s);", classes[k].class_name, objects[total_objects-1].object_name);
          else // object != array, so init via its class' global object
            sprintf(&NEW_FILE[j], " DECLASS__%s_CTOR(%s);", classes[k].class_name, objects[total_objects-1].object_name);
          
          j = strlen(NEW_FILE);
          break;
        }

    // modify object invoking method to fcn call w/ a prepended class-converted-struct name
    if(!in_a_string && total_classes > 0) i += parse_method_invocation(&file_contents[i], NEW_FILE, &j, false, filler_array_argument);
    // save non-class data to file
    NEW_FILE[j++] = file_contents[i++]; 
  }
  NEW_FILE[j] = '\0';

  declass_DECLASSIFIED_ascii_art();
  printf("%s ==DECLASSIFIED=> ", filename);
  NEW_EXTENSION(filename, "_DECLASS.c");
  printf("%s", filename);
  printf("\n=================================================================================\n");

  FPUT(NEW_FILE,filename);
  if(show_class_info) show_l_flag_data();

  printf(" >> Terminating Declassifier.\n");
  printf("=============================\n\n");

  return 0;
}

/******************************************************************************
* MESSAGE FUNCTIONS
******************************************************************************/

// confirms file exists, non-empty, & takes less memory than MAX_FILESIZE
void confirm_valid_file(char *filename) {
  struct stat buf;
  if(stat(filename, &buf)) {
    printf("-:- FILE %s DOES NOT EXIST! -:-\n", filename);
    exit(0);
  }
  if(buf.st_size > MAX_FILESIZE || buf.st_size == 0) {
    if(buf.st_size > MAX_FILESIZE) {
      printf("-:- FILE %s SIZE %lld BYTES EXCEEDS %d BYTE CAP! -:- \n", filename, buf.st_size, MAX_FILESIZE); 
      printf("-:- RAISE 'MAX_FILESIZE' MACRO LIMIT! -:- \n");
    } else printf("-:- CAN'T DECLASSIFY AN EMPTY FILE! -:- \n"); 
    exit(0);
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
  exit(0);
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
        if(classes[i].member_is_pointer[j])    printf(" %c  L_ *%s\n", bar, classes[i].member_names[j]);
        else if(classes[i].member_is_array[j]) printf(" %c  L_ %s[]\n", bar, classes[i].member_names[j]);
        else printf(" %c  L_ %s\n", bar, classes[i].member_names[j]);
      }
    }

    if(classes[i].total_methods > 0) {
        printf(" L_ METHODS: %d\n", classes[i].total_methods);
      for(int j = 0; j < classes[i].total_methods; ++j)
        (class_objects_sum > 0) ? printf(" | L_ %s()\n",classes[i].method_names[j]) : printf("   L_ %s()\n",classes[i].method_names[j]);
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

// skips comments outside of class instances
void skip_comment(char file_contents[], int *i, char NEW_FILE[], int *j) {
  #define copy_file_contents(times) \
    ({for(int k=0; k<times; ++k){NEW_FILE[*j] = file_contents[*i]; file_contents[*i] = ' '; *i += 1; *j += 1;}})
  if(file_contents[*i] == '/' && file_contents[(*i) + 1] == '/')
    while(file_contents[(*i)+1] != '\0' && file_contents[(*i)+1] != '\n') copy_file_contents(1);
  else if(file_contents[*i] == '/' && file_contents[(*i) + 1] == '*') {
    copy_file_contents(2);
    while(file_contents[*i] != '\0' && (file_contents[*i] != '*' || file_contents[(*i) + 1] != '/'))
      copy_file_contents(1);
  }
  #undef copy_file_contents
}

// replaces all comments inside of a class instance with spaces
void whitespace_all_class_comments(char *end) {
  int in_class_scope = 1;
  bool in_a_string = false;
  while(*end != '\0' && in_class_scope > 0) {
    // confirm in class' scope
    if(*end == '{') in_class_scope++;
    else if(*end == '}') in_class_scope--;
    else if(*end == '"') in_a_string = !in_a_string;
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
* OBJECT INITIALIZATION FUNCTIONS
******************************************************************************/

// given a class index, returns an initialization brace for it's member values
void mk_initialization_brace(char brace[], int class_index) {
  char *p = brace;
  *p++ = '{';
  for(int j = 0; j < classes[class_index].total_members; ++j) {
    if(classes[class_index].member_values[j][0] == 0) {               // empty value
      if(classes[class_index].member_names[j][0] == 0) continue;      // if struct's member (struct inner members' name = value = 0), skip
      else if(classes[class_index].member_is_array[j] || (j > 0 && classes[class_index].member_names[j-1][0] == 0)
        || (classes[class_index].member_object_class_name[j][0] != 0 && !classes[class_index].member_is_pointer[j]))
          sprintf(p, "{0}, ");                                        // wrap empty (non-ptr) array/object/struct value in braces
      else if((j > 0 && classes[class_index].member_names[j-1][0] != 0) || j == 0) 
        sprintf(p, "0, "); 
    } else sprintf(p, "%s, ", classes[class_index].member_values[j]); // non-empty value
    p += strlen(p);
  }
  *p++ = '}';
  *p = '\0';
}

// fills 'ctor_macros' string w/ macros for both single & array object constructions/initializations
void mk_ctor_macros(char ctor_macros[], char *class_name) {
  // add macro for a single object construction instance
  sprintf(ctor_macros, "#define DECLASS__%s_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__%s_DFLT;", class_name, class_name);
  int macro_idx = strlen(ctor_macros);
  // Search for members that are also other class objects
  for(int l = 0; l < classes[total_classes].total_members; ++l) {
    if(classes[total_classes].member_object_class_name[l][0] != 0) { // member = class object
      // append macros to initialize any members that are class objects
      if(classes[total_classes].member_is_array[l] && !classes[total_classes].member_is_pointer[l]) { // append array construction
        sprintf(&ctor_macros[macro_idx], "\\\n\tDECLASS__%s_ARR(DECLASS_THIS.%s);", classes[total_classes].member_object_class_name[l], classes[total_classes].member_names[l]);
      } else if(!classes[total_classes].member_is_pointer[l]) {                                       // append single object construction
        sprintf(&ctor_macros[macro_idx], "\\\n\tDECLASS__%s_CTOR(DECLASS_THIS.%s);", classes[total_classes].member_object_class_name[l], classes[total_classes].member_names[l]);
      }
      macro_idx = strlen(ctor_macros);
    }
  }
  strcpy(&ctor_macros[macro_idx], "})");
  macro_idx = strlen(ctor_macros);

  // add macro for a an array of object constructions
  sprintf(&ctor_macros[macro_idx], "\n#define DECLASS__%s_ARR(DECLASS_ARR) ({\\\n\
  for(int DECLASS__%s_IDX=0;DECLASS__%s_IDX<(sizeof(DECLASS_ARR)/sizeof(DECLASS_ARR[0]));++DECLASS__%s_IDX)\\\n\
    DECLASS__%s_CTOR(DECLASS_ARR[DECLASS__%s_IDX]);\\\n})", class_name, class_name, class_name, class_name, class_name, class_name);
}

/******************************************************************************
* OBJECT METHOD PARSER
******************************************************************************/

// parse object method invocations -- 'method_words' only meaningful when 'is_nested_method' == true
int parse_method_invocation(char *s, char *NEW_FILE, int *j, bool is_nested_method, char method_words[][75]) {
  char *p = s, first_char = *s, new_fcn_call[75];
  FLOOD_ZEROS(new_fcn_call, 75);
  int method_name_size = 0;
  if(!(VARCHAR(*p)) && (VARCHAR(*(p + 1)))) {                                      // may be object
    p++;
    for(int i = 0; i < total_objects; ++i) {                                       // check for each object
      int len = strlen(objects[i].object_name);
      if(strlen(p) > len && is_at_substring(p, objects[i].object_name)) {          // found object
        int invoker_size = is_method_invocation(p + len);
        if(invoker_size == 0) continue;                                            // no invocation notation (no '.' nor '->')
        char invoked_member_name[75];
        get_invoked_member_name(p + len + invoker_size, invoked_member_name);      // get member name
        if(invoked_member_is_method(invoked_member_name, objects[i].class_name, is_nested_method)) { // member = method
          while((invoker_size = is_method_invocation(p)) > 0 || VARCHAR(*p)) {     // move p to after object & method names
            if(invoker_size == 0) invoker_size = 1; // VARCHAR
            p += invoker_size, method_name_size += invoker_size;
          }
          method_name_size++;                                                      // account for 1st char ('%c' in sprintf below)
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
void splice_in_prepended_method_name(char*new_fcn_call,char*p,int i,char*NEW_FILE,int*j,int*method_name_size,char method_words[][75]) {
  char objectName[200], objectChain[200]; // 'objectName' refers to outermost object in 'objectChain
  bool objectName_is_pointer = false;
  FLOOD_ZEROS(objectName, 200); 
  FLOOD_ZEROS(objectChain, 200); 
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
void splice_in_prepended_NESTED_method_name(char*new_fcn_call,char*p,int i,char*method_buff_idx,int*method_name_size,char method_words[][75]){
  char objectName[200], objectChain[200]; // 'objectName' refers to outermost object in 'objectChain
  bool objectName_is_pointer = false;
  FLOOD_ZEROS(objectName, 200);
  FLOOD_ZEROS(objectChain, 200);
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
void rmv_excess_buffer_objectChain(char *objectName, char *invokingObject, char *objectChain, char *buffer, char *new_fcn_call) {
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
  while(*open_bracket != '\0' && no_overlap(*open_bracket, "[;\n\t")) --open_bracket;
  while(*close_bracket != '\0' && no_overlap(*close_bracket, "];\n\t")) ++close_bracket, ++array_arg_size;
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
void get_object_name(char *outerMost_objectName, char *objectChain, char *buffer, int i, char method_words[][75], bool is_nested_object, bool *objectName_is_pointer) {
  // * outerMost_objectName is the name - w/o subscripts - of the outermost object leading the object chain of invocation
  // * the objectChain consists of an optional outer container object that contains the method-invoking
  //   object, as well as any potential array subscripts for either the container &/or invoking object.

  // find the chain's start, going past array subscripts, invocation notations, & object names.
  // assumes array subscripts are correct & user follows caveat (8) (will splice 'this->' as needed in subscript though)
  char *chain_head = buffer - 1, *chain_tail = buffer - 1;
  while(no_overlap(*chain_head, "\n\t;{}") && (VARCHAR(*chain_head) || !no_overlap(*chain_head, "[] .>"))) {
    if(*chain_head == ']') while(*chain_head != '[') --chain_head;   // don't parse subscripts yet - skip over
    if(*chain_head == '>' && *(chain_head - 1) == '-') --chain_head; // move past arrow notation
    --chain_head;
  }
  while(!VARCHAR(*chain_head)) ++chain_head;                         // move up to the first object name

  // move chain tail past the method name ('buffer' starts at '(' after method name) to end of invocation
  char invoc_punc[75];      // stores invocation punctuation IN REVERSE (ie '->' stored as '>-')
  FLOOD_ZEROS(invoc_punc, 75);
  int index = 0;
  while(VARCHAR(*chain_tail)) --chain_tail;                                                          // skip method name
  while(!VARCHAR(*chain_tail) && no_overlap(*chain_tail, "[]")) invoc_punc[index++] = *chain_tail--; // cpy/skip method invocation punct
  ++chain_tail;                                                                                      // move to 1 char past the chain's end
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
        read_chain += prefix_local_members_and_cpy_method_args(read_chain, write_chain, method_words, &filler_int_arg, ']');
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
int prefix_local_members_and_cpy_method_args(char *end, char *write_to_buffer, char method_words[][75], int *buffer_length, char delimiter) {
  int end_increment = 0;
  char *word_start = end + 1;
  while(*end != '\0' && *end != delimiter) { // copy method arguments
    // search the invoked local method's args for any local members
    if(!VARCHAR(*end) && VARCHAR(*(end+1))) word_start = end + 1; 
    else if(VARCHAR(*end) && !VARCHAR(*(end+1))) {                            // at argument word's end
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
    *write_to_buffer++ = *end++, *buffer_length += 1, ++end_increment;        // copy-current & traverse-next char in argument
  }
  return end_increment;
}

/******************************************************************************
* STORE OBJECT INFORMATION
******************************************************************************/

// stores an object's name & associated class name/type in global struct
bool store_object_info(char *s) {
  bool not_an_arg = true;
  char *q = s, *p = s, object_name[75], class_type_name[75];
  FLOOD_ZEROS(object_name, 75); 
  FLOOD_ZEROS(class_type_name, 75);
  int i = 0, j = 0;
  while(*q != '\0' && *q != ';') if(*q++ == ')') { not_an_arg = false; break; } // determine if arg
  while(!IS_WHITESPACE(*p)) class_type_name[i++] = *p++; p++;                   // skip class_type declaration
  bool is_class_pointer = false, is_class_array = false;
  if(*p == '*') is_class_pointer = true, p++;                                   // check if object == class pointer
  while(VARCHAR(*p)) object_name[j++] = *p++;                                   // store object's name, class, & ptr status
  if(*p == '[') is_class_array = true;                                          // check if object == class array
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

// parse & prepend function name w/ class (now struct) name
void get_prepended_method_name(char *s, char *class_name, char *prepended_method_name) {
  char method_name[75];
  FLOOD_ZEROS(method_name, 75);
  char *p = s, *name = method_name;
  while(*p != '\0' && IS_WHITESPACE(*p)) p++; p++;                          // skip tab
  while(*p != '\0' && *p++ != ' ');                                         // skip type
  while(*p != '\0' && VARCHAR(*p)) *name++ = *p++;                          // copy name
  *name = '\0';
  sprintf(prepended_method_name, "DECLASS_%s_%s", class_name, method_name); // className_'function name'
  // store method info in global class struct's instance of the current class
  strcpy(classes[total_classes].method_names[classes[total_classes].total_methods], method_name);
  classes[total_classes].total_methods += 1;
}

// returns a member's initialized value (0 by default) & returns how far back name is after value initialization
int get_initialized_member_value(char *member_end) {
  char *start_of_val;
  int i = 0, distance_back = 0;
  --member_end, ++distance_back;
  while(*member_end != '\0' && no_overlap(*member_end, "=;\n\t")) --member_end, ++distance_back; // find '='
  if(*member_end == '=') {                                                                       // is initialized value
    start_of_val = member_end + 1;
    while(IS_WHITESPACE(*start_of_val)) start_of_val++;                                          // find start of value
    for(; *start_of_val != ';'; i++, ++start_of_val)                                             // copy initialized value
      classes[total_classes].member_values[classes[total_classes].total_members][i] = *start_of_val;
    classes[total_classes].member_values[classes[total_classes].total_members][i] = '\0';
    while(IS_WHITESPACE(*member_end) || *member_end == '=') --member_end, ++distance_back;       // move ptr to end of member name to copy
    --distance_back; // [start, end) so move end ptr right after name
  } else {                                                                                       // no initialized value: set to 0
    classes[total_classes].member_values[classes[total_classes].total_members][0] = 0;
    classes[total_classes].member_values[classes[total_classes].total_members][1] = '\0';
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
  FLOOD_ZEROS(member_type, 100);
  FLOOD_ZEROS(member_name, 100);

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

// check for & store member in class - returns 0 = no member, 1 = member, 2 = member initialized with values
int get_class_member(char *end, bool is_fcn_ptr) {
  char *member_start, *member_end = end;
  while(IS_WHITESPACE(*member_end)) --member_end;
  if(*member_end == ';' || is_fcn_ptr) {                             // is member
    register_member_class_objects(member_end);                       // account for whether member is itself a class object
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
    classes[total_classes].total_members += 1;
    return 1 + (classes[total_classes].member_values[classes[total_classes].total_members-1][0] != 0);
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
  // if 'basic_c_types' found before ';' or ') {', & isn't an array declaration elt nor an 
  // array subscript, var is presumed to be a type redefinition/declaration (ie NOT a member)
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
    *method_buff_idx++ = *end++, prepended_size++;                      // move to first letter
    char method_name[75];
    FLOOD_ZEROS(method_name, 75);
    int i = 0;
    while(VARCHAR(*end)) method_name[i++] = *end++, prepended_size++;   // copy method name
    if(*end != '(') return 0;                                           // if no method
    method_name[i] = '\0';
    if(strcmp(classes[total_classes].class_name, class_name) == 0)  
      for(int j = 0; j < classes[total_classes].total_methods; ++j)           // find if class has method name
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
bool valid_member(char *word_start, char *memberName, char nextChar, char period, char method_words[][75], int word_size) {
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

  // # of class or comment chars
  int class_size = 0, class_comment_size, blank_line_size;

  // class scope btwn 'start' & 'end'
  char *start = class_instance, *end, *start_of_line;
  int in_class_scope = 1, in_method_scope;
  bool in_a_string;
  while(*start++ != '{') class_size++;
  end = start;

  // replace all comments in class w/ whitespaces
  whitespace_all_class_comments(end);

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
      get_prepended_method_name(start_of_line, class_name, prepended_method_name);

      // remove method from struct_buff (only members)
      char *wipe_method_line = start_of_line;
      while(wipe_method_line++ != end) *struct_buff_idx-- = '\0';

      // copy method to 'method_buff' & move 'end' forward
      while(IS_WHITESPACE(*start_of_line) && start_of_line != end)  *method_buff_idx++ = *start_of_line++; // copy indent
      while(!IS_WHITESPACE(*start_of_line) && start_of_line != end) *method_buff_idx++ = *start_of_line++; // copy type
      sprintf(method_buff_idx, " %s", prepended_method_name);                                              // copy appended method name
      method_buff_idx += strlen(method_buff_idx);                                                          // move method_buff_idx to '\0'

      // store method's arg words in 'method_words[][]' to discern from local class member vars
      while(*(end + 1) != '\0' && *end != ')') {
        // Check for class objects in method argument
        int k; 
        for(k = 0; k < total_classes + 1; ++k)
          if(is_at_substring(end, classes[k].class_name)) {
            store_object_info(end);                                                       // register object arg as a class object instance
            break; 
          }
        // Save method's words in argument
        if(k == total_classes + 1){
          if(!VARCHAR(*end) && *end != ')' && VARCHAR(*(end + 1))) word_start = end + 1;
          else if(VARCHAR(*end) && !VARCHAR(*(end + 1)))
            add_method_word(method_words, &word_size, word_start, end);                   // add arg word to 'method_words[][]'
        }
        *method_buff_idx++ = *end++, class_size++;                                        // copy method up to ')'
      }

      // splice in'this' class ptr as last arg in method
      if(*(end - 1) == '(') sprintf(method_buff_idx, "%s *this", class_name);             // spliced class ptr is single method arg                      
      else                  sprintf(method_buff_idx, ", %s *this", class_name);           // spliced class ptr is poly method arg
      method_buff_idx += strlen(method_buff_idx); 
      while(*end != '\0' && *(end - 1) != '{') *method_buff_idx++ = *end++, class_size++; // copy method up to '{'

      // copy the rest of the method into 'method_buff'
      in_class_scope++;                                                                   // skip first '{'
      while(*end != '\0' && in_method_scope > 0 && in_class_scope > 0) {                  // copy method body
        // account for current scope & cpy method
        if(*end == '{') in_method_scope++, in_class_scope++;
        else if(*end == '}') in_method_scope--, in_class_scope--;
        else if(*end == '"' && *(end - 1) != '\\') in_a_string = !in_a_string;            // don't prefix '->' to member names in strings
        if(in_method_scope < 0 || in_class_scope < 0) break;

        // check for class object declaration
        for(int k = 0; !in_a_string && k < total_classes + 1; ++k)
          if(is_at_substring(end, classes[k].class_name) 
            && !VARCHAR(*(end-1)) && !VARCHAR(*(end+strlen(classes[k].class_name)))) {
            if(store_object_info(end)) {
              char *already_assigned = end;
              while(*already_assigned != '\0' && no_overlap(*already_assigned, "\n;,=")) ++already_assigned;
              if(*already_assigned == '=') break; 
              // initialization undefined -- use default initial values
              while(*end != '\0' && *(end-1) != ';') *method_buff_idx++ = *end++, ++class_size;
              if(objects[total_objects-1].is_class_array) // object = array, use array macro init
                sprintf(method_buff_idx, " DECLASS__%s_ARR(%s);", classes[k].class_name, objects[total_objects-1].object_name);
              else                                        // object != array, so use single-object macro init
                sprintf(method_buff_idx, " DECLASS__%s_CTOR(%s);", classes[k].class_name, objects[total_objects-1].object_name);
              method_buff_idx += strlen(method_buff_idx);
            }
            break;
          }

        // either add method word to 'method_words[][]' or prepend 'this->' to member
        if(!in_a_string) {
          if(!VARCHAR(*end) && *end != '\'' && VARCHAR(*(end + 1))) word_start = end + 1; // beginning of word
          else if(VARCHAR(*end) && !VARCHAR(*(end + 1))) {                                // end of word - member or var
            // check if a member of the current/local class
            int i = 0;
            for(; i < classes[total_classes].total_members; ++i) {
              if(classes[total_classes].member_names[i][0] == 0) continue; // struct member -- disregard
              char *endOfMember = word_start + strlen(classes[total_classes].member_names[i]);
              char nextChar = *endOfMember, period = *(endOfMember + 1);   // either a whole word or class' struct member invocation
              // word is a member of the local class -- prefix 'this->'
              if(valid_member(word_start - 1, classes[total_classes].member_names[i], nextChar, period, method_words, word_size)) {
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
        int nested_method_size = (!in_a_string)?parse_method_invocation(end,method_buff_idx, &method_buff_idx_position,true,method_words):0;
        end += nested_method_size, class_size += nested_method_size;

        // check whether nested method or not
        if(nested_method_size > 0) {
          method_buff_idx += strlen(method_buff_idx);
        } else {

          // check whether nested method, but for local class (ie NOT for another object)
          int local_nested_method_size = (!in_a_string) ? parse_local_nested_method(end, method_buff_idx, class_name, method_words) : 0;
          if(local_nested_method_size > 0) {
            end += local_nested_method_size, class_size += local_nested_method_size;
            method_buff_idx += strlen(method_buff_idx);
          }
          *method_buff_idx++ = *end++, class_size++;
        }
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
  char macro_comment[100], struct_comment[100], method_comment[100];
  FLOOD_ZEROS(macro_comment, 100); FLOOD_ZEROS(struct_comment, 100); FLOOD_ZEROS(method_comment, 100);
  sprintf(macro_comment, "/* %s CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */\n", class_name);
  sprintf(struct_comment, "\n\n/* %s CLASS CONVERTED TO STRUCT: */\n", class_name);
  sprintf(method_comment, "\n\n/* %s CLASS METHODS SPLICED OUT: */", class_name);

  // make a global class object with default values to initialize client's unassigned class objects with
  char initial_values_brace[200];
  FLOOD_ZEROS(initial_values_brace, 200);
  mk_initialization_brace(initial_values_brace, total_classes);
  char class_global_initializer[375];
  FLOOD_ZEROS(class_global_initializer, 375);
  sprintf(class_global_initializer, "\nconst %s DECLASS__%s_DFLT = %s;", class_name, class_name, initial_values_brace);

  // make macro constructors to assign any objects of this class its default values, as well as
  // initialize any contained class object members too - both for single & array instances of objects
  char ctor_macros[2500];
  FLOOD_ZEROS(ctor_macros, 2500);
  mk_ctor_macros(ctor_macros, class_name);

  // struct before methods to use class/struct type for method's 'this' ptr args
  if(strlen(struct_buff) > 0) {
    APPEND_BUFF_OR_STR_TO_NEW_FILE("/******************************** CLASS START ********************************/\n");
    APPEND_BUFF_OR_STR_TO_NEW_FILE(macro_comment);  APPEND_BUFF_OR_STR_TO_NEW_FILE(ctor_macros);
    APPEND_BUFF_OR_STR_TO_NEW_FILE(struct_comment); APPEND_BUFF_OR_STR_TO_NEW_FILE(struct_buff);
    APPEND_BUFF_OR_STR_TO_NEW_FILE(class_global_initializer);
    if(strlen(method_buff) > 0) APPEND_BUFF_OR_STR_TO_NEW_FILE(method_comment); APPEND_BUFF_OR_STR_TO_NEW_FILE(method_buff);
    APPEND_BUFF_OR_STR_TO_NEW_FILE("\n/********************************* CLASS END *********************************/");
  }

  total_classes++;
  return class_size;
  #undef skip_over_blank_lines // nothing below, but just for the sake of keeping it local to the function
}
