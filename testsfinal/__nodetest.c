struct s_insert16 {
struct node6 * temp;
struct node6 * head;
};

struct node6{

		struct node6* next;
		int elem;
	};

void insert16( struct s_insert16*);


#include<stdio.h>

int main(){
	

	
	struct node6 * head = NULL;
	struct node6 * temp;
		

	
struct s_insert16 sinsert16;
sinsert16.temp = &temp;
sinsert16.head = &head;


insert16(&sinsert16);	
}


void insert16( struct s_insert16* __s ){

		if((*(__s->head)) != NULL){
			(*(__s->temp)) = (*(__s->head));
			while((*(__s->temp))->next != NULL){
				(*(__s->temp)) = (*(__s->temp))->next;	
			}	
		}
		else{

		}	
	}

