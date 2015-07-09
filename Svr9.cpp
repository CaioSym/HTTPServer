/*
 * Svr9: Monothreaded Http Server attempt
 *
 * Caio Freitas Sym, 2013
 * 
 */

//Include Files.
//Custom headers
#include "httpHeaders.h" 

//C++ headers 
#include <string>
#include <iostream>
#include <sstream>

//C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_THREADS     3

using namespace std;

extern "C++" {	
	int yyparse(void);
}

//Parser input redirection function
//@overwrites YY_INPUT()
int readInputForLexer( char *buffer, int *numBytesRead, int maxBytesToRead );
void flush();

void* processConnection (void *socket);

string globalInputText;		//String buffer for http parser
int globalReadOffset;		//Where the parser is in the buffer

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_t threads[MAX_THREADS];
int nThreads = 0;

//HTTP Header 
requestHeader req;

/*
 * Main
 */
main (int argc, char **argv) {
	unsigned short port;	   	//Port server binds to
	struct sockaddr_in client; //Client address information
	struct sockaddr_in server; //Server address information
	int s;					 		//Socket for accepting connections
	int ns;							//Socket connected to client
	int namelen;			   	//Length of client name
	string ret;						//The return header
	//Usage Check
	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}
	
	//Get port number
	port = (unsigned short) atoi(argv[1]);

	//Get a socket for accepting connections.
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket()");
		exit(2);
	}

	//Bind the socket to the server address.
	server.sin_family = AF_INET;
	server.sin_port   = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
	   perror("Bind()");
	   exit(3);
	}

	//Listen for connections. Specify the backlog as 5.
	if (listen(s, 5) != 0) {
		perror("Listen()");
		exit(4);
	}

	//Server operational, loop forever
	printf("\n%s accepting connections from HTTP clients.\n", argv[0]);
	fflush(stdout);
	while (true) {
		//Accept a connection.
		namelen = sizeof(client);
		if ((ns = accept(s, (struct sockaddr *)&client, (socklen_t*)&namelen)) == -1) {
			perror("Accept()");
			break;
		}
		if (nThreads < MAX_THREADS) {
      	if (pthread_create(&threads[nThreads++], NULL, processConnection, (void *)ns)) {
         	printf("\n ERROR: Failed to create thread\n");
         	exit(1);
      	}
			cout << "Created: " << nThreads << endl;
		}
		else {
			ret = req.mountResponseHeader(503, "", 0, 0); //Send request timeout header
		  	write(ns, ret.data(), ret.size());
		  	close(ns);
		}
	}
	//Server error
	close(s);
	printf("Server ended on error\n");
	exit(6);
}

void* processConnection (void *socket) {
	//Connection established
	char buf[1024];				//Buffer for sending and receiving data
	char sReader;					//Buffer for sending and receiving one char at a time 
	
	int ns = (int)socket;
	
	int msglen;						//Length of the received message
	
	fd_set fdset;					//File descriptor monitored for timeout
  	struct timeval timeout;		//Time remaining for timeout
	
	ostringstream strbldr;		//Used to build strings
	string ret;						//The return header
	
	bool lineFeed, doubleLineFeed; //Flags for \n
	bool keepAlive;
	
	do {
		strbldr.str("");
		lineFeed = false;
		doubleLineFeed = false;

		do {
			//Set timeout time
			timeout.tv_sec = 5;
  			timeout.tv_usec = 0;
  			
			//Set ns file descriptor
			FD_ZERO(&fdset);

			//Set ns file descriptor
			FD_SET(ns, &fdset);
			//cout << timeout.tv_sec << endl;
			// Wait for activity or die
		  	if ( select(ns + 1, &fdset, (fd_set *)0, (fd_set *)0, &timeout) == 0) {
				ret = req.mountResponseHeader(408, "", 0, 0); //Send request timeout header
		  		write(ns, ret.data(), ret.size());
				cout << "hex\n";
				break;
				}
			 	
			//Read from socket
		  	if ((msglen = read(ns, &sReader, 1)) == -1) {
				perror("Read()");
			}
			
			//Add to the builder
			strbldr << sReader;
			
			switch (sReader) {
				case '\n':
					if (lineFeed) doubleLineFeed = true;
					else lineFeed = true;
					break;
				case '\r':
					break;
				default:
					lineFeed = false;
					break;
			}
		} while (!doubleLineFeed);
		if (!doubleLineFeed) break;
		pthread_mutex_lock(&mutex); //Enter Crit
		globalInputText.clear();
		globalReadOffset = 0;
		//Build the header string and flush the builder
		globalInputText = strbldr.str();
		strbldr.str("");
		
		//Read request body
		while ((msglen = recv(ns, buf, 1024, MSG_DONTWAIT)) > 0) { 
				strbldr << buf;
		}
		
		//ClearBuffer and req
		flush();
		req.refresh();
		cout << req.parameters.size() << endl;
		//Parse, and process
		if (yyparse() >= 0) {
			switch(req.op) {
				case M_GET: 	 ret = req.processGet(true);	break;
//				case M_HEAD: 	 ret = req.processHead();		break;
				case M_OPTIONS: ret = req.processOptions();	break;
//				case M_TRACE: 	 ret = req.processTrace();		break;
//				case M_POST: 	 ret = req.processPost();		break;
//				case M_PUT: 	 ret = req.processPut();		break;
//				case M_DELETE:  ret = req.processDelete();	break;
//				case M_VERSION: ret = req.processVersion();	break;
				default: ret = req.mountResponseHeader(405, "", 0, 0); //Send method not allowed header
				 			break;
			}
			write(ns, ret.data(), ret.length());
		}
		cout << req.parameters.size() << endl;
		keepAlive = req.isKeepAlive();
		cout << req.parameters.size() << endl;
		pthread_mutex_unlock(&mutex);
	} while (keepAlive); //If keep-alive was passed, stay open
 	cout << "here\n";
	//cout << "outloop\n";
 	//Close the connection
	close(ns);
	nThreads--;
	return NULL;
}




int readInputForLexer( char *buffer, int *numBytesRead, int maxBytesToRead ) {
	int numBytesToRead = maxBytesToRead;
	int bytesRemaining = globalInputText.length() - globalReadOffset;
	int i;
	if ( numBytesToRead > bytesRemaining ) { numBytesToRead = bytesRemaining; }
	for ( i = 0; i < numBytesToRead; i++ ) {
			buffer[i] = globalInputText[globalReadOffset+i];
	}
	*numBytesRead = numBytesToRead;
	return 0;
}
