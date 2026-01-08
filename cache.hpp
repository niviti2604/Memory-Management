#ifndef CACHE_HPP
#define CACHE_HPP
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <cstdint>

struct CacheLine {
    uint64_t tag;
    bool valid;
    std::list<CacheLine*>::iterator lru_it;
    int freq;
};

class Cache {
public:
    Cache(size_t size, size_t block_size, int associativity, std::string policy);
    bool access(uint64_t address, bool& hit);
    void report() const;
    // void insert(int address); 
private:
    size_t size, block_size, sets;
    int associativity;
    std::string policy;
    std::vector<std::vector<CacheLine>> sets_data;
    std::list<CacheLine*> lru_list;
    std::unordered_map<uint64_t, int> lfu_map;
    int hits = 0, misses = 0;
    void replace(CacheLine& line, uint64_t tag);
    size_t capacity;
    std::vector<int> cache_lines;
};


#endif