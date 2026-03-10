
#include <pthread.h>

/* ── Node: each node carries its own per-node lock for HOH locking ── */
typedef struct Node {
  int data;
  struct Node *next;
  pthread_mutex_t lock;
} Node;

/* ── List: wraps the head pointer with a head-level lock ── */
typedef struct List {
  Node *head;
  pthread_mutex_t head_lock;
} List;

/* ── Original (single-threaded) operations on Node* ── */
void printList(Node *head);
Node *createNode(int data);
void append(Node **head, int data);
void freeList(Node *head);
int size(Node *head);
void addAtIndex(Node **head, int index, int data);
void addAtHead(Node **head, int data);
void addAtTail(Node **head, int data);
int front(Node *head);
int back(Node *head);
int get(Node *head, int index);
void removeAtIndex(Node **head, int index);
void removeAtHead(Node **head);
void removeAtTail(Node **head);
void clear(Node **head);

/* ── Hand-over-hand (HOH) concurrent operations on List* ── */
List *createList();
void printListHOH(List *list);
void freeListHOH(List *list);
int  hoh_search(List *list, int data);  /* returns 1 if found, 0 otherwise */
void hoh_insert(List *list, int data);  /* sorted insert                    */
void hoh_delete(List *list, int data);  /* deletes first node with value    */