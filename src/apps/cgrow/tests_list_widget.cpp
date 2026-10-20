#include "test_series_dialog.hpp"

#include "tests_list_widget.hpp"

#include <QBrush>
#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>

namespace
{
QVector< test_series_t > parse_test_series( const QJsonObject& json )
{
  if ( !( json.contains( "Crack Growth Tests" ) && json[ "Crack Growth Tests" ].isArray( ) ) )
  {
    throw std::runtime_error( "Json does not contain Crack Growth test." );
  }

  QVector< test_series_t > parsed;
  const auto               series = json[ "Crack Growth Tests" ].toArray( );
  parsed.reserve( series.size( ) );
  for ( const auto& value : series )
  {
    if ( !value.isObject( ) )
    {
      throw std::runtime_error( "Json Crack Growth Test format is incorrect." );
    }

    test_series_t test;
    test.read( value.toObject( ) );
    parsed.push_back( std::move( test ) );
  }

  return parsed;
}
}

tests_list_widget::tests_list_widget( QWidget* parent ) : QWidget( parent )
{
  list = new QListWidget;
  list->setSelectionMode( QAbstractItemView::ExtendedSelection );

  connect( list, SIGNAL( currentRowChanged( int ) ), this, SIGNAL( current_row_changed( int ) ) );

  auto add = new QPushButton( tr( "Add" ) );
  connect( add, SIGNAL( pressed( ) ), this, SLOT( new_test_series_item( ) ) );

  auto remove = new QPushButton( tr( "Remove" ) );
  connect( remove, SIGNAL( pressed( ) ), this, SLOT( remove_test_series_item( ) ) );

  auto edit = new QPushButton( tr( "Edit" ) );
  connect( edit, SIGNAL( pressed( ) ), this, SLOT( edit_test_series_item( ) ) );

  connect( list, &QListWidget::itemDoubleClicked, [ this ]( QListWidgetItem* ) {
    edit_test_series_item( );
  } );

  auto grid = new QGridLayout;
  grid->addWidget( list, 0, 0, 1, 3 );
  grid->addWidget( add, 1, 0, 1, 1 );
  grid->addWidget( edit, 1, 1, 1, 1 );
  grid->addWidget( remove, 1, 2, 1, 1 );

  grid->setContentsMargins( 2, 2, 2, 2 );

  this->setLayout( grid );

}

const QVector< test_series_t >& tests_list_widget::tests( ) const
{
  return tests_;
}

void tests_list_widget::write( QJsonObject& json )
{
  QJsonArray array;
  for ( const auto& test : tests_ )
  {
    QJsonObject test_json;
    test.write( test_json );
    array.append( test_json );
  }

  json[ "Crack Growth Tests" ] = array;
}

void tests_list_widget::clear( )
{
  const QSignalBlocker signal_blocker( list );
  while ( tests_.size( ) != 0 )
  {
    auto id = tests_.size( ) - 1;

    tests_.remove( id );

    delete list->takeItem( id );
    emit test_series_removed( id );
  }
  current_test_number = 0;
  emit current_row_changed( list->currentRow( ) );
}

void tests_list_widget::read( const QJsonObject& json )
{
  const auto parsed = parse_test_series( json );
  clear( );

  for ( const auto& test : parsed )
  {
    tests_.push_back( test );
    ++current_test_number;
    new QListWidgetItem( test.generate_scatter_icon( ), test.name( ), list );
    emit test_series_updated( tests_.size( ) - 1 );
  }
}

void tests_list_widget::append( const QJsonObject& json )
{
  const auto parsed = parse_test_series( json );
  for ( const auto& test : parsed )
  {
    tests_.push_back( test );
    ++current_test_number;
    new QListWidgetItem( test.generate_scatter_icon( ), test.name( ), list );
    emit test_series_updated( tests_.size( ) - 1 );
  }
}

void tests_list_widget::new_test_series_item( )
{
  double defaultR = 0.2;

  if ( tests_.size( ) != 0 )
  {
    defaultR = tests_.back( ).data.R;
  }

  testDataItemDialog dialog( ++current_test_number, defaultR );

  auto res = dialog.exec( );

  if ( res == QDialog::Rejected )
  {
    return;
  }

  tests_.push_back( dialog.test_data( ) );

  auto& test = *tests_.rbegin( );

  new QListWidgetItem( test.generate_scatter_icon( ), test.name( ), list );

  emit test_series_updated( tests_.size( ) - 1 );
}

void tests_list_widget::remove_test_series_item( )
{
  if ( list->selectedItems( ).size( ) == 0 )
  {
    return;
  }

  auto data_index = list->currentRow( );

  if ( data_index == -1 )
  {
    return;
  }

  auto selected = list->selectionModel( )->selectedRows( );

  QVector< int > ids;

  for ( auto s : selected )
  {
    ids.push_back( s.row( ) );
  }

  std::sort( ids.begin( ), ids.end( ) );

  const auto result = QMessageBox::warning(
    this,
    tr( "HS" ),
    tr( "Are you sure you want to remove %1? items" ).arg( list->selectedItems( ).size( ) ),
    QMessageBox::Yes | QMessageBox::Cancel );

  if ( result != QMessageBox::Yes )
  {
    return;
  }

  {
    const QSignalBlocker signal_blocker( list );
    for ( auto iter = ids.rbegin( ); iter != ids.rend( ); iter++ )
    {
      tests_.remove( *iter );
      delete list->takeItem( *iter );
      emit test_series_removed( *iter );
    }
  }
  emit current_row_changed( list->currentRow( ) );
}

void tests_list_widget::edit_test_series_item( )
{
  auto data_index = list->currentRow( );

  if ( data_index == -1 )
  {
    return;
  }

  if ( list->selectedItems( ).size( ) == 0 )
  {
    return;
  }

  testDataItemDialog dialog( tests_[ data_index ] );

  auto res = dialog.exec( );

  if ( res == QDialog::Rejected )
  {
    return;
  }

  tests_[ data_index ] = dialog.test_data( );

  auto item = list->item( data_index );

  const auto& test = tests_[ data_index ];
  item->setIcon( test.generate_scatter_icon( ) );
  item->setText( test.name( ) );

  emit test_series_updated( data_index );
}


QIcon generate_scatter_icon( QCPScatterStyle::ScatterShape shape,
                             QColor                        color,
                             int                           size,
                             float                         width )
{
  auto pen = QPen( );
  pen.setColor( color );
  pen.setWidthF( width );

  const int padding = 4;

  const QColor background_color( 255, 255, 255, 0 );

  QImage image( size + padding, size + padding, QImage::Format_ARGB32 );
  image.fill( background_color );

  QCPPainter painter( &image );
  painter.setRenderHint( QPainter::Antialiasing );
  painter.fillRect( 0, 0, size + 4, size + 4, QBrush( QColor( 255, 255, 255, 0 ) ) );
  painter.setPen( pen );
  painter.setBrush( QBrush( QColor( 255, 255, 255, 0 ) ) );

  QCPScatterStyle scatter( shape, size );
  scatter.drawShape( &painter, double( size + 2 ) / 2.0, double( size + 2 ) / 2.0 );

  return QIcon( QPixmap::fromImage( image ) );
}
