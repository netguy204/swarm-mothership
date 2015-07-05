
#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

int main(int argc, char** argv) {

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();

  struct curl_slist *list = NULL;
  // The current server requires the data uploaded as app/json type
  list = curl_slist_append(list, "content-type: application/json");

  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/update");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"id\":19}");

  CURLcode res = curl_easy_perform(curl);
  /* Check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    return 1;
  }
  fprintf(stderr, "WebQ: Connection successful\n");

  curl_slist_free_all(list);
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return 0;
}
