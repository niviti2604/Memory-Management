#include "memory.hpp"
#include "allocator.hpp"
#include "cache.hpp"
// #include "virtual_memory.hpp"
#include "stats.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
void accessCacheHierarchy(
    std::vector<std::unique_ptr<Cache>>& caches,
    uint64_t address,
    int& L1_hits, int& L1_misses,
    int& L2_hits, int& L2_misses
) {
    bool hit;

    // ----- L1 -----
    caches[0]->access(address, hit);
    if (hit) {
        L1_hits++;
        return;
    }
    L1_misses++;

    // ----- L2 -----
    if (caches.size() >= 2 && caches[1]) {
        caches[1]->access(address, hit);
        if (hit) {
            L2_hits++;
            // Bring block into L1
            bool dummy;
            caches[0]->access(address, dummy);
            return;
        }
        L2_misses++;
    }
}

int main()
{
    std::unique_ptr<Memory> mem;
    std::unique_ptr<Allocator> alloc;
    std::vector<std::unique_ptr<Cache>> caches;
    // std::unique_ptr<VirtualMemory> vm;
    int next_id = 1;
    int L1_hits = 0, L1_misses = 0;
    int L2_hits = 0, L2_misses = 0;

    std::cout << "Memory Management Simulator CLI" << std::endl;
    std::cout << "Type 'help' for commands or 'exit' to quit." << std::endl;

    std::string line;
    while (std::getline(std::cin, line))
    {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit")
            break;
        else if (cmd == "help")
        {
            std::cout << "Commands:" << std::endl;
            std::cout << "  init memory <size>" << std::endl;
            std::cout << "  set allocator <first_fit|best_fit|worst_fit>" << std::endl;
            std::cout << "  malloc <size>" << std::endl;
            std::cout << "  free <id>" << std::endl;
            std::cout << "  dump memory" << std::endl;
            std::cout << "  stats" << std::endl;
            std::cout << "  init cache <level> <size> <block_size> <associativity> <policy>" << std::endl;
            // std::cout << "  init virtual <va_size> <page_size> <phys_size> <page_policy>" << std::endl;
            // std::cout << "  access <va>" << std::endl;
            std::cout << "  exit" << std::endl;
        }
        else if (cmd == "init" && iss >> cmd)
        {
            if (cmd == "memory")
            {
                size_t size;
                if (iss >> size)
                {
                    mem = std::make_unique<Memory>(size);
                    std::cout << "Memory initialized with size " << size << std::endl;
                }
                else
                    std::cout << "Error: Invalid memory size" << std::endl;
            }
            else if (cmd == "cache")
            {
                int level;
                size_t size, block_size;
                int associativity;
                std::string policy;
                if (iss >> level >> size >> block_size >> associativity >> policy)
                {
                    if (level >= 1 && level <= 3)
                    {
                        while (static_cast<int>(caches.size()) < level)
                            caches.emplace_back(nullptr);
                        caches[level - 1] = std::make_unique<Cache>(size, block_size, associativity, policy);
                        std::cout << "Cache L" << level << " initialized" << std::endl;
                    }
                    else
                        std::cout << "Error: Invalid cache level (1-3)" << std::endl;
                }
                else
                    std::cout << "Error: Invalid cache parameters" << std::endl;
            }
            
            else
                std::cout << "Error: Unknown init subcommand" << std::endl;
        }
        else if (cmd == "set" && iss >> cmd)
        {
            if (cmd == "allocator")
            {
                std::string type;
                if (iss >> type && mem)
                {
                    if (type == "first_fit")
                        alloc = std::make_unique<FirstFit>();
                    else if (type == "best_fit")
                        alloc = std::make_unique<BestFit>();
                    else if (type == "worst_fit")
                        alloc = std::make_unique<WorstFit>();
                    else if (type == "buddy")
                        alloc = std::make_unique<BuddyAllocator>(mem->getTotalSize());
                    else
                    {
                        std::cout << "Error: Unknown allocator type" << std::endl;
                        continue;
                    }
                    std::cout << "Allocator set to " << type << std::endl;
                }
                else
                    std::cout << "Error: Initialize memory first" << std::endl;
            }
            else
                std::cout << "Error: Unknown set subcommand" << std::endl;
        }
        else if (cmd == "malloc")
        {
            size_t size;
            if (iss >> size && mem && alloc)
            {
                Block *block = alloc->allocate(*mem, size, next_id);
                if (block)
                {
                    std::cout << "Allocated block id=" << next_id << " at address=0x" << std::hex << reinterpret_cast<uintptr_t>(block) << std::dec << std::endl;
                    ++next_id;
                }
                else
                    std::cout << "Allocation failed" << std::endl;
            }
            else
                std::cout << "Error: Initialize memory and allocator first" << std::endl;
        }
        else if (cmd == "free")
        {
            int id;
            if (iss >> id && mem)
            {
                mem->deallocate(id);
                std::cout << "Block " << id << " freed and merged" << std::endl;
            }
            else
                std::cout << "Error: Initialize memory first" << std::endl;
        }
        else if (cmd == "dump" && iss >> cmd)
        {
            if (cmd == "memory" && mem)
                mem->dump();
            else
                std::cout << "Error: Initialize memory first" << std::endl;
        }
        else if (cmd == "stats")
        {
            if (mem)
                Stats::report(*mem);
            else
                std::cout << "Error: Initialize memory first" << std::endl;
            if (caches.size() >= 1 && caches[0])
            {
                std::cout << "L1 Cache Hits: " << L1_hits << std::endl;
                std::cout << "L1 Cache Misses: " << L1_misses << std::endl;
            }
            if (caches.size() >= 2 && caches[1])
            {
                std::cout << "L2 Cache Hits: " << L2_hits << std::endl;
                std::cout << "L2 Cache Misses: " << L2_misses << std::endl;
            }
        }
        // else if (cmd == "access")
        // {
        //     uint64_t va;
        //     if (iss >> va && vm)
        //     {
        //         bool page_fault;
        //         uint64_t pa = vm->translate(va, page_fault);
        //         accessCacheHierarchy(
        //             caches,
        //             pa,
        //             L1_hits, L1_misses,
        //             L2_hits, L2_misses);
        //         std::cout << "Virtual Address 0x" << std::hex << va << " -> Physical Address 0x" << pa << std::dec;
        //         if (page_fault)
        //             std::cout << " (Page Fault)";
        //         std::cout << std::endl;
        //     }
        //     else
        //         std::cout << "Error: Initialize virtual memory first" << std::endl;
        // }
        else
            std::cout << "Error: Unknown command" << std::endl;
    };

    std::cout << "Exiting simulator." << std::endl;
    return 0;
}