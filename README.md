# FrogWebProxy
Eugene Lee - 30137489 / CPSC 441 Assignment 2 / October 18th, 2023

This is a frog proxy written in C++, that will replace all occurrences of the word "frog" with the word "Fred", and also replace all JPG (or JPEG) file on the browser with a custom-picked image of a frog instead. It works by first intercepting the HTTP request (GET request) from the client, and then modifying it accordingly. Then, it sends the request to the server, and then the server sends a HTTP response which I intercept once again, to parse and edit it before sending it back to the client. 

User Manual: 
1. Configure the proxy setting on your browser (preferrably Firefox), by setting the address as 'localhost' and the port number as the value defined in the program (8080).
2. Compile and run the file. Go to the working directory of the code on your device and run the following commands in the terminal. 
    Compile: "g++ -o proxy frogProxy.cpp"
    Run: "./proxy"
3. Make sure to clear any cache on browser, and make sure all the sites are only using http. Then, when you compile & run properly, the proxy will be fully functional on websites. 

Testing Description: 
- I tested a alot both at school and at home, using my laptop. Since my laptop is Unix, it was not too challenging to make it also work on Linux devices. I mainly wrote out to the terminal using cout to test my code. By printing out everything, I was able to see which parts are working and which parts aren't. It was a lot of debugging done this way. Being able to locate all the issues helped me understand all the different moving parts of the proxy, and handle all malfunctionalities appropriately. I also utilized WireShark a few times to see what was going on and to debug. 

What Works? 
- Everything in my proxy runs as expected, as it first started off as the transparent proxy, then I slowly built my way up to changing "frog" and eventually images. So it has all the required functionalities of the assignment.

What doesn't work? 
- Sometimes, very rarely, the proxy does not run properly on the first try. I believe that the issue lies with the cache of the browser, and not my code. However, if I simply re-run the proxy, it works perfectly fine, meeting all the requirements. Also, if I run the code, stop it, and then try to run it right after, it may get a "Bind failed: Address already in use" message in the terminal. Then I just need to wait around 30 seconds for the port to close properly, or just define a new port in the program and change it on the browser accordingly. So no really glaring issues. 

Optional Features:
- When the server sends back abnormal HTTP status codes (anything other than 200 OK), I catch these cases, and output the predefined HTTP response in order to let the clients know.