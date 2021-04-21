#include "pagetable.h"
#include <cmath>
#include <set>
PageTable::PageTable(int page_size)
{
    _page_size = page_size;
    
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    int frame = -1; 
    // Find free frame
    std::map<std::string, int>::iterator it;
    std::set<int> frames;
    int i = 0; //used to find frame

    for( it = _table.begin(); it != _table.end(); it++){
        frames.insert(it->second);
    }

    while(frame == -1){
        if(frames.find(i) == frames.end()){ // entry not found, use this frame
            frame = i;
        }
        i++; //move one page of frame at the time.
    }
    _table[entry] = frame; // will crash when no frame is found
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    int n = (int) log2(_page_size);
    int page_number = virtual_address / _page_size; // virtual_address >> n;
    int page_offset = virtual_address % _page_size; //virtual_address & (_page_size - 1); 

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        int frame = _table.at(entry);
        address = frame * _page_size + page_offset;
    }

    return address;
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();
    for (i = 0; i < keys.size(); i++)
    {
        size_t sep1 = keys[i].find("|");
        uint32_t pid = std::stoi(keys[i].substr(0, sep1));
        int page = std::stoi(keys[i].substr(sep1 + 1));
        std::cout << pid << "  |\t\t   " << page << "|\t\t";
        std::cout << _table.at(keys[i]) << std::endl;
        
    }
}

int PageTable::getPageSize(){
    return _page_size;
}

int PageTable::getTableSize() {
    return _table.size();
}
int PageTable::getNextPage(uint32_t pid){
    int i = 0;
    std::string  entry = std::to_string(pid) + "|" + std::to_string(i); 
    while(_table.find(entry) != _table.end()){
        i++;//increment i until the next page is found  
        entry = std::to_string(pid) + "|" + std::to_string(i);
    } 
    return i;
}

int PageTable::countMatches(uint32_t pid, int page_number) {
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    int count = 0;

    if (_table.count(entry) > 0) {
        count++;
    }
    return count;
}