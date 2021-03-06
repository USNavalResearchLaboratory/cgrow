#include "mainwindow.hpp"
#include "qcustomplot.h"
#include <cgrow.hpp>

#include "decorated_spinbox.hpp"
#include "fitting_worker.hpp"
#include "qcp_results_table.hpp"
#include "test_series.hpp"
#include "tests_list_widget.hpp"

#include <QAction>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSplitter>
#include <QStandardPaths>
#include <QToolBar>
#include <QValidator>

#include <QDebug>

#include <array>

namespace cg = crack_growth;

struct lineType
{
  QColor       color;
  Qt::PenStyle ps;
};

std::array< lineType, 20 > standardLineTypes = {
  lineType { QColor { 220, 0, 0 }, Qt::PenStyle::SolidLine },
  lineType { QColor { 0, 220, 0 }, Qt::PenStyle::SolidLine },
  lineType { QColor { 0, 0, 220 }, Qt::PenStyle::SolidLine },
  lineType { QColor { 200, 0, 200 }, Qt::PenStyle::SolidLine },
  lineType { QColor { 0, 200, 200 }, Qt::PenStyle::SolidLine },

  lineType { QColor { 220, 0, 0 }, Qt::PenStyle::DashLine },
  lineType { QColor { 0, 220, 0 }, Qt::PenStyle::DashLine },
  lineType { QColor { 0, 0, 220 }, Qt::PenStyle::DashLine },
  lineType { QColor { 200, 0, 200 }, Qt::PenStyle::DashLine },
  lineType { QColor { 0, 200, 200 }, Qt::PenStyle::DashLine },

  lineType { QColor { 220, 0, 0 }, Qt::PenStyle::DashDotLine },
  lineType { QColor { 0, 220, 0 }, Qt::PenStyle::DashDotLine },
  lineType { QColor { 0, 0, 220 }, Qt::PenStyle::DashDotLine },
  lineType { QColor { 200, 0, 200 }, Qt::PenStyle::DashDotLine },
  lineType { QColor { 0, 200, 200 }, Qt::PenStyle::DashDotLine },

  lineType { QColor { 220, 0, 0 }, Qt::PenStyle::DashDotDotLine },
  lineType { QColor { 0, 220, 0 }, Qt::PenStyle::DashDotDotLine },
  lineType { QColor { 0, 0, 220 }, Qt::PenStyle::DashDotDotLine },
  lineType { QColor { 200, 0, 200 }, Qt::PenStyle::DashDotDotLine },
  lineType { QColor { 0, 200, 200 }, Qt::PenStyle::DashDotDotLine },
};

qreal screenScale( QWidget* widget )
{
  qreal scale = 1.0;

  if ( widget == nullptr )
  {
    return scale;
  }

  QWidget* p      = widget;
  QWidget* parent = p;

  for ( auto p = widget; p != nullptr; p = p->parentWidget( ) )
  {
    parent = p;
  }
  auto screen = QGuiApplication::screenAt(
    parent->geometry( ).center( ) ); // widget->window() is not working as expected here

  if ( screen != nullptr )
  {
    scale = std::max( 0.24, screen->logicalDotsPerInch( ) / 96.0 );
  }

  return scale;
}

template< class T >
std::vector< T > generate_sequence( T from, T to, std::size_t count )
{
  std::vector< T > seq;

  T step = ( to - from ) / ( count - 1 );

  for ( std::size_t i = 0; i != count; i++ )
  {
    seq.push_back( from + step * i );
  }

  return seq;
}

template< class T >
std::vector< T > generate_sequence_log10( T from, T to, std::size_t count )
{
  std::vector< T > seq;

  T froml10 = std::log10( from );
  T step    = ( std::log10( to ) - std::log10( from ) ) / ( count - 1 );

  for ( std::size_t i = 0; i != count; i++ )
  {
    seq.push_back( std::pow( 10.0, froml10 + step * i ) );
  }

  return seq;
}

mainWindow::mainWindow( QWidget* parent ) : QMainWindow( parent )
{
  qRegisterMetaType< hs_parameters_t >( "hs_parameters_t" );

  QSettings defaultSettings( QApplication::organizationName( ), QApplication::applicationName( ) );

  auto homedir       = QDir::homePath( );
  current_directory_ = defaultSettings.value( "current_directory", homedir ).toString( );

  plot = new QCustomPlot;

  plot->xAxis->setLabel( "??K (MPa ?? m????????)" );
  plot->yAxis->setLabel( "da / dN (m/cycle)" );
  plot->yAxis2->setVisible( true );
  plot->xAxis2->setVisible( true );
  plot->xAxis2->setTicks( false );
  plot->yAxis2->setTicks( false );
  plot->xAxis2->setTickLabels( false );
  plot->yAxis2->setTickLabels( false );
  plot->yAxis->grid( )->setSubGridVisible( true );
  plot->xAxis->grid( )->setSubGridVisible( true );

  plot->legend->setVisible( false );
  plot->legend->setBrush( QBrush( QColor( 255, 255, 255, 180 ) ) );
  plot->legend->setBorderPen( Qt::NoPen );
  plot->axisRect( )->insetLayout( )->setInsetAlignment( 0, Qt::AlignLeft | Qt::AlignTop );

  //    text->
  //    QCPItemText *textLabel = new QCPItemText(plot);
  //    textLabel->setPositionAlignment(Qt::AlignRight|Qt::AlignBottom );
  //    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
  //    textLabel->position->setCoords(0.95, 0.95); // bottom right
  //    textLabel->setText("Text Item Demo");
  //    textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
  //    textLabel->setPen(QPen(Qt::black));

  // results_table_ = new qcp_results_table( plot );

  //    auto margins = plot->axisRect()->minimumMargins();

  //    QCPLayoutElement *dummyElement = new QCPLayoutElement( plot );

  //    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
  //    subLayout->addElement(0, 0, plot->legend); // add legend
  //    subLayout->addElement(1, 0, dummyElement); // add dummy element below legend
  //    // subLayout->addElement(0, 1, dummyElement);
  //    subLayout->setRowStretchFactor(0, 0.01);
  //    subLayout->setMargins(QMargins(2,margins.top()-1, 7, margins.bottom()-1));
  //    subLayout->setAutoMargins( QCP::MarginSide::msNone);

  // plot->plotLayout()->setColumnStretchFactor(0,1000);
  // plot->plotLayout()->setColumnStretchFactor(1,0.01);
  // plot->plotLayout()->addElement(0, 1, subLayout);

  worker = new fittingWorker;
  worker->moveToThread( &workerThread );

  connect( &workerThread, &QThread::finished, worker, &QObject::deleteLater );
  connect( worker, &fittingWorker::updatedResults, this, &mainWindow::handleResults );
  connect( worker,
           &fittingWorker::individuallyUpdatedResults,
           this,
           &mainWindow::handleIndividualResults );
  connect( worker, &fittingWorker::progressReport, this, &mainWindow::handleProgressReport );

  connect( worker, &fittingWorker::finished, this, &mainWindow::handle_fitting_finished );

  workerThread.start( );

  int r = 0;

  auto grid = new QGridLayout;

  {
    tests_list = new tests_list_widget( );

    auto hbox = new QHBoxLayout;
    hbox->addWidget( tests_list );
    hbox->setContentsMargins( 2, 2, 2, 2 );
    hbox->setSpacing( 1 );

    auto testsListGroupBox = new QGroupBox( );
    testsListGroupBox->setLayout( hbox );
    testsListGroupBox->setTitle( tr( "Test data" ) );

    grid->addWidget( testsListGroupBox, r, 0, 1, 3 );

    connect( tests_list, SIGNAL( test_series_updated( int ) ), this, SLOT( update_test( int ) ) );

    connect( tests_list, SIGNAL( test_series_removed( int ) ), this, SLOT( remove_test( int ) ) );

    connect( tests_list,
             SIGNAL( current_row_changed( int ) ),
             this,
             SLOT( handle_selected_test_changed( int ) ) );
  }

  {
    auto fgrid = new QGridLayout;
    int  q     = -1;

    {
      QPixmap eqnPic( "://hseqn" );
      auto    eqnLabel = new QLabel;
      eqnLabel->setPixmap(
        eqnPic.scaled( 240, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
      eqnLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
      fgrid->addWidget( eqnLabel, ++q, 0, 1, 5 );

      auto palette = eqnLabel->palette( );
      palette.setColor( eqnLabel->backgroundRole( ), Qt::white );
      eqnLabel->setAutoFillBackground( true );
      eqnLabel->setMargin( 5 );
      eqnLabel->setPalette( palette );

      eqnLabel->setStyleSheet( "border: 1px solid gray; background-color: white;" );
    }

    fgrid->addWidget( new QLabel( tr( "Quantity" ) ), ++q, 0, 1, 1 );
    fgrid->addWidget( new QLabel( tr( "Minumum" ) ), q, 1, 1, 1 );
    fgrid->addWidget( new QLabel( tr( "Auto" ) ), q, 2, 1, 1 );
    fgrid->addWidget( new QLabel( tr( "Maximum" ) ), q, 3, 1, 1 );
    fgrid->addWidget( new QLabel( tr( "Auto" ) ), q, 4, 1, 1 );

    {
      auto validator = new QDoubleValidator( 0, 1e100, 10 );

      D_min = new QLineEdit( "1e-12" );
      D_min->setValidator( validator );

      D_max = new QLineEdit( "5e-5" );
      D_max->setValidator( validator );

      fgrid->addWidget( new QLabel( "D:" ), ++q, 0, 1, 1 );
      fgrid->addWidget( D_min, q, 1, 1, 1 );
      fgrid->addWidget( D_max, q, 3, 1, 1 );
    }

    {
      auto validator = new QDoubleValidator( 0, 10, 4 );

      p_min = new QLineEdit( "0.1" );
      p_min->setValidator( validator );

      p_max = new QLineEdit( "9.0" );
      p_max->setValidator( validator );

      fgrid->addWidget( new QLabel( "p:" ), ++q, 0, 1, 1 );
      fgrid->addWidget( p_min, q, 1, 1, 1 );
      fgrid->addWidget( p_max, q, 3, 1, 1 );
    }

    {
      auto validator = new QDoubleValidator( 0, 1e100, 5 );

      DeltaK_thr_min = new QLineEdit( "0.0001" );
      DeltaK_thr_min->setEnabled( false );
      DeltaK_thr_min->setValidator( validator );

      DeltaK_thr_min_auto = new QCheckBox;
      DeltaK_thr_min_auto->setChecked( true );
      connect( DeltaK_thr_min_auto, &QCheckBox::stateChanged, [ this ]( int ) {
        DeltaK_thr_min_auto_set( );
      } );

      DeltaK_thr_max = new QLineEdit( "60" );
      DeltaK_thr_max->setEnabled( false );
      DeltaK_thr_max->setValidator( validator );

      DeltaK_thr_max_auto = new QCheckBox;
      DeltaK_thr_max_auto->setChecked( true );
      connect( DeltaK_thr_max_auto, &QCheckBox::stateChanged, [ this ]( int ) {
        DeltaK_thr_max_auto_set( );
      } );
      fgrid->addWidget( new QLabel( "??K<sub>thr</sub>:" ), ++q, 0, 1, 1 );
      fgrid->addWidget( DeltaK_thr_min, q, 1, 1, 1 );
      fgrid->addWidget( DeltaK_thr_min_auto, q, 2, 1, 1 );
      fgrid->addWidget( DeltaK_thr_max, q, 3, 1, 1 );
      fgrid->addWidget( DeltaK_thr_max_auto, q, 4, 1, 1 );
    }

    {
      auto validator = new QDoubleValidator( 0, 1e100, 5 );

      A_min = new QLineEdit( "2" );
      A_min->setValidator( validator );
      A_min->setEnabled( false );

      A_min_auto = new QCheckBox;
      A_min_auto->setChecked( true );
      connect( A_min_auto, &QCheckBox::stateChanged, [ this ]( int ) { A_min_auto_set( ); } );

      A_max = new QLineEdit( "300" );
      A_max->setEnabled( false );
      A_max->setValidator( validator );

      A_max_auto = new QCheckBox;
      A_max_auto->setChecked( true );
      connect( A_max_auto, &QCheckBox::stateChanged, [ this ]( int ) { A_max_auto_set( ); } );

      fgrid->addWidget( new QLabel( "A:" ), ++q, 0, 1, 1 );
      fgrid->addWidget( A_min, q, 1, 1, 1 );
      fgrid->addWidget( A_min_auto, q, 2, 1, 1 );
      fgrid->addWidget( A_max, q, 3, 1, 1 );
      fgrid->addWidget( A_max_auto, q, 4, 1, 1 );
    }

    auto ogrid = new QGridLayout;
    int  s     = -1;

    {
      auto validator = new QIntValidator( 3, 1000 );

      subdivisions = new QLineEdit( "14" );
      subdivisions->setValidator( validator );
      ogrid->addWidget( new QLabel( "Subdivisions per dimension:" ), ++s, 0, 1, 3 );
      ogrid->addWidget( subdivisions, s, 3, 1, 1 );
    }

    {
      auto validator = new QDoubleValidator( 1.0000001, 1000, 4 );

      amortization = new QLineEdit( "1.1" );
      amortization->setValidator( validator );
      ogrid->addWidget( new QLabel( "Amortization:" ), ++s, 0, 1, 3 );
      ogrid->addWidget( amortization, s, 3, 1, 1 );
    }

    {
      norm_type = new QComboBox;
      norm_type->addItem( tr( "Ordinary LS" ) );
      norm_type->addItem( tr( "Total LS" ) );

      ogrid->addWidget( new QLabel( "Norm:" ), ++s, 0, 1, 3 );
      ogrid->addWidget( norm_type, s, 3, 1, 1 );
    }

    auto optim_control_group_box = new QGroupBox( );
    optim_control_group_box->setLayout( ogrid );
    optim_control_group_box->setTitle( tr( "Global optimization options" ) );

    fgrid->addWidget( optim_control_group_box, ++q, 0, 1, 5 );

    auto fit_group_box = new QGroupBox( );
    fit_group_box->setLayout( fgrid );
    fit_group_box->setTitle( tr( "Fitting options" ) );

    grid->addWidget( fit_group_box, ++r, 0, 1, 3 );
  }

  grid->addWidget( new QWidget );
  grid->setRowStretch( grid->rowCount( ) - 1, 1 );

  control_widget = new QWidget;
  control_widget->setLayout( grid );

  auto split = new QSplitter;
  split->addWidget( control_widget );
  split->addWidget( plot );

  split->setStretchFactor( 0, 1 );
  split->setStretchFactor( 1, 100 );

  this->setCentralWidget( split );

  {
    QSharedPointer< QCPAxisTickerLog > logTicker( new QCPAxisTickerLog );
    plot->yAxis->setTicker( logTicker );
    plot->yAxis->setScaleType( QCPAxis::stLogarithmic );

    QSharedPointer< QCPAxisTickerLog > logTickerx( new QCPAxisTickerLog );
    plot->xAxis->setTicker( logTickerx );
    plot->xAxis->setScaleType( QCPAxis::stLogarithmic );
  }

  progressBar = new QProgressBar;

  this->statusBar( )->addPermanentWidget( progressBar );

  toolbar = new QToolBar( tr( "Actions" ) );
  {
    addToolBar( Qt::TopToolBarArea, toolbar );

    toolbar->setIconSize( QSize( 64, 64 ) );

    toolbar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
  }

  {
    new_file_action = new QAction( tr( "New" ), this );

    new_file_action->setIcon( QIcon( "://assets/icons/file_new.png" ) );
    new_file_action->setShortcut( tr( "Ctrl+N" ) );
    new_file_action->setStatusTip( tr( "New computation" ) );

    toolbar->addAction( new_file_action );

    connect( new_file_action, SIGNAL( triggered( ) ), this, SLOT( new_file( ) ) );
  }

  {
    open_file_action = new QAction( tr( "Open" ), this );

    open_file_action->setIcon( QIcon( "://assets/icons/file_open.png" ) );
    open_file_action->setShortcut( tr( "Ctrl+O" ) );
    open_file_action->setStatusTip( tr( "Open file." ) );

    toolbar->addAction( open_file_action );

    connect( open_file_action, &QAction::triggered, [ this ]( ) {
      QString fileName
        = QFileDialog::getOpenFileName( nullptr,
                                        tr( "Load data" ),
                                        current_directory_,
                                        tr( "Crack Growth Files (*.cg.json);; All Files (*.*)" ) );

      if ( fileName.isEmpty( ) )
        return;

      QFileInfo fi( fileName );

      QFile read_file( fileName );

      if ( !read_file.open( QIODevice::ReadOnly ) )
      {
        qWarning( "Couldn't open read file." );
        return;
      }

      for ( auto* g : computed_graphs )
      {
        plot->removeGraph( g );
      }
      computed_graphs.clear( );

      QByteArray data = read_file.readAll( );

      QJsonDocument json_document( QJsonDocument::fromJson( data ) );

      tests_list->read( json_document.object( ) );

      current_directory_ = fi.canonicalPath( );

      QSettings defaultSettings( QApplication::organizationName( ),
                                 QApplication::applicationName( ) );

      defaultSettings.setValue( "current_directory", current_directory_ );
    } );
  }

  {
    save_file_action = new QAction( tr( "Save" ), this );

    save_file_action->setIcon( QIcon( "://assets/icons/file_save.png" ) );
    save_file_action->setShortcut( tr( "Ctrl+S" ) );
    save_file_action->setStatusTip( tr( "Save crack growth data." ) );

    toolbar->addAction( save_file_action );

    connect( save_file_action, &QAction::triggered, [ this ]( ) {
      QString defaultFileName;

      if ( tests_list->tests( ).size( ) == 1 )
      {
        defaultFileName = tests_list->tests( )[ 0 ].name( );
      }

      auto dir = QDir( current_directory_ );

      QString fileName
        = QFileDialog::getSaveFileName( nullptr,
                                        tr( "Save data" ),
                                        dir.absoluteFilePath( defaultFileName ),
                                        tr( "Crack Groth Files (*.cg.json);; All Files (*.*)" ) );

      if ( fileName.isEmpty( ) )
        return;

      QFileInfo fi( fileName );

      QJsonObject json_object;

      tests_list->write( json_object );

      QFile saveFile( fi.absoluteFilePath( ) );

      if ( !saveFile.open( QIODevice::WriteOnly ) )
      {
        qWarning( "Couldn't open save file." );
        return;
      }

      QJsonDocument saveDoc( json_object );
      saveFile.write( saveDoc.toJson( ) );

      current_directory_ = fi.canonicalPath( );

      QSettings defaultSettings( QApplication::organizationName( ),
                                 QApplication::applicationName( ) );

      defaultSettings.setValue( "current_directory", current_directory_ );
    } );
  }

  {
    compute_action = new QAction( tr( "Compute" ), this );

    compute_action->setIcon( QIcon( "://assets/icons/process.png" ) );
    compute_action->setShortcut( tr( "Ctrl+G" ) );
    compute_action->setStatusTip( tr( "Compute HS parameters." ) );

    toolbar->addAction( compute_action );

    connect( compute_action, &QAction::triggered, [ this ]( ) {
      if ( !worker->running( ) )
      {
        bool individually = false;
        fit( individually );
      }
      else
      {
        worker->stop( );
      }
    } );
  }

  {
    compute_individually_action = new QAction( tr( "Compute Individually" ), this );

    compute_individually_action->setIcon( QIcon( "://assets/icons/process.png" ) );
    compute_individually_action->setShortcut( tr( "Ctrl+I" ) );
    compute_individually_action->setStatusTip(
      tr( "Compute HS parameters individually for each dataset." ) );

    toolbar->addAction( compute_individually_action );

    connect( compute_individually_action, &QAction::triggered, [ this ]( ) {
      if ( !worker->running( ) )
      {
        bool individually = true;
        fit( individually );
      }
      else
      {
        worker->stop( );
      }
    } );
  }

  {
    to_excel_action = new QAction( tr( "Excel Formula" ), this );

    to_excel_action->setIcon( QIcon( "://assets/icons/to_excel.png" ) );
    to_excel_action->setShortcut( tr( "Ctrl+E" ) );
    to_excel_action->setStatusTip( tr( "Export results as excel formula." ) );

    toolbar->addAction( to_excel_action );

    connect( to_excel_action, &QAction::triggered, [ this ]( ) {
      QClipboard* clipboard = QApplication::clipboard( );

      QString table;
      for ( const auto& c : computed_ )
      {
        auto text
          = QString( "= %2 * power( (%1 - %4) / sqrt( 1 - %1/ ( %5 * ( 1.0 - R ) ) ), %3)\n" )
              .arg( "INDIRECT(\"RC[-1]\",0)" )
              .arg( double( c.D ) )
              .arg( double( c.p ) )
              .arg( double( c.DeltaK_thr ) )
              .arg( double( c.A ) );
        table.append( text );
      }

      clipboard->setText( table, QClipboard::Clipboard );

      if ( clipboard->supportsSelection( ) )
      {
        clipboard->setText( table, QClipboard::Selection );
      }

#if defined( Q_OS_LINUX )
      QThread::msleep( 1 ); // workaround for copied text not being available...
#endif

      QMessageBox::information( this,
                                tr( "Excel formula" ),
                                tr( "Excel formula copied to clipboard.\n"
                                    "Paste as text in a spreadsheet cell\n"
                                    "and replace R." ) );
    } );

    to_excel_action->setEnabled( false );
  }

  {
    to_tabulated_action = new QAction( tr( "To Table" ), this );

    to_tabulated_action->setIcon( QIcon( "://assets/icons/to_table.png" ) );
    to_tabulated_action->setShortcut( tr( "Ctrl+T" ) );
    to_tabulated_action->setStatusTip( tr( "Export results to a tabulated form." ) );

    toolbar->addAction( to_tabulated_action );

    connect( to_tabulated_action, &QAction::triggered, [ this ]( ) {
      QClipboard* clipboard = QApplication::clipboard( );

      QString     table( "Name\tD\tp\t??Kthr\t??\n" );
      std::size_t id = 0;
      for ( const auto& c : computed_ )
      {
        auto text = QString( "%1\t%2\t%3\t%4\t%5\n" )
                      .arg( tests_list->tests( )[ id++ ].name( ) )
                      .arg( double( c.D ) )
                      .arg( double( c.p ) )
                      .arg( double( c.DeltaK_thr ) )
                      .arg( double( c.A ) );
        table.append( text );
      }

      clipboard->setText( table, QClipboard::Clipboard );

      if ( clipboard->supportsSelection( ) )
      {
        clipboard->setText( table, QClipboard::Selection );
      }

#if defined( Q_OS_LINUX )
      QThread::msleep( 1 ); // workaround for copied text not being available...
#endif

      QMessageBox::information( this,
                                tr( "Table export" ),
                                tr( "Table copied to clipboard.\n"
                                    "Order: D p ??Kthr A\n" ) );
    } );

    to_excel_action->setEnabled( false );
  }

  {
    font_size_spinbox
      = new decorated_double_spinbox( "", 9, 0.5, 200, QPixmap( "://assets/icons/font_size.png" ) );
    font_size_spinbox->setToolTip( tr( "Graph elements font size." ) );
    font_size_spinbox->setMaximumWidth( 128 );
    font_size_spinbox->set_single_step( 1 );
    font_size_spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    connect( font_size_spinbox,
             SIGNAL( value_changed( double ) ),
             this,
             SLOT( change_font_size( double ) ) );
  }

  {
    legend_columns_spinbox
      = new decorated_int_spinbox( "", 1, 1, 20, QPixmap( "://assets/icons/legend_columns.png" ) );
    legend_columns_spinbox->setToolTip( tr( "Number of legend columns." ) );
    legend_columns_spinbox->setMaximumWidth( 128 );
    legend_columns_spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    connect( legend_columns_spinbox,
             SIGNAL( value_changed( int ) ),
             this,
             SLOT( change_legend_columns( int ) ) );
  }

  {
    axes_line_width_spinbox = new decorated_double_spinbox(
      "", 1, 0.25, 200, QPixmap( "://assets/icons/axes_line_width.png" ) );
    axes_line_width_spinbox->setToolTip( tr( "Graph axes line width." ) );
    axes_line_width_spinbox->setMaximumWidth( 128 );
    axes_line_width_spinbox->set_single_step( 0.25 );
    axes_line_width_spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    connect( axes_line_width_spinbox,
             SIGNAL( value_changed( double ) ),
             this,
             SLOT( change_axes_line_width( double ) ) );
  }

  {
    grid_line_width_spinbox = new decorated_double_spinbox(
      "", 1, 0.25, 200, QPixmap( "://assets/icons/grid_line_width.png" ) );
    grid_line_width_spinbox->setToolTip( tr( "Grid lines width." ) );
    grid_line_width_spinbox->setMaximumWidth( 128 );
    grid_line_width_spinbox->set_single_step( 0.25 );
    grid_line_width_spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    connect( grid_line_width_spinbox,
             SIGNAL( value_changed( double ) ),
             this,
             SLOT( change_grid_lines_width( double ) ) );
  }

  {
    marker_size_spinbox = new decorated_double_spinbox(
      "", 6, 0.5, 200, QPixmap( "://assets/icons/marker_resize.png" ) );
    marker_size_spinbox->setToolTip( tr( "Data series marker size." ) );
    marker_size_spinbox->setMaximumWidth( 128 );
    marker_size_spinbox->set_single_step( 0.5 );
    marker_size_spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    connect( marker_size_spinbox,
             SIGNAL( value_changed( double ) ),
             this,
             SLOT( change_marker_size( double ) ) );
  }

  {
    marker_line_width_spinbox = new decorated_double_spinbox(
      "", 1.5, 0.25, 200, QPixmap( "://assets/icons/marker_line_width.png" ) );
    marker_size_spinbox->setToolTip( tr( "Data series marker line width." ) );
    marker_line_width_spinbox->setMaximumWidth( 128 );
    marker_line_width_spinbox->set_single_step( 0.25 );
    marker_line_width_spinbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    connect( marker_line_width_spinbox,
             SIGNAL( value_changed( double ) ),
             this,
             SLOT( change_marker_line_width( double ) ) );
  }

  QWidget* axes_spec_labels = new QWidget;
  {
    x_quantity_label = new QLineEdit( tr( "??K" ) );
    x_quantity_label->setMinimumWidth( 64 );
    x_quantity_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    connect(
      x_quantity_label, SIGNAL( textChanged( QString ) ), this, SLOT( change_x_spec_label( ) ) );

    x_units_label = new QLineEdit( tr( "(MPa ?? m????????)" ) );
    x_units_label->setMinimumWidth( 64 );
    x_units_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    connect(
      x_units_label, SIGNAL( textChanged( QString ) ), this, SLOT( change_x_spec_label( ) ) );

    auto y_spec_label = new QLineEdit( tr( "m/cycle" ) );
    y_spec_label->setMinimumWidth( 64 );
    y_spec_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    connect( y_spec_label,
             SIGNAL( textChanged( QString ) ),
             this,
             SLOT( change_y_spec_label( QString ) ) );

    auto ugrid = new QGridLayout;
    ugrid->addWidget( new QLabel( "x quantity label: " ), 0, 0, 1, 1 );
    ugrid->addWidget( x_quantity_label, 1, 0, 1, 1 );
    ugrid->addWidget( new QLabel( "x spec. label: " ), 2, 0, 1, 1 );
    ugrid->addWidget( x_units_label, 3, 0, 1, 1 );
    ugrid->addWidget( new QLabel( "y spec. label: " ), 4, 0, 1, 1 );
    ugrid->addWidget( y_spec_label, 5, 0, 1, 1 );

    axes_spec_labels->setLayout( ugrid );
  }

  QWidget* axes_ranges = new QWidget;
  {
    xmin_label = new QLineEdit( tr( " 0.00" ) );
    xmin_label->setEnabled( false );
    xmin_label->setMinimumWidth( 48 );
    xmin_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    connect( xmin_label, &QLineEdit::textChanged, [ this ]( QString ) {
      rescale_plot( );
      plot->replot( );
    } );

    xmin_auto = new QCheckBox( tr( " Auto" ) );
    xmin_auto->setChecked( true );
    connect( xmin_auto, &QCheckBox::stateChanged, [ this ]( int newState ) {
      xmin_label->setEnabled( !( newState == Qt::Checked ) );
      rescale_plot( );
      plot->replot( );
    } );

    xmax_label = new QLineEdit( tr( "0.00" ) );
    xmax_label->setEnabled( false );
    xmax_label->setMinimumWidth( 48 );
    xmax_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    connect( xmax_label, &QLineEdit::textChanged, [ this ]( QString ) {
      rescale_plot( );
      plot->replot( );
    } );

    xmax_auto = new QCheckBox( tr( " Auto" ) );
    xmax_auto->setChecked( true );
    connect( xmax_auto, &QCheckBox::stateChanged, [ this ]( int newState ) {
      xmax_label->setEnabled( !( newState == Qt::Checked ) );
      rescale_plot( );
      plot->replot( );
    } );

    ymin_label = new QLineEdit( tr( "0.00" ) );
    ymin_label->setEnabled( false );
    ymin_label->setMinimumWidth( 48 );
    ymin_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    connect( ymin_label, &QLineEdit::textChanged, [ this ]( QString ) {
      rescale_plot( );
      plot->replot( );
    } );

    ymin_auto = new QCheckBox( tr( " Auto" ) );
    ymin_auto->setChecked( true );
    connect( ymin_auto, &QCheckBox::stateChanged, [ this ]( int newState ) {
      ymin_label->setEnabled( !( newState == Qt::Checked ) );
      rescale_plot( );
      plot->replot( );
    } );

    ymax_label = new QLineEdit( tr( "0.00" ) );
    ymax_label->setEnabled( false );
    ymax_label->setMinimumWidth( 48 );
    ymax_label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
    connect( ymax_label, &QLineEdit::textChanged, [ this ]( QString ) {
      rescale_plot( );
      plot->replot( );
    } );

    ymax_auto = new QCheckBox( tr( " Auto" ) );
    ymax_auto->setChecked( true );
    connect( ymax_auto, &QCheckBox::stateChanged, [ this ]( int newState ) {
      ymax_label->setEnabled( !( newState == Qt::Checked ) );
      rescale_plot( );
      plot->replot( );
    } );

    auto rgrid = new QGridLayout;
    rgrid->addWidget( new QLabel( "Axes ranges: " ), 0, 0, 1, 3 );

    rgrid->addWidget( new QLabel( "x min:" ), 1, 0, 1, 1 );
    rgrid->addWidget( xmin_auto, 1, 2, 1, 1 );
    rgrid->addWidget( xmin_label, 1, 1, 1, 1 );

    rgrid->addWidget( new QLabel( "x max:" ), 2, 0, 1, 1 );
    rgrid->addWidget( xmax_auto, 2, 2, 1, 1 );
    rgrid->addWidget( xmax_label, 2, 1, 1, 1 );

    rgrid->addWidget( new QLabel( "y min:" ), 3, 0, 1, 1 );
    rgrid->addWidget( ymin_auto, 3, 2, 1, 1 );
    rgrid->addWidget( ymin_label, 3, 1, 1, 1 );

    rgrid->addWidget( new QLabel( "y max:" ), 4, 0, 1, 1 );
    rgrid->addWidget( ymax_auto, 4, 2, 1, 1 );
    rgrid->addWidget( ymax_label, 4, 1, 1, 1 );

    rgrid->setContentsMargins( 1, 2, 4, 2 );
    axes_ranges->setLayout( rgrid );
  }

  QWidget* save_image = new QWidget;
  {
    auto validator        = new QIntValidator;
    auto save_image_width = new QLineEdit( "3000" );
    save_image_width->setValidator( validator );
    save_image_width->setMinimumWidth( 64 );
    save_image_width->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );

    auto save_image_button = new QPushButton( "Save Plot Image" );

    auto sgrid = new QGridLayout;
    sgrid->addWidget( new QLabel( "Save image options:" ), 0, 0, 1, 2 );
    sgrid->addWidget( new QLabel( "Width (pixels): " ), 1, 0, 1, 1 );
    sgrid->addWidget( save_image_width, 1, 1, 1, 1 );
    sgrid->addWidget( save_image_button, 2, 0, 1, 2 );
    save_image->setLayout( sgrid );

    connect( save_image_button, &QPushButton::clicked, [ this, save_image_width ]( ) {
      QString defaultFileName;

      if ( tests_list->tests( ).size( ) == 1 )
      {
        defaultFileName = tests_list->tests( )[ 0 ].name( );
      }

      auto dir = QDir( current_directory_ );

      QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        tr( "Save plot" ),
        dir.absoluteFilePath( defaultFileName ),
        tr( "PNG Files (*.png);; PDF Files (*.pdf);;All Files (*.*)" ) );

      if ( fileName.isEmpty( ) )
        return;

      QFileInfo fi( fileName );

      save_figure( fi.absoluteFilePath( ), save_image_width->text( ).toInt( ) );

      current_directory_ = fi.canonicalPath( );

      QSettings defaultSettings( QApplication::organizationName( ),
                                 QApplication::applicationName( ) );

      defaultSettings.setValue( "current_directory", current_directory_ );
    } );
  }

  {
    auto grid1 = new QGridLayout;

    grid1->setContentsMargins( 2, 2, 2, 2 );
    grid1->setSpacing( 2 );

    grid1->addWidget( font_size_spinbox, 0, 0 );
    grid1->addWidget( legend_columns_spinbox, 0, 1 );
    grid1->addWidget( axes_line_width_spinbox, 1, 0 );
    grid1->addWidget( grid_line_width_spinbox, 1, 1 );
    grid1->addWidget( marker_size_spinbox, 2, 0 );
    grid1->addWidget( marker_line_width_spinbox, 2, 1 );

    auto vbox = new QVBoxLayout;
    vbox->setContentsMargins( 2, 2, 2, 2 );
    vbox->setSpacing( 2 );
    vbox->addWidget( new QLabel( tr( "Plot Options" ) ) );
    vbox->addLayout( grid1 );

    vbox->addWidget( axes_spec_labels );
    vbox->addWidget( axes_ranges );
    vbox->addStretch( 2 );
    vbox->addWidget( save_image );

    QWidget* separator = new QWidget( this );
    separator->setLayout( vbox );
    separator->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    auto plot_toolbar = new QToolBar( tr( "Plot options" ) );
    plot_toolbar->setIconSize( QSize( 64, 64 ) );
    plot_toolbar->addWidget( separator );
    addToolBar( Qt::RightToolBarArea, plot_toolbar );
  }

  auto globalCursorPos = QCursor::pos( );
  auto mouseScreen     = QGuiApplication::screenAt( globalCursorPos );

  auto        sr = mouseScreen->availableGeometry( );
  const QRect wr( { }, QSize( 800, 600 ).boundedTo( sr.size( ) ) );

  this->resize( wr.size( ) );
  this->move( sr.center( ) - wr.center( ) );
}

mainWindow::~mainWindow( )
{
  worker->stop( );
  workerThread.quit( );
  workerThread.wait( 10000 );
  if ( workerThread.isRunning( ) )
  {
    qDebug( ) << "Terminating fitting thread.";
    workerThread.terminate( );
  }
}

void mainWindow::resizeEvent( QResizeEvent* event )
{
  QMainWindow::resizeEvent( event );

  auto scale = screenScale( this );

  toolbar->setIconSize( QSize( int( scale * 64 ), int( scale * 64 ) ) );
}

void mainWindow::new_file( )
{
  tests_list->clear( );

  Rs.clear( );

  for ( auto* g : computed_graphs )
  {
    plot->removeGraph( g );
  }

  plot->replot( );
}

std::vector< test_data_t > mainWindow::pull_tests_list( )
{
  auto& tests = tests_list->tests( );

  std::vector< test_data_t > tests_list;
  for ( const auto& test : tests )
  {
    tests_list.push_back( test.data );
    Rs.insert( test.data.R );
  }
  return tests_list;
}

void mainWindow::fit( bool individually )
{
  auto& tests = tests_list->tests( );

  Rs.clear( );
  auto tests_to_fit = pull_tests_list( );

  for ( auto g : computed_graphs )
  {
    plot->removeGraph( g );
  }

  computed_graphs.clear( );

  if ( lower_bounds_graph != nullptr )
  {
    plot->removeGraph( lower_bounds_graph );
    lower_bounds_graph = nullptr;
  }

  if ( upper_bounds_graph != nullptr )
  {
    plot->removeGraph( upper_bounds_graph );
    upper_bounds_graph = nullptr;
  }

  auto iline = 0;
  if ( individually )
  {
    computed_.resize( tests.size( ) );
    for ( const auto& test : tests )
    {
      auto* g = plot->addGraph( );
      computed_graphs.push_back( g );
      g->setName( QString( "%1 - computed" ).arg( test.name( ) ) );
      auto pen = g->pen( );

      const auto& lt = standardLineTypes[ iline++ ];

      pen.setWidthF( 3.0f );
      pen.setColor( test.marker.color( ) );
      pen.setStyle( lt.ps );
      g->setPen( pen );

      if ( iline == standardLineTypes.size( ) )
      {
        iline = 0;
      }
    }
    compute_individually_action->setIcon( QIcon( "://assets/icons/process_stop.png" ) );
    compute_individually_action->setText( tr( "Stop" ) );
    compute_action->setEnabled( false );
  }
  else
  {
    computed_.resize( 1 );
    for ( double R : Rs )
    {
      auto* g = plot->addGraph( );
      computed_graphs.push_back( g );
      g->setName( QString( "Computed R = %1" ).arg( R ) );
      auto pen = g->pen( );

      const auto& lt = standardLineTypes[ iline++ ];

      pen.setWidthF( 2.0f );
      pen.setColor( lt.color );
      pen.setStyle( lt.ps );
      g->setPen( pen );

      if ( iline == standardLineTypes.size( ) )
      {
        iline = 0;
      }
    }

    // Lower and upper fit bounds
    lower_bounds_graph = plot->addGraph( );
    lower_bounds_graph->setName( "Lower Opt. Bound" );
    lower_bounds_graph->setPen( QPen( Qt::gray ) );
    lower_bounds_graph->setBrush( QBrush( QColor( 0, 0, 255, 20 ) ) );

    upper_bounds_graph = plot->addGraph( );
    upper_bounds_graph->setName( "Upper Opt. Bound" );
    upper_bounds_graph->setPen( QPen( Qt::gray ) );
    lower_bounds_graph->setChannelFillGraph( upper_bounds_graph );

    compute_action->setIcon( QIcon( "://assets/icons/process_stop.png" ) );
    compute_action->setText( tr( "Stop" ) );
    compute_individually_action->setEnabled( false );
  }

  auto params_low = hs_parameters_t { D_min->text( ).toDouble( ),
                                      p_min->text( ).toDouble( ),
                                      DeltaK_thr_min->text( ).toDouble( ),
                                      A_min->text( ).toDouble( ) };

  auto params_high = hs_parameters_t { D_max->text( ).toDouble( ),
                                       p_max->text( ).toDouble( ),
                                       DeltaK_thr_max->text( ).toDouble( ),
                                       A_max->text( ).toDouble( ) };

  bool use_geometric = false;

  if ( norm_type->currentIndex( ) == 1 )
  {
    use_geometric = true;
  }

  new_file_action->setEnabled( false );
  control_widget->setEnabled( false );
  open_file_action->setEnabled( false );
  save_file_action->setEnabled( false );
  to_excel_action->setEnabled( true );

  auto autoRange = Hartman_Schijve_autoRange { DeltaK_thr_min_auto->isChecked( ),
                                               DeltaK_thr_max_auto->isChecked( ),
                                               A_min_auto->isChecked( ),
                                               A_max_auto->isChecked( ) };
  QMetaObject::invokeMethod( worker,
                             "run",
                             Q_ARG( hs_parameters_t, params_low ),
                             Q_ARG( hs_parameters_t, params_high ),
                             Q_ARG( int, subdivisions->text( ).toInt( ) ),
                             Q_ARG( double, amortization->text( ).toDouble( ) ),
                             Q_ARG( bool, use_geometric ),
                             Q_ARG( std::vector< test_data_t >, tests_to_fit ),
                             Q_ARG( Hartman_Schijve_autoRange, autoRange ),
                             Q_ARG( bool, individually ) );
}

void mainWindow::handleResults( hs_parameters_t params,
                                hs_parameters_t params_lower,
                                hs_parameters_t params_upper )
{
  if ( computed_.size( ) != 1 )
  {
    throw std::logic_error( "Illegal size of computed vector." );
  }

  computed_[ 0 ] = params;

  auto generate_hs_curve = []( const auto& params, const auto& R ) {
    auto DeltaK_max = cg::Hartman_Schijve::calc_K_max( params, R );

    auto DKs = generate_sequence_log10( params.DeltaK_thr * ( 1.0 + 1e-6 ), // From
                                        DeltaK_max * ( 1.0 - 1e-6 ),        // To
                                        240                                 // Number of points
    );

    auto dadNs = cg::Hartman_Schijve::evaluate( params, R, DKs );

    return std::make_tuple( DKs, dadNs );
  };

  std::size_t c    = 0;
  real_t      Rmin = 1.0;
  real_t      Rmax = -1.0;

  for ( real_t R : Rs )
  {
    Rmin = std::min( R, Rmin );
    Rmax = std::max( R, Rmax );

    {
      auto [ DKs, dadNs ] = generate_hs_curve( params, R );

      QVector< double > xs;
      for ( auto x : DKs )
      {
        xs.push_back( double( x ) );
      }

      QVector< double > ys;
      for ( auto y : dadNs )
      {
        ys.push_back( double( y ) );
      }

      computed_graphs[ c ]->setData( xs, ys, true );

      computed_graphs[ c ]->setName( QString( "Computed, R=%1, D=%2, p=%3, %4?????????=%5, A=%6" )
                                       .arg( double( R ), 0, 'g', 2 )
                                       .arg( double( params.D ), 0, 'g', 3 )
                                       .arg( double( params.p ), 0, 'g', 3 )
                                       .arg( x_quantity_label->text( ) )
                                       .arg( double( params.DeltaK_thr ), 0, 'g', 3 )
                                       .arg( double( params.A ), 0, 'g', 3 ) );
      c++;
    }
  }

  // TODO: replacements to correct for the virtual abberations think a bit about this
  params_lower.D = params.D;
  params_lower.p = params.p;
  params_upper.D = params.D;
  params_upper.p = params.p;

  auto [ DKs1, dadNs1 ] = generate_hs_curve( params_lower, Rmax );
  auto [ DKs2, dadNs2 ] = generate_hs_curve( params_upper, Rmin );

  if ( std::isnan( double( dadNs2[ 0 ] ) ) )
  {
    dadNs2[ 0 ] = 0;
  }

  DKs2.insert( DKs2.begin( ), DKs1[ 0 ] ); // So we can force QCustomplot to fill the area.
  dadNs2.insert( dadNs2.begin( ), dadNs1[ 0 ] );

  DKs1.push_back( *DKs2.rbegin( ) );
  dadNs1.push_back( *dadNs2.rbegin( ) );

  this->setLow( DKs1, dadNs1 );
  this->setHigh( DKs2, dadNs2 );

  rescale_plot( );

  plot->replot( );
}

void mainWindow::handleIndividualResults( hs_parameters_t params,
                                          hs_parameters_t /*params_lower*/,
                                          hs_parameters_t /*params_upper*/,
                                          int id )
{
  if ( computed_.size( ) != std::size_t( tests_list->tests( ).size( ) ) )
  {
    throw std::logic_error( "Illegal size of computed vector." );
  }

  computed_[ id ] = params;

  const auto& test = tests_list->tests( )[ id ];

  auto dkmin = std::numeric_limits< real_t >::max( );
  auto dkmax = std::numeric_limits< real_t >::min( );

  for ( auto d : test.data.points )
  {
    dkmin = std::min( dkmin, real_t( d.DeltaK ) );
    dkmax = std::max( dkmax, real_t( d.DeltaK ) );
  }

  auto DeltaK_min = std::max( dkmin, params.DeltaK_thr * ( 1.0 + 1e-6 ) );
  auto DeltaK_max = dkmax;

  auto generate_hs_curve = [ DeltaK_min, DeltaK_max ]( const auto& params, const auto& R ) {
    auto DKs = generate_sequence_log10( DeltaK_min, // From
                                        DeltaK_max, // To
                                        240         // Number of points
    );

    auto dadNs = cg::Hartman_Schijve::evaluate( params, R, DKs );

    return std::make_tuple( DKs, dadNs );
  };

  real_t R = tests_list->tests( )[ id ].data.R;

  {
    auto [ DKs, dadNs ] = generate_hs_curve( params, R );

    QVector< double > xs;
    for ( auto x : DKs )
    {
      xs.push_back( double( x ) );
    }

    QVector< double > ys;
    for ( auto y : dadNs )
    {
      ys.push_back( double( y ) );
    }

    computed_graphs[ id ]->setData( xs, ys, true );
    computed_graphs[ id ]->setName( QString( "%1, D=%2, p=%3, %4?????????=%5, A=%6" )
                                      .arg( test.name( ) )
                                      .arg( double( params.D ), 0, 'g', 3 )
                                      .arg( double( params.p ), 0, 'g', 3 )
                                      .arg( x_quantity_label->text( ) )
                                      .arg( double( params.DeltaK_thr ), 0, 'g', 3 )
                                      .arg( double( params.A ), 0, 'g', 3 ) );
  }

  //  // TODO: replacements to correct for the virtual abberations
  //  params_lower.D = params.D;
  //  params_lower.p = params.p;
  //  params_upper.D = params.D;
  //  params_upper.p = params.p;

  //  auto [ DKs1, dadNs1 ] = generate_hs_curve( params_lower, R );
  //  auto [ DKs2, dadNs2 ] = generate_hs_curve( params_upper, R );

  //  if ( std::isnan( double( dadNs2[ 0 ] ) ) )
  //  {
  //    dadNs2[ 0 ] = 0;
  //  }

  //  DKs2.insert( DKs2.begin( ), DKs1[ 0 ] ); // So we can force QCustomplot to fill the area.
  //  dadNs2.insert( dadNs2.begin( ), dadNs1[ 0 ] );

  //  DKs1.push_back( *DKs2.rbegin( ) );
  //  dadNs1.push_back( *dadNs2.rbegin( ) );

  //  this->setLow( DKs1, dadNs1 );
  //  this->setHigh( DKs2, dadNs2 );

  rescale_plot( );

  plot->replot( );
}

void mainWindow::handle_fitting_finished( )
{
  new_file_action->setEnabled( true );
  control_widget->setEnabled( true );
  open_file_action->setEnabled( true );
  save_file_action->setEnabled( true );

  compute_action->setIcon( QIcon( "://assets/icons/process.png" ) );
  compute_action->setText( tr( "Compute" ) );
  compute_action->setEnabled( true );

  compute_individually_action->setIcon( QIcon( "://assets/icons/process.png" ) );
  compute_individually_action->setText( tr( "Compute Individually" ) );
  compute_individually_action->setEnabled( true );

  progressBar->hide( );

  // plot->removeGraph( lower_bounds_graph );
  //  lower_bounds_graph = nullptr;

  // plot->removeGraph( upper_bounds_graph );
  // upper_bounds_graph = nullptr;

  rescale_plot( );
  plot->replot( );
}

void mainWindow::DeltaK_thr_min_auto_set( )
{
  if ( DeltaK_thr_min_auto->isChecked( ) )
  {

    DeltaK_thr_min->setText( "0.0001" );
    DeltaK_thr_min->setEnabled( false );
  }
  else
  {
    DeltaK_thr_min->setEnabled( true );
  }
}

void mainWindow::DeltaK_thr_max_auto_set( )
{
  if ( DeltaK_thr_max_auto->isChecked( ) )
  {
    DeltaK_thr_max->setText( QString( "%1" ).arg( tests_min_DeltaK_thr( ) ) );
    DeltaK_thr_max->setEnabled( false );
  }
  else
  {
    DeltaK_thr_max->setText( "60" );
    DeltaK_thr_max->setEnabled( true );
  }
}

void mainWindow::A_min_auto_set( )
{
  if ( A_min_auto->isChecked( ) )
  {
    A_min->setText( QString( "%1" ).arg( 0.8 * tests_max_A( ) ) );
    A_min->setEnabled( false );
  }
  else
  {
    A_min->setEnabled( true );
  }
}

void mainWindow::A_max_auto_set( )
{
  if ( A_max_auto->isChecked( ) )
  {
    A_max->setText( QString( "%1" ).arg( tests_max_A( ) * 5.0 ) );
    A_max->setEnabled( false );
  }
  else
  {
    A_max->setEnabled( true );
  }
}

void mainWindow::auto_set( )
{
  DeltaK_thr_min_auto_set( );

  DeltaK_thr_max_auto_set( );

  A_min_auto_set( );

  A_max_auto_set( );
}

void mainWindow::handleProgressReport( int i, int total )
{

  progressBar->show( );
  progressBar->setRange( 0, total - 1 );
  progressBar->setValue( i );

  //  if ( i == total - 1 )
  //  {
  //    handle_fitting_finished( );
  //  }
}

void mainWindow::remove_test( int index )
{
  if ( tests_graph_map.size( ) > index )
  {
    if ( tests_graph_map[ index ] != nullptr )
    {
      plot->removeGraph( tests_graph_map[ index ] );
      tests_graph_map[ index ] = nullptr;
      tests_graph_map.remove( index );
    }
    else
    {
      throw std::out_of_range( "not graph at index." );
    }
  }
  else
  {
    throw std::out_of_range( "test index not found." );
  }

  auto_set( );

  rescale_plot( );

  plot->replot( );
}

void mainWindow::change_font_size( double new_size )
{
  {
    auto font = plot->xAxis->labelFont( );
    font.setPointSizeF( new_size );
    plot->xAxis->setLabelFont( font );
  }

  {
    auto font = plot->xAxis->tickLabelFont( );
    font.setPointSizeF( new_size );
    plot->xAxis->setTickLabelFont( font );
  }

  {
    auto font = plot->yAxis->labelFont( );
    font.setPointSizeF( new_size );
    plot->yAxis->setLabelFont( font );
  }

  {
    auto font = plot->yAxis->tickLabelFont( );
    font.setPointSizeF( new_size );
    plot->yAxis->setTickLabelFont( font );
  }
  {
    auto font = plot->legend->font( );
    font.setPointSizeF( new_size );
    plot->legend->setFont( font );
  }

  //  {
  //    auto font = results_table_->font( );
  //    font.setPointSizeF( new_size );
  //    results_table_->set_font( font );
  //  }

  auto font = plot->font( );
  font.setPointSizeF( new_size );
  plot->setFont( font );
  plot->xAxis->setLabelFont( font );

  plot->replot( );
}

void mainWindow::change_legend_columns( int columns )
{
  plot->legend->setWrap( columns );
  plot->legend->setRowSpacing( 0 );
  plot->legend->setColumnSpacing( 4 );
  plot->legend->setFillOrder( QCPLayoutGrid::FillOrder::foColumnsFirst, true );

  plot->replot( );
}

void mainWindow::change_axes_line_width( double new_size )
{
  {
    auto pen = plot->xAxis->tickPen( );
    pen.setWidthF( new_size );
    plot->xAxis->setTickPen( pen );
  }

  {
    auto pen = plot->xAxis->basePen( );
    pen.setWidthF( new_size );
    plot->xAxis->setBasePen( pen );
  }

  {
    auto pen = plot->xAxis2->basePen( );
    pen.setWidthF( new_size );
    plot->xAxis2->setBasePen( pen );
  }

  {
    auto pen = plot->yAxis->tickPen( );
    pen.setWidthF( new_size );
    plot->yAxis->setTickPen( pen );
  }

  {
    auto pen = plot->yAxis->basePen( );
    pen.setWidthF( new_size );
    plot->yAxis->setBasePen( pen );
  }
  {
    auto pen = plot->yAxis2->basePen( );
    pen.setWidthF( new_size );
    plot->yAxis2->setBasePen( pen );
  }

  plot->replot( );
}

void mainWindow::change_grid_lines_width( double new_size )
{
  {
    auto pen = plot->xAxis->grid( )->pen( );
    pen.setWidthF( new_size );
    plot->xAxis->grid( )->setPen( pen );

    pen = plot->xAxis->grid( )->subGridPen( );
    pen.setWidthF( new_size );
    plot->xAxis->grid( )->setSubGridPen( pen );
  }

  {
    auto pen = plot->yAxis->grid( )->pen( );
    pen.setWidthF( new_size );
    plot->yAxis->grid( )->setPen( pen );

    pen = plot->yAxis->grid( )->subGridPen( );
    pen.setWidthF( new_size );
    plot->yAxis->grid( )->setSubGridPen( pen );
  }

  plot->replot( );
}

void mainWindow::change_marker_size( double new_size )
{
  for ( auto* graph : tests_graph_map )
  {
    auto ss = graph->scatterStyle( );
    ss.setSize( new_size );
    graph->setScatterStyle( ss );
  }
  plot->replot( );
}

void mainWindow::change_marker_line_width( double new_width )
{
  for ( auto* graph : tests_graph_map )
  {
    auto ss  = graph->scatterStyle( );
    auto pen = ss.pen( );
    pen.setWidthF( new_width );
    ss.setPen( pen );
    graph->setScatterStyle( ss );
  }
  plot->replot( );
}

void mainWindow::change_x_spec_label( )
{
  plot->xAxis->setLabel(
    QString( "%1 %2" ).arg( x_quantity_label->text( ) ).arg( x_units_label->text( ) ) );
  plot->replot( );
}

void mainWindow::change_y_spec_label( QString spec )
{
  plot->yAxis->setLabel( QString( "da/dN (" ) + spec + ")" );
  plot->replot( );
}

void mainWindow::save_figure( QString filename, int width_pixels )
{

  if ( filename.right( 3 ).toLower( ) == "pdf" )
  {
    plot->savePdf( filename );
  }
  else
  {
    auto size  = plot->size( );
    auto scale = double( width_pixels ) / size.width( );
    plot->savePng( filename, size.width( ), size.height( ), scale );
  }
}

void mainWindow::update_test( int index )
{
  QCPGraph* newGraph = nullptr;

  if ( tests_graph_map.size( ) > index )
  {
    if ( tests_graph_map[ index ] != nullptr )
    {
      newGraph = tests_graph_map[ index ];
    }
    else
    {
      throw std::out_of_range( "Orphan graph index." );
    }
  }
  else if ( index != tests_graph_map.size( ) )
  {
    throw std::logic_error( "Incorrect test_graph_map size." );
  }
  else
  {
    newGraph = plot->addGraph( );
    tests_graph_map.push_back( newGraph );
  }

  auto& test_series = tests_list->tests( )[ index ];

  QVector< double > x;
  QVector< double > y;

  for ( const auto& data_point : test_series.data.points )
  {
    x.push_back( data_point.DeltaK );
    y.push_back( data_point.dadN );
  }
  newGraph->setName( test_series.name( ) );
  newGraph->setData( x, y, false );
  newGraph->setScatterStyle( test_series.marker.get_QCPScatterStyle( ) );
  newGraph->setLineStyle( QCPGraph::lsNone );

  auto_set( );

  plot->legend->setVisible( true );

  rescale_plot( );

  plot->replot( );

  // We are doing the following because if we copy into the testlistwidget from another app, the
  // main window remains below the other app.
  this->raise( );
  this->show( );
  this->activateWindow( );
}

void mainWindow::handle_selected_test_changed( int index )
{
  if ( index >= tests_graph_map.size( ) )
  {
    throw std::out_of_range( "Wrong graph index" );
  }

  if ( previously_selected_graph_index != -1 )
  {
    auto new_style = tests_graph_map[ previously_selected_graph_index ]->scatterStyle( );

    auto pen = new_style.pen( );
    pen.setWidthF( pen.widthF( ) / 1.5 );

    auto new_size = new_style.size( ) / 1.5;
    new_style.setSize( new_size );

    tests_graph_map[ previously_selected_graph_index ]->setScatterStyle( new_style );
  }

  if ( index != -1 )
  {

    auto new_style = tests_graph_map[ index ]->scatterStyle( );

    auto pen = new_style.pen( );
    pen.setWidthF( pen.widthF( ) * 1.5 );

    auto new_size = new_style.size( ) * 1.5;
    new_style.setSize( new_size );

    tests_graph_map[ index ]->setScatterStyle( new_style );

    previously_selected_graph_index = index;
  }

  auto_set( );

  rescale_plot( );

  plot->replot( );
}

void mainWindow::rescale_plot( )
{
  plot->rescaleAxes( );

  QCPRange rangex;
  QCPRange rangey;
  {
    rangex = plot->xAxis->range( );
    rangex.expand( rangex.lower / 1.2 );
    rangex.expand( rangex.upper * 1.2 );
    plot->xAxis->setRange( rangex );
  }

  {
    auto lo = 1.0e-14;
    if ( !ymin_auto->isChecked( ) )
    {
      lo = ymin_label->text( ).toDouble( );
    }

    auto hi = 1.0e-2;
    if ( !ymax_auto->isChecked( ) )
    {
      hi = ymax_label->text( ).toDouble( );
    }

    rangey = plot->yAxis->range( );
    rangey.expand( rangey.lower / 1.2 );
    rangey.expand( rangey.upper * 1.2 );
    rangey = rangey.bounded( lo, hi );
  }

  plot->xAxis->setRange( rangex );
  plot->yAxis->setRange( rangey );

  if ( xmin_auto->isChecked( ) )
  {
    xmin_label->blockSignals( true );
    xmin_label->setText( QString( "%1" ).arg( plot->xAxis->range( ).lower ) );
    xmin_label->blockSignals( false );
  }
  else
  {
    rangex.lower = xmin_label->text( ).toDouble( );
  }

  if ( xmax_auto->isChecked( ) )
  {
    xmax_label->blockSignals( true );
    xmax_label->setText( QString( "%1" ).arg( plot->xAxis->range( ).upper ) );
    xmax_label->blockSignals( false );
  }
  else
  {
    rangex.upper = xmax_label->text( ).toDouble( );
  }

  if ( ymin_auto->isChecked( ) )
  {
    ymin_label->blockSignals( true );
    ymin_label->setText( QString( "%1" ).arg( plot->yAxis->range( ).lower ) );
    ymin_label->blockSignals( false );
  }
  else
  {
    rangey.lower = ymin_label->text( ).toDouble( );
  }

  if ( ymax_auto->isChecked( ) )
  {
    ymax_label->blockSignals( true );
    ymax_label->setText( QString( "%1" ).arg( plot->yAxis->range( ).upper ) );
    ymax_label->blockSignals( false );
  }
  else
  {
    rangey.upper = ymax_label->text( ).toDouble( );
  }

  plot->xAxis->setRange( rangex );
  plot->yAxis->setRange( rangey );
}

double mainWindow::tests_min_DeltaK_thr( )
{
  double max_val = 100000.0;

  auto& tests = tests_list->tests( );

  for ( const auto& test : tests )
  {
    for ( const auto& data_point : test.data.points )
    {
      max_val = std::min( data_point.DeltaK, max_val );
    }
  }

  return max_val;
}

double mainWindow::inividual_test_min_DeltaK_thr( std::size_t id )
{
  double max_val = 100000.0;

  auto& tests = tests_list->tests( );

  const auto& test = tests[ id ];
  {
    for ( const auto& data_point : test.data.points )
    {
      max_val = std::min( data_point.DeltaK, max_val );
    }
  }

  return max_val;
}

double mainWindow::inividual_test_max_A( int id )
{
  double min_val = 0.00001;

  auto& tests = tests_list->tests( );

  const auto& test = tests[ id ];
  {
    for ( const auto& data_point : test.data.points )
    {
      min_val = std::max( data_point.DeltaK / ( 1.0 - test.data.R ), min_val );
    }
  }

  return min_val;
}

double mainWindow::tests_max_A( )
{
  double min_val = 0.00001;

  auto& tests = tests_list->tests( );

  for ( const auto& test : tests )
  {
    for ( const auto& data_point : test.data.points )
    {
      min_val = std::max( data_point.DeltaK / ( 1.0 - test.data.R ), min_val );
    }
  }

  return min_val;
}
