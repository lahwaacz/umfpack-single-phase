#include <iostream>
#include <string>
#include <fstream>
#include <limits>

#include "DenseMatrix.h"
#include "SparseMatrix.h"
#include "Vector.h"
#include "SOR.h"

#include "RectangularMesh.h"
#include "Solver.h"

using namespace std;

void test_mesh(void)
{
    RectangularMesh mesh(1, 1, 3, 4);
    cout << "#cells: " << mesh.num_cells() << endl;
    cout << "#edges: " << mesh.num_edges() << endl;

    cout << "is_inner_edge" << endl;
    int inner_count = 0;
    for( int i = 0; i < mesh.num_edges(); i++ ) {
        cout << "index " << i << " is ";
        bool inner = mesh.is_inner_edge(i);
        if( inner == true ) {
            cout << "inner" << endl;
            inner_count++;
        }
        else
            cout << "outer" << endl;
    }
    cout << "counted " << inner_count << " inner edges, " << mesh.num_edges() - inner_count << " outer edges" << endl;

    cout << "testing edge_for_cell()" << endl;
    for( int cell = 0; cell < mesh.num_cells(); cell++ ) {
        cout << "cell " << cell << ", edges: ";
        for( int j = 0; j < mesh.edges_per_cell(); j++ ) {
            cout << mesh.edge_for_cell( cell, j ) << " ";
        }
        cout << endl;
    }

    cout << "testing cell_for_edge()" << endl;
    for( int edge = 0; edge < mesh.num_edges(); edge++ ) {
        cout << "edge " << edge << ", cells: ";
        for( int j = 0; j < 2; j++ ) {
            cout << mesh.cell_for_edge( edge, j ) << " ";
        }
        cout << endl;
    }
}

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

