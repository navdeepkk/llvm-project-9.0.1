//-------------------------------------------// 
// This code demostrates the ability of code //
// to handle pointers .note if structures are//
// used as an argument for malloc then it has//
// to be globally defined because the argument/
// to malloc is of type that is not handeled //
// in the code and hence it is not renamed.  //
// note arguments of type int, float etc will//
// work perfectly for malloc. the code shows //
// the insertion and traversal of	 the linked//
// list.																		 //
// EXPECTED OUTPUT:													 //
// elem is: 0																 //
// elem is: 1																 //
// elem is: 2																 //
// elem is: 3																 //
// elem is: 4													 			 //
//-------------------------------------------// 



#include<stdio.h>
#include<stdlib.h>

struct node{																	//strucuture declared as global jus beacuse malloc need the type as struct node.
	struct node* next;													//if it was inside then the tool would not rename the argument is malloc and code would break.
	int elem;
};

int main(){

	struct node * head = NULL;
	struct node * temp;
	int toinsert;

insert:{
				 if(head != NULL){
					 temp = head;
					 while(temp->next != NULL){
						 temp = temp->next;	
					 }
					 temp->next = (struct node *) malloc(sizeof(struct node));
					 temp = temp->next;
					 temp->elem = toinsert;	
					 temp->next = NULL;
				 }
				 else{	
					 head = (struct node *) malloc(sizeof(struct node));
					 head->next =NULL;
					 head->elem = toinsert;	
				 }	
			 }

			 int i;
			 for(i = 0; i < 5; i++){
				 toinsert = i;
				 insert();	
			 }

print:{
				temp = head;
				while(temp != NULL){
					printf("elem is: %d\n", temp->elem);
					temp = temp->next;
				}
			}

			print();
			return 0;
}
