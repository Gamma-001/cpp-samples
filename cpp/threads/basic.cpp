/**
 * Basics of thread
 *      A thread object cannot be copied (copy constuctor is deleted)
 *      A thread can be transfered with move semantics (without explicitly specifying move semantic in parameters, the thread object will be moved)
 *      A thread can be either waited for, or be detached from the parent process
 *      A Thread can be identified using ::id or this_thread::id
 *      All arguments >passed to a thread constructor are copied by default, std::move or std::ref should be used when required
 *      The number of actual hardware threads that can truly run concurrently can be fetched with std::thread::hardware_concurrency
 */

#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include "utils.hpp"

// Sort array in range [start, end)
template <typename T, size_t N>
void sortRange(std::array<T, N> &arr, int start, int end) {
    if (start < 0 || end < 0 || start > arr.size() - 1 && end > arr.size()) {
        std::cout << "Range out of bound, returning\n";
        return;
    }
    if (start >= end) {
        std::cout << "Empty or reversed range, returning\n";
        return;
    }

    std::sort(arr.begin() + start, arr.begin() + end);
}

// Merge two ranges of an array within the same array
// Ranges are not inclusive over end
template <typename T, size_t N>
void merge(std::array<T, N> &arr, int start, int split, int end) {
    if (!(start < split && split < end)) {
        std::cout << "Invalid merge range, returning\n";
        return;
    }

    // indices from split
    int s1 = start;
    int e1 = split;
    int s2 = split;
    int e2 = end;
    
    // Merge the arrays
    std::vector<int> merged;
    merged.reserve(end - start);
    while(s1 < e1 && s2 < e2) {
        if (arr[s1] < arr[s2]) {
            merged.push_back(arr[s1]);
            s1 += 1;
        }
        else {
            merged.push_back(arr[s2]);
            s2 += 1;
        }
    }
    while (s1 < e1) merged.push_back(arr[s1++]);
    while (s2 < e2) merged.push_back(arr[s2++]);

    // Assemble back into the source
    for (int i = start; i < end; i++) arr[i] = merged[i - start];
}

int main() {
    constexpr int dataSize = 128;
    std::array<int, dataSize> arr;
    std::cout << "Contents of array before range sorting: ";
    for (int i = 0; i < arr.size(); i++) {
        arr[i] = randInt(-100, 100);
        std::cout << arr[i] << ' ';
    }
    std::cout << '\n';

    size_t numThreads = std::max(2u, std::thread::hardware_concurrency());
    std::cout << "Number of thread: " << numThreads << '\n';
   
    // Divide the data per thread
    size_t sectionSize = arr.size() / numThreads;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++) {
        int start = i * sectionSize;
        int end = (i == numThreads - 1) ? arr.size() : (i + 1) * sectionSize;
        
        threads.push_back(std::thread(sortRange<int, dataSize>, std::ref(arr), start, end));
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    
    // Merge the sorted sections
    for (int i = 1; i < numThreads; i++) {
        int start = i * sectionSize;
        int end = (i == numThreads - 1) ? arr.size() : (i + 1) * sectionSize;
    
        merge(arr, 0, start, end);
    }

    // Sanity check
    for (int i = 0; i < arr.size() - 1; i++) {
        if (arr[i + 1] < arr[i]) {
            std::cout << "Sort failed, [" << i << ", " << i + 1 << "]: {" << arr[i] << ", " << arr[i + 1] << "}\n";
            break;
        }
    }

    // Contents of resultant array
    std::cout << "Contents of array after range sorting: ";
    for (auto &x: arr) std::cout << x << ' ';
    std::cout << '\n';

    return 0;
}
