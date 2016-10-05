#include <stdlib.h>
#include <openssl/ssl.h>
#include <string.h>
#include <stdio.h>
#include <openssl/pem.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define BUFFER_SIZE 1024

using namespace std;

int main(void) {
    char buffer[BUFFER_SIZE];
    int readsize;
    
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server = gethostbyname("fit.cvut.cz");

    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy( (char *)&servaddr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

    servaddr.sin_port = htons(443); // port

    FILE * fWeb = fopen("odkazy.html","w+b");
    const char * request = "GET /student/odkazy HTTP/1.0\r\nHost: www.fit.cvut.cz\r\n\r\n";

    if ( 0 != connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) )
    {
        cout << "Chyba pripojeni" << endl;
        return 0;
    }
    cout << "Pripojeno" << endl;

    SSL_library_init();
    SSL_CTX * ctx = SSL_CTX_new(SSLv23_client_method());
    SSL * ssl = SSL_new(ctx);
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1);
    SSL_set_fd(ssl, sockfd);
    SSL_connect(ssl);
    SSL_write(ssl,request,strlen(request));

    while( (readsize = SSL_read(ssl, buffer, BUFFER_SIZE)) != 0 ) {
        fwrite(buffer, 1, readsize, fWeb);
    }
    
    X509 * cert = SSL_get_peer_certificate(ssl);
    FILE * fCert = fopen("certificate.pem", "w+b");
    PEM_write_X509(fCert, cert);
    
    BIO *bio = BIO_new_fp(stdout, BIO_NOCLOSE);
    X509_print(bio, cert);//STD OUT the details

    fclose(fCert);
    cout << "Certifikat byl vytvoren" << endl;

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    free(cert);
    fclose(fWeb);

    return 0;
}