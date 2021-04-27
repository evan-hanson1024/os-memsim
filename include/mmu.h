#ifndef __MMU_H_
#define __MMU_H_

#include <iostream>
#include <string>
#include <vector>

enum DataType : uint8_t {FreeSpace, Char, Short, Int, Float, Long, Double};

typedef struct Variable {
    std::string name;
    DataType type;
    uint32_t virtual_address;
    uint32_t size;
} Variable;

typedef struct Process {
    uint32_t pid;
    std::vector<Variable*> variables;
} Process;

class Mmu {
private:
    uint32_t _next_pid;
    uint32_t _max_size;
    std::vector<Process*> _processes;
    void mergeFreeSpace(int i, int j);


public:
    Mmu(int memory_size);
    ~Mmu();

    uint32_t createProcess();
    void addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address);
    void print();
    std::vector<uint32_t> removeProcess(uint32_t pid);
    std::vector<Variable*> getVariables(uint32_t pid);
    std::vector<std::string> getProcesses();
    uint32_t getVirtualAddress(uint32_t pid, std::string var_name);
    DataType getDataType(uint32_t pid, std::string var_name);
    void removeVariable(uint32_t pid, std::string var_name);
    void shiftFreespace(uint32_t pid, uint32_t new_address);
    std::vector<Process*> getFullProcesses();
    bool isSpace(uint32_t pid, int var_size);
    int getNewAddress(uint32_t pid, int var_size, bool hasHole);
    void decreaseFreeSpace(uint32_t pid, uint32_t size);

    
};

#endif // __MMU_H