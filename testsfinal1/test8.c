/*
This test case checks implementation of linked list.
It implements insert() and traverse() as nested functions.
Head of linked list i.e. struct node* head is declared in main function and used in insert and traverse.

Basically, this test case verifies one of the case of support of tool for pointers.

Expected Output:
Linked List: 0->10->20->30->40->50->60->70->80->90->NULL
*/

#include<stdio.h>
#include<stdlib.h>

struct node{
		struct node* next;
		int data;
	};

int main(){
	
	struct node * head = NULL;
	struct node * temp;
	int data;
	int i;	

	insert:{
		if(head != NULL){
			temp = head;
			while(temp->next != NULL){
				temp = temp->next;	
			}
			temp->next = (struct node *) malloc(sizeof(struct node));
			temp = temp->next;
			temp->data = data;	
			temp->next = NULL;
		}
		else{	
			head = (struct node *) malloc(sizeof(struct node));
			head->next =NULL;
			head->data = data;	
		}	
	}


	for(i = 0; i < 10; i++)
	{
		data = i*10;
		insert();	
	}
	
	traverse:{
	temp = head;
	printf("\nLinked List: ");
	while(temp != NULL){
			printf("%d->", temp->data);
			temp = temp->next;
		}
	printf("NULL\n");
	}

	traverse();
return 0;
}

