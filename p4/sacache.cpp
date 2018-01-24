#include <fstream>
#include <iostream>
#include <string>
#define LINE_SIZE 8
#define MEM_SIZE 65536
#define NUM_SETS 16
#define BYTES_LINE 4

using namespace std;
using std::string;

class cacheLine{
public:
    unsigned int dirty, tag;
    string data[BYTES_LINE]; // storage for the data
    int count;               // counter for least frequently used

    cacheLine() {				// constructor
        dirty = 0;
        tag = 0;
        for (int i = 0; i < BYTES_LINE; i++){
            data[i] = "OO";
            count = 0;
        }
    }

    void read(int prev_dirty, int hit, string data, ofstream &output){
        output << data << " " << hit << " " << prev_dirty << endl;
    }

    void write(int offset, string data){
        this->data[offset] = data;
        this->dirty = 1;
    }

};

class Set{
public:
    cacheLine lines[LINE_SIZE]; // set has 8 lines

    cacheLine* LRU(){   // function to get the least recently used item.
        int lru_index = 0;
        for ( int i = 1; i < LINE_SIZE; i++ ){      // find which one has the greatest counter, it will be the LRU
            if(lines[i].count > lines[lru_index].count){
                lru_index = i;
            }
        }
        lines[lru_index].count = 0;
        for ( int j = 0; j < LINE_SIZE; j++ ){
            if(j != lru_index)
                lines[j].count++;
        }
        return &(lines[lru_index]);             // return the line with the greatest counter
    }
    cacheLine* desiredLine(unsigned int tag){            // get the tag that you want.
        for (int i = 0; i < LINE_SIZE; i++){
            if (tag == lines[i].tag)
                return &(lines[i]);
        }
        return NULL;
    }

    bool inSet(unsigned int tag){
        for ( int i = 0; i < LINE_SIZE; i++ ){     // find out if the line is in the set
            if( tag == lines[i].tag ){
                return true;
            }
        }
        return false;
    }

    void updateCounter(unsigned int tag){
        for (int i = 0; i < LINE_SIZE; i++)
            if (tag == lines[i].tag)
                lines[i].count = 0;
            else
                lines[i].count++;
        return;
    }

};

int main( int argc, char* argv[])
{
    Set cache[NUM_SETS];		// cache
    string memory[MEM_SIZE];		// memory
    for ( int i = 0; i < MEM_SIZE; i++)
        memory[i] = "00";

    unsigned int address;
    string rw, data;

    ifstream data_file(argv[1]);			// input file
    ofstream out_file("sa-out.txt");

    while (data_file >> hex >> address >> rw >> data) {
        unsigned int offset = address & 3;           // last 2 bits
        unsigned int set = (address & 0x3F) >> 2;   // 4 bits after offset
        unsigned int tag = address >> 6;             // first 10 bits
        unsigned int hit = 1;
        unsigned int prev_dirty = 0;

        if(!cache[set].inSet(tag)){        // if it isnt in the set, then it is a miss
            hit = 0;

        }
        // if hit, then no need to for all this go straight to read/write
        if (!hit){       // if miss
            cacheLine *LRU = cache[set].LRU();
            prev_dirty = LRU->dirty;         // store previous dirty bit from the LRU
            int mem_addr = (LRU->tag << 6) | set;    // mem address index is based off the TAG & LINE, similar tags and line number alone are possible, but both at the same time are unique
            for ( int i = 0; i < BYTES_LINE; i++ )
                memory[mem_addr + i] = LRU->data[i];         // data in memory gets the value of the data in the cache line
            LRU->tag = tag;      // set the new tag
            LRU->dirty = 0;      // reset dirty bit to 0

            int mem_addr2 = address >> 2;   // mem address index based off TAG & LINE, take away the offset ( last 2 bits )
            for ( int i = 0; i < BYTES_LINE; i++ )
                LRU->data[i] = memory[mem_addr2 + i];
        } else {
            cache[set].updateCounter(tag);
        }

        cacheLine *workingLine = cache[set].desiredLine(tag);
        if(hit)
            prev_dirty = workingLine->dirty;

        // write
        if ( rw == "FF" ){
            workingLine->write(offset, data);
        }
        // read
        else if ( rw == "00" ){
            workingLine->read(prev_dirty, hit, data, out_file);
        }
    }

    data_file.close();
    out_file.close();

    return 0;
}
