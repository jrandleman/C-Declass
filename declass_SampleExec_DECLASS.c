/* DECLASSIFIED: declass_SampleExec.c
 * Email jrandleman@scu.edu or see https://github.com/jrandleman for support */

// Author: Jordan Randleman - Sample Implementation of Classes for 'declass.c'
#include <stdio.h>
#include <string.h>
#include <float.h>

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

/******************************** CLASS START ********************************/
/* Student CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */
#define DECLASS__Student_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__Student_DFLT;})
#define DECLASS__Student_ARR(DECLASS_ARR) ({\
  for(int DECLASS__Student_IDX=0;DECLASS__Student_IDX<(sizeof(DECLASS_ARR)/sizeof(DECLASS_ARR[0]));++DECLASS__Student_IDX)\
    DECLASS__Student_CTOR(DECLASS_ARR[DECLASS__Student_IDX]);\
})

/* Student CLASS CONVERTED TO STRUCT: */
typedef struct DECLASS_Student {
  char fullname[50];
  char school[15] ;
  int year ;
  long studentId;
  char *(*copy_fcnPtr)() ;
  struct grade_info {
    char major[50]; 
    float out_of;
    float gpa;
  } grades ;

                                                            
} Student;
const Student DECLASS__Student_DFLT = {{0}, "SCU", 14, 0, strcpy, {"Computer Science Engineering", 4.0}, };

/* Student CLASS METHODS SPLICED OUT: */
  void DECLASS_Student_assignName(char *fname, Student *this) {
    this->copy_fcnPtr(this->fullname, fname);
  }
  void DECLASS_Student_getName(char *name, Student *this) {
    strcpy(name, this->fullname);
  }
  void DECLASS_Student_assignGpa(float grade, Student *this) { this->grades.gpa = grade; }
  void DECLASS_Student_assignId(long id, Student *this) { this->studentId = id; }
  long DECLASS_Student_getId(Student *this) { return this->studentId; }
  void DECLASS_Student_show(Student *this) {
    if(DECLASS_Student_getId(this)) {
      printf("Name: %s, School: %s, Year: %d, id: %ld", this->fullname, this->school, this->year, this->studentId);
      printf(" Major: %s, GPA: %.1f/%.1f\n", this->grades.major, this->grades.gpa, this->grades.out_of);
    }
  }
  Student DECLASS_Student_createAStudent(char *name, long id, float gpa, Student *this) {
    Student methodMadeStudent; DECLASS__Student_CTOR(methodMadeStudent);
    DECLASS_Student_assignName(name, &methodMadeStudent);
    DECLASS_Student_assignId(id, &methodMadeStudent);
    DECLASS_Student_assignGpa(gpa, &methodMadeStudent);
    return methodMadeStudent;
  }
  void DECLASS_Student_swap(Student *blankStudent, Student *this) { 
    Student temp = *this;
    *this = *blankStudent; 
    *blankStudent = temp;
  }
/********************************* CLASS END *********************************/


/******************************** CLASS START ********************************/
/* College CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */
#define DECLASS__College_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__College_DFLT;\
	DECLASS__Student_ARR(DECLASS_THIS.body);})
#define DECLASS__College_ARR(DECLASS_ARR) ({\
  for(int DECLASS__College_IDX=0;DECLASS__College_IDX<(sizeof(DECLASS_ARR)/sizeof(DECLASS_ARR[0]));++DECLASS__College_IDX)\
    DECLASS__College_CTOR(DECLASS_ARR[DECLASS__College_IDX]);\
})

/* College CLASS CONVERTED TO STRUCT: */
typedef struct DECLASS_College {
  Student body[10];                                                                     
  char name[20];
  int foundingYear;
  char state[3] ;

} College;
const College DECLASS__College_DFLT = {{0}, {0}, 0, "CA", };

/* College CLASS METHODS SPLICED OUT: */
  void DECLASS_College_addName(char *uniName, College *this) { strcpy(this->name, uniName); }
  void DECLASS_College_addFoundingAndName(int founded, char *name, College *this) {
    if(founded < 0) printf("A school founded in BC? Really?\n");
    this->foundingYear = founded;
                                                                         
                                                                           
    DECLASS_College_addName(name, this); 
  }
  void DECLASS_College_addStudents(char names[10][20], float gpas[10], long ids[10], College *this) {
    for(int i = 0; i < 10; ++i) {
      if(i % 2 == 0)                                                                          
        strcpy(this->body[i].fullname, names[i]);                                     
      else
        DECLASS_Student_assignName(names[i], &(this->body[i]));                                                    
      DECLASS_Student_assignGpa(gpas[i], &(this->body[i]));
      DECLASS_Student_assignId(ids[i], &(this->body[i]));
    }
  }
  void DECLASS_College_show(College *this) {
    printf("College Name: %s, State: %s, Year Founded: %d.\n", this->name, this->state, this->foundingYear);
    printf("%s's Students:\n", this->name);
    for(int i = 0; i < 10; ++i) {
      printf("\tStudent No%d: ", i + 1);
      DECLASS_Student_show(&(this->body[i]));
    }
  }
/********************************* CLASS END *********************************/


/******************************** CLASS START ********************************/
/* Region CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */
#define DECLASS__Region_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__Region_DFLT;\
	DECLASS__College_ARR(DECLASS_THIS.schools);})
#define DECLASS__Region_ARR(DECLASS_ARR) ({\
  for(int DECLASS__Region_IDX=0;DECLASS__Region_IDX<(sizeof(DECLASS_ARR)/sizeof(DECLASS_ARR[0]));++DECLASS__Region_IDX)\
    DECLASS__Region_CTOR(DECLASS_ARR[DECLASS__Region_IDX]);\
})

/* Region CLASS CONVERTED TO STRUCT: */
typedef struct DECLASS_Region {
  College schools[2];
  int totalSchools;                              
  char regionName[20];
} Region;
const Region DECLASS__Region_DFLT = {{0}, 0, {0}, };

/* Region CLASS METHODS SPLICED OUT: */
  void DECLASS_Region_setRegionName(char *name, Region *this) { strcpy(this->regionName, name); }
  void DECLASS_Region_addSchool(College school, Region *this) {
    if(this->totalSchools == 2) {
      printf("Region Maxed out of Schools!\n");
      return;
    }
    this->schools[this->totalSchools++] = school;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
    DECLASS_Student_assignName("CAMERON", &(this->schools[this->totalSchools-1].body[this->totalSchools - this->totalSchools])); 
  }
  void DECLASS_Region_show(Region *this) {
    printf("Region Name: %s, Total Schools: %d.\n", this->regionName, this->totalSchools);
    for(int i = 0; i < this->totalSchools; ++i) {
      printf("School No%d:\n", i + 1);
      DECLASS_College_show(&(this->schools[i]));
    }
  }
/********************************* CLASS END *********************************/


void showAllNames(Student person, Student *sharpStudent, Student people[6]) {
  char personName[30], people3Name[30], sharpStudentName[30];
  DECLASS_Student_getName(personName, &person);
  DECLASS_Student_getName(people3Name, &people[3]);
  DECLASS_Student_getName(sharpStudentName, sharpStudent);
  printf("person's Name: %s, people[3]'s Name: %s", personName, people3Name);
  printf(", *sharpStudent's Name: %s\n", sharpStudentName);
}


College createCollege(char *name, int foundingYear) {
  College someUniversity; DECLASS__College_CTOR(someUniversity);
  DECLASS_College_addName(name, &someUniversity);
  someUniversity.foundingYear = foundingYear;
  return someUniversity;
}



int main() {

  // Single object
  printf("Working with a single \"Student\" object:\n");
  Student jordanCR; DECLASS__Student_CTOR(jordanCR);
  DECLASS_Student_assignId(1524026, &jordanCR);              // assign ID to Stduent object
  long myId = DECLASS_Student_getId(&jordanCR);            // get ID
  if(2 * DECLASS_Student_getId(&jordanCR) > 1000)
    printf("\tStudent object: school = %s, id = %ld, myId: %ld\n", jordanCR.school, jordanCR.studentId, myId);
  DECLASS_Student_assignName("Jordan Randleman", &jordanCR); // assign a name
  DECLASS_Student_assignGpa(3.9, &jordanCR);                 // assign a GPA
  printf("\t");
  DECLASS_Student_show(&jordanCR);                         // output information


  // Object array
  printf("\nWorking with an array of 10 \"Student\" objects:\n");
  Student class[6]; DECLASS__Student_ARR(class);
  char names[6][20] = {"Cameron", "Sidd", "Austin", "Sabiq", "Tobias", "Gordon"};
  for(int i = 0; i < 6; ++i) {
    DECLASS_Student_assignName(names[i], &class[i]);
    DECLASS_Student_assignId(i * 256 + 1000000, &class[i]);
    DECLASS_Student_assignGpa(i * 0.75, &class[i]); // sorry Cameron
  }
  for(int i = 0; i < 6; ++i) {
    printf("\t");
    DECLASS_Student_show(&class[i]);
  }


  // Object pointer
  printf("\nWorking with a \"Student\" object pointer:\n");
  Student *TJ = &class[5]; // object pointer assigned address of 6th object in object array
  DECLASS_Student_assignName("TJ", TJ);  // arrow notation to invoke pointer object members & methods
  printf("\t");
  DECLASS_Student_show(TJ);
  long tjId = DECLASS_Student_getId(TJ);
  printf("\tTJ \"Student\" object pointer's ID: %ld\n", tjId);


  // Object containment (objects w/in objects)
  printf("\nWorking with contained \"Student\" objects within a \"College\" object:\n");
  College Scu; DECLASS__College_CTOR(Scu); 
  DECLASS_College_addFoundingAndName(1851, "SCU", &Scu);
  char studentNames[10][20] = {"Cameron","Sidd","Austin","Sabiq","Tobias","Gordon","Jason","Ronnie","Kyle","Peter"};
  float studentGpas[10]     = {4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9};
  long studentIds[10]       = {4000000, 3900000, 3800000, 3700000, 3600000, 3500000, 3600000, 3700000, 3800000, 3900000};
  DECLASS_College_addStudents(studentNames, studentGpas, studentIds, &Scu);
  // Note that all member objects in the College object's 'Student' array are initialized with their majors/max GPA.
  // Classes construct their nested class values/arrays w/ the appropriate default values as well & set ptrs to NULL
  DECLASS_College_show(&Scu); 


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
  Student willAR = DECLASS_Student_createAStudent("Will Randleman", 1524027, 4.0, &jordanCR);
  printf("\nHaving a method make & return a Student object:\n");
  printf("\t");
  DECLASS_Student_show(&willAR);


  // Having a method swap 2 objects via '*this' pointer
  printf("\nHaving a method swap 2 Student objects via '*this' pointer in method:\n");
  Student jowiR; DECLASS__Student_CTOR(jowiR);
  DECLASS_Student_assignName("Jowi Randleman", &jowiR);
  DECLASS_Student_assignId(5052009, &jowiR);
  DECLASS_Student_assignGpa(100, &jowiR);
  printf("\tjowiR object pre-swap:   ");
  DECLASS_Student_show(&jowiR);
  printf("\twillAR object pre-swap:  ");
  DECLASS_Student_show(&willAR);
  DECLASS_Student_swap(&jowiR, &willAR); // pass 'jowiR' object by address to be recieved as pointer & change original variable (as w/ any data type)
  printf("\tjowiR object post-swap:  ");
  DECLASS_Student_show(&jowiR);
  printf("\twillAR object post-swap: ");
  DECLASS_Student_show(&willAR);


  // Mylti-layer object containment - A 'Region' object containing a 'College' object array each containing a 'Student' object array
  printf("\nMulti-Layer Object Containment - Region object containing a College object array each containing a Student object array:\n");
  Region SiliconValley; DECLASS__Region_CTOR(SiliconValley);
  DECLASS_Region_setRegionName("Silicon Valley", &SiliconValley);
  DECLASS_Region_addSchool(Scu, &SiliconValley);
  DECLASS_College_addName("S C U", &SiliconValley.schools[0]); // accessing immediate contained object member's method (valid by caveat 8)
  char scuNames[10][20] = {"Joel","Aimee","Danny","Kailyn","Ashley","Brendan","Olivia","Tanner","Guillermo","John"};
  float scuGpas[10]     = {4.0, 9, 10, 7, 8, 0.5, 6, 3.9, 4.0, 3.9};
  long scuIds[10]       = {1000001, 1000002, 1000003, 1000004, 1000005, 1000006, 1000007, 1000008, 1000009, 1000010};
  DECLASS_College_addStudents(scuNames, scuGpas, scuIds, &SantaClara);
  DECLASS_Region_addSchool(SantaClara, &SiliconValley);
  DECLASS_Region_show(&SiliconValley);

  return 0;
}
