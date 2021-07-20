// CGROW: A crack growth model identification framework.

// AUTHORIZATION TO USE AND DISTRIBUTE. By using or distributing the CGROW software
// ("THE SOFTWARE"), you agree to the following terms governing the use and redistribution of
// THE SOFTWARE originally developed at the U.S. Naval Research Laboratory ("NRL"), Computational
// Multiphysics Systems Lab., Code 6394.

// The modules of CGROW containing an attribution in their header files to the NRL have been
// authored by federal employees. To the extent that a federal employee is an author of a portion of
// this software or a derivative work thereof, no copyright is claimed by the United States
// Government, as represented by the Secretary of the Navy ("GOVERNMENT") under Title 17, U.S. Code.
// All Other Rights Reserved.

// Download, redistribution and use of source and/or binary forms, with or without modification,
// constitute an acknowledgement and agreement to the following:

// (1) source code distributions retain the above notice, this list of conditions, and the following
// disclaimer in its entirety,
// (2) distributions including binary code include this paragraph in its entirety in the
// documentation or other materials provided with the distribution, and
// (3) all published research using this software display the following acknowledgment:
// "This work uses the software components contained within the NRL CGROW computer package
// written and developed by the U.S. Naval Research Laboratory, Computational Multiphysics Systems
// lab., Code 6394"

// Neither the name of NRL or its contributors, nor any entity of the United States Government may
// be used to endorse or promote products derived from this software, nor does the inclusion of the
// NRL written and developed software directly or indirectly suggest NRL's or the United States
// Government's endorsement of this product.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR THE U.S. GOVERNMENT BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// NOTICE OF THIRD-PARTY SOFTWARE LICENSES. This software uses open source software packages from third
// parties. These are available on an "as is" basis and subject to their individual license agreements.
// Additional information can be found in the provided "licenses" folder.

#pragma once

#include <QTableWidget>

#include "test_series.hpp"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVector>

class QKeyEvent;

class Cell : public QTableWidgetItem
{
public:
  Cell( );

  QTableWidgetItem* clone( ) const;

  void setData( int role, const QVariant& value );

  QVariant data( int role ) const;

  void setFormula( const QString& formula );

  QString formula( ) const;

private:
  QVariant value( ) const;
};

class SpreadsheetCompare;

class spreadsheet : public QTableWidget
{
  Q_OBJECT
public:
  spreadsheet( QWidget* parent = 0 );

  QString currentLocation( ) const;

  QTableWidgetSelectionRange selectedRange( ) const;

  void clear( );

  test_data_t get_data( bool& ok ) const;

  void set_data( const test_data_t& data );

  //    bool readFile(const QString &fileName);
  //    bool writeFile(const QString &fileName);

  //    void sort(const SpreadsheetCompare &compare);
protected:
  void keyPressEvent( QKeyEvent* event );

public slots:
  void cut( );
  void copy( );
  void paste( );
  void del( );
  void selectCurrentRow( );
  void selectCurrentColumn( );
  void findNext( const QString& str, Qt::CaseSensitivity cs );
  void findPrevious( const QString& str, Qt::CaseSensitivity cs );

private slots:
  void somethingChanged( );

signals:
  void modified( );

private:
  enum
  {
    MagicNumber = 0x7F51C883,
    RowCount    = 999,
    ColumnCount = 2
  };

  Cell* cell( int row, int column ) const;

  QString text( int row, int column ) const;

  void    setFormula( int row, int column, const QString& formula );
  QString formula( int row, int column ) const;
};

class SpreadsheetCompare
{
public:
  bool operator( )( const QStringList& row1, const QStringList& row2 ) const;

  enum
  {
    KeyCount = 3
  };

  int keys[ KeyCount ];

  bool ascending[ KeyCount ];
};
