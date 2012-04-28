
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


#include "caserver.h"
#include "client.h"


CaServer::CaServer()
{
	setup_port();
}

CaServer::~CaServer()
{
	close(listen_socket);
}

// Reads env variable CASERVER_PORT and set variable port
void CaServer::setup_port()
{
	char *v = getenv("CASERVER_PORT");
	if (v == NULL) {
		fprintf(stderr, "Environment variable CASERVER_PORT is not set\n");
		exit(-1);
	}

	if (1 != sscanf(v, "%i", &port) || port < 0 || port > 65535) {
		fprintf(stderr, "Environment variable CASERVER_PORT is not valid port number\n");
		exit(-1);
	}
}

void CaServer::init_listen_socket()
{
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == -1) {
		perror("socket");
		exit(-1);
	}

	struct sockaddr_in sockname;
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(port);
	sockname.sin_addr.s_addr = INADDR_ANY;
	if (bind(listen_socket, (struct sockaddr *)&sockname, sizeof(sockname)) < 0) {
		perror("bind");
		exit(-1);
	}
	if (listen(listen_socket, 1) < 0) {
		perror("listen");
		exit(-1);
	}
}

void CaServer::register_function(const std::string &name, const std::string &definition, CaPublicFn *fn)
{
	functions.push_back(CaPublicFunction(name, definition, fn));
}

void CaServer::run()
{
	CaDynamicPacker welcome_message(sizeof(int) + 1024, sizeof(int));
	welcome_message.pack_int(functions.size());

	for (size_t t = 0; t < functions.size(); t++) {
		welcome_message.pack_string(functions[t].get_name());
		welcome_message.pack_string(functions[t].get_definition());
	}

	int sz = welcome_message.get_size() - sizeof(int);
	memcpy(welcome_message.get_buffer(), &sz, sizeof(int));

	init_listen_socket();

	for(;;) {
		int client_socket = accept(listen_socket, NULL, NULL);

		if (client_socket < 0) {
			perror("accept");
			break;
		}

		send(client_socket, welcome_message.get_buffer(), welcome_message.get_size(), 0);

		CaClient client(*this, client_socket);
		client.run();
		close(client_socket);
	}

}