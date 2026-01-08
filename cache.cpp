#include "cache.hpp"
#include <iostream>

Cache::Cache(size_t size, size_t block_size, int associativity, std::string policy)
    : size(size),
      block_size(block_size),
      associativity(associativity),
      policy(policy),
      hits(0),
      misses(0)
{
    sets = size / (block_size * associativity);
    sets_data.resize(sets, std::vector<CacheLine>(associativity));

    // Initialize cache lines
    for (auto& set : sets_data) {
        for (auto& line : set) {
            line.valid = false;
            line.freq = 0;
        }
    }
}

bool Cache::access(uint64_t address, bool& hit) {
    uint64_t block = address / block_size;
    uint64_t tag = block;
    size_t set_index = block % sets;

    auto& set = sets_data[set_index];

    // --------- HIT CHECK ----------
    for (auto& line : set) {
        if (line.valid && line.tag == tag) {
            hit = true;
            ++hits;

            if (policy == "LRU") {
                lru_list.splice(lru_list.end(), lru_list, line.lru_it);
            } else if (policy == "LFU") {
                ++line.freq;
            }
            return true;
        }
    }

    // --------- MISS ----------
    hit = false;
    ++misses;

    CacheLine* victim = nullptr;

    if (policy == "FIFO") {
        victim = &set[0];
    }
    else if (policy == "LRU") {
        victim = lru_list.front();
        lru_list.pop_front();
    }
    else if (policy == "LFU") {
        for (auto& line : set) {
            if (!line.valid || !victim || line.freq < victim->freq) {
                victim = &line;
            }
        }
    }

    replace(*victim, tag);
    return false;
}

void Cache::replace(CacheLine& line, uint64_t tag) {
    line.tag = tag;
    line.valid = true;

    if (policy == "LRU") {
        lru_list.push_back(&line);
        line.lru_it = std::prev(lru_list.end());
    }
    else if (policy == "LFU") {
        line.freq = 1;
    }
}

void Cache::report() const {
    std::cout
        << "Hits: " << hits
        << ", Misses: " << misses
        << ", Hit Ratio: "
        << ((hits + misses) > 0 ? hits / double(hits + misses) : 0)
        << std::endl;
}
