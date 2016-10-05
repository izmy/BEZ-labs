#include <stdio.h>
#include <string.h>
#include <iostream>
#include <openssl/evp.h>
#include <vector>
#include <algorithm>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" 
#define SIZE 7
#define MAX 268435456 // 2^28 = 268435456
using namespace std;
//------------------------------------------------------------------------------
struct smenka {
  int cena;
  unsigned char hash[SIZE];
};
//------------------------------------------------------------------------------
bool compare(const smenka& a, const smenka& b)
{  
  return memcmp(a.hash, b.hash, SIZE) < 0;
}
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  string text, hashF;
  smenka money;
  vector<smenka> seznam;
  int price = 0;

  string name = "hrachjar";
 
  char hashFunction[] = "sha256";  // zvolena hashovaci funkce ("sha1", "md5" ...)
  int length;  // vysledna delka hashe
  EVP_MD_CTX ctx;  // struktura kontextu
  const EVP_MD *type; // typ pouzite hashovaci funkce
  unsigned char hash[EVP_MAX_MD_SIZE]; // char pole pro hash - 64 bytu (max pro sha 512)
  OpenSSL_add_all_digests(); // Inicializace OpenSSL hash funkci
  type = EVP_get_digestbyname(hashFunction); // Zjisteni, jaka hashovaci funkce ma byt pouzita 
 
  // Pokud predchozi prirazeni vratilo -1, tak nebyla zadana spravne hashovaci funkce 
  if(!type)
  {
    printf("Sifra %s neexistuje.\n", hashFunction);
    exit(1);
  }

  for ( int i = 0; i <= MAX; i++ )
  {
    text = "V Praze dne 24.12.2086. Za tuto smenku zaplatim bez odporu " + to_string(price) + " Kc. " + name;
    const char *textChar = text.c_str();

    // Provedeni hashovani
    EVP_DigestInit(&ctx, type);  // nastaveni kontextu
    EVP_DigestUpdate(&ctx, textChar, strlen(textChar) );  // zahashuje text a ulozi do kontextu
    EVP_DigestFinal(&ctx, hash, (unsigned int *) &length);  // zjiskani hashe z kontextu

    money.cena = price;
    for( int j = 0; j < SIZE; j++ ) money.hash[j] = hash[j];

    if ( i % 1000000 == 0 ) cout << money.cena << endl;

    seznam.push_back(money);
    price++;

    if ( i == MAX )
    {
      // sort
      sort(seznam.begin(), seznam.end(), compare);

      // vypis
      for ( int j = 0; j <= MAX; j++ )
      {
        if ( j < MAX ) {
          if ( memcmp(seznam[j].hash, seznam[j + 1].hash, SIZE) == 0 )
          {
            for( int k = 0; k < SIZE; k++ ) sprintf ( &hashF[k * 2], "%02x", seznam[j].hash[k] );
            cout << "V Praze dne 24.12.2086. Za tuto smenku zaplatim bez odporu " << seznam[j].cena << " Kc. " << name << endl;
            cout << "V Praze dne 24.12.2086. Za tuto smenku zaplatim bez odporu " << seznam[j + 1].cena << " Kc. " << name << endl;
            cout << "Hash = " << hashF.c_str() << endl;
            return 0;
          } 
          if ( memcmp(seznam[j].hash, seznam[j + 1].hash, SIZE) != 0 && j == MAX - 1 )
          {
            cout << "Dalsi pokus od " << price << endl;
            i = 0;
            // smaze vector a hleda znovu
            seznam.clear();
            break;
          }
        }
      }

    }
  }

  return 0;
}