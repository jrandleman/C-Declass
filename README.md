# C-Declassify
## Declassifier Enables Classes in C by Pre-Preprocessing Files!
#### **_Member Default Values, Methods, Object Arrays/Pointers/Containment, and more!_**
-------------------------------------------------------------------------
### Compile: 
```c
$ gcc -o declass declass.c
$ ./declass yourFile.c 
```
* Note: add '`-l`' for class information: `$ ./declass -l yourFile.c`

### Using the Declassifier:
* Process C files (`yourFile.c`) with classes to make a copy with "`_DECLASS`" extension & converted to valid C
* Provided "`declass_SampleExec.c`" file demos class abilities, and "`declass_SampleExec_DECLASS.c`" shows conversion to valid C

### C-Declassify's 8 Caveats, Straight from declass.c:
```c
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
```

