#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport;

int main() {
    TransportCatalogue catalogue;
    
    {
        InputReader reader;
        for (int i = 0; i < reader.ReadCount(cin); ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    for (int i = 0; i < reader.ReadCount(cin); ++i) {
        string line;
        getline(cin, line);
        ParseAndPrintStat(catalogue, line, cout);
    }
}
