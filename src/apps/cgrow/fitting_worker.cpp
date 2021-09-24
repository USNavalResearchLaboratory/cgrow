#include "fitting_worker.hpp"
#include <QDebug>

namespace cg = crack_growth;

void fittingWorker::run( hs_parameters_t                   params_low,
                         hs_parameters_t                   params_high,
                         int                               subdivisions,
                         double                            amortization,
                         bool                              use_geometric,
                         const std::vector< test_data_t >& test_set )
{
  running_        = true;
  stop_requested_ = false;

  std::vector< cg::test_data_t< real_t > > test_set_fitting;
  {
    for ( const auto& test : test_set )
    {
      cg::test_data_t< real_t > test_data;
      test_data.R = test.R;

      for ( const auto point : test.points )
      {
        test_data.points.push_back( { real_t { point.DeltaK }, real_t { point.dadN } } );
      }
      test_set_fitting.emplace_back( std::move( test_data ) );
    }
  }

  using param_t = cg::Hartman_Schijve::parameters< real_t >;

  std::function< void( param_t, param_t, param_t ) > update_callback
    = [ this ]( param_t params, param_t params_lower, param_t params_upper ) {
        calback_mutex.lock( );
        emit updatedResults( params, params_lower, params_upper );
        calback_mutex.unlock( );
      };

  auto progress_report_callback = [ this ]( std::size_t i, std::size_t total ) {
    calback_mutex.lock( );
    emit progressReport( i, total );
    calback_mutex.unlock( );
  };

  cg::Hartman_Schijve::fit( params_low,
                            params_high,
                            test_set_fitting,
                            subdivisions,
                            amortization,
                            0,
                            use_geometric,
                            update_callback,
                            progress_report_callback,
                            stop_requested_ );

  running_        = false;
  stop_requested_ = false;

  emit progressReport( 99, 100 );
}

void fittingWorker::stop( )
{
  qDebug( ) << "Stopping";
  stop_requested_ = true;
}
