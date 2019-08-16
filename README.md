# C-Declassify
## Declassifier Enables Classes in C by Pre-Preprocessing Files!
#### **_Member Default Values, Methods, Object Arrays/Pointers/Containment, and more!_**
-------------------------------------------------------------------------
### Compile: 
* _**Note**: add_ '`-l`' _for class information!_
```c
$ gcc -o declass declass.c
$ ./declass yourFile.c // ./declass -l yourFile.c
```

### Using the Declassifier:
* Processed C programs using classes are copied with a "`_DECLASS`" extension & converted to valid C
* Provided "`declass_SampleExec.c`" demos class abilities, and "`declass_SampleExec_DECLASS.c`" shows conversion to valid C

### C-Declassify's 8 Caveats, Straight from declass.c:
* _**Note**: whereas 1-3 pertain to formatting, 5-8 relate to restricted class operations with possible alternatives_
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

### A Simple Sample Class:
* _**Note**: modified from_ '`declass_SampleExec.c`'_, see whole file for more!_
* _**Note**: for those unfamiliar with OOP, "members" are class variables and "methods" are class functions_
```c
class Student {            // 'class' keyword tips off declass.c
  char fullname[50];       // empty values default to 0 (an array of 0's in this case)
  char school[15] = "SCU"; // all 'Student' objects will default with "SCU" as 'school' member value
  int year = 14;           // default school year as well
  long studentId;
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
  void assignId(long id) { studentId = id; }
  long getId() { return studentId; }                  // return a local member value from method
  void show() {
    if(getId()) {
      printf("Name: %s, School: %s, Year: %d, id: %ld", fullname, school, year, studentId);
      printf(" Major: %s, GPA: %.1f/%.1f\n", grades.major, grades.gpa, grades.out_of);
    }
  }
  
  // note that class objects invoke members/methods by '.' or '->'
  // notation as per whether they aren't/are a class pointer

  // methods can also create class objects!
  Student createMakeAStudent(char *name, long id, float gpa) {
    Student methodMadeStudent; 
    methodMadeStudent.assignName(name);
    methodMadeStudent.assignId(id);
    methodMadeStudent.assignGpa(gpa);
    return methodMadeStudent;
  }
  
  // note that whereas methods can reference an invoking object's members w/o
  // prefixes, using '*this' in a method refers to the ENTIRE object. the following
  // method employs such to swap an invoking object with its method argument:
  void swap(Student *blankStudent) { // object passed by address to change contents
    Student temp = *this;
    *this = *blankStudent; 
    *blankStudent = temp;
  }
}
```
