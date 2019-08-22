# Declass-C
## Declassifier Enables Classes in C by Pre-Preprocessing Files!
#### **_Member Default Values & Allocation, Methods, Object Arrays/Pointers/Containment, Smart Pointers, & more!_**
-------------------------------------------------------------------------

## Using the Declassifier:
### Compile (_add '`-l`' for class info!_): 
```c
$ gcc -o declass declass.c
$ ./declass yourFile.c // ./declass -l yourFile.c
```
### Implementation:
* _Processed C programs using classes are copied with a_ "`_DECLASS`" _extension & converted to valid C_
* _Provided_ "`declass_SampleExec.c`" _demos class abilities, and_ "`declass_SampleExec_DECLASS.c`" _shows conversion_
* _Adhere to the 8 caveats & use_ "`declass_SampleExec.c`" _as a reference for operations!_
--------------
## C-Declassify's 8 Caveats & 2 Notes, Straight From declass.c:
* _**Note**: whereas 1-3 pertain to formatting, 5-8 relate to restricted class operations with possible alternatives_
* _**Note**: the 2 notes pertain to_ "`smrtptr.h`"_'s default inclusion and the_ "`.deepcpy()`" _method_
```c
/*****************************************************************************
 *                       -:- DECLASS.C 8 CAVEATS -:-                        *
 *   (1) 'DECLASS_' PREFIX & 'this' POINTER ARE RESERVED                    *
 *   (2) DECLARE CLASSES GLOBALLY & OBJECTS LOCALLY (NEVER IN STRUCT/UNION) *
 *   (3) DECLARE MEMBERS/METHODS USED IN A METHOD ABOVE ITS DECLARATION     *
 *   (4) DECLARE CLASS MEMBERS, METHODS, & OBJECTS INDIVIDUALLY:            *
 *       * IE NOT:      'myClassName c, e;'                                 *
 *       * ALTERNATIVE: 'myClassName c; <press enter> myClassName e;'       *
 *   (5) NO NESTED CLASS DECLARATIONS NOR METHOD INVOCATIONS:               *
 *       * IE NOT:      'someObj.method1(someObj.method2());'               *
 *       * ALTERNATIVE: 'int x = someObj.method2(); someObj.method1(x);'    *
 *   (6) CLASS ARRAYS RECIEVED AS ARGS MUST BE DENOTED WITH '[]' NOT '*':   *
 *       * IE NOT:     'func(className *classArr){...}'                     *
 *       * ALTERNATIVE:'func(className classArr[]){...}'                    *
 *   (7) NO POINTER TO ARRAY OF OBJECTS:                                    *
 *       * IE NOT:      className (*ptrToArrObj)[10];                       *
 *       * ALTERNATIVE: pointer to an object w/ an array of objects member  *
 *   (8) CONTAINMENT, NOT INHERITANCE: CLASSES CAN ONLY ACCESS MEMBERS &    *
 *       METHODS OF THEIR OWN IMMEDIATE MEMBER CLASS OBJECTS:               *
 *       * IE: suppose classes c1, c2, & c3, with c1 in c2 & c2 in c3.      *
 *             c3 can access c2 members and c2 ca access c1 members,        *
 *             but c3 CANNOT access c1 members                              *
 *       * ALTERNATIVES: (1) simply include a c1 object as a member in c3   *
 *                       (2) create methods in c2 invoking c1 methods as    *
 *                           an interface for c3                            *
 *****************************************************************************
 *                -:- DECLASS.C MEMORY-ALLOCATION NOTES -:-                 *
 *   (1) W/O "#define DECLASS_NSMRTPTR", THE SMRTPTR.H LIBRARY IS INCLUDED, *
 *       W/ IMPROVED MALLOC/CALLOC/REALLOC/FREE FCNS & GARBAGE COLLECTION   *
 *       * "smrtptr.h"'s fcns same as stdlib's all prefixed with "smrt"     *
 *   (2) W/O "#define DECLASS_NDEEPCPY", ALL CLASSES HAVE THE ".deepcpy()"  *
 *       METHOD RETURNING A COPY OF THE INVOKING OBJECT W/ ANY MEMORY       *
 *       ALLOCATED DEFAULT VALUES ALLOCATED ANEW                            *
 *       * ".deepcpy()" allocates members as defined, "smrt" or otherwise   *
 *****************************************************************************/
```
--------------
## Disabling Default Methods & Features:
### Smart Pointer Library:
* _Declass-C also enables my_ "`smrtptr.h`" _library by default to improve upon_ "`stdlib.h`"_'s_ "`malloc`" _,_ "`calloc`" _,_ "`realloc`" _, and_ "`free`" _functions by automating garbage collection (with each function prefixed by "smrt")_
* _See more about_ "`smrtptr.h`" _by checking it out in my [C-Library](https://github.com/jrandleman/C-Libraries) repository or [clicking here](https://github.com/jrandleman/C-Libraries/tree/master/Smart-Pointer)_
* _Disable_ "`smrtptr.h`"_'s default inclusion by including the following:_ 
```c
#define DECLASS_NSMRTPTR
```
### Universal Deep Copying Method:
* _The_ '`.deepcpy()`' _method is also provided for all classes, returning a copy of the invoking object with default mem-alloc'd member values allocated their own block of memory exactly as the default was ("smrt" or otherwise)_
* _Disable the_ '`.deepcpy()`' _method by including the following:_ 
```c
#define DECLASS_NDEEPCPY
```
--------------
## A Simple Sample Class:
* _**Note**: see_ "`declass_SampleExec.c`"_, to learn more about object containment, arrays, pointers, default memory allocation, autonomous freeing via smrtptr.h, the universal_ "`.deepcpy()`" _method, & more!_
* _**Note**: for those unfamiliar with OOP, "members" are class variables and "methods" are class functions_
```c
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>

class Student {            // 'class' keyword tips off declass.c
  char fullname[50];       // empty values default to 0 (an array of 0's in this case)
  char school[15] = "SCU"; // all 'Student' objects will default with "SCU" as 'school' member value
  int schoolYear = 14;     // default school year as well
  bool deansList;
  char *(*copy_fcnPtr)() = strcpy; // function pointer members can also be initialized!

  struct grade_info {
    char major[50]; 
    float out_of;
    float gpa;
  } grades = {"Computer Science Engineering", 4.0}; /* assign structs as in C: brace notation */

  // note that class methods can refer to their class' members & other methods
  // without any prefixes, as declass.c will automatically detect which class
  // object is invoking the method & implement the appropriate operations.
  
  void assignName(char *fname) {  // a class "method"
    copy_fcnPtr(fullname, fname); // invokes local 'copy_fcnPtr' & 'fullname' members declared above
  }
  void assignGpa(float grade) { grades.gpa = grade; } // access member struct variables
  void assignDeansList(bool top10Pct) { deansList = top10Pct; }
  int getYear() { return schoolYear; }                // return a local member value from method
  void show() {
    if(getYear() > 12) {                              // only shows those in college
      printf("Name: %s, School: %s, Year: %d", fullname, school, schoolYear);
      printf(" Major: %s, GPA: %.1f/%.1f\n", grades.major, grades.gpa, grades.out_of);
    }
  }
  
  // note that class objects invoke members/methods by '.' or '->'
  // notation as per whether they aren't/are a class pointer

  // methods can also create class objects!
  Student createAStudent(char *name, bool top10Pct, float gpa, int year) {
    Student methodMadeStudent; 
    methodMadeStudent.assignName(name);  // invoke object's method to assign a member
    methodMadeStudent.assignDeansList(top10Pct);
    methodMadeStudent.assignGpa(gpa);
    methodMadeStudent.schoolYear = year; // assign an object's member directly
    return methodMadeStudent;
  }
  
  // note that whereas methods can reference an invoking object's members w/o
  // prefixes, using '*this' in a method refers to the ENTIRE object. the following
  // method employs such to swap an invoking object with its method argument:
  void swap(Student *StudentPtrArg) { // object passed by address to change contents
    Student temp = *this;
    *this = *StudentPtrArg; 
    *StudentPtrArg = temp;
  }
}



int main() {
  Student JordanCR;                                              // declare object
  JordanCR.copy_fcnPtr(JordanCR.fullname, "Jordan C Randleman"); // invoke members
  JordanCR.assignGpa(3.9);                                       // invoke methods
  JordanCR.assignDeansList(true);
  printf("JordanCR object: ");
  JordanCR.show();
  
  // create another object instance, initialized by a method
  Student foo = JordanCR.createAStudent("Bar", false, 0.5, 100); 
  printf("foo object:      ");
  foo.show();
  
  JordanCR.swap(&foo); // swap members between between 'JordanCR' & 'foo' objects
  printf("JordanCR object: ");
  JordanCR.show();     // outputs swapped values
  printf("foo object:      ");
  foo.show();
  
  return 0;
}
```
--------------
## '`-l`' Output for the Simple Sample Class:
* _**Note**: helps confirm whether or not your class code converted as anticipated!_ 
  * _refer to the 8 caveats &_ "`declass_SampleExec.c`" _otherwise!_
```
--=[ TOTAL CLASSES: 1 ]=--=[ TOTAL OBJECTS: 5 ]=--

CLASS No1, Student:
 L_ MEMBERS: 6
 |  L_ fullname[]
 |  L_ school[]
 |  L_ schoolYear
 |  L_ deansList
 |  L_ *copy_fcnPtr
 |  L_ grades
 L_ METHODS: 7
 | L_ assignName()
 | L_ assignGpa()
 | L_ assignDeansList()
 | L_ getYear()
 | L_ show()
 | L_ createAStudent()
 | L_ swap()
 L_ OBJECTS: 5
   L_ methodMadeStudent
   L_ *StudentPtrArg
   L_ temp
   L_ JordanCR
   L_ foo
```
