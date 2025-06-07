#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <cctype>
#include <stack>
#include <unordered_map>
#include <set>
#include <functional>
#include <iomanip>
#include "Parser.h"

int main() {
    std::string line;
    std::cout << "insert a proposicional expression or 'quit' to end:\n";

    while ( true ) {
        std::cout << "~~> ";
        std::getline( std::cin, line );

        if ( line == "sair" || line == "exit" || line == "x" ) break;
        if ( line.empty() ) continue;

        try {
            Parser p( line );
            p.parse();
            p.eval();
        }
        catch ( ... ) {
            std::cerr << "invalid expr.\n";
        }

        std::cout << "\n";
    }

    std::cout << "Tchau.\n";
    return 0;
}
