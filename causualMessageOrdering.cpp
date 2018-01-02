//============================================================================
// Author      : Deepika Rajani
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include<iostream>
#include <fstream>
#include <sstream>
#include <csignal>
#include <ctime>
#include <sstream>
#include <vector>
#define MAXELEMENTSIZE 22000
using namespace std;

void * clientThread(void *arg);
void * serverThread(void *arg);
fstream file;
void * listenOnPort(void *arg);
int noOfProcesses;
int avgClock = 0;
int portno = 0;
pthread_mutex_t mutex;
void error(const char *msg) {
	perror(msg);
	exit(1);
}
int selfindex;
int processcount;
struct MulticastMsg{
 
	
	int index;
	int vector[MAXELEMENTSIZE];

}msg[2];// objects for reading and sending data as struct
vector<MulticastMsg> buffer;//vector to store buffered messages


char * machineName;

/* Converts string to integer
*/
int convertStringToInt(string a) {
	stringstream s(a);
	int b = 0;
	s >> b;
	return b;
}

/* Converts integer to string
*/
string convertIntToString(int a) {
	stringstream s2;
	s2 << a;
	return s2.str();
}

/*Prints the array of sequence number maintained for each process*/
void printVector(int vector[]){
	cout << "  vector: [";
	for(int q=0;q<=processcount;q++){
		cout <<vector[q];
		if(q!=processcount){
			cout <<",";
		}
	}
	cout <<"]";
	
}
/*Checks for causual voilation, returns true if their is causual voilation else false*/
bool checkCausualVoilation(MulticastMsg local,MulticastMsg received,int processNumber){
	if(local.vector[processNumber]+1 == received.vector[processNumber]){
		for(int i=0;i<processcount;i++){
			if(processNumber!=i){
	
				if(received.vector[i]<=local.vector[i]){
				
				}else{
					return true;
				}
			}
		}
	}else{
		return true;
	}
	return false;
}

/*Checks for causual ordering for the buffered messages*/
bool checkBufferCausual(MulticastMsg local,MulticastMsg rec){
	
	if(local.vector[rec.index]+1 == rec.vector[rec.index]){
		for(int i=0;i<processcount;i++){
			if(rec.index!=i){
	
				if(rec.vector[i]<=local.vector[i]){
				
				}else{
					return true;
				}
			}
		}
	}else{
		return true;
	}
	return false;
}

bool checkForExtraMessage(int vector[]){
	for(int i=0;i<=processcount;i++){
		if(vector[i]!=0){
			return false;
		}
	}
	return true;
	
}

/**
 * readwrite thread method is called for each client
 */
void * readwrite(void *arg) {
	int newsockfd = *((int *) arg);
	int n=0;
	while(n=read(newsockfd,&msg[0], sizeof(msg[0]))){
		string s;
	if (n < 0) {
		perror("ERROR reading from socket");
	}

	if(!checkForExtraMessage(msg[0].vector)){
		if(!checkCausualVoilation(msg[1],msg[0],msg[0].index)){//if true, message is buffered else delivered
			msg[1].vector[msg[0].index]=msg[0].vector[msg[0].index];//when message from Projess Pj is delivered, the counter for process Pj at process Pi is increased by 1
			cout <<"P"<<selfindex<< "---DELIVERED---";
			printVector(msg[0].vector);
			cout<<" from P : " << msg[0].index <<" updated Local";
			printVector(msg[1].vector);
			cout <<endl;
			//sleep(1);
			//buffered messages are checked for delivery to the application
			vector<MulticastMsg>::iterator it;//vector iterator
			for(it=buffer.begin();it!=buffer.end();){
				if(checkBufferCausual(msg[1],*it)){//causuality condition is checked
					//cout <<"###"<<endl;
					it++;
				}else{
					cout <<"P"<<selfindex<< "----BUFFERED DELIVERED----- " <<" from P : " << (*it).index;
					printVector((*it).vector);
					cout<<endl;
					msg[1].vector[(*it).index]+=1;
					//cout <<"**"<<endl;
					it=buffer.erase(it);
					it=buffer.begin();
					//cout <<"^^"<<endl;					
				}
			}
									
					
		}else{
			buffer.push_back(msg[0]);//when a message is buffered, it's stored in a structure of vector
			cout <<"P"<<selfindex<< "---BUFFERED-----";	
			printVector(msg[0].vector);
			cout<<" from P : " << msg[0].index <<" updated Local :";
			printVector(msg[1].vector);
			cout <<endl;
		}
	}else{
		cout <<""<<endl;
	}
	}
}



void * clientThread(void *arg) {
	string str1;
	int i = 0;
	int sockfd[MAXELEMENTSIZE], n;
	char buffer[256];
	int logicalClock[MAXELEMENTSIZE];
	

	string str;
	ifstream file("InformationAboutProcess.txt");//this file contains information about the port number of the processes participating in distributed system
	i = 0;	
	cout <<"Establishing Connections to other processses..." << endl;
	while (getline(file, str1)) {
		
		int x = convertStringToInt(str1);		
		
		if(x != portno){
			struct sockaddr_in serv_addr;
			struct hostent *server;
			sockfd[i] = socket(AF_INET, SOCK_STREAM, 0); //client socket is created
			if (sockfd[i] < 0)
				cout << "ERROR opening socket";
			server = gethostbyname(machineName); //server address
			if (server == NULL) {
				perror("ERROR, no such host\n");
				exit(0);
			}
			//cout <<"port number to connect "<< x<<endl;
			memset((char *) &serv_addr, 0, sizeof(&serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy((char *) server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
			serv_addr.sin_port = htons(x);
			int n = -1;
			
			while (n < 0) {
				n = connect(sockfd[i], (struct sockaddr *) &serv_addr,
						sizeof(serv_addr));
			}
			i++;
		}else{
			
			selfindex=i;
			msg[1].index=selfindex;//this is used to give unique number to each process
			
		}		
	}
	cout <<"Connections established"<< endl<<endl;
	cout <<"This is process P"<<selfindex <<" starting multicast" << endl << endl;

	int k=0;
	
	processcount=i;
	//cout <<"processcount" <<processcount<<endl;
	sleep(4);
	
	while(k<5){//Multicast message is sent k times to processes in the multicast group
		//sleep(3);

		k++;
		msg[1].vector[selfindex]=msg[1].vector[selfindex]+1;
		cout << "P"<< selfindex <<" sent multicast";
		printVector(msg[1].vector);
		cout <<endl;
		for (int j = 0; j < i; j++) {	
				
			n = write(sockfd[j], &msg[1], sizeof(msg[1]));
			if (n < 0)
				cout << "Instruction cannot be written" << endl;		
		}
	}
	for (int j = 0; j < i; j++) {
		close(sockfd[j]);
	}
	
}


void * serverThread(void *arg) {
	int portno = *((int *) arg);
	int sockfd, newsockfd[MAXELEMENTSIZE], n, i = 0, no_of_thread = 0;
	socklen_t clilen;
	char buffer[MAXELEMENTSIZE];
	pthread_t t2[MAXELEMENTSIZE];

	struct sockaddr_in serv_addr, cli_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //bind the socket
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	while (1) {
		newsockfd[no_of_thread] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		pthread_create(&t2[no_of_thread], NULL, &readwrite, &newsockfd[no_of_thread]); //pthread created for each processes that is connected
		no_of_thread++;

	}
	for(int p=0;p<no_of_thread;p++)
	close(newsockfd[p]);
	close(sockfd);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port or file name provided\n");
		exit(1);
	}
	pthread_mutex_init(&mutex,0);
	portno = atoi(argv[1]);
	machineName = (argv[2]);
	pthread_t t1, t2;
	pthread_create(&t1, NULL, &serverThread, (void *) &portno);//thread to accept connections
	pthread_create(&t2, NULL, &clientThread, (void *) &portno);//thread to connect to other processes
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	cout.flush();

	return 0;
}


