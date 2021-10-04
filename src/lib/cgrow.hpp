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

#include <mutex>

inline int        totalevals = 0;
inline std::mutex stdoutmutex;

#define HS_MAX_ITERS 60

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

template< typename T, class Container_t >
T computeAxesScale( const Container_t& test_set )
{
  T DKmin = std::numeric_limits< T >::max( );
  T DKmax = 1e-19;

  T dadNmin = std::numeric_limits< T >::max( );
  T dadNmax = 1e-19;

  for ( const auto& test : test_set )
  {
    for ( const auto& point : test.points )
    {
      DKmin   = std::min( DKmin, point.DeltaK );
      DKmax   = std::max( DKmax, point.DeltaK );
      dadNmin = std::min( dadNmin, point.dadN );
      dadNmax = std::max( dadNmax, point.dadN );
    }
  }

  return ( std::log10( dadNmax ) - std::log10( std::max( dadNmin, T( 1e-19 ) ) ) )
         / ( std::log10( DKmax ) - std::log10( std::max( DKmin, T( 1e-19 ) ) ) );
}

namespace CUHYSO
{
// template< class Parameters, class F >
// Parameters minimize(F &&func, Parameters plow, Parameters phigh )
//{

//}
// parameters< T > fit(
//  const Container_t&  test_set,
//  bool                use_geometric = false,
//  callback_t< T >     callback      = []( parameters< T >, parameters< T >, parameters< T > ) {},
//  progress_callback_t progress_callback                        = []( std::size_t, std::size_t )
//  {}, const bool&         stop_requested                           = false, std::function< void(
//  parameters< T > ) > per_thread_callback = []( parameters< T > ) {} )
//}
}

namespace Hartman_Schijve
{

template< class T >
struct parameters
{
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

// TODO: Instead of using the distance on the two axes, use the product of the distances on the two
// axes. I think this needs finding the horizontal intersection (i.e. the line with constant dadNi),
// which is the minimization we need to perform because the other distance can be directly evaluated
// from the function
template< class T >
T DistanceScaled( const T& DeltaK,
                  const T& DeltaKi,
                  const T& dadNi,
                  const T& R,
                  const T& DD,
                  const T& p,
                  const T& DeltaKthr,
                  const T& A,
                  const T& sc )
{

  return 0.18861169701161387
         * ( pow( log( DeltaK ) - 1. * log( DeltaKi ), 2 )
             + pow( sc, 2 )
                 * pow( log( dadNi )
                          - 1.
                              * log( DD
                                     * pow( ( DeltaK - 1. * DeltaKthr )
                                              / sqrt( 1. + DeltaK / ( A * ( -1. + R ) ) ),
                                            p ) ),
                        2 ) );
}

template< class T >
T DistanceDeriv( const T& DeltaK,
                 const T& DeltaKi,
                 const T& dadNi,
                 const T& R,
                 const T& DD,
                 const T& p,
                 const T& DeltaKthr,
                 const T& A,
                 const T& sc )
{
  T S1 = -1. + R;
  T S2 = DeltaK - DeltaKthr;

  return 0.18861169701161387
         * ( ( 2. * ( log( DeltaK ) - 1. * log( DeltaKi ) ) ) / DeltaK
             - ( 1. * p * ( DeltaK + DeltaKthr + 2. * A * S1 ) * pow( sc, 2 )
                 * ( log( dadNi )
                     - 1. * log( DD * pow( S2 / sqrt( 1. + DeltaK / ( A * S1 ) ), p ) ) ) )
                 / ( ( DeltaK + A * S1 ) * S2 ) );
}

// todo: this needs more work and although CUHYSO works, others don't in test4.
template< typename T >
auto minimum_distance2( const T& DeltaKi,
                        const T& dadNi,
                        const T& R,
                        const T& DD,
                        const T& p,
                        const T& DeltaKthr,
                        const T& A,
                        const T& scale )
{

  T S1 = T( 1.0 ) / A;
  T S2 = T( 1.0 ) / ( -1. + R );
  T S3 = pow( dadNi / DD, 2. / p );

  T v
    = 0.03557437224960084 * pow( scale, 2 )
      * pow(
        log( dadNi )
          - 1.
              * log( DD * pow( ( DeltaKi - 1. * DeltaKthr ) / sqrt( 1. + DeltaKi * S1 * S2 ), p ) ),
        2 )
      * pow( log( DeltaKi )
               - 1.
                   * log( 0.5
                          * ( 2. * DeltaKthr + S1 * S2 * S3
                              + sqrt( 4. * ( -1. * pow( DeltaKthr, 2 ) + S3 )
                                      + pow( -2. * DeltaKthr + S3 / ( A - 1. * A * R ), 2 ) ) ) ),
             2 );

  return std::make_tuple( v, std::size_t( 0 ) );
}

template< typename T >
std::tuple< T, std::size_t > minimum_distance( const T& DeltaKi,
                                               const T& dadNi,
                                               const T& R,
                                               const T& DD,
                                               const T& p,
                                               const T& DeltaKthr,
                                               const T& A,
                                               const T& scale )
{
  if ( dadNi < 1e-17 )
  {
    return std::make_tuple( DeltaKthr, std::size_t( 0 ) );
  }

  constexpr T beta   = 1.0e-4;
  const T     DKlow  = DeltaKthr * ( 1.0 + beta );
  const T     DKhigh = A * ( 1.0 - R ) * ( 1.0 - beta );

  T dlow  = DistanceDeriv( DKlow, DeltaKi, dadNi, R, DD, p, DeltaKthr, A, scale );
  T dhigh = DistanceDeriv( DKhigh, DeltaKi, dadNi, R, DD, p, DeltaKthr, A, scale );

  if ( dlow > 0 && dhigh > 0 )
  {
    return std::make_tuple( DistanceScaled( DKlow, DeltaKi, dadNi, R, DD, p, DeltaKthr, A, scale ),
                            std::size_t( 0 ) );
  }
  else if ( dlow < 0 && dhigh < 0 )
  {
    return std::make_tuple( DistanceScaled( DKhigh, DeltaKi, dadNi, R, DD, p, DeltaKthr, A, scale ),
                            std::size_t( 0 ) );
  }

  T xpos = 0;
  T xneg = 0;

  if ( dlow > 0 )
  {
    xpos = DKlow;
    xneg = DKhigh;
  }
  else
  {
    xpos = DKhigh;
    xneg = DKlow;
  }

  std::size_t niters = 0;

  const T tol  = ( std::log10( DKhigh ) - std::log10( DKlow ) ) * 1.0e-4;
  auto    span = std::abs( std::log10( xpos ) - std::log10( xneg ) );

  while ( niters++ < HS_MAX_ITERS && span > tol )
  {
    T xhalf    = ( xpos + xneg ) / 2.0;
    T dhalfway = DistanceDeriv( xhalf, DeltaKi, dadNi, R, DD, p, DeltaKthr, A, scale );

    if ( dhalfway > 0 )
    {
      xpos = xhalf;
    }
    else
    {
      xneg = xhalf;
    }

    auto span = std::abs( std::log10( xpos ) - std::log10( xneg ) );
  }

  return std::make_tuple(
    DistanceScaled( ( xpos + xneg ) / 2.0, DeltaKi, dadNi, R, DD, p, DeltaKthr, A, scale ),
    std::size_t( 1 ) );
}

template< class T, class Container_t >
Model_Distance_t< T > objective_function( const parameters< T >& hs_params,
                                          bool                   use_geometric,
                                          const Container_t&     test_set,
                                          const T                scale )
{
  T sum = 0.0;

  std::size_t num_rejected_data_points = 0;

  auto num_data_points = 0;

  if ( use_geometric )
  {
    for ( const auto& test : test_set )
    {
      for ( const auto& point : test.points )
      {
        num_data_points++;

        auto [ dis, iters ] = minimum_distance( point.DeltaK,
                                                point.dadN,
                                                test.R,
                                                hs_params.D,
                                                hs_params.p,
                                                hs_params.DeltaK_thr,
                                                hs_params.A,
                                                scale );

        if ( !std::isnan( dis ) && iters < HS_MAX_ITERS )
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
  else
  {
    auto num_data_points = 0;
    for ( const auto& test : test_set )
    {
      for ( const auto& point : test.points )
      {
        num_data_points++;

        auto dis = std::abs( std::log10( evaluate( hs_params, test.R, point.DeltaK ) )
                             - std::log10( point.dadN ) );
        if ( std::isfinite( dis ) )
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

  //  if ( num_rejected_data_points > 0 )
  //  {
  //    std::cout << "Rejected : " << num_rejected_data_points << std::endl;
  //  }

  double utilization = double( num_utlized_points ) / num_data_points;

  if ( num_utlized_points == 0 )
  {
    return Model_Distance_t( T(1000000.0), 0.0 );
  }

  return Model_Distance_t { sum / num_utlized_points, utilization };
}

template< class T >
T sample_parameter( const T&           min,
                    const T&           max,
                    const std::size_t& subdivisions,
                    const std::size_t& m )
{

  if ( m >= subdivisions )
  {
    throw std::runtime_error(
      "Error in parameter sampling: Subdivision number should be less than the total number of "
      "subdivisions." );
  }

  if ( subdivisions > 1 )
  {
    auto dif = max - min;

    T step = 0;

    step = dif / ( subdivisions - 1 );
    return min + step * m;
  }

  return ( max + min ) / 2.0;
};

struct common_among_tests
{
  bool D          = true;
  bool p          = true;
  bool DeltaK_thr = true;
  bool A          = true;

  bool all_common( ) const
  {
    return D && p && DeltaK_thr && A;
  }
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
  progress_callback_t progress_callback                        = []( std::size_t, std::size_t ) {},
  const bool&         stop_requested                           = false,
  std::function< void( parameters< T > ) > per_thread_callback = []( parameters< T > ) {} )
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

  auto subd = subdivisions;

  auto subdD = subd;
  if ( std::fabs( search_space_max.D - search_space_min.D ) < 1e-19 )
  {
    subdD = 1;
  }

  st num_threads = std::max( ( unsigned int )( 4 ), std::thread::hardware_concurrency( ) );
  num_threads    = std::min( num_threads, subdD );

  std::cout << "Num threads: " << num_threads << std::endl;

  std::vector< double >   max_utilization_mins( num_threads, 0.0 );
  std::vector< T >        objective_mins( num_threads );
  std::vector< params_t > params_mins( num_threads );

  for ( st i = 0; i != num_threads; i++ )
  {
    objective_mins[ i ] = std::numeric_limits< T >::max( );
    params_mins[ i ]    = params_t { 0.0, 0.0, 0.0, 0.0 };
  }

  auto scale = crack_growth::computeAxesScale< T >( test_set );

  if ( scale < 1.0e-10 || scale > 1.0e10 )
  {
    throw std::runtime_error( "Test data relative scales vary orders of magnitude." );
  }

  for ( st t = 0; t != iterations && !stop_requested; t++ )
  {
    std::vector< std::thread > threads;

    auto D_index_span = std::floor( ( subdD ) / num_threads );

    for ( st tid = 0; tid != num_threads; tid++ )
    {
      threads.push_back( std::thread( [ per_thread_callback,
                                        D_index_span,
                                        subd,
                                        subdD,
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
                                        &test_set,
                                        scale ]( ) {
        auto start  = D_index_span * tid;
        auto finish = ( tid == num_threads - 1 ) ? subdD : start + D_index_span;

        for ( auto dj = start; dj != finish && !stop_requested; dj++ )
        {
          params_t obj_params;

          // Span of D in the log10 space.
          auto low = search_space_min.D;
          auto hi  = search_space_max.D;

          auto lowl = std::log10( low );
          auto hil  = std::log10( hi );

          auto dl      = sample_parameter( lowl, hil, subdD, dj );
          obj_params.D = std::pow( 10.0, dl );

          auto subdp = subd;
          if ( std::fabs( search_space_max.p - search_space_min.p ) < 1e-19 )
          {
            subdp = 1;
          }

          for ( st pj = 0; pj != subdp && !stop_requested; pj++ )
          {
            auto low = search_space_min.p;
            auto hi  = search_space_max.p;

            obj_params.p = sample_parameter( low, hi, subd, pj );

            auto subdDeltaKj = subd;
            if ( std::fabs( search_space_max.DeltaK_thr - search_space_min.DeltaK_thr ) < 1e-19 )
            {
              subdDeltaKj = 1;
            }

            for ( st DeltaKj = 0; DeltaKj != subdDeltaKj && !stop_requested; DeltaKj++ )
            {
              auto low = search_space_min.DeltaK_thr;
              auto hi  = search_space_max.DeltaK_thr;

              obj_params.DeltaK_thr = sample_parameter( low, hi, subd, DeltaKj );

              auto subdA = subd;
              if ( std::fabs( search_space_max.A - search_space_min.A ) < 1e-19 )
              {
                subdA = 1;
              }

              for ( st Aj = 0; Aj != subdA && !stop_requested; Aj++ )
              {
                auto low = search_space_min.A;
                auto hi  = search_space_max.A;

                obj_params.A = sample_parameter( low, hi, subd, Aj );

                per_thread_callback( obj_params );

                // auto d = objective_function( obj_params, use_geometric, test_set );

                auto d = objective_function( obj_params, use_geometric, test_set, scale );
                totalevals++;
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

    T Dlmin                  = 0;
    T Dlmax                  = 0;
    std::tie( Dlmin, Dlmax ) = contract_range( std::log10( search_space_min.D ),
                                               std::log10( search_space_max.D ),
                                               std::log10( params_at_min.D ),
                                               a );

    search_space_min.D = std::pow( 10.0, Dlmin );
    search_space_max.D = std::pow( 10.0, Dlmax );

    std::tie( search_space_min.p, search_space_max.p )
      = contract_range( search_space_min.p, search_space_max.p, params_at_min.p, a );

    std::tie( search_space_min.DeltaK_thr, search_space_max.DeltaK_thr ) = contract_range(
      search_space_min.DeltaK_thr, search_space_max.DeltaK_thr, params_at_min.DeltaK_thr, a );

    std::tie( search_space_min.A, search_space_max.A )
      = contract_range( search_space_min.A, search_space_max.A, params_at_min.A, a );

    progress_callback( t, iterations );

    std::cout << search_space_min.D << " " << search_space_max.D << " " << params_at_min.D << " "
              << objective_min << std::endl;
  }

  // TODO: Is this correct? For some reason it was not here
  return params_at_min;
}

// template< class T, class Container_t >
// parameters< T > fit3(
//  const common_among_tests& common,
//  parameters< T >           search_space_min,
//  parameters< T >           search_space_max,
//  Container_t               test_set,
//  const std::size_t         subdivisions  = 7,
//  const double&             amortization  = 1.02,
//  std::size_t               iterations    = 0,
//  bool                      use_geometric = false,
//  callback_t< T >           callback = []( parameters< T >, parameters< T >, parameters< T > ) {},
//  progress_callback_t       progress_callback                  = []( std::size_t, std::size_t )
//  {}, const bool&               stop_requested                     = false, std::function< void(
//  parameters< T > ) > per_thread_callback = []( parameters< T > ) {} )
//{
//  using params_t = parameters< T >;

//  auto contract_range
//    = []( const T& lower, const T& upper, const T& new_center, const T& amortization ) {
//        auto dif    = upper - lower;
//        auto newdif = dif / amortization;

//        auto new_lower = new_center - newdif / 2;
//        auto new_upper = new_center + newdif / 2;

//        if ( new_upper > upper )
//        {
//          new_upper = upper;
//          new_lower = new_upper - newdif;
//        }
//        else if ( new_lower < lower )
//        {
//          new_lower = lower;
//          new_upper = new_lower + newdif;
//        }

//        return std::make_tuple( new_lower, new_upper );
//      };

//  using st = std::size_t;

//  auto objective_min = std::numeric_limits< T >::max( );
//  if ( iterations == 0 )
//  {
//    iterations = std::log( 4000.0 ) / std::log( amortization );
//  }

//  auto num_tests = test_set.size( );

//  auto subd = subdivisions;

//  std::vector< params_t > params_at_min;
//  params_at_min.push_back( params_t { 0.0, 0.0, 0.0, 0.0 } );

//  if ( !common.all_common( ) )
//  {
//    for ( std::size_t i = 0; i != test_set.size( ) - 1; i++ )
//    {
//      params_at_min.push_back( params_t { 0.0, 0.0, 0.0, 0.0 } );
//    }
//  }

//  auto subdD = subd;
//  if ( std::fabs( search_space_max.D - search_space_min.D ) < 1e-19 )
//  {
//    subdD = 1;
//  }

//  std::size_t num_threads = num_tests;

//  std::cout << "Num threads: " << num_threads << std::endl;

//  std::vector< T >        objective_mins( num_tests );
//  std::vector< params_t > params_mins( num_tests );

//  for ( st i = 0; i != num_tests; i++ )
//  {
//    objective_mins[ i ] = std::numeric_limits< T >::max( );
//    params_mins[ i ]    = params_t { 0.0, 0.0, 0.0, 0.0 };
//  }

//  auto scale = crack_growth::computeAxesScale< T >( test_set );

//  if ( scale < 1.0e-10 || scale > 1.0e10 )
//  {
//    throw std::runtime_error( "Test data relative scales vary orders of magnitude." );
//  }

//  for ( st t = 0; t != iterations && !stop_requested; t++ )
//  {
//    std::vector< std::thread > threads;

//    auto D_index_span = std::floor( ( subdD ) / num_threads );

//    for ( st tid = 0; tid != num_threads; tid++ )
//    {
//      threads.push_back( std::thread( [ per_thread_callback,
//                                        D_index_span,
//                                        subd,
//                                        subdD,
//                                        tid,
//                                        num_threads,
//                                        stop_requested,
//                                        search_space_min,
//                                        search_space_max,
//                                        use_geometric,
//                                        &max_utilization_mins,
//                                        &objective_mins,
//                                        &params_mins,
//                                        subdivisions,
//                                        &test_set,
//                                        scale ]( ) {
//        auto start  = D_index_span * tid;
//        auto finish = ( tid == num_threads - 1 ) ? subdD : start + D_index_span;

//        std::cout << start << " " << finish << std::endl;

//        for ( auto dj = start; dj != finish && !stop_requested; dj++ )
//        {
//          params_t obj_params;

//          // Span of D in the log10 space.
//          auto low = search_space_min.D;
//          auto hi  = search_space_max.D;

//          auto lowl = std::log10( low );
//          auto hil  = std::log10( hi );

//          auto dl      = sample_parameter( lowl, hil, subdD, dj );
//          obj_params.D = std::pow( 10.0, dl );

//          auto subdp = subd;
//          if ( std::fabs( search_space_max.p - search_space_min.p ) < 1e-19 )
//          {
//            subdp = 1;
//          }

//          for ( st pj = 0; pj != subdp && !stop_requested; pj++ )
//          {
//            auto low = search_space_min.p;
//            auto hi  = search_space_max.p;

//            obj_params.p = sample_parameter( low, hi, subd, pj );

//            auto subdDeltaKj = subd;
//            if ( std::fabs( search_space_max.DeltaK_thr - search_space_min.DeltaK_thr ) < 1e-19 )
//            {
//              subdDeltaKj = 1;
//            }

//            for ( st DeltaKj = 0; DeltaKj != subdDeltaKj && !stop_requested; DeltaKj++ )
//            {
//              auto low = search_space_min.DeltaK_thr;
//              auto hi  = search_space_max.DeltaK_thr;

//              obj_params.DeltaK_thr = sample_parameter( low, hi, subd, DeltaKj );

//              auto subdA = subd;
//              if ( std::fabs( search_space_max.A - search_space_min.A ) < 1e-19 )
//              {
//                subdA = 1;
//              }

//              for ( st Aj = 0; Aj != subdA && !stop_requested; Aj++ )
//              {
//                auto low = search_space_min.A;
//                auto hi  = search_space_max.A;

//                obj_params.A = sample_parameter( low, hi, subd, Aj );

//                per_thread_callback( obj_params );

//                // auto d = objective_function( obj_params, use_geometric, test_set );

//                auto d = objective_function2( obj_params, use_geometric, test_set, scale );
//                totalevals++;
//                if ( d.utilization > max_utilization_mins[ tid ]
//                     || // prefer utilization over minimization
//                     ( objective_mins[ tid ] > d.distance
//                       && d.utilization >= max_utilization_mins[ tid ] ) )
//                {
//                  objective_mins[ tid ]       = d.distance;
//                  params_mins[ tid ]          = obj_params;
//                  max_utilization_mins[ tid ] = d.utilization;
//                }
//              }
//            }
//          }
//        }
//      } ) );
//    }

//    for ( st tid = 0; tid != num_threads; tid++ )
//    {
//      threads[ tid ].join( );
//    }

//    double max_utlization_at_min = 0;

//    for ( st tid = 0; tid != num_threads; tid++ )
//    {
//      if ( max_utilization_mins[ tid ] > max_utlization_at_min
//           || ( objective_min > objective_mins[ tid ]
//                && max_utilization_mins[ tid ] >= max_utlization_at_min ) )
//      {

//        objective_min         = objective_mins[ tid ];
//        params_at_min         = params_mins[ tid ];
//        max_utlization_at_min = max_utilization_mins[ tid ];
//      }
//    }

//    for ( st tid = 0; tid != num_threads; tid++ )
//    {
//      objective_mins[ tid ]       = objective_min;
//      params_mins[ tid ]          = params_at_min;
//      max_utilization_mins[ tid ] = max_utlization_at_min;
//    }

//    callback( params_at_min, search_space_min, search_space_max );

//    const T a = amortization;

//    T Dlmin                  = 0;
//    T Dlmax                  = 0;
//    std::tie( Dlmin, Dlmax ) = contract_range( std::log10( search_space_min.D ),
//                                               std::log10( search_space_max.D ),
//                                               std::log10( params_at_min.D ),
//                                               a );

//    search_space_min.D = std::pow( 10.0, Dlmin );
//    search_space_max.D = std::pow( 10.0, Dlmax );

//    std::tie( search_space_min.p, search_space_max.p )
//      = contract_range( search_space_min.p, search_space_max.p, params_at_min.p, a );

//    std::tie( search_space_min.DeltaK_thr, search_space_max.DeltaK_thr ) = contract_range(
//      search_space_min.DeltaK_thr, search_space_max.DeltaK_thr, params_at_min.DeltaK_thr, a );

//    std::tie( search_space_min.A, search_space_max.A )
//      = contract_range( search_space_min.A, search_space_max.A, params_at_min.A, a );

//    progress_callback( t, iterations );

//    std::cout << search_space_min.D << " " << search_space_max.D << " " << params_at_min.D << " "
//              << objective_min << std::endl;
//  }

//  // TODO: Is this correct? For some reason it was not here
//  return params_at_min;
//}

template< class T, class Container_t >
parameters< T > fit(
  const Container_t&  test_set,
  bool                use_geometric = false,
  callback_t< T >     callback      = []( parameters< T >, parameters< T >, parameters< T > ) {},
  progress_callback_t progress_callback                        = []( std::size_t, std::size_t ) {},
  const bool&         stop_requested                           = false,
  std::function< void( parameters< T > ) > per_thread_callback = []( parameters< T > ) {} )

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

  auto params_low  = parameters< T > { 1.0e-10, 1.7, 0.0001, 0.9 * Amin };
  auto params_high = parameters< T > { 5.0e-10, 2.3, 1.05 * DeltaKmin, 450.0 };

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

  return fit( params_low,
              params_high,
              test_set,
              12,
              1.005,
              0,
              use_geometric,
              callback,
              progress_callback,
              stop_requested,
              per_thread_callback );
}

} // namespace Hartman_Schijve

} // namespace crack_growth
