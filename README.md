# proxy
A simple C-based HTTP proxy with caching functionality, URL parsing, and optional browser integration.
# HTTP Proxy with Caching

## Description
A simple C-based HTTP proxy server that supports caching, URL parsing, and optional integration with the default web browser. This project demonstrates fundamental concepts of socket programming, HTTP communication, and file handling in C. Designed for Linux systems using TCP.

## Features
- Parses URLs into host, path, and port components.
- Fetches web content from servers and caches it locally for future requests.
- Handles HTTP requests and responses with logging and status code validation.
- Opens URLs in the default web browser using a command-line flag.

## Requirements
- GCC or any compatible C compiler
- A Linux-based system (or adaptation for Windows/MacOS)
- Networking libraries (included in most C development environments)

## Compilation
To compile the program, use the following command:
```bash
gcc -o cproxy cproxy.c
```

## Usage
Run the program with the following syntax:
```bash
./cproxy <URL> [-s]
```
### Parameters:
- `<URL>`: The HTTP URL to fetch or process.
- `-s`: Optional flag to open the URL in the default system browser.

### Example:
```bash
./cproxy http://example.com -s
```
This will fetch the content from `example.com`, cache it locally, and open it in the browser.

## How It Works
1. **URL Parsing**: The program extracts the host, path, and port from the provided URL.
2. **Caching**: It checks for a cached version of the URL's content. If found, it serves the cached content.
3. **Fetching Data**: If the content is not cached, it fetches data from the server using TCP and saves it for future requests.
4. **Browser Integration**: If the `-s` flag is provided, the URL is opened in the default web browser.

## Limitations
- Only supports HTTP (not HTTPS).
- Basic error handling and logging.
- Designed as a learning project and may require adjustments for production use.

## File Structure
- `cproxy.c`: The main program file containing all logic for URL parsing, caching, and server communication.

## License
This project is open-source and available under the MIT License. Feel free to modify and distribute.
