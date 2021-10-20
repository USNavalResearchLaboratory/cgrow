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

// NOTICE OF THIRD-PARTY SOFTWARE LICENSES. This software uses open source software packages from
// third parties. These are available on an "as is" basis and subject to their individual license
// agreements. Additional information can be found in the provided "licenses" folder.

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "fitting_worker.hpp"

#include <QMainWindow>

#include <QThread>

#include <vector>

#include "qcustomplot.h"

#include <cgrow.hpp>

#include <set>

class decorated_double_spinbox;
class tests_list_widget;
class qcp_results_table;
class fitting_worker;

class QCPGraph;

class QProgressBar;
class QToolBar;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QAction;

class mainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit mainWindow( QWidget* parent = nullptr );

  ~mainWindow( );


  template< class T >
  void setLow( const std::vector< T >& DeltaKs, const std::vector< T >& dadNs )
  {

    QVector< double > xs;
    for ( auto x : DeltaKs )
    {
      xs.push_back( double( x ) );
    }

    QVector< double > ys;
    for ( auto y : dadNs )
    {
      ys.push_back( double( y ) );
    }

    lower_bounds_graph->setData( xs, ys, true );
  }

  template< class T >
  void setHigh( const std::vector< T >& DeltaKs, const std::vector< T >& dadNs )
  {

    QVector< double > xs;
    for ( auto x : DeltaKs )
    {
      xs.push_back( double( x ) );
    }

    QVector< double > ys;
    for ( auto y : dadNs )
    {
      ys.push_back( double( y ) );
    }

    upper_bounds_graph->setData( xs, ys, true );

    plot->xAxis->setRange( 0, 20 );
    plot->yAxis->setRange( 1e-13, 1e-4 );
  }

signals:
  void operate( );

protected:
  void resizeEvent( QResizeEvent* event ) override;

private slots:

  void new_file( );

  void fit( );

  void handleResults( hs_parameters_t params,
                      hs_parameters_t params_lower,
                      hs_parameters_t params_upper );

  void handleProgressReport( int i, int total );

  void update_test( int index );

  void handle_selected_test_changed( int index );

  void rescale_plot( );

  void remove_test( int index );

  void change_font_size( double new_size );

  void change_axes_line_width( double new_size );

  void change_grid_lines_width( double new_size );

  void change_marker_size( double new_size );

  void change_marker_line_width( double new_width );

  void change_x_spec_label( QString spec );

  void change_y_spec_label( QString spec );

  void save_figure( QString filename, int width_pixels );

  void handle_fitting_finished( );

  void DeltaK_thr_min_auto_set( );

  void DeltaK_thr_max_auto_set( );

  void A_min_auto_set( );

  void A_max_auto_set( );

  void auto_set( );

private:
  double tests_min_DeltaK_thr( );

  double tests_max_A( );

  QString current_directory_;

  QToolBar*     toolbar;
  QProgressBar* progressBar;

  QWidget*     control_widget;
  QCustomPlot* plot;

  QAction* new_file_action;
  QAction* open_file_action;
  QAction* save_file_action;
  QAction* compute_action;
  QAction* to_excel_action;
  QAction *to_tabulated_action;

  QLineEdit* D_min;
  QLineEdit* D_max;

  QLineEdit* p_min;
  QLineEdit* p_max;

  QLineEdit* DeltaK_thr_min;
  QCheckBox* DeltaK_thr_min_auto;

  QLineEdit* DeltaK_thr_max;
  QCheckBox* DeltaK_thr_max_auto;

  QLineEdit* A_min;
  QCheckBox* A_min_auto;

  QLineEdit* A_max;
  QCheckBox* A_max_auto;

  QLineEdit* subdivisions;
  QLineEdit* amortization;

  QComboBox* norm_type;

  decorated_double_spinbox* font_size_spinbox;
  decorated_double_spinbox* axes_line_width_spinbox;
  decorated_double_spinbox* grid_line_width_spinbox;
  decorated_double_spinbox* marker_size_spinbox;
  decorated_double_spinbox* marker_line_width_spinbox;

  QLineEdit* xmin_label;
  QCheckBox* xmin_auto;
  QLineEdit* xmax_label;
  QCheckBox* xmax_auto;

  QLineEdit* ymin_label;
  QCheckBox* ymin_auto;
  QLineEdit* ymax_label;
  QCheckBox* ymax_auto;

  QThread        workerThread;
  fittingWorker* worker;

  tests_list_widget*   tests_list;
  QVector< QCPGraph* > tests_graph_map;

  QVector< QCPGraph* > computed_graphs;
  std::set< real_t >   Rs;

  QCPGraph* lower_bounds_graph = nullptr;
  QCPGraph* upper_bounds_graph = nullptr;

  qcp_results_table* results_table_;

  int previously_selected_graph_index = -1;

  hs_parameters_t computed_;
};

#endif // MAINWINDOW_HPP
