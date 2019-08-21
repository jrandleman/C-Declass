/* DECLASSIFIED: declass_SampleExec_DECLASS.c
 * Email jrandleman@scu.edu or see https://github.com/jrandleman for support */
/************************** GARBAGE COLLECTOR START **************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct DECLASS__dump_set {
  long len, max; // current # of ptrs && max capacity
  void **ptrs;   // unique ptr set to free all ctor alloc's
} DECLASS__dump = {-1};
void DECLASS__add_to_dump(void *ptr) { 
  // malloc garbage collector
  if(DECLASS__dump.len == -1) {
    DECLASS__dump.ptrs = malloc(sizeof(void *) * 10);
    if(!DECLASS__dump.ptrs) {
      fprintf(stderr, "-:- ERROR: COULDN'T MALLOC MEMORY FOR DECLASS.C'S CLASS GARBAGE-COLLECTOR -:-\n");
      exit(EXIT_FAILURE);
    }
    DECLASS__dump.max = 10, DECLASS__dump.len = 0;
  }
  // reallocate if "max" ptrs already added
  if(DECLASS__dump.len == DECLASS__dump.max) { 
    DECLASS__dump.max *= DECLASS__dump.max;
    void **temp = realloc(DECLASS__dump.ptrs, sizeof(void *) * DECLASS__dump.max);
    if(!temp) {
      fprintf(stderr, "-:- ERROR: COULDN'T REALLOC FOR DECLASS.C'S CLASS GARBAGE-COLLECTOR -:-\n");
      fprintf(stderr, "-:-    FREEING ALLOCATED MEMORY THUS FAR AND TERMINATING PROGRAM    -:-\n");
      exit(EXIT_FAILURE); // still frees any ptrs allocated thus far
    }
    DECLASS__dump.ptrs = temp;
  }
  // add ptr to dump if not already present (ensures no double-freeing)
  for(int i = 0; i < DECLASS__dump.len; ++i) if(DECLASS__dump.ptrs[i] == ptr) return;
  DECLASS__dump.ptrs[DECLASS__dump.len++] = ptr;
}
void DECLASS__empty_dump() { // invoked by atexit to free all ctor-alloc'd memory
  for(int i = 0; i < DECLASS__dump.len; ++i) free(DECLASS__dump.ptrs[i]);
  if(DECLASS__dump.len > 0) free(DECLASS__dump.ptrs);
  DECLASS__dump.len = 0;
}
/*************************** GARBAGE COLLECTOR END ***************************/

 
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>







/******************************** CLASS START ********************************/
/* Student CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */
#define DECLASS__Student_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__Student_DFLT();})
#define DECLASS__Student_ARR(DECLASS_ARR) ({\
  for(int DECLASS__Student_IDX=0;DECLASS__Student_IDX<(sizeof(DECLASS_ARR)/sizeof(DECLASS_ARR[0]));++DECLASS__Student_IDX)\
    DECLASS__Student_CTOR(DECLASS_ARR[DECLASS__Student_IDX]);\
})

/* Student CLASS CONVERTED TO STRUCT: */
typedef struct DECLASS_Student {
  char *fullname ;
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
Student DECLASS__Student_DFLT(){
	Student this={malloc(sizeof(char)*50),"SCU",14,0,strcpy,{"Computer Science Engineering", 4.0},};
	if(this.fullname!=NULL)DECLASS__add_to_dump(this.fullname);
	atexit(DECLASS__empty_dump);
	if(this.fullname==NULL){
		fprintf(stderr, "-:- ERROR ALLOCATING MEMBER OF CLASS NAME \"Student\" -:-\n-:- FREEING ALLOCATED MEMERS THUS FAR AND TERMINATING PROGRAM -:-\n");
		exit(EXIT_FAILURE);
	}
	return this;
}

/* Student CLASS DEEP COPY FUNCTIONS: */
Student DECLASS_deepcpy_Student(Student *DECLASS__OLD_Student) {
	Student this = *DECLASS__OLD_Student;unsigned long DECLASS__MEM_SIZE_Student=0;
	this.fullname=NULL;this.fullname=malloc(sizeof(char)*50);if(this.fullname!=NULL)DECLASS__add_to_dump(this.fullname);
	if(this.fullname==NULL){
		fprintf(stderr, "\n-:- UNABLE TO MALLOC IN DEEP COPY FOR CLASS \"Student\" -:-\n-:- FREEING ALLOCATED MEMERS THUS FAR AND TERMINATING PROGRAM -:-\n");
		exit(EXIT_FAILURE);
	}
	DECLASS__MEM_SIZE_Student=sizeof(DECLASS__OLD_Student->fullname);memmove(this.fullname, DECLASS__OLD_Student->fullname, DECLASS__MEM_SIZE_Student);
	return this;
}
#define DECLASS__deepcpyARR_Student(DECLASS__NEW_Student, DECLASS__OLD_Student) ({\
	for(int DECLASS__Student_i = 0; DECLASS__Student_i < (sizeof(DECLASS__OLD_Student)/sizeof(DECLASS__OLD_Student[0])); ++DECLASS__Student_i)\
		DECLASS__NEW_Student[DECLASS__Student_i] = DECLASS_deepcpy_Student(&DECLASS__OLD_Student[DECLASS__Student_i]);\
})

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
#define DECLASS__College_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__College_DFLT();\
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
College DECLASS__College_DFLT(){
	College this={{0},{0},0,"CA",};
	return this;
}

/* College CLASS DEEP COPY FUNCTIONS: */
College DECLASS_deepcpy_College(College *DECLASS__OLD_College) {
	College this = *DECLASS__OLD_College;unsigned long DECLASS__MEM_SIZE_College=0;
	 DECLASS__deepcpyARR_Student(this.body, DECLASS__OLD_College->body);
	return this;
}
#define DECLASS__deepcpyARR_College(DECLASS__NEW_College, DECLASS__OLD_College) ({\
	for(int DECLASS__College_i = 0; DECLASS__College_i < (sizeof(DECLASS__OLD_College)/sizeof(DECLASS__OLD_College[0])); ++DECLASS__College_i)\
		DECLASS__NEW_College[DECLASS__College_i] = DECLASS_deepcpy_College(&DECLASS__OLD_College[DECLASS__College_i]);\
})

/* College CLASS METHODS SPLICED OUT: */
  void DECLASS_College_addName(char *uniName, College *this) { strcpy(this->name, uniName); }
  void DECLASS_College_addFoundingAndName(int founded, char *name, College *this) {
    if(founded < 0) {printf("A school founded in BC? Really?\n");}
    this->foundingYear = founded;


    DECLASS_College_addName(name, this);
  }
  void DECLASS_College_addStudents(char names[10][20], float gpas[10], long ids[10], College *this) {
    for(int i = 0; i < 10; ++i) {
      if(i % 2 == 0)
        {strcpy(this->body[i].fullname, names[i]);}
      else
        {DECLASS_Student_assignName(names[i], &(this->body[i]));}
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
#define DECLASS__Region_CTOR(DECLASS_THIS) ({DECLASS_THIS = DECLASS__Region_DFLT();\
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
Region DECLASS__Region_DFLT(){
	Region this={{0},0,{0},};
	return this;
}

/* Region CLASS DEEP COPY FUNCTIONS: */
Region DECLASS_deepcpy_Region(Region *DECLASS__OLD_Region) {
	Region this = *DECLASS__OLD_Region;unsigned long DECLASS__MEM_SIZE_Region=0;
	 DECLASS__deepcpyARR_College(this.schools, DECLASS__OLD_Region->schools);
	return this;
}
#define DECLASS__deepcpyARR_Region(DECLASS__NEW_Region, DECLASS__OLD_Region) ({\
	for(int DECLASS__Region_i = 0; DECLASS__Region_i < (sizeof(DECLASS__OLD_Region)/sizeof(DECLASS__OLD_Region[0])); ++DECLASS__Region_i)\
		DECLASS__NEW_Region[DECLASS__Region_i] = DECLASS_deepcpy_Region(&DECLASS__OLD_Region[DECLASS__Region_i]);\
})

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


  printf("Working with a single \"Student\" object:\n");
  Student jordanCR; DECLASS__Student_CTOR(jordanCR);
  DECLASS_Student_assignId(1524026, &jordanCR);
  long myId = DECLASS_Student_getId(&jordanCR);
  if(2 * DECLASS_Student_getId(&jordanCR) > 1000)
    {printf("\tStudent object: school = %s, id = %ld, myId: %ld\n", jordanCR.school, jordanCR.studentId, myId);}
  DECLASS_Student_assignName("Jordan Randleman", &jordanCR);
  DECLASS_Student_assignGpa(3.9, &jordanCR);
  printf("\t");
  DECLASS_Student_show(&jordanCR);



  printf("\nWorking with an array of 10 \"Student\" objects:\n");
  Student class[6]; DECLASS__Student_ARR(class);
  char names[6][20] = {"Cameron", "Sidd", "Austin", "Sabiq", "Tobias", "Gordon"};
  for(int i = 0; i < 6; ++i) {
    DECLASS_Student_assignName(names[i], &class[i]);
    DECLASS_Student_assignId(i * 256 + 1000000, &class[i]);
    DECLASS_Student_assignGpa(i * 0.75, &class[i]);
  }
  for(int i = 0; i < 6; ++i) {
    printf("\t");
    DECLASS_Student_show(&class[i]);
  }



  printf("\nWorking with a \"Student\" object pointer:\n");
  Student *TJ = &class[5];
  DECLASS_Student_assignName("TJ", TJ);
  printf("\t");
  DECLASS_Student_show(TJ);
  long tjId = DECLASS_Student_getId(TJ);
  printf("\tTJ \"Student\" object pointer's ID: %ld\n", tjId);



  printf("\nWorking with contained \"Student\" objects within a \"College\" object:\n");
  College Scu; DECLASS__College_CTOR(Scu);
  DECLASS_College_addFoundingAndName(1851, "SCU", &Scu);
  char studentNames[10][20] = {"Cameron","Sidd","Austin","Sabiq","Tobias","Gordon","Jason","Ronnie","Kyle","Peter"};
  float studentGpas[10]     = {4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9, 4.0, 3.9};
  long studentIds[10]       = {4000000, 3900000, 3800000, 3700000, 3600000, 3500000, 3600000, 3700000, 3800000, 3900000};
  DECLASS_College_addStudents(studentNames, studentGpas, studentIds, &Scu);


  DECLASS_College_show(&Scu);



  printf("\nPassing an object pointer, array, & value to another function:\n");
  printf("\t");
  showAllNames(jordanCR, TJ, class);



  printf("\nHaving a function make & return a College object:\n");


  College SantaClara = createCollege("Santa Clara", 1851);
  printf("\tSantaClara College object Name: %s, State: %s, Year Founded: %d\n", SantaClara.name, SantaClara.state, SantaClara.foundingYear);



  Student willAR = DECLASS_Student_createAStudent("Will Randleman", 1524027, 4.0, &jordanCR);
  printf("\nHaving a method make & return a Student object:\n");
  printf("\t");
  DECLASS_Student_show(&willAR);


  printf("\nHaving a method swap 2 Student objects via '*this' pointer in method:\n");
  Student jowiR; DECLASS__Student_CTOR(jowiR);
  DECLASS_Student_assignName("Jowi Randleman", &jowiR);
  DECLASS_Student_assignId(5052009, &jowiR);
  DECLASS_Student_assignGpa(100, &jowiR);
  printf("\tjowiR object pre-swap:   ");
  DECLASS_Student_show(&jowiR);
  printf("\twillAR object pre-swap:  ");
  DECLASS_Student_show(&willAR);
  DECLASS_Student_swap(&jowiR, &willAR);
  printf("\tjowiR object post-swap:  ");
  DECLASS_Student_show(&jowiR);
  printf("\twillAR object post-swap: ");
  DECLASS_Student_show(&willAR);



  printf("\nMulti-Layer Object Containment - Region object containing a College object array each containing a Student object array:\n");
  Region SiliconValley; DECLASS__Region_CTOR(SiliconValley);
  DECLASS_Region_setRegionName("Silicon Valley", &SiliconValley);
  DECLASS_Region_addSchool(Scu, &SiliconValley);
  DECLASS_College_addName("S C U", &SiliconValley.schools[0]);
  char scuNames[10][20] = {"Joel","Aimee","Danny","Kailyn","Ashley","Brendan","Olivia","Tanner","Guillermo","John"};
  float scuGpas[10]     = {4.0, 9, 10, 7, 8, 0.5, 6, 3.9, 4.0, 3.9};
  long scuIds[10]       = {1000001, 1000002, 1000003, 1000004, 1000005, 1000006, 1000007, 1000008, 1000009, 1000010};
  DECLASS_College_addStudents(scuNames, scuGpas, scuIds, &SantaClara);
  DECLASS_Region_addSchool(SantaClara, &SiliconValley);
  DECLASS_Region_show(&SiliconValley);





  Region SF = DECLASS_deepcpy_Region(&SiliconValley);
  DECLASS_Region_setRegionName("San Francisco", &SF);
  printf("\n\"SiliconValley.deepcpy();\" & Renamed \"San Francisco\":\n");
  DECLASS_Region_show(&SF);

  return 0;
}
