#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>

void test1() {
  Node *head = NULL;
  addAtHead(&head, 10);
  addAtTail(&head, 20);
  addAtTail(&head, 30);
  printList(head); // Expected: 10 -> 20 -> 30 -> NULL
  clear(&head);
}

void test2() {
  Node *head = NULL;
  addAtTail(&head, 1);
  addAtTail(&head, 2);
  addAtTail(&head, 3);
  addAtTail(&head, 4);
  addAtTail(&head, 5);
  printList(head); // Expected: 1 -> 2 -> 3 -> 4 -> 5 -> NULL
  removeAtIndex(&head, 2);
  printList(head); // Expected: 1 -> 2 -> 4 -> 5 -> NULL
  clear(&head);
}

void test3() {
  Node *head = NULL;
  addAtTail(&head, 100);
  addAtTail(&head, 200);
  addAtTail(&head, 300);
  printList(head); // Expected: 100 -> 200 -> 300 -> NULL
  removeAtHead(&head);
  printList(head); // Expected: 200 -> 300 -> NULL
  removeAtTail(&head);
  printList(head); // Expected: 200 -> NULL
  clear(&head);
}

void testAll() {
  test1();
  test2();
  test3();
}
