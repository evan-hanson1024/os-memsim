#include "mmu.h"
#include  <iomanip>
#include <algorithm>
Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

std::vector<std::string> Mmu::getProcesses() {
    std::vector<std::string> v;
    for (int i = 0; i < _processes.size(); i++) {
        v.push_back(std::to_string(_processes[i]->pid));
    }
    return v;
}

std::vector<Process*> Mmu::getFullProcesses() {
    std::vector<Process*> v;
    for (int i = 0; i < _processes.size(); i++) {
        v.push_back(_processes[i]);
    }
    return v;
}

void Mmu::shiftFreespace(uint32_t pid, uint32_t new_address) {
    int i;
    int j;
    for (i = 0; i < _processes.size(); i++) {
        if (_processes[i]->pid == pid) {
            for (j = 0; j < _processes[i]->variables.size(); j++) {
                if (std::string(_processes[i]->variables[j]->name) == std::string("<FREE_SPACE>")) {
                    Variable *var = _processes[i]->variables[j];
                    _processes[i]->variables.erase(_processes[i]->variables.begin() + j);
                    std::cout << "HERE" << std::endl;
                    var->virtual_address = new_address;
                    _processes[i]->variables.push_back(var);
                }
            }
        }
    }
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }

    Variable *var = new Variable();
    var->name = var_name;
    var->type = type;
    var->virtual_address = address;
    var->size = size;
    if (proc != NULL)
    {
        decreaseFreeSpace(pid, size);
        proc->variables.push_back(var);
    }
}

void Mmu::decreaseFreeSpace(uint32_t pid, uint32_t size){
    Process *proc = NULL;
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    for(int j = 0; j < proc->variables.size(); j++){
        if(proc->variables[j]->name == "<FREE_SPACE>"){
            if(size == proc->variables[j]->size){
                proc->variables.erase(proc->variables.begin() + j);
                return; // better return after changing the var vector
            }else{
                proc->variables[j]->size -= size; //remove from free space
                proc->variables[j]->virtual_address += size;
            }
        }
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;

    size_t headerWidth[4] ={
        std::string(" PID  ").size(),
        std::string(" Variable Name ").size(),
        std::string(" Virtual Addr ").size(),
        std::string(" Size").size()
    };

    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // TODO: print all variables (excluding <FREE_SPACE> entries)
            if(_processes[i]->variables[j]->name != "<FREE_SPACE>" ){
                std::cout << std::left << std::setw(headerWidth[0]) << _processes[i]->pid << "|";
                std::cout << std::setw(headerWidth[1]) << _processes[i]->variables[j]->name << "|";
                std::cout << std::right << std::setw(headerWidth[2] - 8)<< "0x" << std::setfill('0') << std::setw(8) << std::hex << _processes[i]->variables[j]->virtual_address << "|";
                std::cout << std::setfill(' ') << std::setw(headerWidth[3]) << std::dec << _processes[i]->variables[j]->size << std::endl;
            }
        }
    }
}

bool compareVariables(Variable *a, Variable *b){
    return a->virtual_address < b->virtual_address;
}

std::vector<Variable*> Mmu::getVariables(uint32_t pid){
    std::vector<Variable*> variables;
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; j < _processes[i]->variables.size(); j++){
                if(_processes[i]->variables[j]->name != "<FREE_SPACE>"){
                    variables.push_back(_processes[i]->variables[j]);
                }
            }
        }
    }
    std::sort(variables.begin(), variables.end(), compareVariables); // have all the variables in sorted order
    return variables;
}

uint32_t Mmu::getVirtualAddress(uint32_t pid, std::string var_name){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; j < _processes[i]->variables.size(); j++){
                if(_processes[i]->variables[j]->name == var_name){
                    return _processes[i]->variables[j]->virtual_address;
                }
            }
        }
    }
    return -1; //return -1 value, no address found. 
}

DataType Mmu::getDataType(uint32_t pid, std::string var_name){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; j < _processes[i]->variables.size(); j++){
                if(_processes[i]->variables[j]->name == var_name){
                    return _processes[i]->variables[j]->type;
                }
            }
        }
    }
    return DataType::FreeSpace; // shouldnt happen
}



std::vector<uint32_t> Mmu::removeProcess(uint32_t pid){
    std::vector<uint32_t> virtual_adresses;
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; j < _processes[i]->variables.size(); j++){
                virtual_adresses.push_back(_processes[i]->variables[j]->virtual_address);
            }
            _processes.erase(_processes.begin()+i);
            i--;
        }
    }
    return virtual_adresses;
}

void Mmu::removeVariable(uint32_t pid, std::string var_name){
    for(int i = 0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            for(int j = 0; _processes[i]->variables.size(); j++){
                if(_processes[i]->variables[j]->name == var_name){
                    _processes[i]->variables[j]->name = "<FREE_SPACE>";
                    _processes[i]->variables[j]->type = DataType::FreeSpace;
                    mergeFreeSpace(i, j);
                    return; //just return out of the function after the variable is removed
                }
            }
        }
    }
}

void Mmu::mergeFreeSpace(int i, int j){
    if(j > 0 && _processes[i]->variables[j - 1]->name == "<FREE_SPACE>"){
         _processes[i]->variables[j]->size += _processes[i]->variables[j - 1]->size;
         _processes[i]->variables[j]->virtual_address = _processes[i]->variables[j - 1]->virtual_address;
         _processes[i]->variables.erase(_processes[i]->variables.begin() + j - 1);
         j--;
    }

    if(j < _processes[i]->variables.size() - 1 && _processes[i]->variables[j + 1]->name == "<FREE_SPACE>"){
        _processes[i]->variables[j]->size += _processes[i]->variables[j + 1]->size;
        _processes[i]->variables.erase(_processes[i]->variables.begin() + j + 1);
    }
}

bool Mmu::isSpace(uint32_t pid, int var_size){
    int space_between = 0;
    std::vector<Variable*> variables = getVariables(pid);
    for (int i = 1; i < variables.size(); i++) {
        space_between = variables[i]->virtual_address - variables[i - 1]->virtual_address - variables[i - 1]->size; // check if there is space between
        if (space_between >= var_size) { //the space between two variables are larger than the variable size
            //Found a page already allocated to this process
            return true;
        }
    }
    return false;
}
int Mmu::getNewAddress(uint32_t pid, int var_size, bool hasHole){
    std::vector<Variable*> variables = getVariables(pid);
    int space_between = 0;
    int address = 0;
    if(variables.size() == 0){ // no variables
        return address; //new address should be 0
    }

    if(hasHole){
        for (int i = 1; i < variables.size() && address == 0; i++) {
        space_between = variables[i]->virtual_address - variables[i - 1]->virtual_address - variables[i - 1]->size; // check if there is space between
            if (space_between >= var_size) { //the space between two variables are larger than the variable size
                //Found a page already allocated to this process
                address = variables[i - 1]->virtual_address + variables[i - 1]->size;
            }
        }
    }else{
        address = variables[variables.size() - 1]->virtual_address + variables[variables.size() - 1]->size;
    }
    return address;
}