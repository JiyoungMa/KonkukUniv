#pragma once
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

typedef struct Node {
	int pid;
	long pdbr;
	int PFN;
	long p_pdbr; //바로 직전의 테이블의 나를 가리키는 인덱스의 pdbr
	struct Node* prev;
	struct Node* next;
}node;

typedef struct LinkedList {
	node* head;
	node* tail;
	int count;
}LinkedList;

typedef struct ku_pte {
	char pte;
}ku_pte;

char* physical_memory;
char* swap_space;
LinkedList* PCB;
LinkedList* freelist;
LinkedList* s_free; //swap space의 free list
LinkedList* p_list; //physical memory에 들어가있는 page frame 순서 기록을 위함


node* createNode(long pdbr, long memory_base) {
	node* n = (node*)malloc(sizeof(node));

	n->pid = -1;
	n->pdbr = pdbr;
	n->PFN = (pdbr - memory_base) / 4;
	n->prev = NULL;
	n->next = NULL;
	return n;
}

node* createPCBNode(int pid, long pdbr) { //PCB를 위한 Node 만들기
	node* n = (node*)malloc(sizeof(node));
	n->pid = pid;;
	n->pdbr = pdbr;
	n->PFN = (pdbr - (long)physical_memory) / 4;
	n->prev = NULL;
	n->next = NULL;
	return n;
}

node* createFLNode(int PFN) { //FreeList를 위한 Node 만들기
	node* n = (node*)malloc(sizeof(node));
	n->pid = -1;
	n->pdbr = PFN * 4 + (long)physical_memory;
	n->PFN = PFN;
	n->prev = NULL;
	n->next = NULL;
	return n;
}

node* createSFNode(long spfn) { //s_free를 위한 Node 만들기
	node* n = (node*)malloc(sizeof(node));
	n->pid = -1;
	n->pdbr = spfn*4 + (long)swap_space;
	n->PFN = spfn;
	n->prev = NULL;
	n->next = NULL;
	return n;
}

node* createPLNode(long pdbr) { // p_list를 위한 Node만들기
	node* n = (node*)malloc(sizeof(node));
	n->pid = -1;
	n->pdbr = pdbr;
	n->PFN = (pdbr - (long)physical_memory) / 4;
	n->prev = NULL;
	n->next = NULL;
	return n;
}

LinkedList* createLinkedList(LinkedList* linkedlist) {
	node* head = createNode(-1, -1);
	node* tail = createNode(-1, -1);

	head->next = tail;
	tail->prev = head;

	linkedlist = (LinkedList*)malloc(sizeof(LinkedList));

	linkedlist->head = head;
	linkedlist->tail = tail;
	linkedlist->count = 0;

	return linkedlist;
}



void addFirst(node* n, LinkedList* linkedlist) {
	linkedlist->head->next = n;
}


void insert(node* newNode, LinkedList* linkedlist) {
	if (linkedlist->head->next == NULL) {
		addFirst(newNode, linkedlist);
	}
	else {
		node* middle = linkedlist->tail->prev;
		middle->next = newNode;
		newNode->prev = middle;
		newNode->next = linkedlist->tail;
		linkedlist->tail->prev = newNode;
		linkedlist->count++;
	}
}

node* dequeue(LinkedList* linkedlist) {
	node* current = linkedlist->head->next;
	if (current == linkedlist->tail) {
		return NULL;
	}
	linkedlist->head->next = current->next;
	current->next->prev = linkedlist->head;
	linkedlist->count--;
	return current;

}



node* searchPCB(int pid) {
	node* current = PCB->head;
	while (current->pid != pid && current != PCB->tail) {
		current = current->next;
	}
	if (current == PCB->tail) {
		return NULL;
	}
	else {
		return current;
	}
}

void* ku_mmu_init(unsigned int mem_size, unsigned int swap_size) {
	physical_memory = (char*)malloc(mem_size);
	swap_space = (char*)malloc(swap_size);

	long pa = (long)(physical_memory)+4;

	freelist = createLinkedList(freelist);
	PCB = createLinkedList(PCB);
	s_free = createLinkedList(s_free);
	p_list = createLinkedList(p_list);

	while (pa < (long)physical_memory + mem_size) {
		node* n = createNode(pa, (long)physical_memory);
		pa += 4;
		insert(n, freelist);
	}

	long sa = (long)(swap_space)+4;
	while (sa < (long)swap_space + swap_size) {
		node* n = createNode(sa, (long)swap_space);
		sa += 4;
		insert(n, s_free);
	}
	return physical_memory;
}



int swapping() {
	if (s_free->count <= 0) {
		return -1;
	}
	node* swap_in = dequeue(p_list);
	if (swap_in == NULL) {
		return -1;
	}
	node* sp_free = dequeue(s_free);
	if (sp_free == NULL) {
		return -1;
	}
	int sframe = sp_free->PFN;
	sframe = sframe << 1;

	long p = swap_in->p_pdbr;
	physical_memory[p] = sframe;
	insert(swap_in, freelist);
	return 0;
}

int ku_run_proc(char pid, ku_pte** ku_cr3) {
	node* p = searchPCB(pid);
	ku_pte* ku = (ku_pte*)malloc(1);
	if (p == NULL) { //해당 pid가 처음인 경우
		if (freelist->count == 0) {
			if (swapping() == -1) {
				return -1;
			}
		}
		if (freelist->count != 0) {
			// Page Directory만들고 모든 값을 "00000000"으로 맵핑해주기

			node* free = dequeue(freelist);
			for (int i = 0; i < 4; i++) {
				physical_memory[free->PFN * 4 + i] = 0;
			}
			node* pnode = createPCBNode(pid, free->pdbr);
			insert(pnode, PCB);
			ku = &physical_memory[free->PFN * 4];

			*ku_cr3 = ku;

			//PFN을 2진수의 모양으로 저장하기
		}
		else {
			return -1;
		}

	}
	else {
		ku = &physical_memory[p->PFN * 4];

		*ku_cr3 = ku;


	}
	return 0;
}


int ku_page_fault(char pid, char va) {
	node* now = searchPCB(pid); //pid에 해당하는 노드
	int virtual_address[4] = { 0,0,0,0 };
	int v = (int)va;
	int d = 128;
	for (int i = 0; i < 4; i++) {
		if (v - d >= 0) {
			virtual_address[i] += 2;
			v -= d;
		}
		d /= 2;
		if (v - d >= 0) {
			virtual_address[i] += 1;
			v -= d;
		}
		d /= 2;
	}
	int pfn = now->PFN;
	int now_index = pfn * 4 + virtual_address[0];
	for (int i = 1; i < 4; i++) {
		if (physical_memory[now_index] == 0) { // never mapped or swapped
			if (freelist->count == 0) {
				if (swapping() == -1) {
					return -1;
				}
			}
			if (freelist->count != 0) {
				node* free = dequeue(freelist);
				for (int a = 0; a < 4; a++) {
					physical_memory[free->PFN * 4 + a] = 0;
				}
				int map_pfn = free->PFN;
				map_pfn = map_pfn << 2;
				map_pfn++;
				physical_memory[now_index] = map_pfn;
				free->p_pdbr = now_index;
				now_index = free->PFN * 4 + virtual_address[i];
				if (i == 3) {
					insert(free, p_list);
					physical_memory[now_index] = 1;
				}

			}
			else {
				return -1;
			}
		}
		else {
			char c = physical_memory[now_index];
			if (c % 2 == 1) { // physical memory에 위치함
				c = c - 1;
				now_index = virtual_address[i] + c ;
				if (i == 3) {
					physical_memory[now_index] = 1;
				}
			}
			else {
				if (freelist->count == 0) {
					swapping();
				}
				if (freelist->count == 0) {
					return -1;
				}
				node* free = dequeue(freelist);
				c = c << 1;
				node* sfree = createSFNode(c);
				insert(sfree, s_free);

				int mapping = free->PFN;
				mapping = mapping << 2;
				mapping += 1;

				insert(free, PCB);
				physical_memory[now_index] = mapping;
				now_index = now_index + virtual_address[i];
				if (i == 3) {
					physical_memory[now_index] = 1;
				}
			}
		}

	}

	return 0;
}



