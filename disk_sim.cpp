/*
Authored by Shiran Glasser
208608174

==Description==
The program implements a disk simulator. It simulates the mapping system of the fisical memory of the files on the disk. It uses the 'index allocation' method.
The program divides the disk's memory to even blocks and assigns for each file a managing block to hold the location of it's blocks on the disk, each block holds the file's data values.
In this program the disk will be represented as one file.
Each file has a unique file descriptor number.
The user press on the wanted chois to do on the disk-format, create a file and more, the program implements the given chois and run the disk as needed.

 */

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

// ============================================================================

class FsFile {

    int file_size;

    int block_in_use;

    int index_block;

    int block_size;

public:
        //fsFile class constructor:
        FsFile(int _block_size) {

            file_size = 0;

            block_in_use = 0;

            block_size = _block_size;

            index_block = -1;

        }


    int getfile_size() {  return file_size; }
    int getblock_in_use() {  return block_in_use; }
    int getindex_block() {  return index_block; }
    void setfile_size(int size) {  file_size=size; }
    void setblock_in_use(int blocks) {  block_in_use=blocks; }
    void setindex_block(int indexBlock) { index_block=indexBlock; }


};

// ============================================================================
class FileDescriptor {

    string file_name;

    FsFile* fs_file;

    bool inUse;

public:

    //fileDescriptor class constructor:
    FileDescriptor(string FileName, FsFile* fsi) {

        file_name = FileName;

        fs_file = fsi;

        inUse = true;

    }

    string getFileName() {return file_name;}
    FsFile* getFsFile() {return fs_file; }
    void setFsFile(FsFile* f) {fs_file=f;};
    void setInUse(bool b) {this->inUse=b;}
    bool getInUse() {return inUse;}
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

// ============================================================================

class fsDisk {

    FILE *sim_disk_fd;
    bool is_formated;

    //an array in the size of the number of blocks in the disk. Tell for each block(each index) if its free(0) or not(1);
    int BitVectorSize;
    int *BitVector;

    std::vector<FileDescriptor*> MainDir;  //a vector of FileDeccriptor pointers. Holds all the files on the disk
    std::vector<FileDescriptor*> OpenFileDescriptors; //a vector of FileDeccriptor pointers. Holds only the files that currently open.

    int block_size;

    int maxSize;

    int freeBlocks;

    //MY HELP METHODS:
    int findFileDesNum() //returns the lowest file descriptor number that's available
    {
        int i;
        for (i = 0; i <OpenFileDescriptors.size() ; ++i)
            if (OpenFileDescriptors[i]->getInUse()== 0)
                return i;

            return i;
    }
    // ------------------------------------------------------------------------
    int findFdByName(string fileName, vector<FileDescriptor*>v) //returns the index on the vector of the given name, if not found -1
    {
        for (int i = 0; i < v.size(); ++i) {
            if(v[i]->getFileName()==fileName)
                return i;
        }
        return -1;
    }
    // ------------------------------------------------------------------------
    int clearAblock(int block) //formats the given block from the index to the end of the block. returns 0 if fail
    {
        int test;
        for (int i = 0; i < block_size; ++i)
        {
            test = fseek (sim_disk_fd , block+i , SEEK_SET);
            if(test==-1)
            {
                //there was a problem moving the pointer on the file
                return 0;
            }
            test= fwrite("\0", 1, 1, sim_disk_fd);
            if(test=!1)
            {
                //there was a problem writing to the file
                return 0;
            }
        }
        return 1;
    }
    // ------------------------------------------------------------------------
    int findFreeBlock() //return the number of the next free block(index on Bit vector with value 0), updates the number of free blocks and the bit vector. -1 if fail
    {
        int i;
        for (i = 0; i <BitVectorSize ; ++i) {
            if(BitVector[i]==0)
                break;
        }
        if(i==BitVectorSize)
            return -1;

        BitVector[i]=1;
        freeBlocks--;
        return i;
    }
    // ------------------------------------------------------------------------
    int getBlockNum(int location) //gets a location on file to read from index block and return the number of the block written in that location
    {
        int test;
        char c='\0';
        test = fseek (sim_disk_fd , location , SEEK_SET );
        if(test==-1)
        {
            //there was a problem moving the pointer on the file
            return -1;
        }
        //c will hold the number of current data block of the file in char
        test= fread(&c, 1, 1, sim_disk_fd);
        if(test=!1)
        {
            //there was a problem reading from file
            return -1;
        }
        return (int)c;
    }
    // ------------------------------------------------------------------------
    char  decToBinary(int n , char &c) //receives a block number and a char, saves the block number in his binarys form on c to write on the disk in the index block to mark a block's location.
    {
        // array to store binary number
        int binaryNum[8];

        // counter for binary array
        int i = 0;
        while (n > 0) {
            // storing remainder in binary array
            binaryNum[i] = n % 2;
            n = n / 2;
            i++;
        }

        // printing binary array in reverse order
        for (int j = i - 1; j >= 0; j--) {
            if (binaryNum[j]==1)
                c = c | 1u << j;
        }
        return ' ';
    }
    // ------------------------------------------------------------------------

public:
    //fsDisk class constructor:
    fsDisk()
    {

        sim_disk_fd = fopen(DISK_SIM_FILE , "w+");

        assert(sim_disk_fd);

        for (int i=0; i < DISK_SIZE ; i++) {

            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd);

            assert(ret_val == 1);

        }
        fflush(sim_disk_fd);

        block_size = 0;

        is_formated = false;

    }
    // ------------------------------------------------------------------------
    //destructor:
    ~fsDisk()
    {
        //clear the databases
        for (int i = MainDir.size()-1; i >=0; i--)
        {
            if (i<OpenFileDescriptors.size())
                if(OpenFileDescriptors[i]->getFsFile()==NULL)
                    delete OpenFileDescriptors[i];

            delete MainDir[i]->getFsFile();
            delete MainDir[i];

        }

        free(BitVector);
        fclose(sim_disk_fd);
    }
    // ------------------------------------------------------------------------
    /*The method runs through all of the files in the main directory and prints their name and if
      they're open. */
    void listAll()
    {

        int i = 0;

        for (int i=0; i<MainDir.size(); i++)
        {
            cout << "index: " << i << ": FileName: " << MainDir[i]->getFileName()
            <<  " , isInUse: " <<MainDir[i]->getInUse()<< endl;

        }

        char bufy;

        cout << "Disk content: '" ;

        for (i=0; i < DISK_SIZE ; i++) {

            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );

            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );

            cout << bufy;

        }

        cout << "'" << endl;
    }
    // ------------------------------------------------------------------------
    /*receives the size of each block
     * on the disk, clear all the program's memory and initializes the disk
     * to split by the given block size.
     */
    void fsFormat(int blockSize)
    {
        this->block_size=blockSize;
        this->maxSize=block_size*block_size;
        freeBlocks= DISK_SIZE/block_size;

        BitVectorSize=freeBlocks;
        if(is_formated)
            free (BitVector);

        BitVector=(int*) malloc(sizeof (int)*BitVectorSize);

        for (int i = 0; i < BitVectorSize; ++i)
            BitVector[i]=0;

        //already clear
        if(!is_formated)
        {
            is_formated= true;
            return;
        }

        //free all memory in the databases and clear them
        for (int i = MainDir.size()-1; i >=0; i--)
        {
            if (i<OpenFileDescriptors.size())
                if(OpenFileDescriptors[i]->getFsFile()==NULL)
                    delete OpenFileDescriptors[i];

                delete MainDir[i]->getFsFile();
                delete MainDir[i];

        }
        MainDir.clear();
        OpenFileDescriptors.clear();


        //clear the file
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd);
            assert(ret_val == 1);
        }
    }
    // ------------------------------------------------------------------------
    /*receives a file name and opens a new file on the disk with that name on the main directory.
     * With the opening- assigning an index block to the file and automatically
     * opens it and adding to the Open File Descriptor vector and returns the file descriptor
     * number of the file(its index on the FileDescriptor vector).
     */
    int CreateFile(string fileName)
    {
        if (!is_formated)
            //The disk has never been formatted
            return -1;


        //there is at least 1 block needed for a file- index block and data
        if(freeBlocks<1)
            //there is no room for a new file on disk
            return -1;



        FileDescriptor* temp=new FileDescriptor(fileName, new FsFile(this->block_size));
        temp->setInUse(true);
        int FileIndexBlock=findFreeBlock();
        temp->getFsFile()->setindex_block(FileIndexBlock);

        int ind= findFileDesNum();
        if(ind==OpenFileDescriptors.size())
            OpenFileDescriptors.push_back(temp);
        else
        {
            if (OpenFileDescriptors[ind]->getFsFile()==NULL) //not in main dir, need to free memory
                delete OpenFileDescriptors[ind];

            OpenFileDescriptors[ind]=temp;
        }

        MainDir.push_back(temp);
        return ind;
    }
    // ------------------------------------------------------------------------
    /*if the file isn’t open and exists on the main directory- opens it to write or read from.
when opening- adding to the OpenFileDescriptor vector and returning the file's fd.
     */
    int OpenFile(string fileName)
    {
        int i=findFdByName(fileName, MainDir);

        if(i<0)
            //file doesnt exist at all
            return -1;


        if(MainDir[i]->getInUse())
            // file already opened
            return -1;


        MainDir[i]->setInUse(true);
        int fd=findFileDesNum();
        if(fd == OpenFileDescriptors.size())
            OpenFileDescriptors.push_back(MainDir[i]);
        else
        {
            if (OpenFileDescriptors[fd]->getFsFile()==NULL) //not in main dir, need to free memory
                delete OpenFileDescriptors[fd];

            OpenFileDescriptors[fd]=MainDir[i];
        }

        return fd;
    }
    // ------------------------------------------------------------------------
    /*changing the files inUse to be false.
     */
    string CloseFile(int fd)
    {
        if(fd>=OpenFileDescriptors.size()||OpenFileDescriptors[fd]->getInUse()== false)

            //file is not open
            return "-1";


        OpenFileDescriptors[fd]->setInUse(false);
        return OpenFileDescriptors[fd]->getFileName();
    }
    // ------------------------------------------------------------------------
    /*receives the amount of chars to write in a buffer and the file's fd number.
     * Checks if the file exists on the main dir and if opened,
     * if so- write the data on the files blocks with the space limitation
     * and updates the data bases and index block as need.
     */
    int WriteToFile(int fd, char *buf, int len)
    {
        //check if the has been opened
        //(if the disk has never been formatted, the fd will be bigger than the openFileDescriptors vector size
        if(fd>=OpenFileDescriptors.size()|| !OpenFileDescriptors[fd]->getInUse())
            //The file is not open to write
            return -1;


        int remainLen=len;
        int fileIndexBlock=OpenFileDescriptors[fd]->getFsFile()->getindex_block();
        int fileBlockInUse=OpenFileDescriptors[fd]->getFsFile()->getblock_in_use();

        int lastBlock_location=fileIndexBlock*block_size+fileBlockInUse;

        int test;
        char c;

        //finds if there's an internal fragmentation in the blocks that already assigned for this file
        int frag=OpenFileDescriptors[fd]->getFsFile()->getfile_size()%block_size;
        if(frag>0)
        {
            int DataBlock=getBlockNum(lastBlock_location-1);
            if (DataBlock<0)
                return -1;

            test = fseek (sim_disk_fd , DataBlock*block_size+frag , SEEK_SET );
            if(test==-1)
                //there was a problem moving the pointer on the file
                return -1;

            test= fwrite(buf, 1, block_size-frag, sim_disk_fd);
            if(test=!1)
                //there was a problem writing to file
                return -1;

            remainLen-=(block_size-frag);
            OpenFileDescriptors[fd]->getFsFile()->setfile_size(OpenFileDescriptors[fd]->getFsFile()->getfile_size()+block_size-frag);
        }
        // ------------------------------------------------------------------------
        if (remainLen <= 0)
        {
            cout<<"Writed "<< len<<" chars into File Descriptor #:"<<fd<<endl;
            return 1;
        }

        int blocksNeeded= remainLen / block_size;
        if(remainLen % block_size > 0)
            blocksNeeded++;

        while (blocksNeeded>0)
        {
            if(fileBlockInUse==block_size)
            {
                cout<<"Writed"<< len-remainLen<<" chars into File Descriptor #:"<<fd<<endl;
                return -1;
            }
            if(freeBlocks<=0)
            {
                cout<<"Writed "<< len-remainLen<<" chars into File Descriptor #:"<<fd<<endl;
                return -1;
            }

            //assigning a new block to write on
            int newBlock=findFreeBlock();
            fileBlockInUse++;
            OpenFileDescriptors[fd]->getFsFile()->setblock_in_use(fileBlockInUse);

            //writing the new blocks number in the files index block:
            c='\0';
            test = fseek (sim_disk_fd , lastBlock_location , SEEK_SET);
            if(test==-1)
            {
                //there was a problem moving the pointer on the file
                cout<<"Writed "<< len-remainLen<<" chars into File Descriptor #:"<<fd<<endl;
                return -1;
            }
            decToBinary(newBlock, c);
            test= fwrite(&c, 1, 1, sim_disk_fd);
            if(test=!1)
            {
                //there was a problem writing to the file
                cout<<"Writed "<< len-remainLen<<" chars into File Descriptor #:"<<fd<<endl;
                return -1;
            }

            //writing the data to the current block:
            for (int i = 0; i < block_size; ++i)
            {
                if(remainLen <= 0)
                {
                    cout<<"Writed "<< len<<" chars into File Descriptor #:"<<fd<<endl;
                    return 1;
                }
                test = fseek (sim_disk_fd , newBlock*block_size+i , SEEK_SET );
                if(test==-1)
                {
                    //there was a problem moving the pointer on the file
                    cout<<"Writed "<< len-remainLen<<" chars into File Descriptor #:"<<fd<<endl;
                    return -1;
                }
                test= fwrite(&buf[len - remainLen], 1, 1, sim_disk_fd);
                if(test=!1)
                {
                    //there was a problem writing to the file
                    cout<<"Writed "<< len-remainLen<<" chars into File Descriptor #:"<<fd<<endl;
                    return -1;
                }
                OpenFileDescriptors[fd]->getFsFile()->setfile_size(OpenFileDescriptors[fd]->getFsFile()->getfile_size()+1);
                remainLen--;
            }

            lastBlock_location++;
            blocksNeeded--;
        }

        cout<<"Writed "<< len<<" chars into File Descriptor #:"<<fd<<endl;
        return 1;
    }
    // ------------------------------------------------------------------------
    /* receives the files name and deletes its resources,
     * closes it and erase its location on the main dir
     */
  int DelFile( string FileName )
    {
        int DataBlock;
        int test;
        int ind= findFdByName(FileName, MainDir);

        if(ind<0)
        {
            //file does not exist
            return 0;
        }

        int numOfBlocks=MainDir[ind]->getFsFile()->getblock_in_use();
        freeBlocks+=(numOfBlocks+1); //also index indexBlock
        //deleting the files' data blocks:
        int indexBlock= MainDir[ind]->getFsFile()->getindex_block();
        for (int i = 0; i < numOfBlocks; ++i) //for each indexBlock in use in this file
            {
            DataBlock= getBlockNum(indexBlock*block_size + i);//the offset of the current data indexBlock in the file
            if(DataBlock==-1)
                //there was a problem moving the pointer on the file
                return 0;

            test= clearAblock(DataBlock*block_size); //clearing each data indexBlock
            if(test==0)
                return test;

            BitVector[DataBlock]=0;
            }

        //deleting the index indexBlock
        BitVector[indexBlock]=0;
        test= clearAblock(indexBlock);
        if(test==0)
            return test;

        //erasing the file from the main directory
        delete MainDir[ind]->getFsFile();
        if (findFdByName(FileName, OpenFileDescriptors)!=-1)
            delete MainDir[ind];
        else
        {
            MainDir[ind]->setFsFile(NULL); //to know in the open file descriptor vector that it was deleted and closed
            MainDir[ind]->setInUse(false);
        }

        MainDir.erase(MainDir.begin()+ind);
        return 1;
    }
    // ------------------------------------------------------------------------
    /*receives the amount of chars to read,
     * a buffer to read from and the fd of the file and reads them from its data blocks.
     * (only if the file is open and the chars are written)
     */
    int ReadFromFile(int fd, char *buf, int len)
    {
        //check if the has been opened
        //(if the disk has never been formatted, the fd will be bigger than the openFileDescriptors vector size
        if(fd>=OpenFileDescriptors.size()|| !OpenFileDescriptors[fd]->getInUse())
        {
            //The file is not open to read
            return -1;
        }

        for (int i = 0; i < DISK_SIZE; ++i)
            buf[i]='\0';


        int fileIndexBlock=OpenFileDescriptors[fd]->getFsFile()->getindex_block()*block_size;
        int ind=0;
        int test;
        int remainToRead=len;
        int size=OpenFileDescriptors[fd]->getFsFile()->getfile_size();

        int blocksToRead=len/block_size;
        if(len%block_size>0)
            blocksToRead++;

        while (blocksToRead>0)
        {
            if(ind>=block_size) //reached the end of the index block
                return -1;

            //finding the current block to read from:
            int curBlock= getBlockNum(fileIndexBlock+ind);


            //reading the data from the current block:
            for (int i = 0; i < block_size; ++i)
            {
                if(remainToRead <= 0)
                    return 1;

                if(size<=0)
                    return -1;


                test = fseek (sim_disk_fd , curBlock*block_size+i , SEEK_SET );
                if(test==-1)
                {
                    //there was a problem moving the pointer on the file
                    return -1;
                }
                test= fread(&buf[len - remainToRead], 1, 1, sim_disk_fd);
                if(test=!1)
                {
                    //there was a problem writing to the file
                    return -1;
                }
                remainToRead--;
                size--;
            }
            blocksToRead--;
            ind++;
        }

        return 1;
    }
    // ------------------------------------------------------------------------
};

int main()
{
int blockSize;
int direct_entries;
string fileName;
char str_to_write[DISK_SIZE];
char str_to_read[DISK_SIZE];
int size_to_read;
int _fd;

fsDisk *fs = new fsDisk();
int cmd_;
while(1) {
    cin >> cmd_;
    switch (cmd_)
    {
        case 0:   // exit
        delete fs;
        exit(0);
        break;

        case 1:  // list-file
        fs->listAll();
        break;

        case 2:    // format
        cin >> blockSize;
        fs->fsFormat(blockSize);
        break;

        case 3:    // creat-file
        cin >> fileName;
        _fd = fs->CreateFile(fileName);
        cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
        break;

        case 4:  // open-file
        cin >> fileName;
        _fd = fs->OpenFile(fileName);
        cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
        break;

        case 5:  // close-file
        cin >> _fd;
        fileName = fs->CloseFile(_fd);
        cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
        break;

        case 6:   // write-file
        cin >> _fd;
        cin >> str_to_write;
        fs->WriteToFile( _fd , str_to_write , strlen(str_to_write));
        break;

        case 7:    // read-file
        cin >> _fd;
        cin >> size_to_read ;
        fs->ReadFromFile( _fd , str_to_read , size_to_read );
        cout << "ReadFromFile: " << str_to_read << endl;
        break;

        case 8:   // delete file
        cin >> fileName;
        _fd = fs->DelFile(fileName);
        cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
        break;
        default:
            break;
    }
}
}