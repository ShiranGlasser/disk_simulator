-DISK SIMULATOR-

Authored by Shiran Glasser 

==Description==
The program implements a disk simulator. It simulates the mapping system of the fisical memory of the files on the disk. It uses the 'index allocation' method. 
The program divides the disk's memory to even blocks and assigns for each file a managing block to hold the location of it's blocks on the disk, each block holds the file's data values.
In this program the disk will be represented as one file.
Each file has a unique file descriptor number.
The user press on the wanted chois to do on the disk-format, create a file and more, the program implements the given chois and run the disk as needed.    
==Program DATABASE: ==
There are three main classes implemented in this program:
(1) FsFile- holds the information of a file and its pointer to the index block9the manager block to the data blocks)
(2) FileDescriptor- contain the name of the file and a pointer to its FsFile with the information
(3) fsDisk- the disk itself. Holds all the program's databases and methods, the managing system

MainDir- a vector of FileDeccriptor pointers. Holds all the files on the disk

OpenFileDecriptors-  a vector of FileDeccriptor pointers. Holds only the files that currently open.

BitVector- an array in the size of the number of blocks in the disk. Tell for each block(each index) if its free(0) or not(1);
==functions: ==
Programs functions:
1. listAll- The method runs through all of the files in the main directory and prints their name and if they're open.
2. fsFormat- receives the size of each block on the disk, clear all the program's memory and  initializes the disk to split by the given block size.
3. CreateFile- receives a file name and opens a new file on the disk with that name on the main directory. With the opening- assigning an index block to the file and automatically opens it and adding to the Open File Descriptor vector and returns the file descriptor number of the file(its index on the FileDescriptor vector).
4. openFile- if the file isn’t open and exists on the main directory- opens it to write or read from.
when opening- adding to the OpenFileDescriptor vector and returning the file's fd.
5. Close file- changing the files inUse to be false. 
6. WriteToFile- receives the amount of chars to write in a buffer and the file's fd number. Checks if the file exists on the main dir and if opened, if so- write the data on the files blocks with the space limitation and updates the data bases and index block as need.
7. DelFile- receives the files name and deletes its resources, closes it and erase its location on the main dir 
8. ReadFromFile- receives the amount of chars to read, a buffer to read from and the fd of the file and reads them from its data blocks.(only if the file is open and the chars are written).
Help methods:
1.    findFileDesNum- returns the lowest file descriptor number that's available in the file descriptor vector
2. findFdByName-recives a file name and vector , returns the index on the vector of the given name, if not found -1
3. clearAblock- formats the given block from the index to the end of the block. returns 0 if fail
4. findFreeBlock- return the number of the next free block(index on Bit vector with value 0), updates the number of free blocks and the bit vector. -1 if fail
5. getBlockNum- gets a location on file to read from index block and return the number of the block written in that location
6. decToBinary- receives a block number and a char, saves the block number in his binarys form on c to write on the disk in the index block to mark a block's location.
==Program Files==
disk_sim.cpp
==How to compile?==
 compile: g++ disk_sim.cpp -o disk
 run: ./disk
==Input:== 
The users choices to the program in order to run the disk
==Output:==
The shell that communicates with the user to do changes on the disk.
