#include <iostream>
#include <Graph.h>
#include <algorithm>



int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "Zle argumenty programu!\n";
        std::cout << "Argumenty sa postaci:\n";
        std::cout << "<plik tekstowy grafu 1> <plik tekstowy grafu 2>\n";
        return -1;
    }
    Graph First(argv[1]);
    Graph Second(argv[2]);
    checkIsomorphy(&First,&Second);
    /*int* value = new int[3];
    for( int i =0;i<3;++i)
    {
        value[i] = i;
    }
    for(int j=0;j<4;j++){
        for(int i = 0; i< 3; ++i )
            std::cout << value[i] << "\t";
        std::cout << "\n";
        (std::next_permutation(value,value+3));
    }
    for(int i = 0; i< 3; ++i )
            std::cout << value[i] << "\t";
    return EXIT_SUCCESS;*/
    return 0;
}
