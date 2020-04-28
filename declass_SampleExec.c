// Author: Jordan Randleman - Sample Implementation of Classes for 'declass.c'
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// declass.c - Optional Interpreter Flags (ordered by precedence)
// allows user to tweak how their program gets converted to classes if they so desire
// #define DECLASS_IGNORE       // prevents declass.c from parsing file
// #define DECLASS_STRICTMODE   // disables smrtptr.h & "immortal" keyword, & enables dtors for returned objs
// #define DECLASS_NSMRTPTR     // disables default inclusion of "smrtptr.h" library for garbage collection
// #define DECLASS_NIMMORTAL    // disables default enabling of "immortal" keyword denoting objs that are never destroyed
// #define DECLASS_DTORRETURN   // enables dtors for objects being returned
// #define DECLASS_NOISYSMRTPTR // printf's alerts for every smrtptr.h's alloc/free (superseded by "DECLASS_NSMRTPTR")
// #define DECLASS_NDEBUG       // disables all smrtptr.h's "smrtassert()" statements
// #define DECLASS_NCOMPILE     // declass.c declassifies but DOESN'T GCC compile converted file
// #define DECLASS_NC11         // GCC compiles declassified file w/o "-std=c11"
// #define DECLASS_NCOLA        // prevents declass.c from passing converted file to cola.c for arg # overloads & dflt fcn values

// users can list custom object memory allocation fcns to be recognized
// by declass.c in implementing constructors correctly for object ptrs
// #define DECLASS_ALLOC_FCNS allocer1, allocer2 // NOTE: declass.c default recognizes malloc/calloc/smrtmalloc/smrtcalloc

/*****************************************************************************
 ||^\\ //=\\ //=\ /| |\ /\\  //\ /|==\ /\\ ||==== //^\\ ==== ==== //=\\ /\\ ||
 ||  ))|| || ||   || || ||\\//|| ||=   ||\\|| || |/===\| ||   ||  || || ||\\||
 ||_// \\=// \\=/ \\=// || \/ || \|==/ || \// || ||   || ||  ==== \\=// || \//
 *****************************************************************************
 *                 -:- DECLASS.C 10 CAVEATS WRT CLASSES -:-                 *
 *   (0) RESERVED: "DC_" PREFIX,"this" PTR,"CONST","INLINE", & STORAGE VARS *
 *   (1) DECLARE CLASSES GLOBALLY & OBJECTS LOCALLY (NEVER IN STRUCT/UNION) *
 *   (2) DECLARE MEMBERS/METHODS USED IN A METHOD ABOVE ITS DECLARATION     *
 *   (3) DECLARE CLASS MEMBERS, METHODS, & OBJECTS INDIVIDUALLY:            *
 *       (*) IE NOT:   "className c(), e();"                                *
 *       (*) RATHER:   "className c(); <press enter> className e();"        *
 *   (4) NO NESTED CLASS DECLARATIONS NOR METHOD INVOCATIONS:               *
 *       (*) IE NOT:   "someObj.method1(someObj.method2());"                *
 *       (*) RATHER:   "int x = someObj.method2(); someObj.method1(x);"     *
 *   (5) CLASS ARRAYS RECEIVED AS ARGS MUST BE DENOTED W/ "[]" NOT "*":     *
 *       (*) IE NOT:   "func(className *classArr){...}"                     *
 *       (*) RATHER:   "func(className classArr[]){...}"                    *
 *   (6) NO PTR TO OBJECT ARRAY OR VISE-VERSA (UNDEFINED BEHAVIOR):         *
 *       (*) IE NOT:   "className (*ptrToArrObj)[10];"                      *
 *       (*) RATHER:   ptr to an obj w/ array of obj member (or vise-versa) *
 *   (7) NO MULTIDIMENSIONAL OBJECT ARRAYS (UNDEFINED BEHAVIOR):            *
 *       (*) IE NOT:   className objMatrix[10][10];                         *
 *       (*) RATHER:   array of objects each w/ array of objects member     *
 *   (8) CONTAINMENT, NOT INHERITANCE: CLASSES CAN ONLY ACCESS MEMBERS &    *
 *       METHODS OF THEIR OWN IMMEDIATE MEMBER CLASS OBJECTS:               *
 *       (*) NOTE:     let "->" denote "can access the members of"          *
 *       (*) IE:       suppose classes c1, c2 & c3: w/ c1 in c2 & c2 in c3. *
 *                     c3 -> c2, and c2 -> c1, BUT NOT c3 -> c1.            *
 *       (*) RATHER:   (1) include a c1 object as a member in c3, OR:       *
 *                     (2) mk c2 methods invoking c1 methods: c3 interface  *
 *   (9) ONLY BINARY SINGLE-LINE CONDITIONAL ("?:") OBJECT RETURNS:         *
 *       (*) IE NOT:   "return case ? obj : case2 ? obj2 : obj3;"           *
 *       (*) RATHER:   "if(case) return obj; return case2 ? obj2 : obj3;"   *
 *****************************************************************************
 *                       -:- DECLASS.C & SMRTPTR.H -:-                      *
 *    SMRTPTR.H LIBRARY IS DEFAULT INCLUDED, W/ IMPROVED MALLOC, CALLOC,    *
 *    REALLOC, FREE, & ASSERT FUNCTIONS AS WELL AS GARBAGE COLLECTION       *
 *      (*) "smrtptr.h" fcns same as stdlib/assert's all prefixed w/ "smrt" *
 *****************************************************************************
 *                        -:- DECLASS.C & COLA.C -:-                        *
 *    COLA.C (C OVERLOADED LENGTH ARGS) PARSER DFLT APPLIED PRIOR COMPILING *
 *      => ALLOWS FCN/MACRO POLYMORPHISM (2+ W/ SAME NAME) SO LONG AS THEY  *
 *         TAKE DIFFERENT NUMBERS OF ARGS AS PARAMETERS!                    *
 *           (*) thus "CTOR"s CAN be overloaded but "DTOR"s CANNOT          *
 *      => ALSO ENABLES DEFAULT FCN/METHOD ARG VALUES!                      *
 *      => "ODV" GUIDLINE BELOW HELPS AVOID OVERLOAD AMBIGUITY W/ DFLT VALS *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  *
 *                         -:- COLA.C 8 CAVEATS -:-                         *
 *   (*) NOTE: a "COLA INSTANCE" = a fcn/macro overload OR fcn w/ dflt vals *
 *   (0) NO VARIADIC COLA INSTANCES                                         *
 *   (1) NO FCN PTRS POINTING TO COLA INSTANCES                             *
 *       (*) can't determine overloaded arg # from only overloaded fcn name *
 *   (2) NO REDEFINING COLA INSTANCE NAME TO OTHER VARS REGARDLESS OF SCOPE *
 *   (3) NO OVERLOADED MACROS CAN EVER BE "#undef"'d                        *
 *   (4) ONLY COLA INSTANCES DEFINED/PROTOTYPED GLOBALLY WILL BE RECOGNIZED *
 *   (5) ONLY FUNCTIONS MAY BE ASSIGNED DEFAULT VALUES - NEVER MACROS!      *
 *   (6) NO ARG W/ A DEFAULT VALUE MAY PRECEDE AN ARG W/O A DEFAULT VALUE   *
 *       (*) args w/ default values must always by last in a fcn's arg list *
 *   (7) FCN PROTOTYPES TAKE PRECEDENT OVER DEFINITIONS WRT DEFAULT VALS    *
 *       (*) if a fcn proto has default vals but its defn doesn't (or vise  *
 *           versa) fcn will be treated as if both had the default vals     *
 *       (*) if a fcn proto has DIFFERENT default vals from its defn, the   *
 *           fcn's proto default vals are treated as the only default vals  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  *
 *  -:- ODV GUIDELINE TO COMBINE (O)VERLOADS W/ (D)EFAULT ARG (V)ALUES -:-  *
 * >> overload definitions must satisfy 1 of the following:                 *
 *    (1) an overload's # of non-dflt args must exceed the # of cumulative  *
 *        args (both dflt & not) of all other overloaded instances          *
 *    (2) an overload's # of cumulative args (both dflt & not) must be less *
 *        than the # of non-dflt args of all other overloaded instances     *
 *****************************************************************************
 *                     -:- DECLASS.C CTORS & DTORS -:-                      *
 *   FORMATTING:                                                            *
 *     (0) CONSTRUCTOR(CTOR) DENOTED AS TYPELESS METHOD W/ CLASS' NAME      *
 *         (*) can initialize contained object members as default value     *
 *         (*) user-invoked in obj declaration, auto-assigns defaults 1st   *
 *         (*) can take arguments                                           *
 *     (1) DESTRUCTOR(DTOR) DENOTED LIKE CTOR, BUT PREFIXED W/ '~':         *
 *         (*) container objs auto-dtor any member objs 1st in destruction  *
 *         (*) either invoked by user or autonomously once obj left scope   *
 *         (*) NEVER takes arguments                                        *
 *     (2) FOR BOTH CTORS & DTORS:                                          *
 *         (*) NEVER have a "return" value (being typeless)                 *
 *         (*) user can explicitly invoke (object "oName" & class "cName"): *
 *             => DTOR: destroy oName immediately: "~oName();"              *
 *             => CTOR: return a ctor'd cName obj instance: "cName(args);"  *
 *                => these so-called "dummy" ctors don't init specific objs *
 *                   rather they return a single instance of a ctor'd obj   *
 *                => "cName oName(args);" == "cName oName = cName(args);"   *
 *   DEFAULT PROPERTIES:                                                    *
 *     (0) DEFAULT CLASS CTOR/DTOR PROVIDED IF LEFT UNDEFINED BY USER       *
 *     (1) 3 KINDS OF OBJECTS NEVER DTOR'D IN THEIR IMMEDIATE SCOPE:        *
 *         (*) object args: dtor'd at the end of their invoking fcn's scope *
 *         (*) returned obj: assumed assigned as a val 2B dtor'd externally *
 *         (*) "immortal" obj: never dtor'd, see below to learn more        *
 *         => NOTE: THE LAST 2 ABOVE CAN BE DTOR'D W/ MACRO FLAGS 3-4 BELOW *
 *   OBJECT DECLARATIONS:                                                   *
 *     (0) SINGLE/ARRAY OBJ DEFAULT VALS:                                   *
 *         (*) SINGLE:   "className objectName;"                            *
 *         (*) ARRAY:    "className objectName[size];"                      *
 *     (1) SINGLE/ARRAY OBJ CTORS (AUTO-APPLIES DFLTS 1ST):                 *
 *         (*) SINGLE:   "className objectName(args);"                      *
 *         (*) ARRAY:    "className objectName[size](args);"                *
 *     (2) OBJ PTRS & INITIALIZING W/ CTOR/DFLT/NEITHER ("alloc" = any fcn) *
 *         (*) "className *objectName;" // "dangling" ptr, neither          *
 *         (*) "className *objectName(args);"           // ctor             *
 *         (*) "className *objectName(args) = alloc();" // ctor & dflt      *
 *     (3) OBJ PTR BEST PRACTICES TO REDUCE RISK OF ERRORS:                 *
 *         (*) DANGLING PTR: keep "immortal" unless class designed for ptrs *
 *         (*) NON-DANGLING: allocate memory & ctor upon declaration        *
 *****************************************************************************
 *                  -:- DECLASS.C & "immortal" KEYWORD -:-                  *
 *   (0) OBJECTS DECLARED "immortal" NEVER INVOKE THEIR DTOR; SEE (2) BELOW *
 *       (*) DECLARATION: "immortal className objName(args);"               *
 *       (*) contained objects can also be immortal                         *
 *       (*) WARNING: AN "IMMORTAL" OBJECT'S MEMBERS ALSO ALL = "IMMORTAL"  *
 *   (1) OBJ ARGS ALWAYS IMMORTAL: AS PLACEHOLDERS, THE PASSED OBJ THEY     *
 *       REPRESENT AREN'T OUT OF SCOPE ONCE FCN ENDS (NO DOUBLE DTOR)       *
 *       (*) thus denoting obj args as "immortal" is redundant              *
 *       (*) "MACRO FLAG" (3) disables all immortal obj EXCEPT obj args     *
 *   (2) IMMORTAL OBJECTS CAN ONLY BE DTOR'D IF EXPLICITLY BY THE USER, IE: *
 *       (*) immortal object "oName" only destroyed if "~oName();" invoked  *
 *****************************************************************************
 *                    -:- DECLASS.C 4 CMD LINE FLAGS -:-                    *
 *   (0) SHOW CLASS-OBJECT & COLA-OVERLOADING DATA:                         *
 *         (*) "-l": $ ./declass -l yourFile.c                              *
 *   (1) SAVE TEMP FILE MADE PRIOR TO PASSING CONVERTED FILE TO COLA.C:     *
 *         (*) "-save-temps": $ ./declass -save-temps yourFile.c            *
 *   (2) DON'T AUTO-COMPILE CONVERTED FILE (like #define DECLASS_NCOMPILE)  *
 *         (*) "-no-compile": $ ./declass -no-compile yourFile.c            *
 *   (3) MAKE FATAL ERRORS ASK USER WHETHER TO QUIT (RATHER THAN AUTOMATIC) *
 *         (*) DISCOURAGED last-resort way 2 debug, but errors 4 a reason!  *
 *         (*) "-mortal-errors": $ ./declass -mortal-errors yourFile.c      *
 *   ->> Combine any of the above, so long as "yourFile.c" is the last arg  *
 *         (*) VALID:   $ ./declass -no-compile -l -save-temps yourFile.c   *
 *         (*) INVALID: $ ./declass -no-compile -l yourFile.c -save-temps   *
 *****************************************************************************
 *               -:- DECLASS.C SPECIALIZATION MACRO FLAGS -:-               *
 *   BY PRECEDENCE:                                                         *
 *     (0) "#define DECLASS_IGNORE"       => DECLASS.C WON'T CONVERT FILE   *
 *     (1) "#define DECLASS_STRICTMODE"   => ENABLES MACRO FLAGS 2-4        *
 *     (2) "#define DECLASS_NSMRTPTR"     => DISABLES "SMRTPTR.H"           *
 *     (3) "#define DECLASS_NIMMORTAL"    => DISABLES "immortal" KEYWORD    *
 *     (4) "#define DECLASS_DTORRETURN"   => ALSO DTOR RETURNED OBJECTS     *
 *     (5) "#define DECLASS_NOISYSMRTPTR" => ALERT "SMRTPTR.H"'S ALLOC/FREE *
 *     (6) "#define DECLASS_NDEBUG"       => DISABLE "SMRTPTR.H" SMRTASSERT *
 *     (7) "#define DECLASS_NCOMPILE"     => ONLY CONVERT DON'T GCC COMPILE *
 *     (8) "#define DECLASS_NC11"         => GCC COMPILE W/O "-std=c11"     *
 *     (9) "#define DECLASS_NCOLA"        => DISABLE COLA.C OVERLOADS/DFLTS *
 *   DEFINING CUSTOM MEMORY ALLOCATION FUNCTIONS:                           *
 *     (0) declass.c relies on being able to identify memory allocation     *
 *         fcns to aptly apply dflt vals (not assigning garbage memory)     *
 *     (1) aside from malloc/calloc/smrtmalloc/smrtcalloc, users can        *
 *         declare their own custom class obj memory allocation functions   *
 *         to be recognized by declass.c at the top of their program        *
 *         (*) NOTE: ASSUMES ALL ALLOC FCNS RETURN "NULL" OR EXIT ON FAIL   *
 *     (2) list alloc fcn names after a "#define DECLASS_ALLOC_FCNS" macro  *
 *****************************************************************************
 *                     -:- DECLASS.C & HEADER FILES -:-                     *
 *   DENOTING HEADER FILES TO ALSO BE PARSED BY DECLASS.C:                  *
 *     (0) name must be prefixed w/ "DECLASS_H_" or "declass_h_"            *
 *     (1) name must contain the ".h" file extension                        * 
 *         (*) IE: #include "DECLASS_H_yourHeader.h"                        *
 *   IMPLEMENTATION:                                                        *
 *     (0) declass.c supports including header files that already contain   *
 *         classes (& arg # overloads/fcn dflt values if COLA not disabled) *
 *     (1) thus can write 1 class for several files, reduces redundant code *
 *     (2) as w/ a linker, header files are prepended to the main file to   *
 *         simulate as if user wrote a monolithic code base                 *
 *         (*) THUS HEADER FILES & MAIN FILE MUST CUMULATIVELY TAKE LESS    *
 *             MEMORY THAN THE "MAX_FILESIZE" MACRO!                        *
 *     (3) header files NOT prefixed "DECLASS_H_" or "declass_h_" will NOT  *
 *         be prepended to the main codebase by declass.c                   *
 *****************************************************************************/

class Student {
  // note that class member values default to 0 unless otherwise indicated, &
  // smrtmalloc/smrtcalloc default values are freed automatically atexit if 
  // "DECLASS_NSMRTPTR" not #defined
  char *fullname = smrtmalloc(sizeof(char)*50); 
  char school[15] = "SCU";         // default school name
  int year = 14;                   // default school year 
  long studentId;
  char *(*copy_fcnPtr)() = strcpy; // fcn ptr member default value

  struct grade_info {
    char major[50]; 
    float out_of;
    float gpa;
  } grades = {"Computer Science Engineering", 4.0}; /* assign struct values like vanilla C, via brace notation */

  // note that class methods can refer to their class' members & other methods
  // w/o any prefixes, as declass.c will detect which class object is invoking
  // the method & implement the appropriate operations.
  void assignName(char *fname) {
    copy_fcnPtr(fullname, fname);
  }
  void getName(char *name) {
    strcpy(name, fullname);
  }
  void assignGpa(float grade) { grades.gpa = grade; }
  void assignId(long id) { studentId = id; }
  long getId() { return studentId; }
  void show() {
    if(getId()) {
      printf("Name: %s, School: %s, Year: %d, id: %ld", fullname, school, year, studentId);
      printf(" Major: %s, GPA: %.1f/%.1f\n", grades.major, grades.gpa, grades.out_of);
    }
  }

  // note: neither constructors (ctors) nor destructors (dtors) ever have a return value

  // format a class ctor by making a "typeless" method w/ the class' name
  // => gets invoked at every object delcaration so long as "(<args>)" are provided,
  //    otherwise object only get default values w/o calling its ctor
  Student(char *userName, long id, float gpa) {
    assignName(userName);
    assignId(id);
    assignGpa(gpa);
  }

  // cola.c's default inclusion in declass.c (unless "#define DECLASS_NCOLA" is detected)
  // allows functions & macros to be overloaded based on how many arguments they take, & for 
  // all functions & methods (not macros!) to give their arguments default values
  // => thus this ctor of 2 args overloads the above ctor of 3 args
  // => NOTE: args w/ default values must be listed last in a function/method's arg list, 
  //    "Student(char *userName = "Default Student Name", float gpa)" would throw an error here
  // => dtors can NEVER be COLA (C Overloaded Length Args) overloaded since they never have ANY args
  Student(float gpa, char *userName = "\"Default Student Name\"") {
    assignName(userName);
    assignId(1000000);
    assignGpa(gpa);
  }

  // format a class dtor like a ctor, except w/ a '~' prefix (unary 'not' operator)
  // => note: dtors NEVER have args
  ~Student() {
    printf("\"Student\" object named \"%s\" Destroyed!\n", fullname);
  }

  // method to create a new object
  Student createAStudent(char *name, long id, float gpa) {
    Student methodMadeStudent;
    methodMadeStudent.assignName(name);
    methodMadeStudent.assignId(id);
    methodMadeStudent.assignGpa(gpa);
    return methodMadeStudent;
  }

  // method swaps 2 student objects by using 'this' ptr 
  // to refer to the invoking object as a whole entity
  void swap(Student *blankStudent) { 
    // note that since "DECLASS_NIMMORTAL" is not #defined, the "immortal" prefix 
    // means that the "Student" dtor will never be called for this "temp" object
    immortal Student temp = *this; 
    *this = *blankStudent; 
    *blankStudent = temp;
  }
}



class College {
  // contained array of "Student" objects w/in each "College" object
  immortal Student body[10]; // note the "immortal" prefix
  char name[20];
  int foundingYear;
  char state[3] = "CA";

  void addName(char *uniName) { strcpy(name, uniName); }
  void addFoundingAndName(int founded, char *name) {
    if(founded < 0) printf("A school founded in BC? Really?\n");
    foundingYear = founded;
    // note "name" redefinition from argument overides this class' "name"
    // member, so assign the College object's name value via another method
    addName(name); 
  }
  void addStudents(char names[10][20], float gpas[10], long ids[10]) {
    for(int i = 0; i < 10; ++i) {
      if(i % 2 == 0)                        // same name-assignment operation via 2 approaches
        strcpy(body[i].fullname, names[i]); // access member's fullname directly
      else
        body[i].assignName(names[i]);       // use member's own method to assign its name
      body[i].assignGpa(gpas[i]);
      body[i].assignId(ids[i]);
    }
  }

  void show() {
    printf("College Name: %s, State: %s, Year Founded: %d.\n", name, state, foundingYear);
    printf("%s's Students:\n", name);
    for(int i = 0; i < 10; ++i) {
      printf("\tStudent No%d: ", i + 1);
      body[i].show();
    }
  }
  ~College() { // "College" object destructor
    printf("\"College\" object named \"%s\" Destroyed!\n", name);
  }
}



class Region {
  College schools[2];
  int totalSchools; // class members default to 0, no need to assign
  char regionName[20];

  // note that ctors can be used to initialize contained
  // objects w/ the same notation as w/ any other context:
  // see the main function below for more examples
  Student topStudent("Stephen Prata", 12121212, 4.0);
  Student second3rd4thBestStudents[3]("John Doe", 11111110, 8.0);

  void setRegionName(char *name) { strcpy(regionName, name); }
  void addSchool(College school) {
    if(totalSchools == 2) {
      printf("Region Maxed out of Schools!\n");
      return;
    }
    schools[totalSchools++] = school;
    /* note below the access to immediate contained member 'Student' w/in 'College': despite
     * being in a 'Region' method, outermost object 'College' still only accessing its
     * immediate 'Student' member (so valid by caveat 8). If this were outside of the
     * method and invoked as "RegionObj.schools[0].body[0].assignName("CAMERON")",
     * caveat 8 would be violated & cause undefined behavior since the outermost
     * object class name 'Region' is 2 layers rmvd (no immediate access) from its
     * "College" array's "Student" array member */ 
    schools[totalSchools-1].body[totalSchools - totalSchools].assignName("CAMERON"); 
  }
  void show() {
    printf("Region Name: %s, Total Schools: %d.\n", regionName, totalSchools);
    for(int i = 0; i < totalSchools; ++i) {
      printf("School No%d:\n", i + 1);
      schools[i].show();
    }
  }
  void showTopStudents() {
    printf("\tThe %s Region's Top Student:\n\t\t", regionName);
    topStudent.show();
    printf("\tThe Next 3 Runner-Ups:\n");
    for(int i = 0; i < 3; ++i) {
      printf("\t\t");
      second3rd4thBestStudents[i].show();
    }
  }
}



// note that objects acting as arguments never have their dtor invoked, as the object
// they represent will be dtor'd in its own scope outside this fcn (prevents double dtors)
void showAllNames(Student person, Student *sharpStudent, Student people[6]) {
  char personName[30], people3Name[30], sharpStudentName[30];
  person.getName(personName);
  people[3].getName(people3Name);
  sharpStudent -> getName(sharpStudentName);
  printf("person's Name: %s, people[3]'s Name: %s", personName, people3Name);
  printf(", *sharpStudent's Name: %s\n", sharpStudentName);
}


College createCollege(char *name, int foundingYear) {
  // note that so long as "#define DECLASS_DTORRETURN" is not active, returned
  // objects also don't have their dotrs invoked to ensure objects get the
  // value they expect from a function (w/o it being literally destroyed B4 being returned)
  College someUniversity; 
  someUniversity.addName(name);
  someUniversity.foundingYear = foundingYear;
  return someUniversity;
}



int main() {

  // Single object
  printf("Working with a single \"Student\" object:\n");
  // note that no parenthesis means no ctor, 'jordanCR' 
  // will only start out w/ it's default values
  Student jordanCR; 
  jordanCR.assignId(1524026);              // assign ID to Student object
  long myId = jordanCR.getId();            // get ID
  if(2 * jordanCR.getId() > 1000)
    printf("\tStudent object: school = %s, id = %ld, myId: %ld\n", jordanCR.school, jordanCR.studentId, myId);
  jordanCR.assignName("Jordan Randleman"); // assign a name
  jordanCR.assignGpa(3.9);                 // assign a GPA
  printf("\t");
  jordanCR.show();                         // output information


  // Single object constructor
  printf("\nWorking with an single \"Student\" object initialized via its constructor:\n");
  Student koenR("Koen Randleman", 1122334, 4.0); // invoke an object's class ctor by declaring it like a fcn
  printf("\t");
  koenR.show();


  // Single object COLA-overloaded constructor (overloading enabled via my declass.c-embedded cola.c parser)
  // => note that overloading fcns/macros by their # of args will NOT work if "#define DECLASS_NCOLA" is detected
  printf("\nWorking with an single \"Student\" object initialized via its overloaded constructor:\n");
  Student luluR(5.0, "Louis Randleman"); // invokes the object's class ctor variant accepting 2 args rather than 3
  printf("\t");
  luluR.show();


  // Single object COLA-overloaded ctor that ALSO uses a default value (again via my declass.c-embedded cola.c parser)
  printf("\nWorking with an \"Student\" object constructed via an overloaded ctor w/ a default \"name\" arg value:\n");
  Student charmR(5.5); // invokes the object's class ctor accepting 2 args while using the 2nd arg's default value
  printf("\t");
  charmR.show();


  // Single object "dummy" constructor
  // referred to as a "dummy ctor" since it changes no actual objects directly,
  // rather it returns a premade object initilaized by user's ctors & dflt vals
  printf("\nUsing the \"dummy\" constructor:\n");
  Student tessaR = Student("Tessa Randleman", 1678, 4.0); // also invokes the "Student" ctor while returning a ctor'd obj
  tessaR.show();


  // Object array
  printf("\nWorking with an array of 6 \"Student\" objects:\n");
  Student class[6];
  char names[6][20] = {"Cameron", "Sidd", "Austin", "Sabiq", "Tobias", "Gordon"};
  for(int i = 0; i < 6; ++i) {
    class[i].assignName(names[i]);
    class[i].assignId(i * 256 + 1000000);
    class[i].assignGpa(i * 0.75); // sorry Cameron
  }
  for(int i = 0; i < 6; ++i) {
    printf("\t");
    class[i].show();
  }


  // Object array constructor
  printf("\nWorking with a constructor to initialize an array of 6 \"Student\" objects:\n");
  // => note that to use a ctor w/ an array of objects, denote it like  a single obj
  //    while treating the array subscript as part of its name
  // => note that arrays w/ a ctor apply it across all of its elements
  Student group[6]("group_student", 1111111, 3.0);
  for(int i = 0; i < 6; ++i) {
    printf("\t");
    group[i].show();
  }


  // Object pointer
  printf("\nWorking with a \"Student\" object pointer:\n");
  Student *JT = &class[5]; // object ptr assigned address of 6th object in object array
  JT -> assignName("JT");  // arrow notation to invoke ptr object members & methods
  printf("\t");
  JT->show();
  long jtId = JT-> getId();
  printf("\tJT \"Student\" object pointer's ID: %ld\n", jtId);


  // Object pointer constructor
  // use a constructor and allocator together in order to allocate 
  // memory then construct an object with a ptr
  printf("\nAllocating and constructing a \"Student\" object pointer in a single line:\n");
  Student *Alex("Alex", 88888, 4.0) = smrtmalloc(sizeof(Student));
  Alex -> show();


  // Explicitly invoking an object's destructor
  // immediately invokes user-defined dtor for the object
  printf("\nExplicitly invoking a \"Student\" object's destructor:\n");
  ~Alex();


  // Object containment (objects w/in objects)
  printf("\nWorking with contained \"Student\" objects within a \"College\" object:\n");
  // note that despite not having defined a "College" ctor, declass.c will automatically generate one
  // for any classes w/out a user-defined ctor (same goes w/ dtors) so that the '()' notation is always 
  // valid. in this case, however, it's the same effect as if we left them out (only default values)
  College Scu();  
  Scu.addFoundingAndName(1851, "SCU");
  char studentNames[10][20] = {"Cameron","Sidd","Austin","Sabiq","Tobias","Gordon","Jason","Ronnie","Kyle","Peter"};
  float studentGpas[10]     = {4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9};
  long studentIds[10]       = {4000000,3900000,3800000,3700000,3600000,3500000,3600000,3700000,3800000,3900000};
  Scu.addStudents(studentNames, studentGpas, studentIds);
  // Note that all member objects in the College object's 'Student' array are initialized w/ their majors/max GPA.
  // Classes auto-ctor their contained member objects w/ the appropriate default values as well & set ptrs to NULL
  Scu.show(); 


  // Passing an object by value, pointer, & array to another function
  printf("\nPassing an object pointer, array, & value to another function:\n");
  printf("\t");
  showAllNames(jordanCR, JT, class);


  // Having a function return an object
  printf("\nHaving a function make & return a \"College\" object:\n");
  // note the College object is initialized by a value (in this case a function)
  // thus the otherwise default-value construction won't be triggered
  College SantaClara = createCollege("Santa Clara", 1851); 
  printf("\tSantaClara \"College\" object Name: %s, State: %s, Year Founded: %d\n", 
    SantaClara.name, SantaClara.state, SantaClara.foundingYear);


  // Having a method make & return an object
  Student willAR = jordanCR.createAStudent("Will Randleman", 1524027, 4.0);
  printf("\nHaving a method make & return a \"Student\" object:\n");
  printf("\t");
  willAR.show();


  // Having a method swap 2 objects via '*this' pointer
  printf("\nHaving a method swap 2 \"Student\" objects via '*this' pointer in method:\n");
  Student jowiR;
  jowiR.assignName("Jowi Randleman");
  jowiR.assignId(5052009);
  jowiR.assignGpa(100);
  printf("\tjowiR object pre-swap:   ");
  jowiR.show();
  printf("\twillAR object pre-swap:  ");
  willAR.show();
  willAR.swap(&jowiR); // pass 'jowiR' obj by address to be recieved as ptr & change it's data (as w/ any data type)
  printf("\tjowiR object post-swap:  ");
  jowiR.show();
  printf("\twillAR object post-swap: ");
  willAR.show();


  // Multi-layer object containment - A 'Region' obj containing a 'College' obj array each containing a 'Student' obj array
  printf("\nMulti-Layer Object Containment - \"Region\" object containing");
  printf(" a \"College\" object array each containing a \"Student\" object array:\n");
  // note the "immortal" prefix means that this "Region" object will never be destroyed,
  // including none of its 
  immortal Region SiliconValley; 
  SiliconValley.setRegionName("Silicon Valley");
  SiliconValley.addSchool(Scu);
  SiliconValley.schools[0].addName("S C U"); // accessing immediate contained object member's method (valid by caveat 8)
  char scuNames[10][20] = {"Dennis","Ritchie","Ken","Thompson","Bjarne","Stroustrup","John","McCarthy","Haskell","Curry"};
  float scuGpas[10]     = {9.5, 9.0, 10.0, 10.5, 9.6, 9.1, 80.0, 80.8, 20.0, 20.2};
  long scuIds[10]       = {1000001, 1000002, 1000003, 1000004, 1000005, 1000006, 1000007, 1000008, 1000009, 1000010};
  SantaClara.addStudents(scuNames, scuGpas, scuIds);
  SiliconValley.addSchool(SantaClara);
  SiliconValley.show();


  // Showing a "Region" object's contained "Student" object & object array initialized w/ a ctor
  printf("\nShowing a \"Region\" object's contained \"Student\" object & object array initialized w/ a ctor:\n");
  SiliconValley.showTopStudents();

  return 0;
}
