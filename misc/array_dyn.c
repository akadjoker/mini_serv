#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>


typedef struct
{
		int socket;
		int id;
		char *msg;
}Client;


typedef struct 
{
	Client **data;
	int count;
	int size;
}Array;



void init_array(Array *array, int size)
{
	array->data = (Client**) malloc(size * sizeof(Client*));
	array->size=size;
	array->count=0;
}

void free_array(Array *array)
{
	for (int i=0;i<array->count;i++)
	{
		if (array->data[i]->msg!=NULL)
		{
			free(array->data[i]->msg);
		}
		free(array->data[i]);
		
	}
	free(array->data);
}

Client* array_add(Array *a, const char* msg, int id,int socket)
{
	if (a->count==a->size)
	{
		int  newSize = a->size *2;
		a->size = newSize;
		Client **array =(Client**) realloc(a->data, newSize *  sizeof(Client*));
		a->data=array;
	}
	Client* client;
	client=(Client *)malloc(sizeof(Client));
	client->id=id;
	client->socket=socket;
	client->msg = strdup(msg);
	a->data[a->count]=client;
	a->count++;
	return client;
}

int array_get_index(Array* a,int id)
{

	for (int i=0;i<a->count;i++)
	{
		if (a->data[i]->id==id)
			return i;
	}
	return -1;
}

void array_remove(Array *a,int id)
{
	int index = array_get_index(a, id);
	if (index==-1)
	{
		return;
	}
	free(a->data[index]->msg);
	free(a->data[index]);
	for (int i=index;i<a->count-1;i++)
	{
		a->data[i]=a->data[i+1];
	}
	a->count--;
}


int main()
{
	Array array;
	init_array(&array,5);

	array_add(&array,"1",1,0);
	array_add(&array,"2",2,0);
	array_add(&array,"3",3,0);
	array_add(&array,"4",4,0);
	array_add(&array,"5",5,0);
	for (int i=0;i<1000;i++)
	{
	array_add(&array,"6",6,0);
	array_add(&array,"7",7,0);
	}

	array_remove(&array,5);


	array_add(&array,"8",8,0);
	array_add(&array,"9",9,0);
	array_add(&array,"10",10,0);

	for (int i=0;i<array.count;i++)
	{
		printf("[%d] %d %d %s \n",i, array.data[i]->id,array.data[i]->socket,array.data[i]->msg);
	}
	

	free_array(&array);
	return 0;
}