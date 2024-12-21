// michal mandelbaum

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define MAX_HOST_PATH 2048
#define DEFULT_PORT 80

typedef struct {
    char *data;
    size_t size;
} CacheEntry;

// Parses a given URL into its host, path, and port components.
// If no port is provided, defaults to port 80.
void parse_url(const char *url, char *host, char *path, int *port);

// Generates a cache file path based on the host and path of the URL.
// Ensures unique filenames for caching purposes.
char* get_cache_filename(const char *host, const char *path);

// Reads data from a cache file if it exists.
// Returns a CacheEntry structure containing the cached data and its size.
CacheEntry* read_from_cache(const char *cache_file);

// Saves given data to a cache file, creating necessary directories.
void save_to_cache(const char *cache_file, const char *data, size_t length);

// Frees memory allocated for a CacheEntry structure.
void free_cache_entry(CacheEntry *entry);

// Fetches data from the server if it is not present in the cache.
// Saves the fetched data to the cache for future use.
void fetch_from_server(const char *host, const char *path, int port, const char *cache_file);

// Opens the given URL in the system's default web browser.
void open_in_browser(const char *url);

// Handles command-line arguments, extracting the URL and optional flags.
void handle_arguments(int argc, char *argv[], int *open_browser, char *url);

// Processes the provided URL by either reading from the cache or fetching from the server.
// Optionally opens the URL in a browser if requested.
void process_url(const char *url, int open_browser);


int main(int argc, char *argv[]) {
    int open_browser = 0;
    char url[MAX_HOST_PATH];

    handle_arguments(argc, argv, &open_browser, url);
    process_url(url, open_browser);

    return 0;
}

void handle_arguments(int argc, char *argv[], int *open_browser, char *url) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <URL> [-s]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strncpy(url, argv[1], MAX_HOST_PATH - 1);
    url[MAX_HOST_PATH - 1] = '\0';

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            *open_browser = 1;
        }
    }
}

// Processes the given URL by checking the cache and fetching data if needed.
// Steps:
// 1. Parse the URL into host, path, and port.
// 2. Check if the data exists in the cache.
// 3. If not in cache, fetch the data from the server and save it to the cache.
// 4. Optionally open the URL in a browser if the -s flag is provided.
void process_url(const char *url, int open_browser) {
    char host[MAX_HOST_PATH], path[MAX_HOST_PATH];
    int port;

    parse_url(url, host, path, &port);

    char *cache_file = get_cache_filename(host, path);
    CacheEntry *cached_data = read_from_cache(cache_file);

    if (cached_data) {
    size_t total_bytes = 0;
    printf("File is given from local filesystem\n");
    total_bytes += printf("HTTP/1.0 200 OK\r\n");
    total_bytes += printf("Content-Length: %zu\r\n\r\n", cached_data->size);
    size_t body_bytes = fwrite(cached_data->data, 1, cached_data->size, stdout);
    total_bytes += body_bytes;
    total_bytes += printf("\n Total response bytes: %zu\n", total_bytes);


    } else {
        fetch_from_server(host, path, port, cache_file);
    }

    free(cache_file);

    if (open_browser) {
        open_in_browser(url);
    }
}

void parse_url(const char *url, char *host, char *path, int *port) {
    *port = DEFULT_PORT;
    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }

    char *colon = strchr(url, ':');
    char *slash = strchr(url, '/');

    if (colon && (!slash || colon < slash)) {
        *colon = '\0';
        strncpy(host, url, MAX_HOST_PATH - 1);
        host[MAX_HOST_PATH - 1] = '\0';
        *port = atoi(colon + 1);
        *colon = ':';
    } else {
        if (slash) {
            strncpy(host, url, slash - url);
            host[slash - url] = '\0';
        } else {
            strncpy(host, url, MAX_HOST_PATH - 1);
            host[MAX_HOST_PATH - 1] = '\0';
        }
    }

    if (slash) {
        strncpy(path, slash, MAX_HOST_PATH - 1);
        path[MAX_HOST_PATH - 1] = '\0';
    } else {
        strcpy(path, "/index.html");
    }
}

char* get_cache_filename(const char *host, const char *path) {
    if (strcmp(path, "/") == 0) {
        path = "/index.html";
    } else if (path[strlen(path) - 1] == '/') {
        char *new_path = malloc(strlen(path) + 11);
        sprintf(new_path, "%sindex.html", path);
        path = new_path;
    }

    char *filename = malloc(strlen(host) + strlen(path) + 2);
    sprintf(filename, "%s/%s", host, path);
    return filename;
}

CacheEntry* read_from_cache(const char *cache_file) {
    struct stat st;

    if (stat(cache_file, &st) == 0) {
        FILE *fp = fopen(cache_file, "rb");
        if (fp) {
            CacheEntry *entry = malloc(sizeof(CacheEntry));
            entry->size = st.st_size;
            entry->data = malloc(entry->size);

            if (fread(entry->data, 1, entry->size, fp) == entry->size) {
                fclose(fp);
                return entry;
            }

            free(entry->data);
            free(entry);
            fclose(fp);
        }
    }
    return NULL;
}

void save_to_cache(const char *cache_file, const char *data, size_t length) {
    char *dir_path = strdup(cache_file);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            mkdir(dir_path, 0755);
        }
    }
    free(dir_path);

    FILE *fp = fopen(cache_file, "wb");
    if (fp) {
        fwrite(data, 1, length, fp);
        fclose(fp);
    }
}

void free_cache_entry(CacheEntry *entry) {
    if (entry) {
        free(entry->data);
        free(entry);
    }
}
// fetch_from_server: Connects to a server, sends an HTTP GET request, and saves the response to a cache file.
// Parameters:
//   - host: The hostname or IP address of the server.
//   - path: The specific path of the resource on the server (e.g., "/index.html").
//   - port: The port number to connect to (e.g., 80 for HTTP).
//   - cache_file: The file path where the server's response body will be saved.
void fetch_from_server(const char *host, const char *path, int port, const char *cache_file) {
    // Create a socket for communication
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket"); // Print an error message if socket creation fails
        exit(EXIT_FAILURE);
    }

    // Resolve the server's hostname to an IP address
    struct hostent *server = gethostbyname(host);
    if (!server) {
        herror("gethostbyname"); // Print an error message if hostname resolution fails
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Convert port number to network byte order
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Prepare the HTTP GET request
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE,
             "GET %s HTTP/1.0\r\n"
             "Host: %s:%d\r\n"
             "Connection: close\r\n\r\n",
             path, host, port);

    printf("HTTP request =\n%s\nLEN = %ld\n", request, strlen(request));

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); // Print an error message if connection fails
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send the HTTP request
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("send"); // Print an error message if sending data fails
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive the server's response
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    char *response = NULL;
    size_t total_size = 0;

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        // Dynamically resize the response buffer to store incoming data
        response = realloc(response, total_size + bytes_received);
        memcpy(response + total_size, buffer, bytes_received);
        total_size += bytes_received;
    }

    if (bytes_received < 0) {
        perror("recv"); // Print an error message if receiving data fails
    }

    // Process the response if any data was received
    if (total_size > 0) {
        char status_line[1024] = {0};
        int status_code = 0;
        char *end_of_line = strstr(response, "\r\n");

        size_t total_bytes = 0; // Tracks the total response size

        // Extract and print the status line
        if (end_of_line) {
            int status_len = end_of_line - response;
            strncpy(status_line, response, status_len);
            status_line[status_len] = '\0';
            printf("%s\n", status_line);

            // Parse the status code from the status line
            sscanf(status_line, "HTTP/1.%*d %d", &status_code);
            total_bytes += status_len + 4; // Include "\r\n"

            // If the status code indicates an error, exit early
            if (status_code >= 400) {
                free(response);
                close(sockfd);
                return;
            }
        }
    
         printf("HTTP/1.0 200 OK\r\n", status_line);
         sscanf(status_line, "HTTP/1.%*d %d", &status_code);  // עכשיו זה יעבוד
        // Locate and calculate "Content-Length"
        char *content_length = strstr(response, "Content-Length: ");
        if (content_length) {
            char *end_of_header = strstr(content_length, "\r\n");
            if (end_of_header) {
                int header_length = end_of_header - content_length + 2; // Include "\r\n"
                total_bytes += header_length;
            }
        }

        // Locate and process the response body
        char *body_start = strstr(response, "\r\n\r\n");
        if (body_start && status_code >= 200 && status_code < 400) {
            body_start += 4; // Skip "\r\n\r\n"
            size_t body_size = total_size - (body_start - response);

            // Write the response body to the console
            fwrite(body_start, 1, body_size, stdout);

            // Save the response body to the cache file
            save_to_cache(cache_file, body_start, body_size);

            // Add the body size to the total response size
            total_bytes += body_size;
        }

        // Print the total response size
        printf("\nTotal response bytes: %zu\n", total_bytes);
    }

    free(response); // Free dynamically allocated memory
    close(sockfd);  // Close the socket
}

// open_in_browser: Opens the given URL in the default web browser.
// Parameters:
//   - url: The URL to open.
void open_in_browser(const char *url) {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command),
             "xdg-open '%s' 2>/dev/null || open '%s' 2>/dev/null || start '%s'",
             url, url, url);
    printf("Opening browser for URL: %s\n", url);
    system(command); // Execute the command to open the browser
}
