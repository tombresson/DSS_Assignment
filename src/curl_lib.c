#include <stdio.h>
#include <stdint.h>
#include "inc/curl/curl.h"



static size_t printJsonData(void *ptr, size_t size, size_t nmemb, void *stream);


void curlLibGetData(void)
{
    CURL *curl_handle = curl_easy_init();

    if (curl_handle)
    {
        CURLcode res;
        curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, printJsonData);
        curl_easy_setopt(curl_handle, CURLOPT_URL, "http://statsapi.mlb.com/api/v1/schedule?hydrate=game(content(editorial(recap))),decisions&date=2018-06-10&sportId=1");
        res = curl_easy_perform(curl_handle);
        curl_easy_cleanup(curl_handle);
    }
}



static size_t printJsonData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    int bytes_written = printf(ptr, "%s*", size);
    return bytes_written;
}