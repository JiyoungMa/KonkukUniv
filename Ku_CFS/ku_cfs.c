#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <malloc.h>
#include "ku_cfs.h"

typedef struct Node {
	int value;
	double vrun;
	double nice;
	struct Node* next;
	char c;
}node;

struct LinkedList {
	node* head;
};
struct LinkedList* readyQ;

node* createNode(int value) {
	node* n = malloc(sizeof(node));
	n->value = value;
	n->next = NULL;
	n->vrun = 0;
	n->nice = 0;
	return n;
}

void addFirst(node* n) {
	readyQ->head->next = n;
}

void insert(node* newNode) {
	if (readyQ->head->next == NULL) {
		addFirst(newNode);
	}
	else {
		node* current = readyQ->head;
		while (current->next != NULL && current->next->vrun <= newNode->vrun) {
			current = current->next;
		}
		node* mid = current->next;
		current->next = newNode;
		newNode->next = mid;
	}
}

node* dequeue() {
	node* current = readyQ->head->next;
	readyQ->head->next = current->next;
	return current;
}

node* now;
node* before;
int timecount = 0;

void catcher(int sig) {
	if (now != NULL) {
		kill(now->value, SIGSTOP);
		before = now;
		before->vrun = before->vrun + before->nice;
		insert(before);
	}
	now = dequeue();
	kill(now->value, SIGCONT);
	timecount++;
}

void main(int argc, char** argv) {
	readyQ = (struct LinkedList*)malloc(sizeof(struct LinkedList));
	int time_slices = atoi(argv[6]);
	int n[5];
	for (int i = 1; i < 6; i++) {
		n[i - 1] = atoi(argv[i]);
	}

	double nice[5] = { 0.64, 0.8, 1,1.25,1.5625 };
	readyQ->head = malloc(sizeof(node));

	int which = ITIMER_REAL;
	struct sigaction sact;
	sact.sa_handler = &catcher;
	sigaction(SIGALRM, &sact, NULL);
	struct itimerval value;
	value.it_interval.tv_sec = 1;
	value.it_interval.tv_usec = 0;
	value.it_value.tv_usec = 0;
	value.it_value.tv_sec = 5;

	char c = 'A';
	char* cin = malloc(sizeof(char));
	node* new = malloc(sizeof(node));
	int pid = 0;

	for (int i = 0; i < 5; i++) {
		if (n[i] != 0) {
			for (int a = 0; a < n[i]; a++) {
				pid = fork();
				if (pid != 0) {
					new = createNode(pid);
					new->nice = nice[i];
					new->c = c;
					c++;
					insert(new);
				}
				else {
					*cin = c;
					break;
				}

			}
			if (pid == 0) break;
		}
	}

	if (pid != 0) {
		setitimer(which, &value, NULL);
		
		while (timecount <= time_slices) {

		}
		value.it_interval.tv_sec = 0;
		value.it_interval.tv_usec = 0;
		value.it_value.tv_usec = 0;
		value.it_value.tv_sec = 0;
		setitimer(which, &value, NULL);
		free(new);
		free(cin);
		free(readyQ->head);

		node* child = readyQ->head->next;
		while(child->next != NULL){
			kill(child->value,SIGKILL);
			child = child->next;
		}
	}
	else {
		execl("./ku_app", "ku_app", cin, NULL);
	}

}