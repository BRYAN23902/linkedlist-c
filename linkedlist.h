
typedef struct Node {
  int data;
  struct Node *next;
} Node;

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