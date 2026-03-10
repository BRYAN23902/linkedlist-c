#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>

Node *createNode(int data) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  newNode->data = data;
  newNode->next = NULL;
  pthread_mutex_init(&newNode->lock, NULL);
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
    pthread_mutex_destroy(&temp->lock);
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

int get(Node *head, int index) {
  if (index < 0) {
    fprintf(stderr, "Index out of bounds\n");
    exit(EXIT_FAILURE);
  }
  Node *temp = head;
  for (int i = 0; temp != NULL && i < index; i++) {
    temp = temp->next;
  }
  if (temp == NULL) {
    fprintf(stderr, "Index out of bounds\n");
    exit(EXIT_FAILURE);
  }
  return temp->data;
}

/* ═══════════════════════════════════════════════════════════════
 * Hand-Over-Hand (lock-coupling) concurrent linked list
 *
 * Each node has its own mutex.  When traversing, a thread always
 * holds the lock on the "current" node and grabs the lock on the
 * "next" node before releasing the current one – like climbing a
 * ladder one rung at a time.  This allows multiple threads to be
 * inside the list simultaneously as long as they are at different
 * positions, giving better throughput than a single list-level lock.
 * ═══════════════════════════════════════════════════════════════ */

List *createList() {
  List *list = (List *)malloc(sizeof(List));
  list->head = NULL;
  pthread_mutex_init(&list->head_lock, NULL);
  return list;
}

/* Print (hand-over-hand traversal – safe to call concurrently) */
void printListHOH(List *list) {
  pthread_mutex_lock(&list->head_lock);
  Node *curr = list->head;
  if (curr == NULL) {
    pthread_mutex_unlock(&list->head_lock);
    printf("NULL\n");
    return;
  }
  pthread_mutex_lock(&curr->lock);
  pthread_mutex_unlock(&list->head_lock);  /* hand off from head_lock to first node */

  while (curr != NULL) {
    printf("%d -> ", curr->data);
    Node *next = curr->next;
    if (next != NULL)
      pthread_mutex_lock(&next->lock);  /* grab next before releasing current */
    pthread_mutex_unlock(&curr->lock);
    curr = next;
  }
  printf("NULL\n");
}

/* Free all nodes (call only when no other thread uses the list) */
void freeListHOH(List *list) {
  pthread_mutex_lock(&list->head_lock);
  Node *curr = list->head;
  list->head = NULL;
  pthread_mutex_unlock(&list->head_lock);

  while (curr != NULL) {
    Node *next = curr->next;
    pthread_mutex_destroy(&curr->lock);
    free(curr);
    curr = next;
  }
  pthread_mutex_destroy(&list->head_lock);
  free(list);
}

/*
 * hoh_search – returns 1 if data is in the list, 0 otherwise.
 *
 * Locking protocol:
 *   1. Lock head_lock.
 *   2. If list is empty, unlock and return.
 *   3. Lock first node, then release head_lock.
 *   4. Hand-over-hand until we find the value or run off the end.
 */
int hoh_search(List *list, int data) {
  pthread_mutex_lock(&list->head_lock);
  Node *curr = list->head;
  if (curr == NULL) {
    pthread_mutex_unlock(&list->head_lock);
    return 0;
  }
  pthread_mutex_lock(&curr->lock);
  pthread_mutex_unlock(&list->head_lock);  /* hand off */

  while (curr != NULL) {
    if (curr->data == data) {
      pthread_mutex_unlock(&curr->lock);
      return 1;
    }
    Node *next = curr->next;
    if (next != NULL)
      pthread_mutex_lock(&next->lock);
    pthread_mutex_unlock(&curr->lock);
    curr = next;
  }
  return 0;
}

/*
 * hoh_insert – inserts data maintaining sorted (ascending) order.
 *
 * A sorted list makes the HOH benefit clearer: concurrent threads
 * can be walking different segments at the same time.
 *
 * Locking protocol:
 *   1. Lock head_lock.
 *   2. If inserting before (or as) head, splice in and unlock.
 *   3. Otherwise lock head, release head_lock, walk hand-over-hand
 *      until the correct gap is found.
 */
void hoh_insert(List *list, int data) {
  Node *newNode = createNode(data);

  pthread_mutex_lock(&list->head_lock);

  /* Case: empty list or new node goes before current head */
  if (list->head == NULL || data <= list->head->data) {
    newNode->next = list->head;
    list->head = newNode;
    pthread_mutex_unlock(&list->head_lock);
    return;
  }

  /* Lock the first node and hand off from head_lock */
  Node *prev = list->head;
  pthread_mutex_lock(&prev->lock);
  pthread_mutex_unlock(&list->head_lock);

  Node *curr = prev->next;
  while (curr != NULL) {
    pthread_mutex_lock(&curr->lock);
    if (data <= curr->data) {
      /* insert between prev and curr */
      newNode->next = curr;
      prev->next = newNode;
      pthread_mutex_unlock(&curr->lock);
      pthread_mutex_unlock(&prev->lock);
      return;
    }
    pthread_mutex_unlock(&prev->lock);
    prev = curr;
    curr = curr->next;
  }
  /* reached the tail */
  prev->next = newNode;
  pthread_mutex_unlock(&prev->lock);
}

/*
 * hoh_delete – removes the first node whose data matches.
 *
 * Locking protocol:
 *   1. Lock head_lock.
 *   2. If head is the target, lock head (ensures no traverser holds
 *      it), relink, release both locks, free.
 *   3. Otherwise lock head, release head_lock, walk hand-over-hand
 *      keeping both prev and curr locked until the target is found.
 */
void hoh_delete(List *list, int data) {
  pthread_mutex_lock(&list->head_lock);

  if (list->head == NULL) {
    pthread_mutex_unlock(&list->head_lock);
    return;
  }

  /* Case: head is the target */
  if (list->head->data == data) {
    Node *victim = list->head;
    pthread_mutex_lock(&victim->lock); /* wait for any traverser to pass */
    list->head = victim->next;
    pthread_mutex_unlock(&list->head_lock);
    pthread_mutex_unlock(&victim->lock);
    pthread_mutex_destroy(&victim->lock);
    free(victim);
    return;
  }

  /* Walk hand-over-hand holding prev + curr simultaneously */
  Node *prev = list->head;
  pthread_mutex_lock(&prev->lock);
  pthread_mutex_unlock(&list->head_lock);

  Node *curr = prev->next;
  while (curr != NULL) {
    pthread_mutex_lock(&curr->lock);
    if (curr->data == data) {
      prev->next = curr->next;
      pthread_mutex_unlock(&curr->lock);
      pthread_mutex_unlock(&prev->lock);
      pthread_mutex_destroy(&curr->lock);
      free(curr);
      return;
    }
    pthread_mutex_unlock(&prev->lock);
    prev = curr;
    curr = curr->next;
  }
  pthread_mutex_unlock(&prev->lock); /* value not found */
}