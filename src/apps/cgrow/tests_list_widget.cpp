#include "test_series_dialog.hpp"

#include "tests_list_widget.hpp"

#include <QBrush>
#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>

#include <QDebug>

tests_list_widget::tests_list_widget( QWidget* parent ) : QWidget( parent )
{
  list = new QListWidget;
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
  grid->setMargin( 2 );

  this->setLayout( grid );

  QTimer::singleShot( 2000, [ this ]( ) {
    QFile read_file( QStringLiteral( "save.json" ) );

    if ( !read_file.open( QIODevice::ReadOnly ) )
    {
      qWarning( "Couldn't open read file." );
      return;
    }

    QByteArray saveData = read_file.readAll( );

    QJsonDocument json_document( QJsonDocument::fromJson( saveData ) );

    append( json_document.object( ) );
  } );
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
  while ( tests_.size( ) != 0 )
  {
    auto id = tests_.size( ) - 1;

    tests_.remove( id );

    delete list->takeItem( id );

    current_test_number = 0;

    emit test_series_removed( id );
  }
}

void tests_list_widget::read( const QJsonObject& json )
{
  clear( );

  append( json );
}

void tests_list_widget::append( const QJsonObject& json )
{
  if ( !( json.contains( "Crack Growth Tests" ) && json[ "Crack Growth Tests" ].isArray( ) ) )
  {
    throw std::runtime_error( "Json does not contain Crack Growth test." );
  }
  else
  {
    auto series = json[ "Crack Growth Tests" ].toArray( );
    for ( const auto& v : series )
    {

      if ( !v.isObject( ) )
      {
        throw std::runtime_error( "Json Crack Growth Test format is incorrect." );
      }

      test_series_t test;
      test.read( v.toObject( ) );

      tests_.push_back( test );

      current_test_number++;

      new QListWidgetItem( test.generate_scatter_icon( ), test.name( ), list );

      emit test_series_updated( tests_.size( ) - 1 );
    }
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
  auto data_index = list->currentRow( );

  if ( data_index == -1 )
  {
    return;
  }

  if ( list->selectedItems( ).size( ) == 0 )
  {
    return;
  }

  auto name = list->item( data_index )->text( );

  int ret = QMessageBox::warning( this,
                                  tr( "HS" ),
                                  tr( "Are you sure you want to remove %1?" ).arg( name ),
                                  QMessageBox::Yes | QMessageBox::Cancel );

  if ( ret == QDialog::Rejected )
  {
    return;
  }

  tests_.remove( data_index );

  delete list->takeItem( data_index );

  emit test_series_removed( data_index );
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
