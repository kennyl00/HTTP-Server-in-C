# __A Simple HTTP Server__ #
This simple server program is written in C. 
* It processes only `GET` request (e.g. `GET /css/style.css HTTP/1.0`) 
* It can only handle file types of `.html`, `.jpg`, `.css`, and `.js`.
* It then returns the requested file if it exists. 
* It uses forking to handle multi-threading 

To run the program 
```
make server
./server [valid port number (e.g. 8080] [path to web root (e.g. /folder/folder)]
```
