#pragma once
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <malloc.h>

typedef struct Node node;
struct LinkedList;
node* createNode(int);
void addFirst(node*);
void insert( node*);
node* dequeue();
void catcher(int);

