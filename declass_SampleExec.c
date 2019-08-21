// Author: Jordan Randleman - Sample Implementation of Classes for 'declass.c'
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>

// declass.c - Specific Memory Handling Flags
// #define DECLASS_NFREE // 'no free' disables garbage collector & '.freenow()' method -- alloc'd default values must be freed by user
// #define DECLASS_NDEEP // 'no deep' disables universal '.deepcpy()' method for classes -- in case user wants to make their own

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
 *  *(1) W/O "#define DECLASS_NFREE", (C/M)ALLOC DEFAULT VALUES ARE FREED   *
 *       AUTOMATICALLY ATEXIT BY GARBAGE COLLECTOR (USER SHOULD NEVER FREE) *
 *       * to free a ptr prior "atexit()", classes have ".freenow()" method *
 *         by default to free a ptr passed as an argument immmediately from *
 *         the garbage collector, UNLESS "#define DECLASS_NFREE" is enabled *
 *   (2) W/O "#define DECLASS_NDEEP", ALL CLASSES HAVE THE ".deepcpy()"     *
 *       METHOD RETURNING A COPY OF THE INVOKING OBJECT W/ ANY MEM-ALLOC8ED *
 *       DEFAULT VALUES NEWLY ALLOCATED (ALSO FREED BY NOTE *(1) IF ACTIVE) *
 *       * w/o mem-alloc8ed default vals ".deepcpy()" just returns same obj *
 *****************************************************************************/

class Student {
  // note that class member values default to 0 unless otherwise indicated, and
  // malloc/calloc default values are freed automatically atexit if "DECLASS_NFREE" not #defined
  char *fullname = malloc(sizeof(char)*50); 
  char school[15] = "SCU";         // default school name
  int year = 14;                   // default school year 
  long studentId;
  char *(*copy_fcnPtr)() = strcpy; // function pointer member default value

  struct grade_info {
    char major[50]; 
    float out_of;
    float gpa;
  } grades = {"Computer Science Engineering", 4.0}; /* assign struct values like vanilla C, via brace notation */

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
      printf(" Major: %s, GPA: %.1f/%.1f\n", grades.major, grades.gpa, grades.out_of);
    }
  }

  // method to create a new object
  Student createAStudent(char *name, long id, float gpa) {
    Student methodMadeStudent;
    methodMadeStudent.assignName(name);
    methodMadeStudent.assignId(id);
    methodMadeStudent.assignGpa(gpa);
    return methodMadeStudent;
  }
  // method swaps 2 student objects by using 'this' pointer 
  // to refer to the invoking object as a whole
  void swap(Student *blankStudent) { 
    Student temp = *this;
    *this = *blankStudent; 
    *blankStudent = temp;
  }
}


class College {
  Student body[10]; // contained array of "Student" objects within each "College" object
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
}


class Region {
  College schools[2];
  int totalSchools; // class members default to 0
  char regionName[20];

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
     * object class name 'Region' has no immediate 'Student' member to invoke */ 
    schools[totalSchools-1].body[totalSchools - totalSchools].assignName("CAMERON"); 
  }
  void show() {
    printf("Region Name: %s, Total Schools: %d.\n", regionName, totalSchools);
    for(int i = 0; i < totalSchools; ++i) {
      printf("School No%d:\n", i + 1);
      schools[i].show();
    }
  }
}


void showAllNames(Student person, Student *sharpStudent, Student people[6]) {
  char personName[30], people3Name[30], sharpStudentName[30];
  person.getName(personName);
  people[3].getName(people3Name);
  sharpStudent -> getName(sharpStudentName);
  printf("person's Name: %s, people[3]'s Name: %s", personName, people3Name);
  printf(", *sharpStudent's Name: %s\n", sharpStudentName);
}


College createCollege(char *name, int foundingYear) {
  College someUniversity;
  someUniversity.addName(name);
  someUniversity.foundingYear = foundingYear;
  return someUniversity;
}



int main() {

  // Single object
  printf("Working with a single \"Student\" object:\n");
  Student jordanCR;
  jordanCR.assignId(1524026);              // assign ID to Student object
  long myId = jordanCR.getId();            // get ID
  if(2 * jordanCR.getId() > 1000)
    printf("\tStudent object: school = %s, id = %ld, myId: %ld\n", jordanCR.school, jordanCR.studentId, myId);
  jordanCR.assignName("Jordan Randleman"); // assign a name
  jordanCR.assignGpa(3.9);                 // assign a GPA
  printf("\t");
  jordanCR.show();                         // output information


  // Object array
  printf("\nWorking with an array of 10 \"Student\" objects:\n");
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


  // Object pointer
  printf("\nWorking with a \"Student\" object pointer:\n");
  Student *TJ = &class[5]; // object pointer assigned address of 6th object in object array
  TJ -> assignName("TJ");  // arrow notation to invoke pointer object members & methods
  printf("\t");
  TJ->show();
  long tjId = TJ-> getId();
  printf("\tTJ \"Student\" object pointer's ID: %ld\n", tjId);


  // Object containment (objects w/in objects)
  printf("\nWorking with contained \"Student\" objects within a \"College\" object:\n");
  College Scu; 
  Scu.addFoundingAndName(1851, "SCU");
  char studentNames[10][20] = {"Cameron","Sidd","Austin","Sabiq","Tobias","Gordon","Jason","Ronnie","Kyle","Peter"};
  float studentGpas[10]     = {4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9};
  long studentIds[10]       = {4000000, 3900000, 3800000, 3700000, 3600000, 3500000, 3600000, 3700000, 3800000, 3900000};
  Scu.addStudents(studentNames, studentGpas, studentIds);
  // Note that all member objects in the College object's 'Student' array are initialized with their majors/max GPA.
  // Classes construct their nested class values/arrays w/ the appropriate default values as well & set ptrs to NULL
  Scu.show(); 


  // Passing an object by value, pointer, & array to another function
  printf("\nPassing an object pointer, array, & value to another function:\n");
  printf("\t");
  showAllNames(jordanCR, TJ, class);


  // Having a function return an object
  printf("\nHaving a function make & return a College object:\n");
  // note the College object is initialized by a value (in this case a function)
  // thus the otherwise default-value construction won't be triggered
  College SantaClara = createCollege("Santa Clara", 1851); 
  printf("\tSantaClara College object Name: %s, State: %s, Year Founded: %d\n", SantaClara.name, SantaClara.state, SantaClara.foundingYear);


  // Having a method make & return an object
  Student willAR = jordanCR.createAStudent("Will Randleman", 1524027, 4.0);
  printf("\nHaving a method make & return a Student object:\n");
  printf("\t");
  willAR.show();

  // Having a method swap 2 objects via '*this' pointer
  printf("\nHaving a method swap 2 Student objects via '*this' pointer in method:\n");
  Student jowiR;
  jowiR.assignName("Jowi Randleman");
  jowiR.assignId(5052009);
  jowiR.assignGpa(100);
  printf("\tjowiR object pre-swap:   ");
  jowiR.show();
  printf("\twillAR object pre-swap:  ");
  willAR.show();
  willAR.swap(&jowiR); // pass 'jowiR' object by address to be recieved as pointer & change original variable (as w/ any data type)
  printf("\tjowiR object post-swap:  ");
  jowiR.show();
  printf("\twillAR object post-swap: ");
  willAR.show();


  // Multi-layer object containment - A 'Region' object containing a 'College' object array each containing a 'Student' object array
  printf("\nMulti-Layer Object Containment - Region object containing a College object array each containing a Student object array:\n");
  Region SiliconValley;
  SiliconValley.setRegionName("Silicon Valley");
  SiliconValley.addSchool(Scu);
  SiliconValley.schools[0].addName("S C U"); // accessing immediate contained object member's method (valid by caveat 8)
  char scuNames[10][20] = {"Joel","Aimee","Danny","Kailyn","Ashley","Brendan","Olivia","Tanner","Guillermo","John"};
  float scuGpas[10]     = {4.0, 9, 10, 7, 8, 0.5, 6, 3.9, 4.0, 3.9};
  long scuIds[10]       = {1000001, 1000002, 1000003, 1000004, 1000005, 1000006, 1000007, 1000008, 1000009, 1000010};
  SantaClara.addStudents(scuNames, scuGpas, scuIds);
  SiliconValley.addSchool(SantaClara);
  SiliconValley.show();


  // If "DECLASS_NDEEP" not #defined, all classes have the '.deepcpy()' method by default to return
  // a copy of the invoking object w/ its memory-allocated default values allocated their own block 
  // of memory -- w/ the copy's newly allocated memory also freed automatically atexit if "DECLASS_NFREE" not #defined
  Region SF = SiliconValley.deepcpy();
  SF.setRegionName("San Francisco");
  printf("\n\"SiliconValley.deepcpy();\" & Renamed \"San Francisco\":\n");
  SF.show();


  // If "DECLASS_NFREE" not #defined, all classes have the '.freenow()' method by default to 
  // immediately free the default-allocated member value passed as an arg from the garbage 
  // collector prior to "atexit"
  Student RandomStudent;
  College RandomCollege;
  RandomStudent.freenow(RandomStudent.fullname); // immediately free default allocated full name
  // immediately free default allocated member of contained class object array
  RandomCollege.freenow(RandomCollege.body[0].fullname); 

  return 0;
}
