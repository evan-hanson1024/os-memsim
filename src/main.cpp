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
void splitArgument(std::string s, std::vector<std::string> &v);
void printVector(std::vector<std::string> v);
int getVariableSize(DataType type, uint32_t num_elements);
DataType stodt(std::string in);
void printVariable(void *memory, int physical_address, DataType type, int size);

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
    uint32_t pid;
    std::string var_name;
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
        }else if(v[0] == "allocate"){
            pid = stoi(v[1]);
            var_name = v[2];
            DataType type = stodt(v[3]);
            uint32_t num_elems = stoi(v[4]);
            allocateVariable(pid, var_name, type, num_elems, mmu, page_table);
        }else if(v[0] == "set"){
            pid = stoi(v[1]);
            var_name = v[2];
            uint32_t offset = stoi(v[3]);
            DataType type = mmu->getDataType(pid, var_name);
            for(int i = 4; i < v.size(); i++){
                void * value;
                int type_size = 0;
                if(type == DataType::Int || type == DataType::Float){
                    value = malloc(4);
                    value = (void*)stoi(v[i]);
                    offset *= 4;
                    type_size = 4;
                }else if(type == DataType::Long) {
                    value = malloc(8);
                    long in = stol(v[i]);
                    memcpy(&value, &in, 8);
                    offset *= 8;
                    type_size = 8;

                }else if(type == DataType::Double){
                    value = malloc(8);
                    double in = stod(v[i]);
                    memcpy(value, &in, 8);

                    offset *= 8;
                    type_size = 8;
                }else if(type == DataType::Short){
                    value = malloc(2);
                    value = (void*)stoi(v[i]);
                    offset *= 2;
                    type_size = 2;
                }else{
                    value = malloc(1);
                    value = (void *)v[i][0]; 
                    type_size = 1;
                }

                setVariable(pid, var_name, offset, value, mmu, page_table, memory);
                offset+=type_size;
            }
        }else if(v[0] == "free"){
            pid = stoi(v[1]);
            var_name = v[2];
            freeVariable(pid, var_name, mmu, page_table);
        }else if(v[0] == "terminate"){
            pid = stoi(v[1]);
            terminateProcess(pid, mmu, page_table);
        }else if(v[0] == "print"){
            if (v[1] == "processes") {
                //Print all PIDs
                std::vector<std::string> processes = mmu->getProcesses();
                for (int i = 0; i < processes.size(); i++) {
                    std::cout << processes[i] << std::endl;
                }
            } else if (v[1] == "mmu") {
                //Print MMU values
                mmu->print();
            } else if (v[1] == "page") {
                //Print page table values
                page_table->print();
            } else {
                //Print the value of the variable for a given PID
                std::vector<std::string> arguments;
                std::vector<Process*> processes = mmu->getFullProcesses();
                splitArgument(v[1], arguments);

                for (int process_index = 0; process_index < processes.size(); process_index++) {
                    if (processes[process_index]->pid == stoul(arguments[0])) {
                        pid = processes[process_index]->pid;
                        std::vector<Variable*> variables = mmu->getVariables(processes[process_index]->pid);
                        for (int variable_index = 0; variable_index < variables.size(); variable_index++) {
                            if (variables[variable_index]->name == arguments[1]) {
                                var_name = arguments[1];
                                DataType type = mmu->getDataType(pid, var_name);
                                int physical_address = page_table->getPhysicalAddress(stoi(arguments[0]), variables[variable_index]->virtual_address);
                                printVariable(memory, physical_address, type, variables[variable_index]->size);
                            }
                        }
                    }
                }
            }
        } else if (command.size() > 0) {
            printf("error: command not recognized\n");
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
    allocateVariable(processPID, std::string("<TEXT>"), DataType::Char, text_size, mmu, page_table);
    allocateVariable(processPID, std::string("<GLOBALS>"), DataType::Char, data_size, mmu, page_table);
    allocateVariable(processPID, std::string("<STACK>"), DataType::Char, 65536, mmu, page_table);
    //   - print pid
    //mmu->print();
    std::cout << processPID << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    bool holeFound = false;
    int i;
    int pageSize = page_table->getPageSize();
    int page = -1;
    int variable_size = getVariableSize(type, num_elements);
    std::vector<Variable*> variables = mmu->getVariables(pid);
    int address = 0;
    int count = 0;

    //go through every variable in a process to check if there is room at the same page
    // do this with the virtual addresses?
    // check distance between previous and current variable
    // if the distance is >= type * num_elements we have a room between the variables
    // then we can insert there
    // if not we continue to the next page
    if(mmu->isSpace(pid, variable_size)){
        address = mmu->getNewAddress(pid, variable_size, true);
        holeFound = true;
    }else{
        address = mmu->getNewAddress(pid, variable_size, false);
    }
    

    //check the last hole
    if(!holeFound){
        int old_address = 0;
        if(variables.size() > 0) {
            old_address = variables[variables.size() - 1]->virtual_address;
        }
        int space_left = pageSize - (address % pageSize);
        if(address/pageSize == old_address/pageSize && space_left >= variable_size){ //same page and room for the variable
            holeFound = true; // holeFound for entire variable
        }else if(space_left < getVariableSize(type, 1)){
                mmu->addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, space_left, address);
                address += space_left;
        }
    }
    //   - if no hole is large enough, allocate new page(s)
    if (!holeFound) {
        for(int i = page_table->getNextPage(pid); i <= (address + variable_size)/pageSize; i++){
            page = i;
            page_table->addEntry(pid, page);
        }
    }
    //   - insert variable into MMU
    mmu->addVariableToProcess(pid, var_name, type, variable_size, address);

    //mmu->shiftFreespace(processPID, text_size+data_size+65536);
    //   - print virtual memory address
    if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
        std::cout << address << std::endl;
    }

}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    int physical_address = page_table->getPhysicalAddress(pid, mmu->getVirtualAddress(pid, var_name) + offset);
    if(physical_address == -1){
        std::cout << "error: variable not found"<< std::endl;
        return; // return early so no damage is done.
    }
    DataType type = mmu->getDataType(pid, var_name);
    char * memory_location = (char *)(memory) + physical_address;
    if(type == DataType::Char){
        memcpy(memory_location, &value, 1);
    }else if (type == DataType::Short){
        memcpy(memory_location, &value, 2);
    }else if(type == DataType::Int || type == DataType::Float){
        memcpy(memory_location, &value, 4);
    }else if(type == DataType::Long || type == DataType::Double){
        memcpy(memory_location, &value, 8);
    }
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    if(mmu->getVirtualAddress(pid, var_name) == -1){
        std::cout << "error: variables doesn't exist" << std::endl;
        return;
    }
    mmu->removeVariable(pid, var_name);
    //   - free page if this variable was the only one on a given page
    bool shared_page = false;
    
    int page = mmu->getVirtualAddress(pid, var_name) / page_table->getPageSize();
    std::vector<std::string> pages = page_table->getPages(pid);
    std::vector<Variable*> variables = mmu->getVariables(pid);
    for(int i = 0; i < variables.size() && !shared_page; i++){
        if(page == variables[i]->virtual_address / page_table->getPageSize()){
            shared_page == true;
        }
    }
    if(!shared_page){
        page_table->freePage(pid, page);
    }
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    std::vector<uint32_t> virtualAddresses = mmu->removeProcess(pid);
    //   - free all pages associated with given process
    uint32_t virual_address;
    for (int i = 0; i < virtualAddresses.size(); i++) {
        //TODO: free all pages
        virual_address = virtualAddresses[i];
        page_table->freePage(pid, virual_address/page_table->getPageSize());
    }
}

int getVariableSize(DataType type, uint32_t num_elements){
    int value = 0;
    //not sure how c++ switch syntax works, might need to change this
    if(type == DataType::Char){
        value = num_elements;
    }else if( type == DataType::Short){
        value = 2 * num_elements;
    }else if (type == DataType::Int || type == DataType::Float){
         value = 4 * num_elements;
    }else if(type == DataType::Long || type == DataType::Double){
        value = 8 * num_elements;
    }
    return value;

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

void splitArgument(std::string s, std::vector<std::string> &v) {
	
	std::string temp = "";
	for(int i=0; i < s.length(); i++){
		
		if(s[i]==':'){
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


 void printVariable (void* memory, int physical_address, DataType type, int size){
    int offset;
    int amount = 0;
    bool overflow = false;
    if(type == DataType::Int){
        for(offset = 0; offset < 5*4; offset += 4){
            int * memory_location = (int *)(memory) + physical_address + offset;
            std::cout << *memory_location << ", ";
            
        }
        if(offset > 5*4 - 1){
            overflow = true;
            amount = size/4;
        }
    }else if(type == DataType::Float){
        for(offset = 0; offset < 5*4; offset += 4){
            float * memory_location = (float *)(memory) + physical_address + offset;
            std::cout << *memory_location << ", ";
        }
        if(offset > 5*4 - 1){
            overflow = true;
            amount = size/4;
        }
    }
    else if(type == DataType::Long ){
        for(offset = 0; offset < size;offset += 8){
            char * memory_location = (char *)(memory) + physical_address + offset;
            long value;
            memcpy(&value, memory_location, 8);
            std::cout << value << ", ";
        }
        if(offset > 5*8-1){
            overflow = true;
            amount = size/8;
        }
    }else if(type == DataType::Double){
        for(offset = 0; offset < 5*8;offset += 8){
            char * memory_location = (char *)(memory) + physical_address + offset;
            double value;
            memcpy(&value, memory_location, 8);
            std::cout << value << ", ";
        }
        if(offset > 5*8-1){
            overflow = true;
            amount = size/8;
        }
    }else if(type == DataType::Short){
        for(offset = 0; offset < 5*2;offset += 2){
            short * memory_location = (short *)(memory) + physical_address + offset;
            std::cout << *memory_location << ", ";
        }
        if(offset > 5*2-1){
            overflow = true;
            amount = size/2;
        }
    }else{
        for(offset = 0; offset < 5;offset += 1){
            char * memory_location = (char *)(memory) + physical_address + offset;
            std::cout << *memory_location << ", ";
        }
        if(offset > 5-1){
            overflow = true;
            amount = size;
        }
    }
    if(overflow){
        std::cout << "... [" << amount << " items]"; 
    }
    std::cout << std::endl;
 }


DataType stodt(std::string in){
    DataType out;
    if(in == "int"){
        out = DataType::Int;
    }else if(in == "char"){
        out = DataType::Char;
    }else if(in == "long"){
        out = DataType::Long;
    }else if(in == "short"){
        out = DataType::Short;
    }else if(in == "float"){
        out = DataType::Float;
    }else if(in == "double"){
        out = DataType::Double;
    }else{
        out = DataType::FreeSpace;
    }
    return out;
}