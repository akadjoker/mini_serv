#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>


typedef struct Client
{
		int socket;
		int id;
		char *msg;
		struct Client* next;
}Client;

Client *head=NULL;

Client *createClient(int id,int socket)
{
	Client* client=(Client*) malloc(1* sizeof(Client));	
	client->id=id;
	client->socket=socket;
//	char string[20]; 
    // sprintf(string, "%d", socket);

	// client->msg = strdup(string);
	client->msg=NULL;
	client->next=NULL;

	
	if (head==NULL)
	{
		head=client;
		return client;
	}

	client->next= head;
	head   =  client;
	return client;
}

int reset_client(int id)
{
	Client* current = head;
	while(current!=NULL)
	{
		if (current->id==id)
		{
			current->id=-1;
			current->socket=0;
			if (current->msg!=NULL)
			{
				free(current->msg);
				current->msg=NULL;
			}
			return 1;
		}
		current = current->next;
		
	}
	return 0;
}

void clear_clients()
{
	Client* current = head;
	while(current!=NULL)
	{
		Client* tmp = current;
		current = current->next;
		if (tmp->msg!=NULL)
		{
			free(tmp->msg);
		}
		free(tmp);
	}
}

int main()
{

	

for (int i=0;i<=60000;i++)
{
	createClient(i,i+4);
}

reset_client(5);
reset_client(6);
reset_client(7);



	Client *tmp = head;
	int i=0;
	while(tmp!=NULL)
	{	
		if (tmp->socket>0)
			printf("[%d] %d %d \n",i, tmp->id,tmp->socket);
		tmp = tmp->next;
		i++;
	}
	

	
	clear_client();

	
	return 0;
}