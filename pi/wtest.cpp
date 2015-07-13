
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <ArduinoJson.h>

constexpr auto MAX_MSG_SIZE = 512;

void post_curl() {
	CURL *curl;
	CURLcode res;
	struct curl_slist *list;
	const char *endpoint = "http://localhost:8080";
	char json[] = "{ \"id\": 17 }";
	StaticJsonBuffer<MAX_MSG_SIZE> buffer;
	JsonObject& root = buffer.parseObject(json);

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	list = NULL;
	// The current server requires the data uploaded as app/json type
	list = curl_slist_append(list, "Content-type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

	curl_easy_setopt(curl, CURLOPT_URL, endpoint);
	// Add a byte at both the allocation and printing steps
	// for the NULL.
	char buf[MAX_MSG_SIZE];
	root.printTo(buf, 1+root.measureLength());
	printf("posting %s\n", buf);

	auto fstatus = fmemopen(buf, MAX_MSG_SIZE, "w");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fstatus);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
	// curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"id\":0,\"long\":1}");

	res = curl_easy_perform(curl);
	fclose(fstatus);
	/* Check for errors */
	if(res != CURLE_OK) {
		fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
				curl_easy_strerror(res));
	}
	else {
		fprintf(stderr, "WebQ: Connection successful\n");
		printf("'%s'\n", buf);
	}

	curl_slist_free_all(list);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void put_curl() {
	CURL *curl;
	CURLcode res;
	struct curl_slist *list;
	const char *endpoint = "http://localhost:8080/status";

	StaticJsonBuffer<MAX_MSG_SIZE> jsonBuffer;
	auto& root = jsonBuffer.createObject();
	root["pid"] = 18;
	// Add a byte at both the allocation and printing steps
	// for the NULL.
	char status[MAX_MSG_SIZE];
	root.printTo(status, 1+root.measureLength());
	fprintf(stderr, "Built %zu'%s'\n", root.measureLength(), status);
	auto fstatus = fmemopen(status, root.measureLength(), "r");


  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  list = NULL;
   // The current server requires the data uploaded as app/json type
  list = curl_slist_append(list, "Content-type: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	fprintf(stderr, "Header\n");

  curl_easy_setopt(curl, CURLOPT_URL, endpoint);
	fprintf(stderr, "Endpoint\n");

  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	fprintf(stderr, "PUT\n");
  // As opposed to CURLOPT_READDATA
  curl_easy_setopt(curl, CURLOPT_READDATA, fstatus);
  //curl_easy_setopt(curl, CURLOPT_READFUNCTION, put_read);
	fprintf(stderr, "read call\n");
  //curl_easy_setopt(curl, CURLOPT_INFILESIZE, MAX_MSG_SIZE);
	fprintf(stderr, "file sz\n");

	fprintf(stderr, "write call\n");

  res = curl_easy_perform(curl);
	fclose(fstatus);
	fprintf(stderr, "done\n");
  /* Check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
  }
	else {
		fprintf(stderr, "WebQ: Connection successful\n");
	}

  curl_slist_free_all(list);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

int main(int argc, char** argv) {
	put_curl();
}
