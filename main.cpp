#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

#include "sudoku.h"

int main(int argc, char *argv[])
{
    int na = 0;
    int nb = 0;
    int sn = 0;
    ubyte *data = 0;
    bool log = false;
    bool debug = false;
    bool verb = false;

    for (int i = 1; i < argc; ++i)
        // options handling
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "--log") || (!strcmp(argv[i], "-l")))
            {
                log = true;
            }
            else if (!strcmp(argv[i], "--debug") || (!strcmp(argv[i], "-d")))
            {
                debug = true;
            }
            else if (!strcmp(argv[i], "--help") || (!strcmp(argv[i], "-h")))
            {
                cout << "USAGE: sudoku [-v | --verbose] [-d | --debug] [-l|--log] [<FILENAME>] | [--version] | [-h|--help]\n";
                return 0;
            }
            else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose"))
            {
                verb = true;
            }
            else if (!strcmp(argv[i], "--version"))
            {
                cout << "sudoku version 0.8\nDmytro Sirenko, 2009\n\n" <<
                        "This program is a BEERWARE :)\n";
                return 0;
            }
            else
            {
                cout << "sudoku: invalid option - '" << argv[i] << "'\n" <<
                        "try --help for information\n";
                return -1;
            }
        }
        else
        {
            ifstream fin(argv[i]);
            if (!fin)
            {
                cout << "Error: file " << argv[i] << " not found\n";
                return 1;
            }
            //cout << "File '" << argv[i] << "' opened successsfully\n";
            fin >> na >> nb;
            sn = na * nb;
            data = new ubyte[sn * sn];

            for (int i = 0; i < sn; ++i)
                for (int j = 0; j < sn; ++j)
                {
                    int it;
                    fin >> it;
                    data[i * sn + j] = (ubyte)it;  // 0 for unknown
                }
        }

    if (data == 0)  // interactive mode
    {
        if (verb) cout << "Enter na, nb:" << endl;
        cin >> na >> nb;
        sn = na * nb;
        data = new ubyte[sn * sn];

        if (verb) cout << "Enter your sudoku:\n";
        for (int i = 0; i < sn; ++i)
            for (int j = 0; j < sn; ++j)
            {
                int it;
                cin >> it;
                data[i * sn + j] = (ubyte)it;  // 0 for unknown
            }
    }

    Sudoku *sudoku = new Sudoku(data, na, nb, log, debug);
    delete []data;

    cout << na << "\t" << nb << endl;
    for (int i = 0; i < sn; ++i)
    {
        for (int j = 0; j < sn; ++j)
            cout << (int)sudoku->result(i, j) << ((j + 1) % nb == 0? "  " : " ");
        cout << "\n";
        if ((i +1)% na == 0) cout << "\n";
    }
    cout << "\nResult: " << (sudoku->is_correct() ? "correct" : "errors") << endl;

    delete sudoku;
    return 0;
}
