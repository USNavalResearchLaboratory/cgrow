#include "mainwindow.hpp"
#include "marker.hpp"
#include "spreadsheet.hpp"
#include "tests_list_widget.hpp"

#include <QApplication>
#include <QClipboard>
#include <QListWidget>
#include <QMessageBox>
#include <QTimer>
#include <QtTest>

#include <stdexcept>

namespace
{
QJsonObject test_document( int count = 2 )
{
  QJsonArray tests;
  for ( int index = 0; index != count; ++index )
  {
    QJsonObject color;
    color[ "red" ]   = 10 + index;
    color[ "green" ] = 20 + index;
    color[ "blue" ]  = 30 + index;

    QJsonObject marker;
    marker[ "shape" ]      = "Circle";
    marker[ "color" ]      = color;
    marker[ "line width" ] = 1.5;
    marker[ "size" ]       = 9.0;

    QJsonArray delta_k;
    delta_k.append( 10.0 );
    delta_k.append( 15.0 );
    QJsonArray dad_n;
    dad_n.append( 1.0e-8 );
    dad_n.append( 2.0e-8 );

    QJsonObject test;
    test[ "name" ]   = QString( "Test %1" ).arg( index );
    test[ "marker" ] = marker;
    test[ "R" ]      = 0.5;
    test[ "DeltaK" ] = delta_k;
    test[ "dadN" ]   = dad_n;
    tests.append( test );
  }

  QJsonObject document;
  document[ "Crack Growth Tests" ] = tests;
  return document;
}

void answer_next_message_box( QMessageBox::StandardButton button )
{
  QTimer::singleShot( 0, qApp, [ button ] {
    for ( QWidget* widget : QApplication::topLevelWidgets( ) )
    {
      if ( auto* message_box = qobject_cast< QMessageBox* >( widget ) )
      {
        if ( auto* message_button = message_box->button( button ) )
        {
          message_button->click( );
          return;
        }
      }
    }
  } );
}
}

class gui_data_test : public QObject
{
  Q_OBJECT

private slots:
  void invalid_import_keeps_existing_data( )
  {
    tests_list_widget widget;
    widget.append( test_document( ) );
    QCOMPARE( widget.tests( ).size( ), 2 );

    QJsonObject invalid = test_document( 1 );
    invalid[ "Crack Growth Tests" ] = QJsonArray { QJsonValue( "not an object" ) };

    bool rejected = false;
    try
    {
      widget.read( invalid );
    }
    catch ( const std::runtime_error& )
    {
      rejected = true;
    }

    QVERIFY( rejected );
    QCOMPARE( widget.tests( ).size( ), 2 );
  }

  void cancel_keeps_selected_data( )
  {
    tests_list_widget widget;
    widget.append( test_document( ) );
    auto* list = widget.findChild< QListWidget* >( );
    QVERIFY( list != nullptr );
    list->setCurrentRow( 0, QItemSelectionModel::ClearAndSelect );

    answer_next_message_box( QMessageBox::Cancel );
    QVERIFY( QMetaObject::invokeMethod( &widget, "remove_test_series_item", Qt::DirectConnection ) );

    QCOMPARE( widget.tests( ).size( ), 2 );
    QCOMPARE( list->count( ), 2 );
  }

  void confirm_removes_selected_data( )
  {
    tests_list_widget widget;
    widget.append( test_document( ) );
    auto* list = widget.findChild< QListWidget* >( );
    QVERIFY( list != nullptr );
    list->setCurrentRow( 0, QItemSelectionModel::ClearAndSelect );

    answer_next_message_box( QMessageBox::Yes );
    QVERIFY( QMetaObject::invokeMethod( &widget, "remove_test_series_item", Qt::DirectConnection ) );

    QCOMPARE( widget.tests( ).size( ), 1 );
    QCOMPARE( list->count( ), 1 );
  }

  void ragged_paste_is_rejected( )
  {
    spreadsheet table;
    table.setCurrentCell( 0, 0 );
    QApplication::clipboard( )->setText( "10\t1e-8\n15" );

    answer_next_message_box( QMessageBox::Ok );
    table.paste( );

    QVERIFY( table.item( 0, 0 ) == nullptr );
    QVERIFY( table.item( 0, 1 ) == nullptr );
  }

  void invalid_marker_values_are_rejected( )
  {
    QJsonObject invalid_marker = test_document( 1 )[ "Crack Growth Tests" ]
                                   .toArray( )[ 0 ]
                                   .toObject( )[ "marker" ]
                                   .toObject( );
    auto color = invalid_marker[ "color" ].toObject( );
    color[ "red" ]           = 1.0e100;
    invalid_marker[ "color" ] = color;

    marker_t marker;
    bool     rejected = false;
    try
    {
      marker.read( invalid_marker );
    }
    catch ( const std::runtime_error& )
    {
      rejected = true;
    }
    QVERIFY( rejected );
  }

  void deleting_a_selected_series_keeps_selection_state_valid( )
  {
    mainWindow window;
    auto*      series = window.findChild< tests_list_widget* >( );
    QVERIFY( series != nullptr );
    series->append( test_document( ) );

    auto* list = series->findChild< QListWidget* >( );
    QVERIFY( list != nullptr );
    list->setCurrentRow( 1, QItemSelectionModel::ClearAndSelect );
    QCoreApplication::processEvents( );

    answer_next_message_box( QMessageBox::Yes );
    QVERIFY( QMetaObject::invokeMethod( series, "remove_test_series_item", Qt::DirectConnection ) );
    QCOMPARE( series->tests( ).size( ), 1 );

    list->setCurrentRow( 0, QItemSelectionModel::ClearAndSelect );
    QCoreApplication::processEvents( );
    QCOMPARE( series->tests( ).size( ), 1 );
  }
};

QTEST_MAIN( gui_data_test )

#include "main.moc"
