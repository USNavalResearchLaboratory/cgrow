#include "fitting_worker.hpp"
#include <QDebug>

#include <cmath>
#include <limits>
#include <stdexcept>

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
  running_.store( true, std::memory_order_release );
  stop_requested_.store( false, std::memory_order_relaxed );

  try
  {
    if ( subdivisions <= 0 )
    {
      throw std::invalid_argument( "Subdivisions must be greater than zero." );
    }
    if ( !std::isfinite( amortization ) || amortization <= 1.0 )
    {
      throw std::invalid_argument( "Amortization must be finite and greater than one." );
    }

    if ( !compute_individually )
    {
      std::vector< cg::test_data_t< real_t > > test_set_fitting;
      {
        for ( const auto& test : test_set )
        {
          cg::test_data_t< real_t > test_data;
          test_data.R = test.R;

          for ( const auto& point : test.points )
          {
            test_data.points.push_back( { real_t { point.DeltaK }, real_t { point.dadN } } );
          }
          test_set_fitting.emplace_back( std::move( test_data ) );
        }
      }

      using param_t = cg::Hartman_Schijve::parameters< real_t >;

      std::function< void( param_t, param_t, param_t ) > update_callback
        = [ this ]( param_t params, param_t params_lower, param_t params_upper ) {
            QMutexLocker lock( &calback_mutex );
            emit updatedResults( params, params_lower, params_upper );
          };

      auto progress_report_callback = [ this ]( std::size_t i, std::size_t total ) {
        QMutexLocker lock( &calback_mutex );
        emit progressReport( i, total );
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
              QMutexLocker lock( &calback_mutex );
              emit individuallyUpdatedResults( params, params_lower, params_upper, id );
            };

      auto progress_report_callback = [ this ]( std::size_t i, std::size_t total ) {
        QMutexLocker lock( &calback_mutex );
        emit progressReport( i, total );
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
        real_t maxA = minA < 0 ? Hartman_Schijve::calc_max_A( Hartman_Schijve::calc_min_A( test ) )
                               : Hartman_Schijve::calc_max_A( minA );
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
  }
  catch ( const std::exception& error )
  {
    stop_requested_.store( true, std::memory_order_relaxed );
    running_.store( false, std::memory_order_release );
    emit failed( QString::fromUtf8( error.what( ) ) );
    emit progressReport( 0, 1 );
    emit finished( );
    return;
  }
  catch ( ... )
  {
    stop_requested_.store( true, std::memory_order_relaxed );
    running_.store( false, std::memory_order_release );
    emit failed( tr( "Unknown fitting error." ) );
    emit progressReport( 0, 1 );
    emit finished( );
    return;
  }

  running_.store( false, std::memory_order_release );
  stop_requested_.store( false, std::memory_order_relaxed );

  emit progressReport( 1, 1 );
  emit finished();
}

void fittingWorker::stop( )
{
  qDebug( ) << "Stopping";
  stop_requested_.store( true, std::memory_order_relaxed );
}

real_t Hartman_Schijve::calc_max_DeltaK_thr( const test_data_t& test )
{
  if ( test.points.empty( ) )
  {
    throw std::invalid_argument( "Cannot calculate DeltaK threshold from an empty test." );
  }

  real_t max_val = std::numeric_limits< real_t >::max( );

  {
    for ( const auto& data_point : test.points )
    {
      if ( !std::isfinite( data_point.DeltaK ) || data_point.DeltaK <= 0 )
      {
        throw std::invalid_argument( "DeltaK values must be finite and positive." );
      }
      max_val = std::min( real_t( data_point.DeltaK ), max_val );
    }
  }

  return max_val;
}

real_t Hartman_Schijve::calc_max_DeltaK_thr( const std::vector< test_data_t >& tests )
{
  if ( tests.empty( ) )
  {
    throw std::invalid_argument( "Cannot calculate DeltaK threshold without tests." );
  }

  real_t max_val = std::numeric_limits< real_t >::max( );
  for ( const auto& test : tests )
  {
    max_val = std::min( max_val, calc_max_DeltaK_thr( test ) );
  }

  return max_val;
}

real_t Hartman_Schijve::calc_min_A( const test_data_t& test )
{
  if ( test.points.empty( ) || !std::isfinite( test.R ) || test.R < -1.0 || test.R >= 1.0 )
  {
    throw std::invalid_argument( "Cannot calculate A from an invalid test." );
  }

  real_t min_val = 0.00001;

  for ( const auto& data_point : test.points )
  {
    if ( !std::isfinite( data_point.DeltaK ) || data_point.DeltaK <= 0 )
    {
      throw std::invalid_argument( "DeltaK values must be finite and positive." );
    }
    min_val = std::max( real_t( data_point.DeltaK ) / ( real_t( 1.0 ) - real_t( test.R ) ), min_val );
  }

  return min_val;
}

real_t Hartman_Schijve::calc_min_A( const std::vector< test_data_t >& tests )
{
  if ( tests.empty( ) )
  {
    throw std::invalid_argument( "Cannot calculate A without tests." );
  }

  real_t min_val = std::numeric_limits< real_t >::lowest( );
  for ( const auto& test : tests )
  {
    min_val = std::max( min_val, calc_min_A( test ) );
  }

  return min_val;
}

real_t Hartman_Schijve::calc_max_A( const real_t& Amin )
{
  if ( !std::isfinite( Amin ) || Amin <= 0 )
  {
    throw std::invalid_argument( "A minimum must be finite and positive." );
  }
  return real_t( 10.0 ) * Amin;
}
