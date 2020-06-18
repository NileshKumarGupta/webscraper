#include <iostream>
#include <curl/curl.h>

int main()
{
  CURL *easy_handle = curl_easy_init();
  char url[] = "https://en.wikipedia.com/wiki/CURL";
}
