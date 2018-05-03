#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
using namespace std;


void client_main()
{
    cout << "This is UDP client\n";
    int sock;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        exit(0);

    int one=1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));

    memset(&server_socket, 0, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = inet_addr("localhost");
    server_socket.sin_port = htons(8643);
    while(1) {
	    string x;
	    getline(cin,x);
	    if("quit" == x) break;
	    char *buffer = new char[x.size()];
	    for(int i = 0; i < x.size(); i++) buffer[i] = x [i];
	    if (sendto(sock, buffer,x.size(), 0, (struct sockaddr *) &server_socket, sizeof(server_socket)) < 0)
	    {
	        cout<<"here";
	        exit(0);
	    }
	    sleep(1);
        socklen_t server_len = sizeof(server_socket);
        char *rev_client = new char[255];
        memset(rev_client, 0, sizeof(rev_client));
        if ((recvfrom(sock, rev_client, 255, 0, (struct sockaddr *) &server_socket, &server_len)) < 0) 
            exit(0);
        cout << rev_client << endl << endl;
	    delete[] buffer;
	}
}


int main(int argc, char* argv[]) {
	client_main();
    return 0;
}
    
