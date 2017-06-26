#include "Graph.h"
//Tablica permutacji (uzywana jest funkcja std::next_premutation do uzyskania wszystkich mozliwych
//permutacji jednego z grafow:
int* Value = nullptr;

Graph::Graph(): nodes(0), edges(0), graphNodes(nullptr) {};

Graph::Graph(const char* filename)
{
    std::ifstream file;

    //otwieramy pierwszy plik z grafem
    file.open( filename );
    if (!file.good())
    {
        std::cout << "Nie udalo sie otworzyc pliku z grafem!\n";
        return;
    }
    //wyciagamy informacje o grafie z pliku
    file >> nodes;
    edges = 0;
    graphNodes = new std::vector<int>[nodes];
    int a,b;
    while( file >> a >> b )
    {
        ++edges;
        graphNodes[a].push_back(b);
        if( a != b )
            graphNodes[b].push_back(a);
    }
    file.close();

};

Graph::~Graph()
{
    delete[] graphNodes;
};

int Graph::getNodes()
{
    return nodes;
};

int Graph::getEdges()
{
    return edges;
};


//Funkcja przekazywana do std::sort():
bool sortFun(int a, int b)
{
    return Value[a] < Value[b];
};

void sort(Graph* graph)
{
    int n = graph->getNodes();
    for(int i=0; i<n; ++i)
    {
        std::sort(graph->graphNodes[i].begin() , graph->graphNodes[i].end(), sortFun);
    }
}

//Funkcja sprawdzajaca czy po permutacji grafy sa identyczne:
bool checkPermutation(Graph* graphFirst,Graph* graphSecond)
{
    int n = graphFirst->getNodes();
    for(int i=0; i<n; ++i)
        {
            if(graphFirst->graphNodes[i].size() != graphSecond->graphNodes[Value[i]].size())
                return false;

            std::sort(graphFirst->graphNodes[i].begin() , graphFirst->graphNodes[i].end(), sortFun);

            for(int j=0; j<graphFirst->graphNodes[i].size(); ++j)
            {
                if( Value[ graphFirst->graphNodes[i][j] ] != graphSecond->graphNodes[Value[i]][j] )
                {
                    return false;
                }
            }
        }
    return true;
}

//Funkcja sprawdzajaca czy grafy sa izomorficzne:
bool checkIsomorphy(Graph* graphFirst, Graph* graphSecond)
{
    int n = graphFirst->nodes;
    //Sprawdzenie, czy zgadza sie liczba wierzcholkow i krawedzi:
    if(graphFirst->nodes != graphSecond->nodes || graphFirst->edges != graphSecond->edges)
    {
            std::cout << "Nieizomorficzne\n";
            return false;
    }
    //Posortowanie numerow krawedzi:
    for(int i=0; i<n; ++i)
    {
        std::sort(graphFirst->graphNodes[i].begin(), graphFirst->graphNodes[i].end());
        std::sort(graphSecond->graphNodes[i].begin(), graphSecond->graphNodes[i].end());
    }
    //Sprawdzenie, czy zgadzaja sie stopnie wierzcholkow:
    //Tablice stopni wiercholkow obu grafow:
    int degsGraphFirst[n];
    int degsGraphSecond[n];

    for(int i=0; i<n; ++i)
    {
        degsGraphFirst[i] = graphFirst->graphNodes[i].size();
        degsGraphSecond[i] = graphSecond->graphNodes[i].size();
    }
    //Posortowanie do sprawdzenia
    std::sort(degsGraphFirst, degsGraphFirst+n);
    std::sort(degsGraphSecond, degsGraphSecond+n);
    //Jezeli ktorys stopien sie nie zgadza to nie sa izomorficzne:
    for(int i=0; i<n; ++i)
        if(degsGraphFirst[i] != degsGraphSecond[i])
        {
            std::cout << "Nieizomorficzne\n";
            return false;
        }

    //Tworzenie tablicy permutacji:
    Value = new int[n];
    for(int i=0; i<n; ++i)
        Value[i]=i;
    //Sprawdzanie kazdej permutacji:
    do
    {
        if(checkPermutation(graphFirst,graphSecond)){
        //Wypisanie permutacji spelniajacej izomorfizm:
        std::cout << "Izomorficzne\n";
        for(int i=0; i < n; ++i)
            std::cout << i << " -> " << Value[i] << "\n";
        delete Value;
        return true;
        }
    }
    while(std::next_permutation(Value,Value+n));

    std::cout << "Nieizomorficzne\n";
    delete Value;
    return false;
};
