#include <cstddef>
#include <cstdlib>
#include <string>
#include <chrono>
#include <random>
#include <iostream>
#include "TreeMap.h"
#include "HashMap.h"

template<class Collection>
void randInsert(int n) {
    Collection map;
    std::mt19937 seed;
    std::uniform_int_distribution<int> distribution(0, n);
    for (int i = 0; i < n; ++i) {
        map[distribution(seed)] = i;
    }
}

template<class Collection>
void randAccess(int n) {
    Collection map;
    for (int i = 0; i < n; ++i) {
        map[i] = i;
    }

    std::mt19937 seed;
    std::uniform_int_distribution<int> distribution(0, n);
    auto Start = std::chrono::steady_clock::now();
    for(int i =0; i < n; ++i){
        map[distribution(seed)];
    }
    auto End = std::chrono::steady_clock::now();
    auto diff = End - Start;
    std::cout << "Random Access: Elements "<<n<<", Time: "<<std::chrono::duration <double, std::nano> (diff).count() << " ns" << std::endl;
}



int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  auto Start = std::chrono::steady_clock::now();
  auto End = std::chrono::steady_clock::now();
  auto diff = End - Start;
  for (std::size_t i = 100; i <= 100000; i*=10){
      Start = std::chrono::steady_clock::now();
      randInsert<aisdi::HashMap<int, int>>(i);
      End = std::chrono::steady_clock::now();
      diff = End - Start;
      std::cout << "HashMap: Elements "<<i<<", Time: "<<std::chrono::duration <double, std::nano> (diff).count() << " ns" << std::endl;
      randAccess<aisdi::HashMap<int, int>>(i);
      Start = std::chrono::steady_clock::now();
      randInsert<aisdi::TreeMap<int, int>>(i);
      End = std::chrono::steady_clock::now();
      diff = End - Start;
      std::cout << "TreeMap: Elements "<<i<<", Time: "<<std::chrono::duration <double, std::nano> (diff).count() << " ns" << std::endl;
      randAccess<aisdi::HashMap<int, int>>(i);
  }

  return 0;
}
