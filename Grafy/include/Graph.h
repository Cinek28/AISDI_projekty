#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>



//Kontener dla grafu przechowujący go w postaci wektorów sąsiedztwa:

class Graph
{
public:
    Graph();
    //Konstruktor tworzący graf z pliku:
    Graph(const char*);
    virtual ~Graph();
    int getNodes();
    int getEdges();
    //Funkcje zaprzyjaznione:
    friend void sort(Graph*);
    friend bool checkIsomorphy(Graph*, Graph*);
    friend bool checkPermutation(Graph*,Graph*);
private:
    int nodes;//liczba wierzcholkow
    int edges;//liczba krawedzi
    //Tablica wektorow sasiedztwa:
    std::vector<int>* graphNodes;
};

//Funkcja przekazywana do std::sort():
bool sortFun(int a, int b);

void sort(Graph* graph);


//Funkcja sprawdzajaca czy grafy sa izomorficzne:
bool checkIsomorphy(Graph* graphFirst, Graph* graphSecond);



#endif // GRAPH_H
