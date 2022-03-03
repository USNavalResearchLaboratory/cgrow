#include "fitting_worker.hpp"
#include <QDebug>

namespace cg = crack_growth;

void fittingWorker::run( hs_parameters_t                   params_low,
                         hs_parameters_t                   params_high,
                         int                               subdivisions,
                         double                            amortization,
                         bool                              use_geometric,
                         const std::vector< test_data_t >& test_set,
                         Hartman_Schijve_autoRange         autoRange,
                         bool                              compute_individually )
{
  running_        = true;
  stop_requested_ = false;

  if ( !compute_individually )
  {
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
  }
  else
  {

    int id = 0;
    for ( const auto& test : test_set )
    {
      std::vector< cg::test_data_t< real_t > > test_set_fitting;

      cg::test_data_t< real_t > test_data;
      test_data.R = test.R;

      for ( const auto point : test.points )
      {
        test_data.points.push_back( { real_t { point.DeltaK }, real_t { point.dadN } } );
      }
      test_set_fitting.emplace_back( std::move( test_data ) );

      using param_t = cg::Hartman_Schijve::parameters< real_t >;

      std::function< void( param_t, param_t, param_t ) > update_callback
        = [ this, &id ]( param_t params, param_t params_lower, param_t params_upper ) {
            calback_mutex.lock( );
            emit individuallyUpdatedResults( params, params_lower, params_upper, id );
            calback_mutex.unlock( );
          };

      auto progress_report_callback = [ this ]( std::size_t i, std::size_t total ) {
        calback_mutex.lock( );
        emit progressReport( i, total );
        calback_mutex.unlock( );
      };

      if ( autoRange.DeltaK_thr_low )
      {
        params_low.DeltaK_thr = Hartman_Schijve::min_DeltaK_thr;
        std::cout << "DeltaKthr_low = " << params_low.DeltaK_thr << std::endl;
      }

      if ( autoRange.DeltaK_thr_high )
      {
        params_high.DeltaK_thr = 1.2*Hartman_Schijve::calc_max_DeltaK_thr( test );
        std::cout << "DeltaKthr_high = " << params_high.DeltaK_thr << std::endl;
      }

      real_t minA = -1;
      if ( autoRange.A_low )
      {
        minA         = 0.8 * Hartman_Schijve::calc_min_A( test );
        params_low.A = minA;
        std::cout << "minA = " << params_low.A << std::endl;
      }

      if ( autoRange.A_high )
      {
        real_t maxA   = minA < 0 ? 10.0 * Hartman_Schijve::calc_min_A( test ) : 10.0 * minA;
        params_high.A = maxA;
        std::cout << "maxA = " << params_high.A << std::endl;
      }

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
      id++;
    }
  }

  running_        = false;
  stop_requested_ = false;

  emit progressReport( 99, 100 );
  emit finished();
}

void fittingWorker::stop( )
{
  qDebug( ) << "Stopping";
  stop_requested_ = true;
}

real_t Hartman_Schijve::calc_max_DeltaK_thr( const test_data_t& test )
{
  real_t max_val = 100000.0;

  {
    for ( const auto& data_point : test.points )
    {
      max_val = std::min( real_t( data_point.DeltaK ), max_val );
    }
  }

  return max_val;
}

real_t Hartman_Schijve::calc_max_DeltaK_thr( const std::vector< test_data_t >& tests )
{
  real_t max_val = 100000.0;
  for ( const auto& test : tests )
  {
    max_val = std::min( max_val, calc_max_DeltaK_thr( test ) );
  }

  return max_val;
}

real_t Hartman_Schijve::calc_min_A( const test_data_t& test )
{
  double min_val = 0.00001;

  for ( const auto& data_point : test.points )
  {
    min_val = std::max( data_point.DeltaK / ( 1.0 - test.R ), min_val );
  }

  return min_val;
}

real_t Hartman_Schijve::calc_min_A( const std::vector< test_data_t >& tests )
{
  real_t min_val = 100000.0;
  for ( const auto& test : tests )
  {
    min_val = std::max( min_val, calc_min_A( test ) );
  }

  return min_val;
}
