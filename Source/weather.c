#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#define CACHE_FILE "/tmp/weather.txt"
#define CACHE_DURATION 3600  // in seconds

// Callback function to write the received data from the HTTP request
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    char *data = (char *)userdata;

    // Ensure we don't overflow the buffer
    if (strlen(data) + total_size < 1000 - 1) {
        strncat(data, (char *)ptr, total_size);
    }

    return total_size;
}

// Function to get the weather data
void fetch_weather(char *weather_data) {
    CURL *curl;
    CURLcode res;

    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing libcurl\n");
        return;
    }

    // Set the URL to fetch the weather data
    curl_easy_setopt(curl, CURLOPT_URL, "https://wttr.in/?format=1");

    // Set the write callback function to handle the received data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Pass the buffer to store the received data to the write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, weather_data);

    // Perform the HTTP request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error fetching the weather data: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return;
    }

    // Clean up libcurl
    curl_easy_cleanup(curl);
}

// Function to get the weather data, using cache if available and valid
void get_weather(char *weather_data) {
    struct stat st;
    time_t current_time = time(NULL);

    if (stat(CACHE_FILE, &st) == 0) {
        time_t last_modified = st.st_mtime;
        if ((current_time - last_modified) < CACHE_DURATION) {
            FILE *file = fopen(CACHE_FILE, "r");
            if (file) {
                fgets(weather_data, 1000, file);
                fclose(file);
                return;
            }
        }
    }

    fetch_weather(weather_data);

    // Update the cache file
    FILE *file = fopen(CACHE_FILE, "w");
    if (file) {
        fprintf(file, "%s", weather_data);
        fclose(file);
    }
}

int main() {
    char weather_data[1000] = {0};

    // Call the get_weather() function to retrieve the weather data
    get_weather(weather_data);

    // Print the weather data
    printf("%s\n", weather_data);

    return 0;
}

