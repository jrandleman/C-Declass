// Author: Jordan Randleman - Sample Implementation of Classes for 'declass.c'
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>


// #define DECLASS_ALLOC_FCNS allocer, callocer, alloc

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
 *       (*) RATHER:   (1) include a c1 object as a member in c3            *
 *                     (2) mk c2 methods invoking c1 methods: c3 interface  *
 *   (9) ONLY BINARY SINGLE-LINE CONDITIONAL ("?:") OBJECT RETURNS:         *
 *       (*) IE NOT:   "return case ? obj : case2 ? obj2 : obj3;"           *
 *       (*) RATHER:   "if(case) return obj; return case2 ? obj2 : obj3;"   *
 *****************************************************************************
 *                       -:- DECLASS.C & SMRTPTR.H -:-                      *
 *    SMRTPTR.H LIBRARY IS DEFAULT INCLUDED, W/ IMPROVED MALLOC, CALLOC,    *
 *    REALLOC, & FREE FUNCTIONS AS WELL AS GARBAGE COLLECTION               *
 *      (*) "smrtptr.h"'s fcns same as stdlib's all prefixed with "smrt"    *
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
 *             => CTOR: return ctor'd cName object instance: "cName(args);" *
 *                => these so-called "dummy" ctors don't init specific objs *
 *                   rather they return an instance of a sample ctor'd obj  *
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
 *   (0) OBJECTS DECLARED "immortal" NEVER INVOKE THEIR DTOR                *
 *       (*) DECLARATION: "immortal className objName(args);"               *
 *       (*) contained objects can also be immortal                         *
 *   (1) OBJ ARGS ALWAYS IMMORTAL: AS PLACEHOLDERS, THE PASSED OBJ THEY     *
 *       REPRESENT AREN'T OUT OF SCOPE ONCE FCN ENDS (NO DOUBLE DTOR)       *
 *       (*) thus denoting obj args as "immortal" is redundant              *
 *       (*) "MACRO FLAG" (3) disables all immortal obj EXCEPT obj args     *
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
 *   DEFINING CUSTOM MEMORY ALLOCATION FUNCTIONS:                           *
 *     (0) declass.c relies on being able to identify memory allocation     *
 *         fcns to aptly apply dflt vals (not assigning garbage memory)     *
 *     (1) aside from malloc/calloc/smrtmalloc/smrtcalloc, users can        *
 *         declare their own custom class obj memory allocation functions   *
 *         to be recognized by declass.c at the top of their program        *
 *         (*) NOTE: ASSUMES ALL ALLOC FCNS RETURN "NULL" OR EXIT ON FAIL   *
 *     (2) list alloc fcn names after a "#define DECLASS_ALLOC_FCNS" macro  *
 *****************************************************************************/

class Student {
  // note that class if(member) values; default to 0 unless otherwise indicated
  char *fullname = smrtmalloc(sizeof(char) * 50);
  char school[15] = "SCU";         // default school name
  int year = 14;                   // default school year 
  long studentId;
  char *(*copy_fcnPtr)() = strcpy; // function pointer member default value

  struct grade_info {
    char major[50]; 
    float out_of;
    float gpa;
    float how;
    float about;
    float that;
  } grades;//={"Computer Science Engineering", 4.0}; /* assign struct values like vanilla C, via brace notation */


  bool unionIsInt = true; // else is char
  union MyU {
    int n;
    char c;
  } MYU;

  int stable = 1;


  Student(int x, int y) {
    printf("CONSTRUCTOR: %d && %d\n", x, y);
  }

  ~Student() {
    printf("\"Student\" object named \"%s\" Destroyed!\n", fullname);
  }


  // note that class methods can refer to their class' members & other methods
  // without any prefixes, as declass.c will automatically detect which class
  // object is invoking the method & implement the appropriate operations.
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
      // printf(" Major: %s, GPA: %.1f/%.1f\n", grades.major, grades.gpa, grades.out_of);
    }
  }


  int get10() {
    int sum = 0;
    for(int i = 0; i < 11; ++i)
      sum += i;
    return sum;
  }

  // // method to create a new object
  // Student createAStudent(char *name, long id, float gpa) {
  //   Student methodMadeStudent;
  //   methodMadeStudent.assignName(name);
  //   methodMadeStudent.assignId(id);
  //   methodMadeStudent.assignGpa(gpa);
  //   return methodMadeStudent;
  // }
  // // method swaps 2 student objects by using 'this' pointer 
  // // to refer to the invoking object as a whole
  // void swap(Student *blankStudent) { 
  //   Student temp = *this;
  //   *this = *blankStudent; 
  //   *blankStudent = temp;
  // }
}



class Box {
  int num = 222;
  immortal Student child[3](100, 200);
  void showNum() { printf("%d\n", num); }
  ~Box() {
    printf("\"Box\" object Destroyed!\n");
  }

  Student *bonzo(9999, 9999) = smrtmalloc(sizeof(Student));

  void demo() {
    Student *myGal = smrtmalloc(sizeof(Student)); // assigns default values since memory gets allocated
    if(9 > 10) {
      ~myGal();
      return;
    }
  }

  Box(int num) {
    printf("\"BOX\" OBJECT CREATED!, NUMBER PASSED: %d\n", num);
  }

}

int rando() {
  return (8 < 9) ? 3 : 82;
}


int main() {

  // Single object


  Box card = Box(88); 
  card.demo();


  Student myGuy = Student(7, 13);
  ~myGuy();


  immortal Student kids[3](88, 99);

  Student *bobby(8,8) = smrtmalloc(sizeof(Student));
  strcpy(bobby->fullname, "heelo");
  ~bobby();


  // 3 TYPES OF PTR INIT:
  // ONLY DFLT: className *objName = malloc(sizeof(className));
  // ONLY CTOR: className *objName(args);
  // DFLT CTOR: className *objName(args) = malloc(sizeof(className));


  // ALLOW USERS TO REGISTER THEIR OWN UNIQUE MEMORY ALLOCATION FCN NAMES 
  // TO ADD TO DETECTED "MALLOC" "CALLOC" & SMRT VARIANTS FOR PTR ASSIGNMENT ???


  // assigns default values AND invokes ctor since memory gets allocated
  // Student *razor(101, 200); // would only invoke ctor since no guarantee alloc'd memory for dflts 
  immortal Student *razor; 


  Student random(1, 2);


  printf("Working with a single \"Student\" object:\n");
  Student jordanCR(3, 4);
  jordanCR.assignId(1524026);              // assign ID to Student object
  long myId = jordanCR.getId();            // get ID
  if(2 * jordanCR.getId() > 1000)
    printf("\tStudent object: school = %s, id = %ld, myId: %ld\n", jordanCR.school, jordanCR.studentId, myId);
  jordanCR.assignName("Jordan Randleman"); // assign a name
  jordanCR.assignGpa(3.9);                 // assign a GPA
  printf("\t");
  jordanCR.show();                         // output information

  if(jordanCR.stable) printf("\nstable: %d\n", jordanCR.stable);

  if(jordanCR.unionIsInt) {
    printf("union int: %d\n", jordanCR.MYU.n);
  }

  jordanCR.MYU.c = 65;

  jordanCR.unionIsInt = !jordanCR.unionIsInt;

  if(!jordanCR.unionIsInt)
    printf("union char: %c\n", jordanCR.MYU.c);

  int total = jordanCR.get10();
  printf("total: %d\n", total);

  return 0;
}
