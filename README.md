HTTP Server in C
This project is an HTTP server implemented in C for CSE 130: Principles of Computer Systems Design. The server handles basic HTTP/1.1 requests, specifically supporting GET and PUT methods. It listens for incoming client connections, processes HTTP requests, and responds appropriately based on the request type and file availability.

Features:
GET Requests: Retrieves and serves the content of requested files.
PUT Requests: Creates or updates files with the provided data from the client.
Error Handling: Gracefully handles malformed requests, unsupported methods, and missing files, returning appropriate HTTP status codes.
Robustness: Designed to run indefinitely without crashing, even when handling malicious or malformed requests.
Efficient Resource Management: No memory leaks or file descriptor leaks, adhering to best practices in memory management.
Technologies Used:
C (with POSIX sockets)
Custom string parsing and memory management
Standard Linux system calls for networking
Running the Server:
bash
Copy
Edit
./httpserver <port>
Replace <port> with the desired port number (ensure ports below 1024 are run with sudo).

Example Requests:
Using curl to test the server:

GET: curl http://localhost:1234/filename.txt
PUT: curl -T filename.txt http://localhost:1234/filename.txt
