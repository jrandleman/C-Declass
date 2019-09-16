/* DECLASSIFIED: stack_class.c
 * Email jrandleman@scu.edu or see https://github.com/jrandleman for support */
#define immortal // immortal keyword active
/****************************** SMRTPTR.H START ******************************/
// Source: https://github.com/jrandleman/C-Libraries/tree/master/Smart-Pointer
#ifndef SMRTPTR_H_
#define SMRTPTR_H_
#include <stdio.h>
#include <stdlib.h>
// garbage collector & smart pointer storage struct
static struct SMRTPTR_GARBAGE_COLLECTOR {
  long len, max; // current # of ptrs && max capacity
  void **ptrs;   // unique ptr set to free all smrt ptrs
} SMRTPTR_GC = {-1};
// invoked by atexit to free all ctor-alloc'd memory
static void smrtptr_free_all() {
  int i = 0;
  for(; i < SMRTPTR_GC.len; ++i) free(SMRTPTR_GC.ptrs[i]);
  if(SMRTPTR_GC.len > 0) free(SMRTPTR_GC.ptrs);
  // if(SMRTPTR_GC.len > 0) printf("FREED %ld SMART POINTERS!\n", SMRTPTR_GC.len); // optional
  SMRTPTR_GC.len = 0;
}
// throws invalid allocation errors
static void smrtptr_throw_bad_alloc(char *alloc_type, char *smrtptr_h_fcn) {
  fprintf(stderr, "\n-:- \033[1m\033[31mERROR\033[0m COULDN'T %s MEMORY FOR SMRTPTR.H'S %s -:-\n\n", alloc_type, smrtptr_h_fcn);
  fprintf(stderr, "-:- FREEING ALLOCATED MEMORY THUS FAR AND TERMINATING PROGRAM -:-\n\n");
  exit(EXIT_FAILURE); // still frees any ptrs allocated thus far
}
// acts like assert, but exits rather than abort to free smart pointers
#ifndef DECLASS_NDEBUG
#define smrtassert(condition) ({\
  if(!(condition)) {\
    fprintf(stderr, "\n\033[1m\033[31mERROR\033[0m Smart Assertion failed: (%s), function %s, file %s, line %d.\n", #condition, __func__, __FILE__, __LINE__);\
    fprintf(stderr, ">> Freeing Allocated Smart Pointers & Terminating Program.\n\n");\
    exit(EXIT_FAILURE);\
  }\
})
#else
#define smrtassert(condition)
#endif
// smrtptr stores ptr passed as arg to be freed atexit
void smrtptr(void *ptr) {
  // free ptrs atexit
  atexit(smrtptr_free_all);
  // malloc garbage collector
  if(SMRTPTR_GC.len == -1) {
    SMRTPTR_GC.ptrs = malloc(sizeof(void *) * 10);
    if(!SMRTPTR_GC.ptrs) {
      fprintf(stderr, "\n-:- \033[1m\033[31mERROR\033[0m COULDN'T MALLOC MEMORY TO INITIALIZE SMRTPTR.H'S GARBAGE COLLECTOR -:-\n\n");
      exit(EXIT_FAILURE);
    }
    SMRTPTR_GC.max = 10, SMRTPTR_GC.len = 0;
  }
  // reallocate if "max" ptrs already added
  if(SMRTPTR_GC.len == SMRTPTR_GC.max) {
    SMRTPTR_GC.max *= SMRTPTR_GC.max;
    void **SMRTPTR_TEMP = realloc(SMRTPTR_GC.ptrs, sizeof(void *) * SMRTPTR_GC.max);
    if(!SMRTPTR_TEMP) smrtptr_throw_bad_alloc("REALLOC", "GARBAGE COLLECTOR");
    SMRTPTR_GC.ptrs = SMRTPTR_TEMP;
  }
  // add ptr to SMRTPTR_GC if not already present (ensures no double-freeing)
  int i = 0;
  for(; i < SMRTPTR_GC.len; ++i) if(SMRTPTR_GC.ptrs[i] == ptr) return;
  SMRTPTR_GC.ptrs[SMRTPTR_GC.len++] = ptr;
  // printf("SMART POINTER #%ld STORED!\n", SMRTPTR_GC.len); // optional
}
// malloc's a pointer, stores it in the garbage collector, then returns ptr
void *smrtmalloc(size_t alloc_size) {
  void *smtr_malloced_ptr = malloc(alloc_size);
  if(smtr_malloced_ptr == NULL) smrtptr_throw_bad_alloc("MALLOC", "SMRTMALLOC FUNCTION");
  smrtptr(smtr_malloced_ptr);
  return smtr_malloced_ptr;
}
// calloc's a pointer, stores it in the garbage collector, then returns ptr
void *smrtcalloc(size_t alloc_num, size_t alloc_size) {
  void *smtr_calloced_ptr = calloc(alloc_num, alloc_size);
  if(smtr_calloced_ptr == NULL) smrtptr_throw_bad_alloc("CALLOC", "SMRTCALLOC FUNCTION");
  smrtptr(smtr_calloced_ptr);
  return smtr_calloced_ptr;
}
// realloc's a pointer, stores it anew in the garbage collector, then returns ptr
// compatible both "smart" & "dumb" ptrs!
void *smrtrealloc(void *ptr, size_t realloc_size) {
  int i = 0;
  void *smtr_realloced_ptr;
  // realloc a "smart" ptr already in garbage collector
  for(; i < SMRTPTR_GC.len; ++i)
    if(SMRTPTR_GC.ptrs[i] == ptr) {
      smtr_realloced_ptr = realloc(ptr, realloc_size); // frees ptr in garbage collector
      if(smtr_realloced_ptr == NULL) smrtptr_throw_bad_alloc("REALLOC", "SMRTREALLOC FUNCTION");
      SMRTPTR_GC.ptrs[i] = smtr_realloced_ptr; // point freed ptr at realloced address
      // printf("SMART POINTER REALLOC'D!\n"); // optional
      return smtr_realloced_ptr;
    }
  // realloc a "dumb" ptr then add it to garbage collector
  smtr_realloced_ptr = realloc(ptr, realloc_size);
  if(smtr_realloced_ptr == NULL) smrtptr_throw_bad_alloc("REALLOC", "SMRTREALLOC FUNCTION");
  smrtptr(smtr_realloced_ptr);
  return smtr_realloced_ptr;
}
// prematurely frees ptr arg prior to atexit (if exists)
void smrtfree(void *ptr) {
  int i = 0, j;
  for(; i < SMRTPTR_GC.len; ++i) // find ptr in garbage collector
    if(SMRTPTR_GC.ptrs[i] == ptr) {
      free(ptr);
      for(j = i; j < SMRTPTR_GC.len - 1; ++j) // shift ptrs down
        SMRTPTR_GC.ptrs[j] = SMRTPTR_GC.ptrs[j + 1];
      SMRTPTR_GC.len--;
      // printf("SMART POINTER FREED!\n"); // optional
      return;
    }
}
#endif
/******************************* SMRTPTR.H END *******************************/

 
#include <stdio.h>
#include <stdbool.h>

/******************************** CLASS START ********************************/
/* "Stack" CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */
#define DC__Stack_CTOR(DC_THIS) ({DC_THIS = DC__Stack_DFLT();})
#define DC__Stack_ARR(DC_ARR) ({\
  for(int DC__Stack_IDX=0;DC__Stack_IDX<(sizeof(DC_ARR)/sizeof(DC_ARR[0]));++DC__Stack_IDX)\
    DC__Stack_CTOR(DC_ARR[DC__Stack_IDX]);\
})
#define DC__Stack_UCTOR_ARR(DC_ARR, DC___A1_Stack, DC___A2_Stack) ({\
  for(int DC__Stack_UCTOR_IDX=0;DC__Stack_UCTOR_IDX<(sizeof(DC_ARR)/sizeof(DC_ARR[0]));++DC__Stack_UCTOR_IDX)\
    DC_Stack_(DC___A1_Stack, DC___A2_Stack, &DC_ARR[DC__Stack_UCTOR_IDX]);\
})
/* "Stack" CLASS OBJECT ARRAY MACRO DESTRUCTOR: */
#define DC__Stack_UDTOR_ARR(DC_ARR) ({\
  for(int DC__Stack_UDTOR_IDX=0;DC__Stack_UDTOR_IDX<(sizeof(DC_ARR)/sizeof(DC_ARR[0]));++DC__Stack_UDTOR_IDX)\
		DC__NOT_Stack_(&DC_ARR[DC__Stack_UDTOR_IDX]);\
})

/* "Stack" CLASS CONVERTED TO STRUCT: */
typedef struct DC_Stack {
  int *arr;
  int len;
  int max;


} Stack;
Stack DC__Stack_DFLT(){
	Stack this={smrtmalloc(sizeof(int) * 10),0,10,};
	return this;
}

/* DEFAULT PROVIDED "Stack" CLASS CONSTRUCTOR/DESTRUCTOR: */
#define DC__DUMMY_Stack(DC___D1_Stack, DC___D2_Stack)({\
	Stack DC__Stack__temp;\
	DC__Stack_CTOR(DC__Stack__temp);\
	DC_Stack_(DC___D1_Stack, DC___D2_Stack,  &DC__Stack__temp);\
})

/* "Stack" CLASS METHODS SPLICED OUT: */
  void DC_Stack_push(int elt, Stack *this) {
    if(this->len == this->max) {
      this->max *= this->max;
      this->arr = smrtrealloc(this->arr, sizeof(int) * this->max);
    } else
      {this->arr[this->len++] = elt;}
  }
  bool DC_Stack_pop(int *elt, Stack *this) {
    if(this->len == 0) {return false;}
    *elt = this->arr[--this->len];
    return true;
  }
  bool DC_Stack_top(int *elt, Stack *this) {
    if(this->len == 0) {return false;}
    *elt = this->arr[this->len-1];
    return true;
  }
  void DC_Stack_show(Stack *this) { for(int i = 0; i < this->len; ++i) {printf("%d ", this->arr[i]);} printf("\n"); }
  int DC_Stack_size(Stack *this) { return this->len; }
  Stack DC_Stack_(int array[], int length, Stack *this) {
    for(int i = 0; i < length; ++i)
      {DC_Stack_push(array[i], this);}
  	return *this;
	}
  void DC__NOT_Stack_(Stack *this) {
    printf("Stack object destroyed!\n");
  }
/********************************* CLASS END *********************************/


int main() {

  printf("Working with a single \"Stack\" object initializaed with its default values:\n");
  Stack myStack; DC__Stack_CTOR(myStack); int DC_myStack=0;
  DC_Stack_push(8, &myStack);
  DC_Stack_push(10, &myStack);
  DC_Stack_push(12, &myStack);
  printf("Pushed 8, 10, then 12:\n");
  DC_Stack_show(&myStack);
  int x;
  bool popped = DC_Stack_pop(&x, &myStack);
  if(popped) {printf("Popped Value: %d\n", x);}

  DC_Stack_show(&myStack);
  DC_Stack_push(100, &myStack);
  printf("Pushing 100:\n");
  DC_Stack_show(&myStack);
  printf("Pushing 888:\n");
  DC_Stack_push(888, &myStack);
  DC_Stack_show(&myStack);

  int y;
  bool topped = DC_Stack_top(&y, &myStack);
  if(topped) {printf("Top Value: %d\n", y);}
  int size = DC_Stack_size(&myStack);
  printf("Stack's size: %d, Stack's current max capacity: %d\n", size, myStack.max);


  printf("\nInitializing a \"Stack\" object via its default values & class constructor:\n");
  int arr[20] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181};
  Stack newStack; DC__Stack_CTOR(newStack); DC_Stack_(arr, 20, &newStack); int DC_newStack=0;
  printf("\"Stack\" object made with its class constructor:\n");
  DC_Stack_show(&newStack);

  if(!DC_myStack){DC__NOT_Stack_(&myStack);DC_myStack=1;}
if(!DC_newStack){DC__NOT_Stack_(&newStack);DC_newStack=1;}
return 0;
}
