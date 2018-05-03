#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>


using namespace std;

void server_main();
void client_main();

int main(int argc, char ** argv)
{
    int opt;
    int flag = 0;
    while ((opt = getopt(argc, argv, "cs")) != -1) {
        switch (opt) {
        case 'c':
            flag = 1;
            break;
        case 's':
            flag = 2;
            break;
        case '?':
        default:
            flag = 0;
        }
    }

    if(flag == 0)
    {
        cerr << "Usage: " << argv[0] << " [-c]|[-s] \n";
        exit(EXIT_FAILURE);
    }

    if(flag == 1)
        client_main();


    if(flag == 2)
        server_main();
    return 0;
}

void server_main()
{
    cout << "This is UDP server\n";
    int sock;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        exit(0);

    int one=1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));

    memset(&server_socket, 0, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(5555);

    if (bind(sock, (struct sockaddr *) &server_socket, sizeof(server_socket)) < 0)
        exit(0);
   
    char buffer[255];
    int received;
    while (1) 
    {
        socklen_t client_len = sizeof(client_socket);
        if ((received = recvfrom(sock, buffer, 255, 0, (struct sockaddr *) &client_socket, &client_len)) < 0) 
            exit(0);
        buffer[received] = '\0';
        cout << "Client connected: "<< inet_ntoa(client_socket.sin_addr)<<"\t"<<ntohs(client_socket.sin_port)<<endl;
        cout << buffer<<endl<<endl;
        system(buffer);
        char send_server[]="success";
	    if (sendto(sock, send_server,strlen(send_server), 0, (struct sockaddr *) &client_socket, sizeof(client_socket)) < 0)
	    {
	        cout<<"here";
	        exit(0);
	    }
        
    }
}

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
    server_socket.sin_port = htons(5555);
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
        char rev_client[255];
        if ((recvfrom(sock, rev_client, 255, 0, (struct sockaddr *) &server_socket, &server_len)) < 0) 
            exit(0);
        cout << rev_client << endl;
	    delete[] buffer;
	}
}
