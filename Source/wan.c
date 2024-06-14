#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define BUFFER_SIZE 1000

// Callback function to write the received data from the HTTP request
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    char *data = (char *)userdata;

    // Ensure we don't overflow the buffer
    if (strlen(data) + total_size < BUFFER_SIZE - 1) {
        strncat(data, (char *)ptr, total_size);
    }

    return total_size;
}

// Function to get the WAN IP address of the host
void get_wan_ip(char *wan_ip) {
    CURL *curl;
    CURLcode res;
    char data[BUFFER_SIZE] = {0}; // Buffer to store the received data

    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing libcurl\n");
        return;
    }

    // Set the URL to fetch the WAN IP address
    curl_easy_setopt(curl, CURLOPT_URL, "http://checkip.dyndns.org");

    // Set the write callback function to handle the received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Pass the buffer to store the received data to the write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

    // Perform the HTTP request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error fetching the WAN IP address: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return;
    }

    // Extract the IP address from the received data
    char *ip_start = strchr(data, ':');
    if (ip_start) {
        char *ip_end = strchr(ip_start, '<');
        if (ip_end) {
            *ip_end = '\0'; // Null-terminate the IP address
            strcpy(wan_ip, ip_start + 2); // Skip ': ' characters
        }
    }

    // Clean up libcurl
    curl_easy_cleanup(curl);
}

int main() {
    char wan_ip[50] = {0};

    // Call the get_wan_ip() function to retrieve the WAN IP address
    get_wan_ip(wan_ip);

    // Print the WAN IP address
    printf("%s\n", wan_ip);

    return 0;
}

