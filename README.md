# Declass-C
## Declassifier Enables Classes in C by Pre-Preprocessing Files!
#### **_Ctors, Member Default Values/Allocation, Methods, Object Arrays/Pointers/Containment, Smart Ptrs, and more!_**
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
## C-Declassify's 8 Caveats & 3 Notes, Straight From declass.c:
* _**Note**: whereas 1-3 pertain to formatting, 5-8 relate to restricted class operations with possible alternatives_
* _**Note**: the 3 notes pertain to_ "`smrtptr.h`"_'s default inclusion and how to use class constructors_
```c
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
```
--------------
## Disabling the Smart Pointer Library:
* _Declass-C also enables my_ "`smrtptr.h`" _library by default to improve upon_ "`stdlib.h`"_'s_ "`malloc`" _,_ "`calloc`" _,_ "`realloc`" _, and_ "`free`" _functions by automating garbage collection (with each function prefixed by "smrt")_
* _Learn more about_ "`smrtptr.h`" _by checking it out in [my C-Library repository](https://github.com/jrandleman/C-Libraries) or [by clicking here](https://github.com/jrandleman/C-Libraries/tree/master/Smart-Pointer)_
* _Disable_ "`smrtptr.h`"_'s default inclusion by including the following:_ 
```c
#define DECLASS_NSMRTPTR
```
--------------
## A Simple Sample Stack Class:
* _**Note**:_ "`declass_SampleExec.c`"_, has **much** more on object ctors, containment, arrays, ptrs, default memory allocation, struct/fcn-ptr members, smrtptr.h's autonomous freeing, using "this" ptr in methods, and more!_
* _**Note**: for those unfamiliar with OOP, "members" are class variables and "methods" are class functions_
```c
#include <stdio.h>
#include <stdbool.h>

class Stack {   // "class" keyword tips off declass.c
  int *arr = smrtmalloc(sizeof(int) * 10); // "smrtmalloc" via smrtptr.h handles freeing
  int len;      // empty member values default 0
  int max = 10; // all "Stack" objects will now default to "max" = 10

  // note that class methods can refer to their class' members & other methods
  // without any prefixes, as declass.c will automatically detect which class
  // object is invoking the method & implement the appropriate operations.

  void push(int elt) { // a class "method"
    if(len == max) {   // "len" and "max" invoke the current class' members
      max *= max;
      arr = smrtrealloc(arr, sizeof(int) * max); // via smrtptr.h
    } else
      arr[len++] = elt;
  }
  bool pop(int *elt) {
    if(len == 0) return false;
    *elt = arr[--len];
    return true;
  }
  bool top(int *elt) {
    if(len == 0) return false;
    *elt = arr[len-1];
    return true;
  }
  void show() { for(int i = 0; i < len; ++i) printf("%d ", arr[i]); printf("\n"); }
  int size() { return len; } // return a local member value from method

  // create class constructors by making a "typeless" method w/ the same name of the class
  // -- gets invoked at every object delcaration so long as "(<args>)" are provided,
  // otherwise object only get default values without calling its constructor.

  Stack(int array[], int length) {
    for(int i = 0; i < length; ++i) 
      push(array[i]);
  }
}


// note that class objects invoke members/methods by '.' or '->'
// notation as per whether they aren't/are a class pointer


int main() {
  // Single "Stack" object initialized with default values
  printf("Working with a single \"Stack\" object initializaed with its default values:\n");
  Stack myStack;   // declare object
  myStack.push(8); // invoke "Stack" object's method
  myStack.push(10);
  myStack.push(12);
  printf("Pushed 8, 10, then 12:\n");
  myStack.show();
  int x;
  bool popped = myStack.pop(&x);
  if(popped) printf("Popped Value: %d\n", x);

  myStack.show();
  myStack.push(100);
  printf("Pushing 100:\n");
  myStack.show();
  printf("Pushing 888:\n");
  myStack.push(888);
  myStack.show();

  int y;
  bool topped = myStack.top(&y);
  if(topped) printf("Top Value: %d\n", y);
  int size = myStack.size(); // invoke Stack object's member in the "printf" below:
  printf("Stack's size: %d, Stack's current max capacity: %d\n", size, myStack.max);


  // Single "Stack" object initialized with default values & constructor
  printf("\nInitializing a \"Stack\" object via its default values & class constructor:\n");
  int arr[20] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181};
  Stack newStack(arr, 20);
  printf("\"Stack\" object made with its class constructor:\n");
  newStack.show();

  return 0;
}
```
--------------
## '`-l`' Output for the Simple Sample Class:
* _**Note**: helps confirm whether or not your class code converted as anticipated!_ 
  * _refer to the 8 caveats &_ "`declass_SampleExec.c`" _otherwise!_
```
--=[ TOTAL CLASSES: 1 ]=--=[ TOTAL OBJECTS: 2 ]=--

CLASS No1, Stack:
 L_ MEMBERS: 3
 |  L_ *arr (( ALLOCATED MEMORY ))
 |  L_ len
 |  L_ max
 L_ METHODS: 6
 | L_ push()
 | L_ pop()
 | L_ top()
 | L_ show()
 | L_ size()
 | L_ Stack() (( CONSTRUCTOR ))
 L_ OBJECTS: 2
   L_ myStack
   L_ newStack
```
