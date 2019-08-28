/* DECLASSIFIED: deque_class_DECLASS.c
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
  fprintf(stderr, "-:- ERROR: COULDN'T %s MEMORY FOR SMRTPTR.H'S %s -:-\n\n", alloc_type, smrtptr_h_fcn);
  fprintf(stderr, "-:- FREEING ALLOCATED MEMORY THUS FAR AND TERMINATING PROGRAM -:-\n\n");
  exit(EXIT_FAILURE); // still frees any ptrs allocated thus far
}
// acts like assert, but exits rather than abort to free smart pointers
#ifndef DECLASS_NDEBUG
#define smrtassert(condition) ({\
  if(!(condition)) {\
    fprintf(stderr, "Smart Assertion failed: (%s), function %s, file %s, line %d.\n", #condition, __func__, __FILE__, __LINE__);\
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
      fprintf(stderr, "-:- ERROR: COULDN'T MALLOC MEMORY TO INITIALIZE SMRTPTR.H'S GARBAGE COLLECTOR -:-\n\n");
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
#include <stdlib.h>
#include <stdbool.h>


typedef struct node {
  int data;
  struct node *next;
  struct node *prev;
} NODE;


/******************************** CLASS START ********************************/
/* "Deque" CLASS DEFAULT VALUE MACRO CONSTRUCTORS: */
#define DC__Deque_CTOR(DC_THIS) ({DC_THIS = DC__Deque_DFLT();})
#define DC__Deque_ARR(DC_ARR) ({\
  for(int DC__Deque_IDX=0;DC__Deque_IDX<(sizeof(DC_ARR)/sizeof(DC_ARR[0]));++DC__Deque_IDX)\
    DC__Deque_CTOR(DC_ARR[DC__Deque_IDX]);\
})
#define DC__Deque_UCTOR_ARR(DC_ARR) ({\
  for(int DC__Deque_UCTOR_IDX=0;DC__Deque_UCTOR_IDX<(sizeof(DC_ARR)/sizeof(DC_ARR[0]));++DC__Deque_UCTOR_IDX)\
    DC_Deque_(&DC_ARR[DC__Deque_UCTOR_IDX]);\
})
/* "Deque" CLASS OBJECT ARRAY MACRO DESTRUCTOR: */
#define DC__Deque_UDTOR_ARR(DC_ARR) ({\
  for(int DC__Deque_UDTOR_IDX=0;DC__Deque_UDTOR_IDX<(sizeof(DC_ARR)/sizeof(DC_ARR[0]));++DC__Deque_UDTOR_IDX)\
		DC__NOT_Deque_(&DC_ARR[DC__Deque_UDTOR_IDX]);\
})

/* "Deque" CLASS CONVERTED TO STRUCT: */
typedef struct DC_Deque {
  NODE *head;
  int length;


} Deque;
Deque DC__Deque_DFLT(){
	Deque this={smrtmalloc(sizeof(NODE)),0,};
	return this;
}

/* DEFAULT PROVIDED "Deque" CLASS CONSTRUCTOR/DESTRUCTOR: */
#define DC__DUMMY_Deque()({\
	Deque DC__Deque__temp;\
	DC__Deque_CTOR(DC__Deque__temp);\
	DC_Deque_(&DC__Deque__temp);\
})

/* "Deque" CLASS METHODS SPLICED OUT: */
  Deque DC_Deque_(Deque *this) {
    this->head -> next = this->head;
    this->head -> prev = this->head;
    this->head -> data = 0;
  	return *this;
	}
  void DC__NOT_Deque_(Deque *this){
    NODE *pdel = this->head, *pnext;
    do {
      pnext = pdel -> next;
      smrtfree(pdel);
      pdel = pnext;
    } while(pdel != this->head);
    this->length = 0;
  }
  int DC_Deque_size(Deque *this) {return this->length;}
  void DC_Deque_addFirst(int elt, Deque *this) {
    NODE *p = smrtmalloc(sizeof(NODE));
    smrtassert(p != NULL);
    p -> data = elt;
    p -> next = this->head -> next;
    p -> prev = this->head;
    this->head -> next -> prev = p;
    this->head -> next = p;
    ++this->length;
  }
  void DC_Deque_addLast(int elt, Deque *this) {
    NODE *p = smrtmalloc(sizeof(NODE));
    smrtassert(p != NULL);
    p -> data = elt;
    p -> next = this->head;
    p -> prev = this->head -> prev;
    this->head -> prev -> next = p;
    this->head -> prev = p;
    ++this->length;
  }
  void DC_Deque_rmvNode(NODE *pdel, Deque *this) {
    pdel -> next -> prev = pdel -> prev;
    pdel -> prev -> next = pdel -> next;
    smrtfree(pdel);
    --this->length;
  }
  int DC_Deque_rmvFirst(Deque *this) {
    smrtassert(this->length > 0);
    NODE *pdel = this->head -> next;
    int elt = pdel -> data;
    DC_Deque_rmvNode(pdel, this);
    return elt;
  }
  int DC_Deque_rmvLast(Deque *this) {
    smrtassert(this->length > 0);
    NODE *pdel = this->head -> prev;
    int elt = pdel -> data;
    DC_Deque_rmvNode(pdel, this);
    return elt;
  }
  void DC_Deque_rmvItem(int elt, Deque *this) {
    NODE *pdel = this->head -> next;
    while(pdel != this->head) {
      if(pdel -> data == elt) {
        DC_Deque_rmvNode(pdel, this);
        return;
      }
      pdel = pdel -> next;
    }
  }
  int DC_Deque_getFirst(Deque *this) {
    smrtassert(this->length > 0);
    return this->head -> next -> data;
  }
  int DC_Deque_getLast(Deque *this) {
    smrtassert(this->length > 0);
    return this->head -> prev -> data;
  }
  bool DC_Deque_findItem(int elt, Deque *this) {
    NODE *p = this->head -> next;
    while(p != this->head) {
      if(p -> data == elt) {return true;}
      p = p -> next;
    }
    return false;
  }
  void DC_Deque_show(Deque *this) {
    if(this->length == 0) {return;}
    NODE *p = this->head -> next;
    while(p != this->head) {
      printf("%d ", p -> data);
      p = p -> next;
    }
    printf("\n");
  }
/********************************* CLASS END *********************************/


int main() {


  Deque list; DC__Deque_CTOR(list); DC_Deque_(&list); int DC_list=0;


  DC_Deque_addFirst(8, &list);
  DC_Deque_addFirst(9, &list);
  DC_Deque_addFirst(10, &list);
  DC_Deque_addLast(16, &list);
  DC_Deque_addLast(17, &list);


  printf("Added to Front: 8, 9, 10\nAdded to Back: 16, 17\n");
  printf("\"Deque\" object of length %d has contents: ", DC_Deque_size(&list));
  DC_Deque_show(&list);


  int rmvdFirstElt = DC_Deque_rmvFirst(&list);
  int rmvdLastElt = DC_Deque_rmvLast(&list);
  printf("\nRemoved First Node's Data: %d\n", rmvdFirstElt);
  printf("Removed Last Node's Data: %d\n", rmvdLastElt);


  DC_Deque_addLast(18, &list);
  int firstElt = DC_Deque_getFirst(&list);
  int lastElt = DC_Deque_getLast(&list);
  printf("\nRemoved First & Last, then Added to Back: 18\n");
  printf("Got the new First Node's Data: %d\n", firstElt);
  printf("Got the new Last Node's Data: %d\n", lastElt);


  DC_Deque_addLast(19, &list);
  DC_Deque_addLast(20, &list);
  DC_Deque_addFirst(7, &list);
  printf("Added to Front: 7\nAdded to Back: 19, 20\n");
  printf("\"Deque\" object of length %d has contents: ", DC_Deque_size(&list));
  DC_Deque_show(&list);
  int rmvData = 16;
  DC_Deque_rmvItem(rmvData, &list);
  printf("\"Deque\" object after rmving data \"%d\" has length %d & contents: ", rmvData, DC_Deque_size(&list));
  DC_Deque_show(&list);


  int soughtData = 20;
  bool dataFound = DC_Deque_findItem(soughtData, &list);
  printf("\nBoolean as to whether data \"%d\" was found: %d\n", soughtData, dataFound);

  if(!DC_list){DC__NOT_Deque_(&list);DC_list=1;}
return 0;
}
