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

#include "nelder_mead.hpp"

#include <atomic>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

namespace crack_growth
{

template< typename T >
struct data_point_t
{
  T DeltaK;
  T dadN;
};

template< class T >
struct test_data_t
{
  using point_type = data_point_t< T >;

  T R = 0.0;

  std::vector< point_type > points;
};

namespace Hartman_Schijve
{

template< class T >
struct parameters
{
  parameters( ) = default;

  parameters( T D, T p, T DeltaK_thr, T A )
  {
    this->D          = D;
    this->p          = p;
    this->DeltaK_thr = DeltaK_thr;
    this->A          = A;
  }

  T D          = 0.0;
  T p          = 0.0;
  T DeltaK_thr = 0.0;
  T A          = 0.0;
};

template< class T >
struct Model_Distance_t
{
  Model_Distance_t( T d, double u ) : distance( d ), utilization( u )
  {
  }

  T      distance;
  double utilization;
};

template< class T >
using callback_t = std::function< void( parameters< T >, parameters< T >, parameters< T > ) >;

using progress_callback_t = std::function< void( std::size_t, std::size_t ) >;

template< class T >
T calc_K_max( const parameters< T >& params, const T& R )
{
  return params.A * ( T { 1.0 } - R );
}

template< class T >
T evaluate( const T& D, const T& p, const T& DeltaK_thr, const T& A, const T& R, const T& DeltaK )
{
  auto s = DeltaK / ( A * ( T { 1.0 } - R ) );

  return D * std::pow( ( DeltaK - DeltaK_thr ) / ( std::sqrt( 1.0 - s ) ), p );
}

template< class T >
T evaluate( const parameters< T >& params, const T& R, const T& DeltaK )
{
  return evaluate( params.D, params.p, params.DeltaK_thr, params.A, R, DeltaK );
}

template< class T, class Container_t >
Container_t evaluate( const parameters< T >& params, const T& R, const Container_t& DeltaKs )
{
  Container_t dadNs;
  for ( const auto& DeltaK : DeltaKs )
  {
    dadNs.push_back( evaluate( params.D, params.p, params.DeltaK_thr, params.A, R, DeltaK ) );
  }

  return dadNs;
}

//! Brief: Pointwise distance of the HS curve defined by params and data points
template< class T, class Container_t >
Model_Distance_t< T > distance( const parameters< T >& hs_params,
                                const T&               R,
                                const Container_t&     DeltaKs,
                                const Container_t&     dadNs )
{
  if ( dadNs.size( ) != DeltaKs.size( ) )
  {
    throw std::runtime_error( " DeltaKs and dadNs sizes should be the same." );
  }

  T sum = 0.0;

  std::size_t rejected_data_points_count = 0;

  for ( std::size_t i = 0; i != DeltaKs.size( ); i++ )
  {
    auto dif
      = std::abs( std::log10( evaluate( hs_params, R, DeltaKs[ i ] ) ) - std::log10( dadNs[ i ] ) );

    if ( !std::isnan( dif ) )
    {
      sum += dif;
    }
    else
    {
      rejected_data_points_count++;
    }
  }

  double utilization = double( DeltaKs.size( ) - rejected_data_points_count ) / DeltaKs.size( );

  return Model_Distance_t { sum / DeltaKs.size( ), utilization };
}

template< class T >
T absolute_square_distance( const T& DeltaK,
                            const T& DeltaKi,
                            const T& dadNi,
                            const T& R,
                            const T& DD,
                            const T& p,
                            const T& DeltaKthr,
                            const T& A )
{

  return ( pow( log( DeltaK ) - log( DeltaKi ), 2 )
           + pow(
             log( dadNi )
               - log(
                 DD * pow( ( DeltaK - DeltaKthr ) / sqrt( 1 + DeltaK / ( A * ( -1 + R ) ) ), p ) ),
             2 ) )
         / pow( log( 10 ), 2 );
}

template< class T >
T halley_step( const T& DeltaK,
               const T& DeltaKi,
               const T& dadNi,
               const T& R,
               const T& DD,
               const T& p,
               const T& DeltaKthr,
               const T& A )
{
  T S1  = DeltaK - 1. * DeltaKthr;
  T S2  = -1. + R;
  T S3  = DeltaK + A * S2;
  T S4  = pow( S1, 2 );
  T S5  = pow( S3, 2 );
  T S6  = pow( DeltaK, 2 );
  T S7  = DeltaK + DeltaKthr + 2. * A * S2;
  T S8  = log( dadNi );
  T S9  = log( DD * pow( S1 / sqrt( 1. + DeltaK / ( A * S2 ) ), p ) );
  T S10 = S8 - 1. * S9;
  T S11 = log( DeltaK ) - 1. * log( DeltaKi );
  T S12 = pow( p, 2 );
  T S13 = pow( DeltaK, 3 );
  T S14 = pow( S7, 2 );
  T S15 = pow( S1, 3 );
  T S16 = pow( S3, 3 );
  T S17 = pow( DeltaK, -2 );
  T S18 = 1 / S1;
  T S19 = pow( S3, -2 );
  T S20 = pow( S1, -2 );
  T S21 = 1 / S3;

  return ( 8. * pow( DeltaK, 4 ) * pow( S1, 4 ) * pow( S3, 4 )
           * ( ( 2. * S11 ) / DeltaK - 1. * p * S10 * S18 * S21 * S7 )
           * ( -2. * ( -1. + S11 ) * S17 + 0.5 * S12 * S14 * S19 * S20
               + p * S10 * ( S18 * S19 + S20 * S21 ) * S7 + p * S18 * S21 * ( -1. * S8 + S9 ) ) )
         / ( ( 2. * S1 * S11 * S3 - 1. * DeltaK * p * S10 * S7 )
               * ( 3. * S1 * S12 * S13 * S14 + 24. * S15 * S16 - 16. * S11 * S15 * S16
                   + 6. * S12 * S13 * S14 * S3 + DeltaK * S1 * S12 * S13 * S7
                   + 3. * DeltaKthr * S1 * S12 * S13 * S7 + 4. * A * S1 * S12 * S13 * S2 * S7
                   - 4. * S1 * S12 * S13 * S3 * S7
                   + 8. * p * S10 * S13
                       * ( -1. * S1 * S5 + ( S4 + S5 ) * S7 + S3 * ( -1. * S4 + S1 * S7 ) ) )
             + 2.
                 * pow( -4. * ( -1. + S11 ) * S4 * S5
                          + S6
                              * ( S12 * S14 + 2. * p * S10 * ( S3 * S7 + S1 * ( -1. * S3 + S7 ) ) ),
                        2 ) );
}

#define HS_MAX_ITERS 30

template< class T >
auto distance( const T& DeltaKi,
               const T& dadNi,
               const T& R,
               const T& DD,
               const T& p,
               const T& DeltaKthr,
               const T& A )
{
  T DeltaKmax = A * ( 1.0 - R );

  const T asymptotic_threshold = 1.0e-5;

  auto restrict_asymptotic = []( const auto& value,
                                 const auto& asymptotic_threshold,
                                 const auto& min_value,
                                 const auto& max_value ) {
    auto lo = ( 1.0 + asymptotic_threshold ) * min_value;
    auto hi = ( 1.0 - asymptotic_threshold ) * max_value;

    return std::min( std::max( value, lo ), hi );
  };

  auto DeltaK = restrict_asymptotic( DeltaKi, asymptotic_threshold, DeltaKthr, DeltaKmax );

  T Amin = DeltaK / ( 1.0 - R );

  if ( A <= Amin )
  {
    return std::make_tuple( std::numeric_limits< T >::max( ), std::size_t( HS_MAX_ITERS ) );
  }

  std::size_t i = 0;
  for ( ; i != HS_MAX_ITERS; i++ )
  {
    auto step = halley_step( DeltaK, DeltaKi, dadNi, R, DD, p, DeltaKthr, A );

    DeltaK = DeltaK - step;

    if ( std::abs( step ) < 1.0e-3 )
    {
      break;
    }

    DeltaK = restrict_asymptotic( DeltaK, asymptotic_threshold, DeltaKthr, DeltaKmax );
  }

  auto d = absolute_square_distance( DeltaK, DeltaKi, dadNi, R, DD, p, DeltaKthr, A );

  return std::make_tuple( d, i );
}

template< class T >
auto distance_nondimensional( const T& DeltaKi,
                              const T& dadNi,
                              const T& R,
                              const T& DD,
                              const T& p,
                              const T& DeltaKthr,
                              const T& A )
{
  T DeltaKmax = A * ( 1.0 - R );

  const T asymptotic_threshold = 1.0e-5;

  auto restrict_asymptotic = []( const auto& value,
                                 const auto& asymptotic_threshold,
                                 const auto& min_value,
                                 const auto& max_value ) {
    auto lo = ( 1.0 + asymptotic_threshold ) * min_value;
    auto hi = ( 1.0 - asymptotic_threshold ) * max_value;

    return std::min( std::max( value, lo ), hi );
  };

  auto DeltaK = restrict_asymptotic( DeltaKi, asymptotic_threshold, DeltaKthr, DeltaKmax );

  T Amin = DeltaK / ( 1.0 - R );

  if ( A <= Amin )
  {
    return std::make_tuple( std::numeric_limits< T >::max( ), std::size_t( HS_MAX_ITERS ) );
  }

  // Non-dimensionalization
  T DeltaK_d    = DeltaK / DeltaKthr;
  T DeltaKthr_d = 1.0;
  T DeltaKi_d   = DeltaKi / DeltaKthr;
  T A_d         = A / DeltaKthr;
  T dadNi_d     = dadNi / ( DD * std::pow( DeltaKthr, p ) );
  T DD_d        = 1.0;
  T DeltaKmax_d = A_d * ( 1.0 - R );

  std::size_t i = 0;
  for ( ; i != HS_MAX_ITERS; i++ )
  {
    auto step = halley_step( DeltaK_d, DeltaKi_d, dadNi_d, R, DD_d, p, DeltaKthr_d, A_d );

    DeltaK_d = DeltaK_d - step;

    if ( std::abs( step ) < 1.0e-3 )
    {
      break;
    }

    DeltaK_d = restrict_asymptotic( DeltaK_d, asymptotic_threshold, DeltaKthr_d, DeltaKmax_d );
  }

  auto d = absolute_square_distance( DeltaK_d, DeltaKi_d, dadNi_d, R, DD_d, p, DeltaKthr_d, A_d );

  return std::make_tuple( d, i );
}

template< class T, class Container_t >
Model_Distance_t< T > objective_function( const parameters< T >& hs_params,
                                          bool                   use_geometric,
                                          const Container_t&     test_set )
{
  T sum = 0.0;

  std::size_t num_rejected_data_points = 0;

  auto num_data_points = 0;
  for ( const auto& test : test_set )
  {
    for ( const auto& point : test.points )
    {
      num_data_points++;

      if ( use_geometric )
      {
        auto [ dis, iters ] = distance_nondimensional( point.DeltaK,
                                                       point.dadN,
                                                       test.R,
                                                       hs_params.D,
                                                       hs_params.p,
                                                       hs_params.DeltaK_thr,
                                                       hs_params.A );

        if ( !std::isnan( dis ) && iters != HS_MAX_ITERS )
        {
          sum += dis;
        }
        else
        {
          num_rejected_data_points++;
        }
      }
      else
      {
        auto dis = std::abs( std::log10( evaluate( hs_params, test.R, point.DeltaK ) )
                             - std::log10( point.dadN ) );
        if ( !std::isnan( dis ) )
        {
          sum += dis;
        }
        else
        {
          num_rejected_data_points++;
        }
      }
    }
  }

  auto num_utlized_points = num_data_points - num_rejected_data_points;

  double utilization = double( num_utlized_points ) / num_data_points;

  return Model_Distance_t { sum / num_utlized_points, utilization };
}

template< class T >
T sample_parameter( const T&           min,
                    const T&           max,
                    const std::size_t& subdivisions,
                    const std::size_t& m )
{
  auto dif = max - min;

  T step = 0;

  if ( subdivisions > 1 )
  {
    step = dif / ( subdivisions - 1 );
  }

  return min + step * m;
};

template< class T, class Container_t >
parameters< T > fit(
  parameters< T >     search_space_min,
  parameters< T >     search_space_max,
  Container_t         test_set,
  const std::size_t   subdivisions  = 7,
  const double&       amortization  = 1.02,
  std::size_t         iterations    = 0,
  bool                use_geometric = false,
  callback_t< T >     callback      = []( parameters< T >, parameters< T >, parameters< T > ) {},
  progress_callback_t progress_callback = []( std::size_t, std::size_t ) {},
  const bool&         stop_requested    = false )
{
  using params_t = parameters< T >;

  auto contract_range
    = []( const T& lower, const T& upper, const T& new_center, const T& amortization ) {
        auto dif    = upper - lower;
        auto newdif = dif / amortization;

        auto new_lower = new_center - newdif / 2;
        auto new_upper = new_center + newdif / 2;

        if ( new_upper > upper )
        {
          new_upper = upper;
          new_lower = new_upper - newdif;
        }
        else if ( new_lower < lower )
        {
          new_lower = lower;
          new_upper = new_lower + newdif;
        }

        return std::make_tuple( new_lower, new_upper );
      };

  using st = std::size_t;

  auto objective_min = std::numeric_limits< T >::max( );
  auto params_at_min = params_t { 0.0, 0.0, 0.0, 0.0 };

  if ( iterations == 0 )
  {
    iterations = std::log( 4000.0 ) / std::log( amortization );
  }

  st num_threads = std::max( ( unsigned int )( 4 ), std::thread::hardware_concurrency( ) );
  num_threads    = std::min( num_threads, subdivisions );

  std::vector< double >   max_utilization_mins( num_threads, 0.0 );
  std::vector< T >        objective_mins( num_threads );
  std::vector< params_t > params_mins( num_threads );

  for ( st i = 0; i != num_threads; i++ )
  {
    objective_mins[ i ] = std::numeric_limits< T >::max( );
    params_mins[ i ]    = params_t { 0.0, 0.0, 0.0, 0.0 };
  }

  auto subd = subdivisions;

  for ( st t = 0; t != iterations && !stop_requested; t++ )
  {
    std::vector< std::thread > threads;

    auto D_index_span = std::floor( ( subdivisions ) / num_threads );

    for ( st tid = 0; tid != num_threads; tid++ )
    {
      threads.push_back( std::thread( [ callback,
                                        D_index_span,
                                        subd,
                                        tid,
                                        num_threads,
                                        stop_requested,
                                        search_space_min,
                                        search_space_max,
                                        use_geometric,
                                        &max_utilization_mins,
                                        &objective_mins,
                                        &params_mins,
                                        subdivisions,
                                        &test_set ]( ) {
        auto start  = D_index_span * tid;
        auto finish = ( tid == num_threads - 1 ) ? subd : start + D_index_span;

        for ( auto dj = start; dj != finish && !stop_requested; dj++ )
        {
          params_t obj_params;

          auto low = search_space_min.D;
          auto hi  = search_space_max.D;

          obj_params.D = sample_parameter( low, hi, subd, dj );

          // auto steps = search_space_min.p == search_space_max.p ? 1 : subd;

          for ( st pj = 0; pj != subd && !stop_requested; pj++ )
          {
            auto low = search_space_min.p;
            auto hi  = search_space_max.p;

            obj_params.p = sample_parameter( low, hi, subd, pj );

            //                        auto steps = search_space_min.DeltaK_thr ==
            //                        search_space_max.DeltaK_thr ?
            //                                    1 :
            //                                    subd;

            for ( st DeltaKj = 0; DeltaKj != subd && !stop_requested; DeltaKj++ )
            {
              auto low = search_space_min.DeltaK_thr;
              auto hi  = search_space_max.DeltaK_thr;

              obj_params.DeltaK_thr = sample_parameter( low, hi, subd, DeltaKj );

              // auto steps = search_space_min.A == search_space_max.A ? 1 : subd;
              // TODO: there is an issue if we try to not have any steps
              // if min and max are the same.

              for ( st Aj = 0; Aj != subd && !stop_requested; Aj++ )
              {
                auto low = search_space_min.A;
                auto hi  = search_space_max.A;

                obj_params.A = sample_parameter( low, hi, subd, Aj );

                auto d = objective_function( obj_params, use_geometric, test_set );

                if ( d.utilization > max_utilization_mins[ tid ]
                     || // prefer utilization over minimization
                     ( objective_mins[ tid ] > d.distance
                       && d.utilization >= max_utilization_mins[ tid ] ) )
                {
                  objective_mins[ tid ]       = d.distance;
                  params_mins[ tid ]          = obj_params;
                  max_utilization_mins[ tid ] = d.utilization;
                }
              }
            }
          }
        }
      } ) );
    }

    for ( st tid = 0; tid != num_threads; tid++ )
    {
      threads[ tid ].join( );
    }

    double max_utlization_at_min = 0;

    for ( st tid = 0; tid != num_threads; tid++ )
    {
      if ( max_utilization_mins[ tid ] > max_utlization_at_min
           || ( objective_min > objective_mins[ tid ]
                && max_utilization_mins[ tid ] >= max_utlization_at_min ) )
      {

        objective_min         = objective_mins[ tid ];
        params_at_min         = params_mins[ tid ];
        max_utlization_at_min = max_utilization_mins[ tid ];
      }
    }

    for ( st tid = 0; tid != num_threads; tid++ )
    {
      objective_mins[ tid ]       = objective_min;
      params_mins[ tid ]          = params_at_min;
      max_utilization_mins[ tid ] = max_utlization_at_min;
    }

    callback( params_at_min, search_space_min, search_space_max );

    const T a = amortization;

    std::tie( search_space_min.D, search_space_max.D )
      = contract_range( search_space_min.D, search_space_max.D, params_at_min.D, a );

    std::tie( search_space_min.p, search_space_max.p )
      = contract_range( search_space_min.p, search_space_max.p, params_at_min.p, a );

    std::tie( search_space_min.DeltaK_thr, search_space_max.DeltaK_thr ) = contract_range(
      search_space_min.DeltaK_thr, search_space_max.DeltaK_thr, params_at_min.DeltaK_thr, a );

    std::tie( search_space_min.A, search_space_max.A )
      = contract_range( search_space_min.A, search_space_max.A, params_at_min.A, a );

    progress_callback( t, iterations );
  }

  // TODO: Is this correct? For some reason it was not here
  return params_at_min;
}

template< class T, class Container_t >
parameters< T > fit(
  const Container_t&  test_set,
  callback_t< T >     callback = []( parameters< T >, parameters< T >, parameters< T > ) {},
  progress_callback_t progress_callback = []( std::size_t, std::size_t ) {},
  const bool&         stop_requested    = false )

{
  T Amin      = std::numeric_limits< T >::lowest( );
  T DeltaKmin = std::numeric_limits< T >::max( );

  for ( const auto& test : test_set )
  {
    for ( const auto point : test.points )
    {
      Amin      = std::max( Amin, point.DeltaK / ( T { 1.0 } - test.R ) );
      DeltaKmin = std::min( DeltaKmin, point.DeltaK );
    }
  }

  auto params_low  = parameters< T > { 1.0e-10, 1.5, 0.0001, 0.9 * Amin };
  auto params_high = parameters< T > { 5.0e-10, 2.5, 1.05 * DeltaKmin, 450.0 };

  // return fit( params_low, params_high, R, DeltaKs, dadNs, 12, 1.01, 0, callback ); // GOOD

  //  return fit( params_low, params_high, R, DeltaKs, dadNs, 9, 1.05, 0, callback ); // GOOD

  //      return fit( params_low, params_high, R, DeltaKs, dadNs, 11, 1.025, 0, callback ); // very
  //      Good

  // good for fast:                    return fit( params_low, params_high, R, DeltaKs, dadNs,
  // 12, 1.10, 0, callback, progress_callback, stop_requested ); good for fast return fit(
  // params_low, params_high, R, DeltaKs, dadNs, 14, 1.2, 0, callback, progress_callback,
  // stop_requested ); very good: return fit( params_low, params_high, R, DeltaKs, dadNs, 24, 2, 0,
  // callback, progress_callback, stop_requested ); super fast return fit( params_low, params_high,
  // R, DeltaKs, dadNs, 12, 2, 0, callback, progress_callback, stop_requested );

  bool use_geometric = false;

  return fit( params_low,
              params_high,
              test_set,
              10,
              1.01,
              0,
              use_geometric,
              callback,
              progress_callback,
              stop_requested );
}

} // namespace Hartman_Schijve

} // namespace crack_growth
