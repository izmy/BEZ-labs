#include <stdlib.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <openssl/pem.h>
#include <openssl/err.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define BUFFER_SIZE 1024

using namespace std;

int main(int argc, char * argv[]) {
  const char * filename = argv[1];
  const char * keyFile = argv[2];
  const char * outFilename = argv[3];
	
  if ( argc != 4 )
  {
    fprintf(stderr, "Chybi parametry, zadejte: ./decKey soubor_s_daty soubor_se_soukromym_klicem vystupni_soubor \n" );
    return errno;
  }
  unsigned char st[BUFFER_SIZE];  // sifrovany text
  unsigned char * key;  // klic pro sifrovani
  unsigned char iv[EVP_MAX_IV_LENGTH];  // inicializacni vektor
  unsigned char readBuffer[BUFFER_SIZE];

  int stLength = 0;
  int tmpLength = 0;
  int readlen = 0;
  int keyLength = 0;
  EVP_CIPHER_CTX ctx;
  int nid = 0;
  unsigned long errn = 0;

  EVP_PKEY * privKey = NULL;
  FILE * fCipherIn = fopen(filename, "rb");
  if ( fCipherIn == 0 ) {
    cout << "Soubor " << filename << " nelze otevrit." << endl;
    return 0;
  }
  FILE * fPlainOut = fopen(outFilename, "w+b");
  
  FILE * fPrivKey = fopen(keyFile, "rb");
  if ( fPrivKey == 0 ) {
    cout << "Soubor " << keyFile << " nelze otevrit." << endl;
    return 0;
  }
 
  privKey = PEM_read_PrivateKey(fPrivKey, NULL, NULL, NULL);
  fclose(fPrivKey);

  fread(&nid,sizeof(int),1,fCipherIn);
  fread(iv,1,EVP_MAX_IV_LENGTH,fCipherIn);
  fread(&keyLength,sizeof(int),1,fCipherIn);
  key = (unsigned char*) malloc(keyLength);
  fread(key,1,keyLength,fCipherIn);

  //cout << "NID: " << nid << ", keyLength: " << keyLength << endl;
  
  OpenSSL_add_all_algorithms();
  OpenSSL_add_all_ciphers();

  EVP_OpenInit(&ctx, EVP_get_cipherbynid(nid), key, keyLength, iv, privKey);
  
  errn = ERR_get_error();
  char err_str[256];
  int check = 0;
  while( errn != 0 ) {
    ERR_error_string_n(errn, err_str, 256);
    cout << "Chyba " << err_str << endl;
    errn = ERR_get_error();
    check = 1;
  }

  if ( check == 1 ) {
    cout << "Pri desifrovani nastala chyba. Ujistete se, ze jste vybrali spravny desifrovaci soubor a privatni klic." << endl;
    fclose(fCipherIn);
    fclose(fPlainOut);
    if( remove( outFilename ) != 0 ) {
      perror( "Error deleting file" );
      cout << "Chyba pri mazani souboru " << outFilename << endl;
    } 
    else {
      cout << "Soubor " << outFilename << " byl smazan." << endl;
      return 0;
    }
    free(key);
    return 0;
  }

  while( (readlen = fread(readBuffer, 1, BUFFER_SIZE, fCipherIn)) != 0 )
  {
	  EVP_OpenUpdate(&ctx, st,&stLength, readBuffer, readlen);
	  fwrite(st,1,stLength,fPlainOut);
  }
  EVP_OpenFinal(&ctx, st, &tmpLength);

  fwrite(st,1,tmpLength,fPlainOut);
  fclose(fCipherIn);
  fclose(fPlainOut);
  free(key);

  cout << "Soubor uspesne rozsifrovan jako " << outFilename << endl;
  
  return 0;
}