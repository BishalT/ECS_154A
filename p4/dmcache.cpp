#include <fstream>
#include <iostream>
#include <string>
#define LINE_SIZE 8
#define NUM_LINES 32
#define MEM_SIZE 65536
#define DEBUG 0

using namespace std;
using std::string;

class cacheLine{
public:
    int dirty, tag;
    string data[LINE_SIZE];

    cacheLine() {				// constructor
        dirty = 0;
        tag = 0;
        for (int i = 0; i < LINE_SIZE; i++)
            data[i] = "OO";
    }

    void read(int prev_dirty, int hit, string data, ofstream &output){
        output << data << " " << hit << " " << prev_dirty << endl;
        if (DEBUG)
            cout << data << " " << hit << " " << prev_dirty << endl;
    }

    void write(int offset, string data){
        this->data[offset] = data;
        this->dirty = 1;
    }

};


int main( int argc, char* argv[])
{
    cacheLine cache[NUM_LINES];		// cache
    string memory[MEM_SIZE];		// memory
    for ( int i = 0; i < MEM_SIZE; i++)
        memory[i] = "00";

    int address;
    string rw, data;

    ifstream data_file(argv[1]);			// input file
    ofstream out_file("dm-out.txt");

    while (data_file >> hex >> address >> rw >> data) {
        int offset = address & 7;           // last 3 bits
        int line = (address & 0xFF) >> 3;   // 5 bits after offset
        int tag = address >> 8;             // first 8 bits
        int prev_dirty = cache[line].dirty;         // store previous dirty bit

        int hit;                            // detect hit or a miss
        if (cache[line].tag != tag)         // if different tags, miss
            hit = 0;
        else                                // same tag, hit
            hit = 1;

        // if hit, then no need to for all this go straight to read/write
        if (!hit){       // if miss
            int mem_addr = (cache[line].tag << 5) | line;    // mem address index is based off the TAG & LINE, similar tags and line number alone are possible, but both at the same time are unique
            for ( int i = 0; i < LINE_SIZE; i++ )
                memory[mem_addr + i] = cache[line].data[i];         // data in memory gets the value of the data in the cache line
            cache[line].tag = tag;      // set the new tag
            cache[line].dirty = 0;      // reset dirty bit to 0

            int mem_addr2 = address >> 3;   // mem address index based off TAG & LINE, take away the offset ( last 3 bits )
            for ( int i = 0; i < LINE_SIZE; i++ )
                cache[line].data[i] = memory[mem_addr2 + i];
        }

        // write
        if ( rw == "FF" ){
            cache[line].write(offset, data);
        }
        // read
        else if ( rw == "00" ){
            cache[line].read(prev_dirty, hit, data, out_file);
        }
    }

    data_file.close();
    out_file.close();

    return 0;
}
