// AUTHOR: JORDAN RANDLEMAN -- stack_class.c -- implementing a stack via declass.c
#include <stdio.h>
#include <stdbool.h>

class Stack {
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
  // => gets invoked at every object delcaration so long as "(<args>)" are provided,
  //    otherwise object only get default values without calling its constructor.
  Stack(int array[], int length) {
    for(int i = 0; i < length; ++i) 
      push(array[i]);
  }

  // class destructor, formatted like a ctor but prefixed w/ '~',
  // is invoked once an object is out of scope
  ~Stack() {
    printf("Stack object destroyed!\n");
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
