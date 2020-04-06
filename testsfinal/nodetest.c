#include<stdio.h>

int main(){
	struct node{
		struct node* next;
		int elem;
	};
	
	struct node * head = NULL;
	struct node * temp;
		

	insert:{
		if(head != NULL){
			temp = head;
			while(temp->next != NULL){
				temp = temp->next;	
			}	
		}
		else{

		}	
	}

insert();	
}
