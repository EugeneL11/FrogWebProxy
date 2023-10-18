// CPSC 441 Assignment 2
// Eugene Lee - 30137489
// Frog Web Proxy

// Necessary imports
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>

//define the port for the browser to connect to
#define PORT 8080

using namespace std;

int main() {
    int listenSocket, commSocket;
    struct sockaddr_in addr;
    int addrLen = sizeof(addr);

    //creating the socket for listening
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("Socket failed");
        exit(EXIT_FAILURE);
    } 

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT); //assigning the port number
    
    //bind the listening socket
     if (bind(listenSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    } 

    //listen for connections
    if (listen(listenSocket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    int counter = 0;
    const char* ipAddr;
    
    //this loop will allow the proxy to run continuously
    while(1) {
        cout << "Waiting for new connection..." << endl;
        
        //socket to communicate with browsers
        if ((commSocket = accept(listenSocket, (struct sockaddr *)&addr, (socklen_t*)&addrLen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        cout << "Connection accepted!" << endl;
        char buffer[4096] = {0}; //buffer to store the HTTP requests
        recv(commSocket, buffer, sizeof(buffer), 0); //receive the request using the socket, and store in buffer

        cout << buffer;
        counter++;
        cout << "Count: " << counter << endl;
        //received HTTP request from browser, now need to send this to the server and receive a response which I need to send back to the browser

        //need to read the GET message to get the host name
        std::string getReq(buffer); //convert buffer to string
        std::string getCheck = getReq.substr(0, 3); //isolate the first three characters of the request
        if (getCheck == "GET") { //only if it is a GET request
            size_t jpgLoc = getReq.find(".jpg HTTP"); //look for requests for a .jpg image
            size_t jpegLoc = getReq.find(".jpeg HTTP"); //look for requests for a .jpeg image
            if ((jpgLoc != string::npos) || (jpegLoc != string::npos)) { //if there are requests for .jpg or .jpeg
                //swap the GET request with the GET request for a FROGUE.jpg
                getReq = 
                    "GET http://pages.cpsc.ucalgary.ca/~jcleahy/Frogue.jpg HTTP/1.1\r\n"
                    "Host: pages.cpsc.ucalgary.ca\r\n"
                    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:109.0) Gecko/20100101 Firefox/118.0\r\n"
                    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
                    "Accept-Language: en-CA,en-US;q=0.7,en;q=0.3\r\n"
                    "Accept-Encoding: gzip, deflate\r\n"
                    "Connection: keep-alive\r\n"
                    "Upgrade-Insecure-Requests: 1\r\n"
                    "\r\n";
            }

            size_t hostLoc = getReq.find("Host:"); //look for the hostname
            
            //If "Host:" is found, extract the hostname
            if (hostLoc != string::npos) { //if it exists
                size_t hostLineEnd = getReq.find("\r\n", hostLoc); //Find the end of the "Host:" line

                //Extract the hostname
                string hostLine = getReq.substr(hostLoc, hostLineEnd - hostLoc); //the whole line with Host: 
                size_t hostStart = hostLine.find(" "); //where the actual hostname starts
                string hostName = hostLine.substr(hostStart + 1); //the actual hostname

                //Get the IP address using the hostname
                struct hostent *he = gethostbyname(hostName.c_str());
                if (he == NULL) { //something went wrong
                    herror("gethostbyname");
                } else {
                    //gethostbyname returns multiple things about the hostname, only need to solve for the IP address
                    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
                    for (int i = 0; addr_list[i] != NULL; i++) {
                        ipAddr = inet_ntoa(*addr_list[i]); //the IP address of the host
                        cout << "IP Address " << i + 1 << ": " << ipAddr << endl;
                    }
                }
            } else { //If "Host:" is not found
                cout << "Host not found in the request." << endl;
            }

            //send GET request to server, receive HTTP response from server, and send it back to the client
            int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //socket to interact with the server
            if (serverSocket == -1) {
                perror("Socket failed");
                exit(EXIT_FAILURE);
            }
            
            struct sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(80); //the server uses port 80
            serverAddr.sin_addr.s_addr = inet_addr(ipAddr); //IP address resolved from hostname

            //connect the socket to server
            int status = connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
            if (status == -1) {
                perror("Connection failed");
                exit(EXIT_FAILURE);
            }

            send(serverSocket, getReq.c_str(), strlen(getReq.c_str()), 0); //send the GET request to the server

            //receiving the HTTP response
            char buffer2[1024] = {0}; //buffer to store chunks of HTTP response
            int bytesReceived = 0; //will store the length of the HTTP response in bytes
            int chunkCount = 0; //to keep track of the first chunk

            //receive all the information from the server, 1024 bytes at a time
            while ((bytesReceived = recv(serverSocket, buffer2, 1024, 0)) > 0) { //receive reponse from server and store in buffer2
                std::string respChunk(buffer2, bytesReceived); //convert the response to a string

                //need to change "Frog " / "frog " / "FROG" / "frog-"
                std::string frogVariations[] = {"Frog ", "frog ", "frog-"}; //lower case
                for (int i = 0; i < sizeof(frogVariations) / sizeof(frogVariations[0]); i++) { //loop through all posibiliies
                    const std::string& frog = frogVariations[i]; //one of the variations
                    size_t frogPos = respChunk.find(frog); //find location of word
                    while (frogPos != std::string::npos) { //find all instances of the word
                        respChunk.replace(frogPos + 2, 2, "ed"); //change to make fred
                        frogPos = respChunk.find(frog, frogPos + 5); //find in rest of the content
                    }
                }

                size_t frogPos = respChunk.find("FROG"); //for upper cases
                while (frogPos != std::string::npos) {
                    respChunk.replace(frogPos, 4, "FRED");
                    frogPos = respChunk.find("FROG", frogPos + 4);
                }

                chunkCount++; //increment counter
                string statusCode = respChunk.substr(9,3); //getting just the code

                //handle common HTTP response codes
                if ((chunkCount == 1) && (statusCode != "200")) { //only check the first chunk
                    size_t statusLineEnd = respChunk.find("\r\n", 8); //end of line for the status code
                    string statusLine = respChunk.substr(0, statusLineEnd); //isolate the status msg
                    string statusType = respChunk.substr(9, statusLineEnd - 9); //whole status code

                    //customized HTTP response to output
                    string errorMsg = statusLine + "\r\nContent-Type: text/html\r\n\r\n<html><body style=\"text-align:center;padding-top:80px;\"><h1>" + statusType + "!</h1><p>Sorry, there has been a froggy error...</p></body><html>";

                    //send the custom response back to the browser
                    send(commSocket, errorMsg.c_str(), errorMsg.size(), 0);
                } else {
                    //send back the normal HTTP response
                    send(commSocket, respChunk.c_str(), bytesReceived, 0);
                }

                // Clear the buffer to prepare for the next chunk
                memset(buffer2, 0, sizeof(buffer2));
            }
            close(commSocket); //close the socket for communication with browser
            close(serverSocket); //close socket for communication with server
            cout << "Connection closed" << endl << endl;
        } //end of GET conditional

    } //end of while loop
    close(listenSocket); //close the socket for listening to new connections
} //end of main