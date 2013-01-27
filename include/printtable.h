/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Route table printout
 * Author:   Pavel Saviankou
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 */
#include <iostream>
#include <vector>
using namespace std;


#ifndef __PRINTTABLE_H__
#define __PRINTTABLE_H__

#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>
#include <wx/string.h>
#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "ocpn_types.h"
#include "navutil.h"

/**
 * \brief
 *  Enumeration is used to notice the state of the table.
 *
 *  Different states are used to signalize different semanic of the data in
 *  the operator << of the class Table.
 *  If the state is "setup columns widths" -> then the data is used to store
 *  the width of the columns.
 *  If the state is "fill with data" -> then the data is the cell content.
 */
enum TableState { TABLE_SETUP_WIDTHS = 0, TABLE_FILL_DATA, TABLE_FILL_HEADER };


/**
 *  \brief Represents a NxM simple table with captions.
 *
 *  Input operator is "<<"
 *  Number of columns and rows are given dynamically by the input data.
 *  Captions are given by first input line.
 *  Every cell is given column by column.
 *  Next row is given by "<< '\n'" (or << endl)
 *
 *
 *
 */
class Table {
protected:
    int nrows;
    int ncols;

    bool create_next_row;

    vector< vector < wxString > > data;
    vector< double > widths;
    vector< wxString > header;
    TableState state;

    void Start();

    void NewRow();

public:
    Table();
    ~Table();

    Table& operator<<( const string& );
    Table& operator<<( const int& );
    Table& operator<<( const double& );

    vector< vector < wxString > > & GetData()
    {
        return data;
    };

    void StartFillData()
    {
        state = TABLE_FILL_DATA;
    };

    void StartFillHeader()
    {
        state = TABLE_FILL_HEADER;
    };

    void StartFillWidths()
    {
        state = TABLE_SETUP_WIDTHS;
    };

    int GetRowHeight( int i )
    {
        return widths[ i ];
    };
};


ostream& operator<<( ostream&,
            Table& );



/**
 *  \brief This class takes multilined string and modifies it to fit into given width
 *         for given device. If it is too wide for given DC (by class PrintTable )
 *        it introduces new lines between words
 *
 */

class PrintCell {
protected:
    // Copy of printing device
    wxDC* dc;

    // Target width
    int width;

    // Target height
    int height;

    // Cellpadding
    int cellpadding;

    // Content of a cell
    wxString content;

    // Result of modification
    wxString modified_content;

    // Rect for printing of modified string
    wxRect rect;

    // Stores page, where this cell will be printed
    int page;

    // Stores, if one has to ovveride property "weight" of the font with the value "bold" - used to print header of the table.
    bool bold_font;


    // Adjust text
    void Adjust();


public:

    // Constructor with content to print and device
    PrintCell ()
    {
    };

    // Constructor with content to print and device
    void Init( const wxString& _content,
          wxDC*           _dc,
          int             _width,
          int             _cellpadding,
          bool            bold_font = false );

    // Returns rect for printing
    wxRect GetRect()
    {
        return rect;
    };

    // Returns modified cell content
    wxString GetText()
    {
        return modified_content;
    };

    // Returns height of the cell
    int GetHeight()
    {
        return height;
    };

    // Returns width of the cell
    int GetWidth()
    {
        return width;
    };

    // sets the page to print
    void SetPage( int _page )
    {
        page = _page;
    };

    // sets the height
    void SetHeight( int _height )
    {
        height = _height;
    };

    // Returns the page, where this element should be painted
    int GetPage()
    {
        return page;
    };
};



/**
 *  \brief Extension of a class Table with printing into dc.
 *
 *   It takes all elements, takes DC as a printing device, takes  a maximal
 *   possible table width,  calculate width of every column.
 *   For printing of every cell it modifies its content so, that it fits into cell by inserting
 *   new lines.
 *
 */
class PrintTable : public Table {
protected:
    vector< vector < PrintCell > > contents;
    vector < PrintCell >           header_content;
    vector< int >                  rows_heights;
    int                            header_height;


    int number_of_pages;                // stores the number of pages for printing of this table. It is set by AdjustCells(...)

public:
    PrintTable();

    // creates internally vector of PrintCell's, to calculate columns widths and row sizes
    void AdjustCells( wxDC* _dc,
                 int   marginX,
                 int   marginY );

    // delivers content of the table
    vector< vector < PrintCell > >& GetContent()
    {
        return contents;
    };
    // delivers header  of the table
    vector < PrintCell > & GetHeader()
    {
        return header_content;
    };
    // returns the total number of needed pages;
    int GetNumberPages()
    {
        return number_of_pages;
    };

    // Returns the height of the header
    int GetHeaderHeight()
    {
        return header_height;
    };
};
#endif
