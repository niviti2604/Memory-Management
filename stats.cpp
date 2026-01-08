#include "stats.hpp"
#include <iostream>
#include <algorithm>

void Stats::report(const Memory& mem) {
    std::cout << std::dec;

    size_t total_memory = mem.getTotalSize();
    size_t used_memory = mem.getUsedSize();

    size_t total_free = 0;
    size_t largest_free = 0;
    size_t internal_fragmentation = 0;

    const Block* curr = mem.getHead();

    while (curr) {
        if (curr->free) {
            total_free += curr->size;
            largest_free = std::max(largest_free, curr->size);
        } else {
            internal_fragmentation += (curr->size - curr->requested_size);
        }
        curr = curr->next;
    }


double external_fragmentation =
    (total_free == 0) ? 0.0 :
    (1.0 - (static_cast<double>(largest_free) / total_free)) * 100.0;
      
      

    double memory_utilization =
        (double)used_memory / total_memory * 100.0;

    std::cout << "\n===== Memory Statistics =====\n";
    std::cout << "Total memory: " << total_memory << "\n";
    std::cout << "Used memory: " << used_memory << "\n";
    std::cout << "Free memory: " << total_free << "\n";

    std::cout << "Memory utilization: "
              << memory_utilization << "%\n";

    std::cout << "Internal fragmentation: "
              << internal_fragmentation << " bytes\n";

    std::cout << "External fragmentation: "
              << external_fragmentation << "%\n";

    std::cout << "==============================\n";
}
