#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h> 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>



typedef struct Client
{
    int id;
    int fd;
    char *msg;
    struct Client *next;
}Client;





int server=0;
int total_clients=0;
int max_fd=0;
int clients_id=0;
fd_set master;
fd_set write_fd;
fd_set fd_list;

Client *head=NULL;
Client *tail=NULL;

Client *create_client(int fd)
{
        Client *client = (Client*) malloc(sizeof(Client)) ;
        client->fd=fd;
        client->id=clients_id;
        client->msg=NULL;
        client->next=NULL;
        if (head==NULL)
        {
            head=client;
            tail=client;
        } else
        {
            tail->next = client;
            tail = client;
        }
        clients_id++;
        total_clients++;
        return client;
}



Client *get_client(int fd)
{
    
    if (head!=NULL)
    {
        if (head->fd==fd)
        {
            return head;
        }
    }

    Client *current=head;
    while ( current!=NULL) 
    {
        if (current->fd==fd)
            return current;
        current=current->next;
    }
    
    return NULL;
}

int remove_client(int fd)
{
    int id=-1;
    if (head!=NULL)
    {
        if (head->fd==fd)
        {
            Client *tmp = head->next;
            id = head->id;
            printf(" remove head %d \n",head->fd);

            if (head->msg!=NULL)
                free(head->msg);
            free(head);
            head = tmp;
            total_clients--;
            return id;
        }
    }

    Client *current=head;
    Client *prev=NULL;
    while ( (current!=NULL) && current->fd!=fd)
    {
        prev=current;
        current=current->next;
    }
    if (!current)
    {
        printf("WRNING nenhum socket com id  %d  \n",fd);
        return id;
    }
    
    
    
    total_clients--;
    
    if (current==tail)
    {
        printf("WRNING remove  %d  pref %d last \n",current->fd,prev->fd);
        prev->next  = NULL;
        tail=prev;        
    } else
    {
        prev->next  = current->next;
        printf(" remove  %d  pref %d \n",current->fd,prev->fd);

    }
    id = current->id;
    if (current->msg!=NULL)
        free(current->msg);
    free(current);
    current=NULL;
    return id;

}

void clean_all()
{
    if(server>0)
        close(server);


    if (!head)
    {
        return;
    }
    
        Client *current=head;
        while(current!=NULL)
        {
                Client *tmp=current;
                current = current->next;
                if (tmp!=NULL)
                {
                    printf(" delete %d \n",tmp->fd);
                    if (tmp->fd>0)
                    {
                       close(tmp->fd);
                    }
                    if (tmp->msg!=NULL)
                        free(tmp->msg);
                    total_clients--;
                    free(tmp);
                    tmp=NULL;
                    
                }
        }
        head=NULL;
        tail=head;
}

   
        

void send_msg(const char *msg, int ignore)
{
    
        Client *current=head;
        while(current!=NULL)
        {       
                if (current->fd>0 && current->fd!=ignore)
                {
                    send(current->fd,msg,strlen(msg),0);
                }

                current = current->next;
        }
   
}

   
   

void erro_msg(char *msg)
{
  write(2, msg, strlen(msg));
  clean_all();
  exit(1);
}



void handlesginal(int sig)
{
    (void)sig;
	erro_msg("\nAborting...\n");
    
}




int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int main(int argc, char **argv) 
{

 if (argc != 2)
 {
  erro_msg("Wrong number of arguments\n");
 }

  	signal(SIGINT, handlesginal);
	signal(SIGQUIT, handlesginal);
	signal(SIGTERM, handlesginal);



 struct sockaddr_in servaddr, cli; 
  socklen_t len;
 


// socket create and verification 
server = socket(AF_INET, SOCK_STREAM, 0); 
if (server == -1)
    erro_msg("Fatal error\n");

bzero(&servaddr, sizeof(servaddr)); 

servaddr.sin_family = AF_INET; 
servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
servaddr.sin_port = htons(atoi(argv[1])); 


if ((bind(server, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    erro_msg("Fatal error\n");
if (listen(server, 10) != 0)
    erro_msg("Fatal error\n");

clients_id=0;
FD_ZERO(&fd_list);
FD_SET(server,&fd_list);
max_fd =server;

int sockets[(FD_SETSIZE*5)+1]={-1};

while(1)
{

    max_fd = server;
    sockets[0]=server;
    // Client *current= head;
    // while(current)
    // {
    //     if(current->fd>0)
    //     {
    //            max_fd= (current->fd>max_fd) ?current->fd: max_fd;
    //     }
    //     current =current->next;
    // }
    
   for (int i =0; i < FD_SETSIZE;i++)
   {
        //printf(" %d - %d \n",i,socekets[i]);
        if (sockets[i]>0)
        {
            if (sockets[i]>max_fd)
                max_fd=sockets[i];
        }
    }

    printf("process Max fd: %d - total: %d  IDS: %d \n",max_fd,total_clients,clients_id);

    master =   fd_list;
    if (select(max_fd + 1, &master, NULL, NULL, NULL)<=0)
    {
        erro_msg("Fatal error\n");
    }

     for (int i=0;i<=max_fd;i++)
     {
        if (FD_ISSET(i,&master))
        {

            if (i==server)
            {
                len = sizeof(cli);
                int socket = accept(server, (struct sockaddr *)&cli, &len);
                if (socket>0)
                {
                        char msg[50] = {'\0'};
                        sprintf(msg, "server: client %d just arrived\n", clients_id);
                        send_msg(msg, socket);
                        sockets[socket]=socket;
                        create_client(socket);
                        FD_SET(socket,&fd_list);
                }
            } else
            {

                char buffer[4086]={'\0'};
                int bytes = recv(sockets[i],buffer,sizeof(buffer)-1,MSG_DONTWAIT);
                if (bytes<=0)
                {
                    int id = remove_client(i);


                    FD_CLR(i,&fd_list);
                    close(i);

                    char msg[50] = {'\0'};
                    sprintf(msg, "server: client %d just left\n", id);
                    send_msg(msg, i);


                    
                    sockets[i]=-1;
                } else
                {
                    Client *client = get_client(sockets[i]);
                    if (client)
                    {
                        buffer[bytes]='\0';
                        char *text=NULL;
                        client->msg= str_join(client->msg, buffer);
                        while(extract_message(&client->msg,&text))
                        {
                            char msg[50] = {'\0'};
                            sprintf(msg, "client %d: ", client->id);
                            send_msg(msg, client->fd);
                            send_msg(text, client->fd);
                            if(text!=NULL)
                                free(text);
                        }
                    }
                }
            }
        }
     }

            
        
        
        //paulo tavares
    


}



clean_all();

return 0;
}