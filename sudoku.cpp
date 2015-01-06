/*
*   Dmytro Sirenko, Jan 2010
*   This program is BEERWARE - if it is useful for you, use and enjoy,
*   otherwise you can write your own version with blackjack and hookers :)
*/
#include "sudoku.h"

Sudoku::Sudoku(ubyte *data, ubyte nA, ubyte nB, bool log, bool debug):
    na(nA), nb(nB), logging(log), log1(debug)
{
    sn = na * nb;

    // memory allocation for fields
    f = new Field * [sn];
    for (int i = 0; i < sn; ++i)
        f[i] = new Field[sn];

    // init: all possible options
    ulong utemp = (0x00000001 << sn) - 1;
    for (int i = 0; i < sn; ++i)
        for (int j = 0; j < sn; ++j)
            f[i][j].p = utemp;

    // copying data and filling posibilities
    for (int i = 0; i < sn; ++i)
        for (int j = 0; j < sn; ++j)
        {
            f[i][j].val = data[i*sn + j];
            if (f[i][j].val != 0)
                _remove_possibility(i, j, f[i][j].val);
        }

    // just solve :)
    solve();
}

Sudoku::Sudoku(Sudoku &sudoku)
{
    na = sudoku.na;
    nb = sudoku.nb;
    sn = na * nb;

    // memory allocation for fields
    f = new Field * [sn];
    for (int i = 0; i < sn; ++i)
        f[i] = new Field[sn];

    // we don't copy possibilities because when forking
    //      parent's ones are not evaluated!
    // init: all possible options
    ulong utemp = (0x00000001 << sn) - 1;
    for (int i = 0; i < sn; ++i)
        for (int j = 0; j < sn; ++j)
            f[i][j].p = utemp;

    // copying data and filling posibilities
    for (int i = 0; i < sn; ++i)
        for (int j = 0; j < sn; ++j)
        {
            f[i][j].val = sudoku.f[i][j].val;
            if (f[i][j].val != 0)
                _remove_possibility(i, j, f[i][j].val);
        }

    // just solve :)
    solve();
}

Sudoku::~Sudoku()
{
    // memory disposition
    for (int i = 0; i < sn; ++i)
        delete []f[i];
    delete []f;
    f = 0;
    na = nb = 0;
}

Sudoku& Sudoku::operator =(Sudoku& s)
{
    if (&s == this) return *this;
    // memory disposition for s
    for (int i = 0; i < sn; ++i)
        delete [](f[i]);
    delete []f;

    // copy
    na = s.na;
    nb = s.nb;
    sn = s.sn;
    f = new Field*[sn];
    for (int i = 0; i < sn; ++i)
        f[i] = new Field[sn];
    for (int i = 0; i < sn; ++i)
        for (int j = 0; j < sn; ++j)
            f[i][j] = s.f[i][j];
    finished = s.finished;
    correct = s.correct;
    return *this;
}

int Sudoku::result(int i, int j, bool *ok) const
{
    if (ok != 0)
        *ok = !finished;
    return f[i][j].val;
}

ulong Sudoku::possibility(int i, int j) const
{
    return f[i][j].p;
}

bool Sudoku::is_correct() const
{
    return correct;
}

// gets a count of possible options written in p.
inline short Sudoku::ones_count(ulong p)
{
    short sum = 0;
    for (int k = 0; k < sn; ++k)
        if ((p >> k) % 2) ++sum;
    return sum;
}

// seeks for a last present option in k and returns its number
inline ubyte Sudoku::pos_to_number(ulong k)
{
    for (int i = 0; i < sn; ++i)
    {
        if (k % 2) return i + 1;
        k /= 2;
    }
    return 0xFF;
}

// Stub!
bool Sudoku::check_correct()
{
    for (int i = 0; i < sn; ++i)
        for (int j = 0; j < sn; ++j)
        {
            if (0 == f[i][j].p && 0 == f[i][j].val)
                return false;
            // there should be something more
        }
    return true;
}

void Sudoku::_output_possibility() const
{
    if (!logging) return;
    // debug only! - output of sudoku
    std::cout << "Possibilities: \n";
    for (int _i = 0; _i < sn; ++_i)
    {
        for (int _j = 0; _j < sn; ++_j)
            if (f[_i][_j].val == 0)
            {
                std::cout << "(";
                if ((0x00000001 << sn) - 1 != f[_i][_j].p)
                for (int _k = 0; _k < sn; ++_k)
                {
                    if ((f[_i][_j].p >> _k) % 2)
                        std::cout << _k + 1;
                }
                else std::cout << "any";
                std::cout << ")\t";
            }
            else std::cout << (int)f[_i][_j].val << "\t";
        std::cout << std::endl;
    }
    std::cout << '\n';
}

inline ulong Sudoku::_make_mask(ubyte at)
{
    ulong mask = 0;
    // generating
    for (int k = sn - 1; k >= 0; --k)
        if (k == at - 1) mask += mask; // mul by 2, cannot use << because of its fuckness
        else mask += (mask + 1);
    return mask;
}

// if one has determined that position (i, j) definitely has value _with
//      we need to clear line, column and area of (i, j)
void Sudoku::_remove_possibility(int i, int j, ubyte _with)
{
    ulong _temp = _make_mask(_with);

    // applying changes
    for (int k = 0; k < sn; ++k)
    {
        f[k][j].p &= _temp;     //   remove possible option over column
        f[i][k].p &= _temp;     //  - " - over line
        f[na*(i/na) + k/nb][nb*(j/nb) + k%nb].p &= _temp;       // over area [na x nb]
    }
    f[i][j].p = 0;
    if (log1) _output_possibility();
}

inline bool Sudoku::_rem_pos_from_area_excl_line(ubyte areax, ubyte areay, ulong mask, ubyte except_line) {
    bool res = false;

    for (int y = (areay*na); y < areay * (1 + na); ++y)
        if (y != except_line)
            for (int x = (areax * nb); x < areax * (1 + nb); ++x) {
                if (f[y][x].p & !mask) {
                    f[y][x].p &= mask;
                    res = true;
                }
            }
    return res;
}

inline bool Sudoku::_rem_pos_from_area_excl_column(ubyte areax, ubyte areay, ulong mask, ubyte except_col) {
    bool res = false;
    for (int x = (areax*nb); x < areax * (1 + nb); ++x)
        if (x != except_col)
            for (int y = (areay * na); y < areay * (1 + na); ++y)
                if (f[x][y].p & !mask) {
                    f[x][y].p &= mask;
                    res = true;
                }
    return res;
}

inline bool Sudoku::_rem_pos_from_line_excl_area(ubyte line, ubyte except_area, ulong mask) {
    bool res = false;
    for (int k = 0; k < sn; ++k)
        if ((k < except_area * nb) && (except_area * (1 + nb) <= k))
            if (f[line][k].p & !mask) {
                f[line][k].p &= mask;
                res = true;
            }
    return res;
}

inline bool Sudoku::_rem_pos_from_col_excl_area(ubyte col, ubyte except_area, ulong mask) {
    bool res = false;
    for (int k = 0; k < sn; ++k)
        if ((k < except_area * nb) && (except_area * (1 + nb) <= k))
            if (f[k][col].p & !mask) {
                f[k][col].p &= mask;
                res = true;
            }
    return res;
}

/// APPLY BASIC RULES
// search for fields wi th a single possible option.
// _remove_possibilty() removes unneccessary options, so we just have to check
//      whether there is a single option in a field.
// returns whether a field has changed
bool Sudoku::apply_rules() {
    for (int i = 0; i < sn; ++i)
        for (int j = 0; j < sn; ++j)
            // if there is an only option for field, we don't have to deal with this field anymore
            if (ones_count(f[i][j].p) == 1)
            {
                // determine value of f[i][j]
                f[i][j].val = pos_to_number(f[i][j].p);
                if (logging)
                    std::cout << "\ta[" << i << ", " << j << "] = " << (int)f[i][j].val << " [single option in field]\n";
                _remove_possibility(i, j, f[i][j].val);
                return true;
            }
    return false;
}

/// EURISTIC 1: {Plan B :)}
// search for fields with a single possibility over area, column or line
// like (1234)  (123)   (23)
//      (256)   (2356)  (67)
//      (25)    (238)   (28)    -   4 or 7 is in only place, area must have a 4 -> (1234) = 4;
// returns whether a field has changed
bool Sudoku::euristic1() {
    struct {
        ubyte x, y;
    } c, cc; // contains (0xFF, 0xFF) if there are no matches, (x, y) for only match, (x, 0xFF) for few matches

    // over columns and lines
    for (int k = 0; k < sn; ++k)        // over possible numbers in fields
    {
        for (int i = 0; i < sn; ++i)    // over lines and columns simultenuously
        {
            // there are no matches yet
            c.x = cc.x = 0xFF;
            c.y = cc.y = 0xFF;

            for (int j = 0;   j < sn; ++j)  // go through i'th line/column
            {
                // checking i'th line
                if ((f[i][j].p >> k) % 2)   // if k in (ij) options
                    if (c.x == 0xFF)        // is this a first match?
                    {                       // then assign c current position
                        c.x = i;
                        c.y = j;
                    }
                    else c.y = 0xFF;
                // checking i'th column
                if ((f[j][i].p >> k) % 2)
                    if (cc.x == 0xFF)    // first match
                    {
                        cc.x = j;
                        cc.y = i;
                    }
                    else cc.y = 0xFF;
            }

            if ((c.x != 0xFF) && (c.y != 0xFF))     //  an only match
            {
                f[c.x][c.y].val = k + 1;
                if (logging)
                    std::cout << "\ta[" << (int)c.x << ", " << (int)c.y << "] = " <<
                                (int)(k + 1) << " [single option in line]\n";
                _remove_possibility(c.x, c.y, k + 1);
                return true;
            }
            if ((cc.x != 0xFF) && (cc.y != 0xFF))
            {
                f[cc.x][cc.y].val = k + 1;
                if (logging)
                    std::cout << "\ta[" << (int)cc.x << ", " << (int)cc.y << "] = "
                                << (int)(k + 1) << " [single option in column]\n";
                _remove_possibility(cc.x, cc.y, k + 1);
                return true;
            }
        }
    }

    // over areas
    for (int x = 0; x < na; ++x)
    {
        for (int y = 0; y < nb; ++y)
        {
            for (int k = 0; k < sn; ++k)        // search for matches of k in area [x,y]
                {
                    // init: no matches
                    c.x = 0xFFu;
                    c.y = 0xFFu;
                    // scan fields of area [x,y] for option k
                    for (int i = x*nb; i < (x+1)*nb; ++i)
                        for (int j = y*na; j < (y+1)*na; ++j)
                        if ((f[i][j].p >> k) % 2)  // match found
                            if (c.x == 0xFF)    //  first match
                            {   // remember first match
                                c.x = i;
                                c.y = j;
                            }
                            else if (c.y != 0xFF) // second match
                                c.y = 0xFF; // more than one match, we don't need data anymore
                    if ((c.x != 0xFFu) && (c.y != 0xFFu))
                    {   // k has single possibility in area (x,y)
                        f[c.x][c.y].val = k + 1;
                        if (logging)
                            std::cout << "\ta[" << (int)c.x << ", " << (int)c.y << "] = " <<
                                        (int)(k + 1) << " [single option in area]\n";
                        _remove_possibility(c.x, c.y, k + 1);
                        return true;
                    }
                }
            }
        }
    return false;
}

/// EURISTIC 2:
// This euristic does not find something, actually its purpose is excluding
// reduntant possibilities
//
// If an area's column/line contains ambiguous options and there're no such options
// over the rest of global column/line, this area's column/line must contain this
// options, hence the rest of the area does not, we may exclude them
//      returns whether a possibility has been reduced
bool Sudoku::euristic2() {
        for (int j = 0; j < sn; ++j)
        {
            // each item contains composite possibility for area's column which current global cloumn contains
            ulong *areapos = new ulong[na];

            // sum up possibilities of area's columns
            for (int i = 0; i < na; ++i)
            {
                areapos[i] = 0;
                for (int k = 0; k < nb; ++k)
                    areapos[i] |= f[nb * i + k][j].p;
            }

            // is there option in only area's column?
            for (ubyte k = 0; k < sn; ++k)      // possible numbers
            {
                ubyte is_there = sn;        // contains a number of area with single option
                    // sn   stands for there has not been neccesary option yet
                    // 0xFF stands for more than one occurance

                for (ubyte i = 0; i < na; ++i)  //  over summed possibilities
                    if ((areapos[i] >> k) % 2)  // - is k in area[i]?
                    if (is_there == sn) is_there = i;   // - if so, if it is first then remember where
                    else { is_there = 0xFF; break; }//      -   else break from loop

                // if there was single k then remove its possibility in the rest of area:
                if (is_there < sn)
                {
                    if (logging)
                        std::cout << "\tcolumn[" << j << "]: reducing reduntant option <" << (int)k+1 <<
                                    "> in area " << (int)is_there << "\n";
                    ulong _mask = 0;        // mask for removing kth bit
                    // generating _mask
                    for (int l = sn - 1; l >= 0; --l)
                        if (l == k) _mask += _mask; // mul by 2, cannot use << because of its fuckness
                        else _mask += (_mask + 1);

                    // removing in area
                    for (int x = (j/na)*na; x < (1 + j/na)*na; ++x)
                        if (x != j)
                        for (int y = (is_there * nb); y < ((is_there + 1)*nb); ++y)
                        {
                            //std::cout << "        deleting (" << x << ", " << y <<
                            //          ") with mask " << std::hex << _mask << std::dec << "\n";
                            f[y][x].p &= _mask;
                            // what the heck, David Blain, why does this work?!
                        }
                    if (log1) _output_possibility();
                    return true;
                }
            }

            delete []areapos;
        }
        //if (changed) continue;

        // over lines
        for (int i = 0; i < sn; ++i)
        {
            // each item contains composite possibility for area's line which current global line contains
            ulong *areapos = new ulong[nb];

            // sum up possibilities of area's lines
            for (int j = 0; j < nb; ++j)
            {
                areapos[j] = 0;
                for (int k = 0; k < na; ++k)
                    areapos[j] |= f[i][na * j + k].p;
            }

            // is there option in only area's column?
            for (ubyte k = 0; k < sn; ++k)      // possible numbers
            {
                ubyte is_there = sn;        // contains a number of area with single option
                    // sn   stands for there has not been neccesary option yet
                    // 0xFF stands for more than one occurance

                for (ubyte j = 0; j < nb; ++j)  //  over summed possibilities
                    if ((areapos[j] >> k) % 2)  // - is k in area[j]?
                    if (is_there == sn) is_there = j;   // - if so, if it is first then remember where
                    else { is_there = 0xFF; break; }//      -   else break from loop

                // if there was single k then remove its possibility in the rest of area:
                if (is_there < sn)
                {
                    if (logging)
                        std::cout << "\tline[" << i << "]: reducing reduntant option <" << (int)k+1 <<
                                    "> in area " << (int)is_there << "\n";
                    ulong _mask = 0;        // mask for removing kth bit
                    // generating _mask
                    for (int l = sn - 1; l >= 0; --l)
                        if (l == k) _mask += _mask; // mul by 2, cannot use << because of its fuckness
                        else _mask += (_mask + 1);

                    // removing in area
                    for (int y = (i/nb)*nb; y < (1 + i/nb)*nb; ++y)
                        if (y != i)
                        for (int x = is_there * na; x < (1 + is_there)*na; ++x)
                        {
                            //std::cout << "        deleting (" << y << ", " << x <<
                            //          ") with mask " << std::hex << _mask << std::dec << "\n";
                            f[y][x].p &= _mask;
                            // what the heck, David Blain, why does this work?!
                        }
                    return true;
                }
            }

            delete []areapos;
            if (log1) _output_possibility();
        }
    return false;
}

/// Euristic 3
// Excluding redundant possibilities
//
// when there are n < na (or nb) uncertainties and not greater than n possibilities
// we may exclude this possibiilities from rest of area and line
// Example:
//      (15)    (25)    (125)       - 1, 2, 5 must be here (3 uncertainty, 3 possib.)
//      (2345)  (2458)  (1249)      -> (34) (48) (49)
bool Sudoku::euristic3() {
    bool res = false;
        // over lines
        for (int i = 0; i < na; ++i)
            for (int j = 0; j < nb; ++j)
            {
                for (int x = i*na; x < (1 + i)*na; ++x)
                {
                    ulong sumpos = 0;
                    ubyte uncert_count = 0;
                    for (int y = j*nb; y < (1 + j)*nb; ++y)
                        if (f[x][y].val == 0) {
                            ++uncert_count;
                            sumpos |= f[x][y].p;
                        }
                    if (uncert_count == ones_count(sumpos)) {
                        if (logging) {
                            std::cout << "\tarea [" << (int)i << ", " << (int)j << "]-" << (int)x
                                << " : reducing multiple reduntant options <";
                            for (int ii = 0; ii < sn; ++ii)
                                if ((sumpos >> ii) % 2)
                                    std::cout << 1 + (int)ii << " ";
                            std::cout << ">" << std::endl;
                        }
                        // delete possibilities in area
                        /*for (int _x = i*na; x < (1 + i/na)*na; ++x)
                            if (_x != x)
                                for (int _y = (j*nb))*/
                        res |= _rem_pos_from_area_excl_line(j, i, ~sumpos, x);

                        // in global line
                        //res |= _rem_pos_from_line_excl_area(x, i, !sumpos);

                        if (log1) _output_possibility();
                    }
                }
            }
    return res;
}

// fills fields.
void Sudoku::solve()
{
    bool changed;
        // - is being used for abortive terminating of current loop if there's a progress; see below
    bool cycled = false, cycled3 = false;
        // used to avoid cycling: euristics which do not find values have to verify if their
        //  actions really help to find something. If even after that euristics nothing found
        //  we need not to try to use them again

    while (true)            // solving loop
    {
        changed = false;    // we're sceptics before any field is actually found :)

        changed = apply_rules();
        if (changed) {
            //  keep it simply stupid: if there is a progress, keep going above way.
            cycled3 = cycled = false;
            continue;
        }   // else it's time for plan B (below)

        changed = euristic1();
        if (changed) {
            cycled3 = cycled = false;
            continue;
        }

        if (!cycled) {
            cycled = euristic2();
            if (cycled) continue;
        }

        if (!cycled3) {
            cycled3 = euristic3();
            if (cycled3) continue;
        }

        /// STUPID BRUTEFORCE:
        // stupidly fork if there are two options
        // and check whether option is correct

            // Kill it with fire!
        // End of BRUTEFORCE

        // checking whether all fields have been filled
        correct = check_correct();
        finished = true;
        for (int i = 0; (i < sn) && finished; ++i)
            for (int j = 0; finished && (j < sn); ++j)
            if (f[i][j].val == 0)
                finished = false;

        if (logging && !correct)
            std::cerr << "\n Error: given sudoku is contradictory!\n\n";
        if (finished) break;

        if (logging)
            std::cerr << "\n Panic: all methods were unavailing! panic!\n\n";
        break;
    }/// End of solution loop
}
