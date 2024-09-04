#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport;

int main() {
    TransportCatalogue catalogue;
    
    ReadAndApply(catalogue, cin);

    ReadAndEval(catalogue, cin, cout);
}
