#include <fstream>
#include <iostream>
#include <string>
#define PAGE_TABLE_SIZE 32
#define RAM_SIZE 4
#define DEBUG 0

using namespace std;
using std::string;

class address{
public:
    unsigned long int addr;
    int used;

    address(){
        addr = 0;
        used = 0;
    }

    void setUsed(){ used = 1; }
    void printAddr(){ cout << "Address: " << hex << addr << endl; }
};



class pageTable{
public:
    address table[PAGE_TABLE_SIZE];    // array of size 32 filled with page addresses

    int findIndex(unsigned long int address){
        unsigned long int addr = address &0xFFFFF000;       // last 12 bits are offsets, unnecessary
        for(int i = 0; i < PAGE_TABLE_SIZE; i++){
            unsigned long int pageAddr = table[i].addr & 0xFFFFF000;
            if(pageAddr == addr)
                return i;
        }
        return -1;
    }

};


class RAM {
public:
    address table[RAM_SIZE];         // array of size 4 filled with addresses
    int star_index;
    RAM() {
        star_index = 0;
    }

    int star(){ return star_index; }

    bool inMem(unsigned long int address){  // find if the address is in Memory
        for( int i = 0; i < RAM_SIZE; i++){
            if( table[i].addr == address )
                return true;
        }
        return false;
    }

    int findIndex(unsigned long int address){
        for(int i = 0; i < RAM_SIZE; i++){
            if(table[i].addr == address)
                return i;
        }
        return -1;
    }

    void printRAM(){
        for (int i = 0; i < RAM_SIZE; i++){
            if(table[i].addr){
                if(i) cout << " ";
                cout << hex << table[i].addr;
            }
        }
        cout << endl;
    }

    void printFile(ofstream &output){
        for (int i = 0; i < RAM_SIZE; i++){
            if(table[i].addr){
                if(i) output << " ";
                output << hex << table[i].addr;
            }
        }
        output << endl;
    }

};

int main(int argc, char *argv[])
{
    ifstream data_file(argv[1]);			// input file
    ofstream out_file("vm-out.txt");        // output file

    RAM memory;                  // address array of size 4
    pageTable page;

    unsigned long int addr;
    for(int i = 0; i < PAGE_TABLE_SIZE; i++){
        data_file >> hex >> addr;
        page.table[i].addr = addr;
        //page.table[i].printAddr();
    }

    while( data_file >> hex >> addr ){
        int pageIndex = page.findIndex(addr);
        unsigned long int pg = page.table[pageIndex].addr;


        if(memory.inMem(pg)){    // if present in RAM, similar to cache hit
            int memIndex = memory.findIndex(pg);
            memory.table[memIndex].used = 1;
        }
        else{                       // if not present in RAM, similar to cache miss
            while(!memory.inMem(pg)){  // while the address isn't in memory
                int index = memory.star() % RAM_SIZE;   // star might go too far, so mod 4.
                if(memory.table[index].used){           // if used = 1
                    memory.table[index].used = 0;       // set used to 0
                    index++;                            // shift index
                }
                else{          // if used bit is 0;
                    memory.table[index].addr = pg;    // give the addr its spot in memory
                    memory.table[index].used = 1;
                    index++;
                    if(DEBUG)
                        cout << "INSERTED: " << hex << memory.table[index-1].addr << endl;
                }
                memory.star_index = index;          // give updated index back to memory
                if(DEBUG){
                    cout << "STAR INDEX: " << dec << memory.star_index << endl;
                    cout << endl;
                }
            }
        }
        if(DEBUG)
            memory.printRAM();
        memory.printFile(out_file);
    }

    data_file.close();
    out_file.close();

    return 0;
}
