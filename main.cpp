#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#include <getopt.h>

#include "Solver.h"

using namespace std;

bool parse_options( int argc,
                    char** argv,
                    IndexType & size_x,
                    IndexType & size_y,
                    RealType & time_step,
                    RealType & time_step_order )
{
    int c;
    while (1) {
        static struct option long_options[] = {
            { "size-x", required_argument, 0, 'x' },
            { "size-y", required_argument, 0, 'y' },
            { "time-step", required_argument, 0, 't' },
            { "time-step-order", required_argument, 0, 'o' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long( argc, argv, "", long_options, NULL );

        // detect end of options
        if( c == -1 )
            break;

        switch (c) {
            case 'x':
            {
                stringstream ss(optarg);
                ss >> size_x;
                break;
            }
            case 'y':
            {
                stringstream ss(optarg);
                ss >> size_y;
                break;
            }
            case 't':
            {
                stringstream ss(optarg);
                ss >> time_step;
                break;
            }
            case 'o':
            {
                stringstream ss(optarg);
                ss >> time_step_order;
                break;
            }
            default:
            {
                cerr << "parsing error";
                return false;
            }
        }
    }

    if( size_x <= 0 ) {
        cerr << "size-x must be positive integer" << endl;
        return false;
    }
    if( size_y <= 0 ) {
        cerr << "size-y must be positive integer" << endl;
        return false;
    }
    if( time_step <= 0.0 ) {
        cerr << "time-step must be positive value (type double)" << endl;
        return false;
    }
    return true;
}

bool report_peak_memory( void )
{
    ifstream meminfo("/proc/self/status");
    if (meminfo.fail()) {
        cerr << "error: unable to open /proc/self/status" << endl;
        return false;
    }

    string desc;
    unsigned value;
    while (meminfo.good()) {
        // extract description (first column)
        meminfo >> desc;

        if( desc == "VmHWM:" ) {
            meminfo >> value;
            cout << "Peak memory usage: " << value / 1024 << " MiB" << endl;
            return true;
        }

        // ignore the rest of irrelevant lines
        meminfo.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return false;
}

int main( int argc, char** argv )
{
    bool status = true;
    IndexType size_x = 0;
    IndexType size_y = 0;
    RealType time_step = 0.0;
    RealType time_step_order = 0;

    status &= parse_options( argc, argv,
                             size_x, size_y, time_step, time_step_order );
    if( ! status ) {
        cerr << endl;
        cerr << "Usage: " << argv[ 0 ] << " options..." << endl;
        cerr << "  where the options are:" << endl;
        cerr << "    --size-x <int>             mesh size in direction x (required)" << endl;
        cerr << "    --size-y <int>             mesh size in direction y (required)" << endl;
        cerr << "    --time-step <double>       initial time step (required)" << endl;
        cerr << "    --time-step-order <int>    time step is set to: time-step * pow( space-step, time-step-order ); default value is 0" << endl;
        return EXIT_FAILURE;
    }

    cout << "Configured parameters:" << endl;
    cout << "  size-x = " << size_x << endl;
    cout << "  size-y = " << size_y << endl;
    cout << "  time-step = " << time_step << endl;
    cout << "  time-step-order = " << time_step_order << endl;

    Solver s( size_x, size_y, time_step, time_step_order );
    status &= s.run();

    // print peak memory usage
    status &= report_peak_memory();

    return !status;
}

