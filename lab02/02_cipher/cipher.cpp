#include <iostream>
#include <stdlib.h>
#include <openssl/evp.h>
#include <string.h>
#include <fstream>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" 

using namespace std;

char hexToAscii(char first, char second)
{
  char hex[5], *stop;
  hex[0] = '0';
  hex[1] = 'x';
  hex[2] = first;
  hex[3] = second;
  hex[4] = 0;
  return strtol(hex, &stop, 16);
}

int main(int argc, char *argv[])
{
  unsigned char ot[1024] = "Tajemna tricetiznakova zprava.";  // open text
  unsigned char st[1024];  // sifrovany text
  unsigned char decryptOT[1024] = ""; 
  unsigned char key[EVP_MAX_KEY_LENGTH] = "enjoythesilence";  // klic pro sifrovani
  unsigned char iv[EVP_MAX_IV_LENGTH] = "";  // inicializacni vektor
  EVP_CIPHER_CTX ctx; // struktura pro kontext

  int otLength = strlen((const char*) ot);
  int stLength = 0;
  int tmpLength = 0;

  cout << "Otevreny text: " << ot << endl;
 
  /* Sifrovani */
  EVP_EncryptInit(&ctx, EVP_rc4(), key, iv);  // nastaveni kontextu pro sifrovani
  EVP_EncryptUpdate(&ctx,  st, &stLength, ot, otLength);  // sifrovani ot
  EVP_EncryptFinal(&ctx, &st[stLength], &tmpLength);  // ziskani sifrovaneho textu z kontextu
  stLength += tmpLength;
 
  //printf ("Zasifrovano %d znaku.\n", stLength);
 
  /* Desifrovani */
  EVP_DecryptInit(&ctx, EVP_rc4(), key, iv);  // nastaveni kontextu pro desifrovani
  EVP_DecryptUpdate(&ctx, decryptOT, &otLength,  st, stLength);  // desifrovani st
  EVP_DecryptFinal(&ctx, &decryptOT[otLength], &tmpLength);  // ziskani desifrovaneho textu z kontextu
  otLength += tmpLength;
 
  /* Vypsani zasifrovaneho a rozsifrovaneho textu. */
  cout << "Sifrovy text: " << st << endl;
  cout << "Rozsifrovany text: " << decryptOT << endl;

  //---------------------------------
  cout << "------------" << endl;

  ifstream inFile;
  string line;
  int pozice;
  char puvodniOT[128];

  // znamy text - "abcdefghijklmnopqrstuvwxyz0123"
  string otvZnamy = "abcdefghijklmnopqrstuvwxyz0123";  // open text

  // zasifrovany znamy text - "abcdefghijklmnopqrstuvwxyz0123"
  string siftZnamy;
  string siftZnamyHex;
  inFile.open("siftZnamy.bin");
  if ( inFile.is_open() ) {
    getline (inFile,siftZnamyHex);
    inFile.close();
  }

  pozice = 0;
  for ( int i = 0; i < siftZnamyHex.length(); i += 2 ) {
    siftZnamy[pozice] = hexToAscii(siftZnamyHex[i], siftZnamyHex[i + 1]);
    pozice++;
  }

  // zasifrovany neznamy text
  string siftTajny;
  string siftTajnyHex;
  inFile.clear();
  inFile.open("siftTajny.bin");
  if ( inFile.is_open() ) {
    getline (inFile,siftTajnyHex);
    inFile.close();
  }

  pozice = 0;
  for ( int i = 0; i < siftTajnyHex.length(); i += 2 ) {
    siftTajny[pozice] = hexToAscii(siftTajnyHex[i], siftTajnyHex[i + 1]);
    pozice++;
  }

  for (int i = 0; i < otvZnamy.length(); i++) puvodniOT[i] = otvZnamy[i]^siftZnamy[i]^siftTajny[i];

  cout << "Puvodni OT: " << puvodniOT << endl;

  return 0;
 }