/*
*   Dmytro Sirenko, Jan 2010
*   This program is BEERWARE - if it is useful for you, use and enjoy,
*   otherwise you can write your own version with blackjack and hookers :)
*/
#ifndef _SUDOKU_H
#define _SUDOKU_H

#include <iostream>
// for debug only

typedef unsigned int    uint;
typedef unsigned char   ubyte;
typedef unsigned long   ulong;

class Sudoku
{
private:
    struct Field                // a cell of sudoku
    {
        ubyte val;              // just number (1..sn) in cell, 0 for unknown
        ulong p;                // bitfield for possible options: 1 = 0x1, 2 = 0x2, 3 = 0x4, 4 = 0x8, ... n = (1 << (sn-1))
    };
    ubyte pos_to_number(ulong); // returns last or only option in ulong possibility record (Field::p)
    short ones_count(ulong);    // returns count of options in ulong (of binary ones in ulong)
    ulong _make_mask(ubyte);    // makes mask of kind 111111011 with 0 at 'at'
    void _output_possibility() const; // show field of options
    void _remove_possibility(int, int, ubyte);
    bool _rem_pos_from_area_excl_line(ubyte areax, ubyte areay, ulong mask, ubyte except_line);
    bool _rem_pos_from_area_excl_column(ubyte areax, ubyte areay, ulong mask, ubyte except_col);
    bool _rem_pos_from_line_excl_area(ubyte line, ubyte except_area, ulong mask);
    bool _rem_pos_from_col_excl_area(ubyte col, ubyte except_area, ulong mask);

    // solving algorithms
    bool apply_rules();
    bool euristic1();
    bool euristic2();
    bool euristic3();


                        // removes possible options over column, line and area when inserting a number
    bool check_correct();   // checks whether sudoku is correct
    void solve();       // fills fields.

    ubyte na, nb;       // lengths of sides of an area, i. g. na = nb = 3 for set [1..9]
                        //  na - vertical, nb - horizontal
    uint sn;            // sn = na * nb, count of numbers in set for sudoku, sn=9 most typically
    Field **f;          // array [sn][sn]

    bool finished;      // all fields are filled?
    bool correct;
    bool logging, log1;

public:
    Sudoku(ubyte *data, ubyte nA = 3, ubyte nB = 0, bool log = false, bool debug = false);
        // recieves data (array of ubyte - (nA x nB) items)
        // nB = 0 means nB = nA
    Sudoku(Sudoku &sudoku);
    int result(int i, int j, bool *ok = 0) const;       // returns state of cell [i][j], i - vert, j - hor
    ulong possibility(int i, int j) const;              // returns options bitfield for [i][j], see struct Field
    bool is_correct() const;
    virtual ~Sudoku();

    Sudoku& operator =(Sudoku &s);
};

#endif // _SUDOKU_H
