import socket
import time
import signal
import sys
SERVER = 'localhost'
PORT = 1478
FILES = False
TESTINOU = False
MAXCLIENST = 15


def send_msg(sock, msg):
	sock.send(f"{msg}".encode())
	print(msg)
	time.sleep(1)

def send_file(sock, file_path):
    with open(file_path, 'r') as file:
        file_content = file.read()
        send_msg(sock, file_content)
        
client1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client1.connect((SERVER, PORT))

send_msg(client1,f"Ceci est un message\n")
send_msg(client1,f"This is a text without newline at the end")
send_msg(client1,f"now with end \n")

client1.close()
client1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client1.connect((SERVER, PORT))

send_msg(client1,f"And here is a text with several\nreturns\na\nthe\nline\n")
client1.close()
client1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client1.connect((SERVER, PORT))

send_msg(client1,f"This is a test of  in your program here:\04\n")
send_msg(client1,f"message no newline")

client1.close()
client1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client1.connect((SERVER, PORT))

send_msg(client1,f"")
send_msg(client1,f"\n\nhello\n\nworld\n\n")
send_msg(client1,f"hello\t\tbonjour\n")

client1.close()
client1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client1.connect((SERVER, PORT))


send_msg(client1,f"Ceci est un message\n")
send_msg(client1,f"This is a text without backline at the end")
send_msg(client1,f"This is a test  in your program here:\04\n")
send_msg(client1,f"message")
send_msg(client1,f"")
send_msg(client1,f"\n\nhello\n\nworld\n\n")
send_msg(client1,f"hello\t\tbonjour\n")

if FILES:
	send_msg(client1,f"file null\n")
	send_file(client1,'./files/0.txt')	
	send_msg(client1,f"file 100\n")
	send_file(client1,'./files/100.txt')

	send_msg(client1,f"file 1000\n")
	send_file(client1,'./files/1000.txt')

	send_msg(client1,f"file 4096\n")
	send_file(client1,'./files/4096.txt')

	send_msg(client1,f"file 10000\n")
	send_file(client1,'./files/10000.txt')

clients = []
if TESTINOU:
	for i in range(0,MAXCLIENST):
		#time.sleep(1)	
		client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client.connect((SERVER, PORT))
		clients.append(client)
		send_msg(client,f"hello client {i} \n")

time.sleep(3)	

if TESTINOU:
	i =MAXCLIENST
	for client in clients:
		#time.sleep(1)	
		send_msg(client,f"byby client {i} \n")
		client.close()
		i-=1

		

send_msg(client1,f"✅\n")


