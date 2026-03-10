#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

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

/* ─────────────────────────────────────────────────────────────────────────
 * Hand-Over-Hand locking tests
 * ───────────────────────────────────────────────────────────────────────── */

#define NUM_THREADS   4
#define OPS_PER_THREAD 1000

typedef struct {
  List *list;
  int   thread_id;
} ThreadArgs;

/* Each thread inserts OPS_PER_THREAD values into the shared HOH list */
static void *insert_worker(void *arg) {
  ThreadArgs *a = (ThreadArgs *)arg;
  for (int i = 0; i < OPS_PER_THREAD; i++) {
    hoh_insert(a->list, a->thread_id * OPS_PER_THREAD + i);
  }
  return NULL;
}

/* Each thread deletes OPS_PER_THREAD values from the shared HOH list */
static void *delete_worker(void *arg) {
  ThreadArgs *a = (ThreadArgs *)arg;
  for (int i = 0; i < OPS_PER_THREAD; i++) {
    hoh_delete(a->list, a->thread_id * OPS_PER_THREAD + i);
  }
  return NULL;
}

/* Each thread searches for OPS_PER_THREAD values */
static void *search_worker(void *arg) {
  ThreadArgs *a = (ThreadArgs *)arg;
  int found = 0;
  for (int i = 0; i < OPS_PER_THREAD; i++) {
    found += hoh_search(a->list, a->thread_id * OPS_PER_THREAD + i);
  }
  return (void *)(long)found;
}

/* Helper: return wall-clock time in seconds */
static double now_sec() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/*
 * testHOH_concurrent –
 *   Spawns NUM_THREADS threads that insert concurrently, then NUM_THREADS
 *   threads that search concurrently, then NUM_THREADS threads that delete
 *   concurrently.  Checks that the list is empty at the end.
 */
static void testHOH_concurrent() {
  printf("\n--- testHOH_concurrent (%d threads x %d ops) ---\n",
         NUM_THREADS, OPS_PER_THREAD);

  List *list = createList();
  pthread_t threads[NUM_THREADS];
  ThreadArgs args[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    args[i].list = list;
    args[i].thread_id = i;
  }

  /* ── concurrent inserts ── */
  double t0 = now_sec();
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(&threads[i], NULL, insert_worker, &args[i]);
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(threads[i], NULL);
  double t1 = now_sec();
  printf("Insert: %d elements in %.4f s\n",
         NUM_THREADS * OPS_PER_THREAD, t1 - t0);
  printf("List size after inserts: %d (expected %d)\n",
         size(list->head), NUM_THREADS * OPS_PER_THREAD);

  /* ── concurrent searches ── */
  t0 = now_sec();
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(&threads[i], NULL, search_worker, &args[i]);
  long total_found = 0;
  for (int i = 0; i < NUM_THREADS; i++) {
    void *ret;
    pthread_join(threads[i], &ret);
    total_found += (long)ret;
  }
  t1 = now_sec();
  printf("Search: found %ld / %d in %.4f s\n",
         total_found, NUM_THREADS * OPS_PER_THREAD, t1 - t0);

  /* ── concurrent deletes ── */
  t0 = now_sec();
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(&threads[i], NULL, delete_worker, &args[i]);
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(threads[i], NULL);
  t1 = now_sec();
  printf("Delete: %d elements in %.4f s\n",
         NUM_THREADS * OPS_PER_THREAD, t1 - t0);
  printf("List size after deletes: %d (expected 0)\n", size(list->head));

  freeListHOH(list);
}

/*
 * testHOH_basic –
 *   Single-threaded correctness check of the HOH operations.
 */
static void testHOH_basic() {
  printf("\n--- testHOH_basic (single-threaded correctness) ---\n");

  List *list = createList();

  hoh_insert(list, 30);
  hoh_insert(list, 10);
  hoh_insert(list, 20);
  hoh_insert(list, 5);
  printf("After inserts (5,10,20,30 sorted): ");
  printListHOH(list); /* Expected: 5 -> 10 -> 20 -> 30 -> NULL */

  printf("Search 10: %s\n", hoh_search(list, 10) ? "found" : "not found"); /* found */
  printf("Search 99: %s\n", hoh_search(list, 99) ? "found" : "not found"); /* not found */

  hoh_delete(list, 10);
  printf("After delete(10): ");
  printListHOH(list); /* Expected: 5 -> 20 -> 30 -> NULL */

  hoh_delete(list, 5);  /* delete head */
  printf("After delete(5):  ");
  printListHOH(list); /* Expected: 20 -> 30 -> NULL */

  hoh_delete(list, 30); /* delete tail */
  printf("After delete(30): ");
  printListHOH(list); /* Expected: 20 -> NULL */

  hoh_delete(list, 99); /* delete non-existent – should be a no-op */
  printf("After delete(99): ");
  printListHOH(list); /* Expected: 20 -> NULL */

  freeListHOH(list);
}

void testAllHOH() {
  printf("\n=== Hand-Over-Hand Locking Tests ===\n");
  testHOH_basic();
  testHOH_concurrent();
}
