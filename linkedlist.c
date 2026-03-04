#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>

Node *createNode(int data) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  newNode->data = data;
  newNode->next = NULL;
  return newNode;
}

void printList(Node *head) {
  Node *temp = head;
  while (temp != NULL) {
    printf("%d -> ", temp->data);
    temp = temp->next;
  }
  printf("NULL\n");
}

void append(Node **head, int data) {
  Node *newNode = createNode(data);
  if (*head == NULL) {
    *head = newNode;
    return;
  }
  Node *temp = *head;
  while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->next = newNode;
}

void freeList(Node *head) {
  Node *temp;
  while (head != NULL) {
    temp = head;
    head = head->next;
    free(temp);
  }
}

int size(Node *head) {
  int count = 0;
  Node *temp = head;
  while (temp != NULL) {
    count++;
    temp = temp->next;
  }
  return count;
}

void addAtIndex(Node **head, int index, int data) {
  if (index < 0)
    return;
  Node *newNode = createNode(data);
  if (index == 0) {
    newNode->next = *head;
    *head = newNode;
    return;
  }
  Node *temp = *head;
  for (int i = 0; temp != NULL && i < index - 1; i++) {
    temp = temp->next;
  }
  if (temp == NULL) {
    free(newNode);
    return;
  }
  newNode->next = temp->next;
  temp->next = newNode;
}

void addAtHead(Node **head, int data) { addAtIndex(head, 0, data); }

void addAtTail(Node **head, int data) { append(head, data); }

int front(Node *head) {
  if (head == NULL) {
    fprintf(stderr, "List is empty\n");
    exit(EXIT_FAILURE);
  }
  return head->data;
}

int back(Node *head) {
  if (head == NULL) {
    fprintf(stderr, "List is empty\n");
    exit(EXIT_FAILURE);
  }
  Node *temp = head;
  while (temp->next != NULL) {
    temp = temp->next;
  }
  return temp->data;
}
void removeAtIndex(Node **head, int index) {
  if (*head == NULL || index < 0)
    return;
  Node *temp = *head;
  if (index == 0) {
    *head = temp->next;
    free(temp);
    return;
  }
  for (int i = 0; temp != NULL && i < index - 1; i++) {
    temp = temp->next;
  }
  if (temp == NULL || temp->next == NULL)
    return;
  Node *next = temp->next->next;
  free(temp->next);
  temp->next = next;
}

void removeAtHead(Node **head) { removeAtIndex(head, 0); }

void removeAtTail(Node **head) {
  if (*head == NULL)
    return;
  if ((*head)->next == NULL) {
    free(*head);
    *head = NULL;
    return;
  }
  Node *temp = *head;
  while (temp->next->next != NULL) {
    temp = temp->next;
  }
  free(temp->next);
  temp->next = NULL;
}

void clear(Node **head) {
  freeList(*head);
  *head = NULL;
}