// AUTHOR: JORDAN RANDLEMAN -- deque_class.c -- implementing a deque linked list via declass.c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct node {
  int data;
  struct node *next;
  struct node *prev;
} NODE;


class Deque {
  NODE *head = smrtmalloc(sizeof(NODE)); // sentinel node
  int length; // as a member, length's default is 0

  Deque() { // constructor
    head -> next = head;
    head -> prev = head;
    head -> data = 0;
  }

  ~Deque(){ // destructor
    NODE *pdel = head, *pnext;
    do {
      pnext = pdel -> next;
      smrtfree(pdel);
      pdel = pnext;
    } while(pdel != head);
    length = 0;
  }

  int size() {return length;}

  void addFirst(int elt) {
    NODE *p = smrtmalloc(sizeof(NODE));
    smrtassert(p != NULL);
    p -> data = elt;
    p -> next = head -> next;
    p -> prev = head;
    head -> next -> prev = p;
    head -> next = p;
    ++length;
  }

  void addLast(int elt) {
    NODE *p = smrtmalloc(sizeof(NODE));
    smrtassert(p != NULL);
    p -> data = elt;
    p -> next = head;
    p -> prev = head -> prev;
    head -> prev -> next = p;
    head -> prev = p;
    ++length;
  }

  void rmvNode(NODE *pdel) {
    pdel -> next -> prev = pdel -> prev;
    pdel -> prev -> next = pdel -> next;
    smrtfree(pdel);
    --length;
  }

  int rmvFirst() {
    smrtassert(length > 0);
    NODE *pdel = head -> next;
    int elt = pdel -> data;
    rmvNode(pdel);
    return elt;
  }

  int rmvLast() {
    smrtassert(length > 0);
    NODE *pdel = head -> prev;
    int elt = pdel -> data;
    rmvNode(pdel);
    return elt;
  }

  void rmvItem(int elt) {
    NODE *pdel = head -> next;
    while(pdel != head) {
      if(pdel -> data == elt) {
        rmvNode(pdel);
        return;
      }
      pdel = pdel -> next;
    }
  }

  int getFirst() {
    smrtassert(length > 0);
    return head -> next -> data;
  }

  int getLast() {
    smrtassert(length > 0);
    return head -> prev -> data;
  }

  bool findItem(int elt) {
    NODE *p = head -> next;
    while(p != head) {
      if(p -> data == elt) return true;
      p = p -> next;
    }
    return false;
  }

  void show() {
    if(length == 0) return;
    NODE *p = head -> next;
    while(p != head) {
      printf("%d ", p -> data);
      p = p -> next;
    }
    printf("\n");
  }
}


int main() {

  // construct deque
  Deque list();

  // add some data to deque
  list.addFirst(8);
  list.addFirst(9);
  list.addFirst(10);
  list.addLast(16);
  list.addLast(17);

  // output deque length & data
  printf("Added to Front: 8, 9, 10\nAdded to Back: 16, 17\n");
  printf("\"Deque\" object of length %d has contents: ", list.size());
  list.show();

  // removing the first & last nodes
  int rmvdFirstElt = list.rmvFirst();
  int rmvdLastElt = list.rmvLast();
  printf("\nRemoved First Node's Data: %d\n", rmvdFirstElt);
  printf("Removed Last Node's Data: %d\n", rmvdLastElt);

  // adding data to the end of the deque & getting (not rmving)
  // first & last node data
  list.addLast(18);
  int firstElt = list.getFirst();
  int lastElt = list.getLast();
  printf("\nRemoved First & Last, then Added to Back: 18\n");
  printf("Got the new First Node's Data: %d\n", firstElt);
  printf("Got the new Last Node's Data: %d\n", lastElt);

  // adding data then removing an specific instance of data
  list.addLast(19);
  list.addLast(20);
  list.addFirst(7);
  printf("Added to Front: 7\nAdded to Back: 19, 20\n");
  printf("\"Deque\" object of length %d has contents: ", list.size());
  list.show();
  int rmvData = 16;
  list.rmvItem(rmvData);
  printf("\"Deque\" object after rmving data \"%d\" has length %d & contents: ", rmvData, list.size());
  list.show();

  // checking whether an item exists in deque
  int soughtData = 20;
  bool dataFound = list.findItem(soughtData);
  printf("\nBoolean as to whether data \"%d\" was found: %d\n", soughtData, dataFound);

  return 0;
}
