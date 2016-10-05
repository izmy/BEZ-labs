#include <iostream>
#include <fstream>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace std;

#define BUFFER_SIZE 256

int main() {
	struct sockaddr_in addressInfo;
	bzero(&addressInfo, sizeof (addressInfo));
	addressInfo.sin_family = AF_INET;
	addressInfo.sin_port = htons(443);
	addressInfo.sin_addr.s_addr = inet_addr("147.32.232.248");

	int socketHandler = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	connect(socketHandler, (struct sockaddr *) &addressInfo, sizeof (addressInfo));

	SSL_library_init();
	SSL_CTX *context = SSL_CTX_new(SSLv23_client_method());

	int stat = SSL_CTX_load_verify_locations(context, NULL, "/System/Library/OpenSSL/certs");
	if ( stat != 1 ) {
		cout << "SSL_CTX_load_verify_locations chyba" << endl;
	}

	SSL *ssl = SSL_new(context);
	SSL_set_fd(ssl, socketHandler);
	SSL_connect(ssl);

	string request = "GET /student/odkazy HTTP/1.0\r\n\r\n";
	SSL_write(ssl, request.c_str(), request.length());

	cout << "Pouzita sifra: " << SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)) << endl;
	cout << "Dostupne sifry:" << endl;
	int cipherIterator = 0;
	const char *cipher;
	while ((cipher = SSL_get_cipher_list(ssl, cipherIterator++)) != NULL) {
		cout << cipher << endl;
	}

	stat = SSL_get_verify_result(ssl);
	if ( stat == 0) {
		cout << "Overeni uspesne" << endl;
	} else {
		cout << "Chyba overeni" << endl;
	}

	string html;
	int length = 0;
	char buffer[BUFFER_SIZE];
	while ( (length = SSL_read(ssl, buffer, BUFFER_SIZE)) > 0 ) {
		html.append(string(buffer, length));
	}

	ofstream page("output.verification.html");
	page << html;
	page.close();

//	openssl s_client -host 147.32.232.248 -port 443 -showcerts
//	openssl x509 -purpose -in certificate.verification.pem

	PEM_write_X509(fopen("certificate.verification.pem", "w+"), SSL_get_peer_certificate(ssl));

	SSL_shutdown(ssl);
	SSL_free(ssl);
	SSL_CTX_free(context);

	close(socketHandler);

	return EXIT_SUCCESS;
}