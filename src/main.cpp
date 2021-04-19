#include <iostream>
#include <string>
#include <cstring>
#include <bits/stdc++.h>
#include "mmu.h"
#include "pagetable.h"

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);
void splitString(std::string s, std::vector<std::string> &v);
void printVector(std::vector<std::string> v);

std::vector<std::string> getv(std::string command);


int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        fprintf(stderr, "Error: you must specify the page size\n");
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory'
    uint32_t mem_size = 67108864;
    void *memory = malloc(mem_size); // 64 MB (64 * 1024 * 1024)

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(mem_size);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline (std::cin, command);
    std::vector<std::string> v;
    int text_size;
    int data_size;
    while (command != "exit") {
        // Handle command
        // TODO: implement this!
        v.clear();
        splitString(command, v);
        //printVector(v);
        if (v[0] == std::string("create")) {
            if (v.size() != 3) {
                printf("Create must have 3 arguments");
            } else {
                text_size = std::stoi(v[1]);
                data_size = std::stoi(v[2]);
                createProcess(text_size, data_size, mmu, page_table);
            }
        }else if(v[0] == std::string("allocate")){

        }else if(v[0] == "set"){

        }else if(v[0] == "free"){
            freeVariable(stoi(v[1]), v[2], mmu, page_table);
        }else if(v[0] == "terminate"){
            terminateProcess(stoi(v[1]), mmu, page_table);
        }else if(v[0] == "print"){
            
        }


        // Get next command
        std::cout << "> ";
        std::getline (std::cin, command);
    }

    // Clean up
    free(memory);
    delete mmu;
    delete page_table;

    return 0;
}


std::vector<std::string> getv(std::string command){
    std::stringstream ss(command);
    std::string word;
    std::vector<std::string> v;
    while(ss >> word){
        v.push_back(word);
    }
    return v;
}
void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - create new process in the MMU
    uint32_t processPID;
    processPID = mmu->createProcess();
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    allocateVariable(processPID, std::string("<TEXT>", DataType::Char, text_size, mmu, page_table));
    allocateVariable(processPID, std::string("<GLOBALS>", DataType::Char, data_size, mmu, page_table));
    allocateVariable(processPID, std::string("<STACK>", DataType::Char, 65536, mmu, page_table));
    //   - print pid
    std::cout << processPID << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    //   - if no hole is large enough, allocate new page(s)
    //   - insert variable into MMU
    //   - print virtual memory address
    

}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    mmu->removeVariable(pid, var_name);
    //   - free page if this variable was the only one on a given page
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    std::vector<uint32_t> virtualAddresses = mmu->removeProcess(pid);
    //   - free all pages associated with given process
    for(int i = 0; i < virtualAddresses.size(); i++){
        //TODO: free all pages
    }
}

void splitString(std::string s, std::vector<std::string> &v) {
	
	std::string temp = "";
	for(int i=0; i < s.length(); i++){
		
		if(s[i]==' '){
			v.push_back(temp);
			temp = "";
		}
		else{
			temp.push_back(s[i]);
		}
		
	}
	v.push_back(temp);
	
}

void printVector(std::vector<std::string> v) {
	for(int i=0; i < v.size(); i++)
		std::cout << v[i] << std::endl;
	std::cout << "\n";
}
