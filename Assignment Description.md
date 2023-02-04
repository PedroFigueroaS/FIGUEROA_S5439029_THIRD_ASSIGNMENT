# ARP-Assignment 3-Summary

The Assignment in work on a code to design, develop, test and deploy is a modified version of Assignment 2, including client/server features. The modified version will be considered as a "application"

## The master process
The main process of the simulation, calls the other processes and gives each one of them a PID.

In this assignment, now the Master asks the user to choose 1 of 3 options:

1. Normal mode: The program works as in the assignment 2
2. Client mode: The program will ask the user to input the IP address and the port of the desired server to connect, after that will also launch the processB.
3. Server mode: The program will ask the user to input the port to set the connection, and will wait until receive the response of a client.

Process A now is divided between ProcessAserver and ProcessAclient

## The processAserver
Corresponds to the server part of the simulation, which will be waiting to receive the "handshake" of a client to start the simulation. 

After receiving the signal from the client, the process will enter in the screen as in the ProcessA of the assignment 2. In this case the cursor will not be moved in the same process,
but will receieve in a buffer the corresponding keyboard commands (UP, DOWN, RIGHT, LEFT), as strings, that have to convert to int values and will move cursor

At the same time the server prints the message that the server accept the client, will send a message using a pipe to the master, allowing the program to continue with the executation of ProcessB.

## The processAclient

The process corresponds to the client of the simulation, which will require the user to input an IP address and a Port, to connect to the server. The client will send the keyboard commands UP, DOWN, RIGHT, LEFT) as string message to the server.

## The ProcessB 

Because both the server and the client in each device will move a cursor, in every case, the ProcessB is spawned. Process B will use the shared memory communication to access the shared memory segment, in which there is the matrix, which has a dimension of 1600x600. After resizing the 1D pointer, which stores the address of the shared memory, into a 2D matrix, calculates the center and divides it by 20, to obtain the same coordinates as the ones in process A, and proceeds to plot it as a "0" in the process B windows. 

