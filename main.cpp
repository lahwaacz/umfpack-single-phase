#include <string>
#include <iostream>
#include <fstream>
#include <limits>

#include "Solver.h"

using namespace std;

int main( int argc, char** argv )
{
    Solver s;
    bool status = s.run();

    ifstream meminfo("/proc/self/status");
    if (meminfo.fail()) {
        cerr << "error: unable to open /proc/self/status" << endl;
        return EXIT_FAILURE;
    }

    string desc;
    unsigned value;
    while (meminfo.good()) {
        // extract description (first column)
        meminfo >> desc;

        if( desc == "VmHWM:" ) {
            meminfo >> value;
            cout << "Peak memory usage: " << value / 1024 << " MiB" << endl;
        }

        // ignore the rest of irrelevant lines
        meminfo.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return !status;
}

