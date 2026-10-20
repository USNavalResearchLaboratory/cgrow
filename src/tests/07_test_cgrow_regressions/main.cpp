#include <cgrow.hpp>

#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

int main()
{
  using T = double;
  using Params = std::vector< T >;

  static_assert( crack_growth::Hartman_Schijve::HS_MAX_ITERS == 60 );

  // This is a genuine bisection case.  A converged result must report its
  // actual iteration count, not the old hard-coded value one.
  const auto distance = crack_growth::Hartman_Schijve::minimum_distance< T >(
    10.0, 1.0e-8, 0.8, 3.9e-10, 2.29, 0.1, 50.0, 8.12 );
  std::cerr << "distance=" << std::get< 0 >( distance ) << " iterations="
            << std::get< 1 >( distance ) << "\n";
  if ( !std::isfinite( std::get< 0 >( distance ) ) || std::get< 1 >( distance ) <= 1
       || std::get< 1 >( distance ) >= crack_growth::Hartman_Schijve::HS_MAX_ITERS )
  {
    return 10;
  }

  std::atomic_bool stop_requested{ false };
  std::atomic< std::size_t > evaluations{ 0 };
  const auto objective = [ &evaluations ]( const Params& p ) {
    evaluations.fetch_add( 1, std::memory_order_relaxed );
    return p[ 0 ] * p[ 0 ] + p[ 1 ] * p[ 1 ];
  };

  {
    bool rejected_invalid_index = false;
    Params upstream{ 0.0, 0.0 };
    std::vector< Params > generated;
    try
    {
      cuhyso::generate_eval_set< T >( objective,
                                       Params{ -1.0, -1.0 },
                                       Params{ 1.0, 1.0 },
                                       2,
                                       generated,
                                       stop_requested,
                                       upstream,
                                       2 );
    }
    catch ( const std::out_of_range& )
    {
      rejected_invalid_index = true;
    }
    if ( !rejected_invalid_index )
    {
      return 15;
    }

  {
    crack_growth::test_data_t< T > repeated_delta_k;
    repeated_delta_k.R = 0.5;
    repeated_delta_k.points = { { 10.0, 1.0e-8 }, { 10.0, 2.0e-8 } };
    bool rejected_degenerate_scale = false;
    try
    {
      (void)crack_growth::computeAxesScale< T >(
        std::vector< crack_growth::test_data_t< T > >{ repeated_delta_k } );
    }
    catch ( const std::invalid_argument& )
    {
      rejected_degenerate_scale = true;
    }
    if ( !rejected_degenerate_scale )
    {
      return 19;
    }
  }
  }

  // Collapsing one dimension must not collapse the dimensions that follow it.
  // With the old recursive call, a fixed first parameter propagated a local
  // subdivision count of one to every later parameter.
  {
    Params upstream{ 0.0, 0.0 };
    std::vector< Params > generated;
    if ( !cuhyso::generate_eval_set< T >( objective,
                                          Params{ 2.0, -1.0 },
                                          Params{ 2.0, 1.0 },
                                          2,
                                          generated,
                                          stop_requested,
                                          upstream )
         || generated.size( ) != 2 )
    {
      return 18;
    }
  }

  const auto invalid_objective = crack_growth::Hartman_Schijve::objective_function< T >(
    { 0.0, 0.0, 0.0, 0.0 }, false, std::vector< crack_growth::test_data_t< T > >{}, 1.0 );
  if ( invalid_objective.utilization != 0.0
       || invalid_objective.distance
            != T( crack_growth::Hartman_Schijve::constants::invalid_objective_distance ) )
  {
    return 16;
  }

  // Two iterations must evaluate exactly two fresh 2x2 grids.  This catches
  // the old missing eval_set.clear(), which accumulated prior rounds.
  std::cerr << "before minimize\n";
  try
  {
    cuhyso::minimize< T >( objective,
                           Params{ -1.0, -1.0 },
                           Params{ 1.0, 1.0 },
                           2,
                           2.0,
                           stop_requested,
                           cuhyso::callback_t< Params >( []( Params, Params, Params ) {} ),
                           []( std::size_t, std::size_t ) {},
                           2,
                           2 );
  }
  catch ( const std::exception& error )
  {
    std::cerr << "minimize failed: " << error.what() << "\n";
    return 13;
  }
  std::cerr << "after minimize\n";
  std::cerr << "evaluations=" << evaluations.load() << "\n";
  if ( evaluations.load() != 8 )
  {
    return 11;
  }

  evaluations.store( 0, std::memory_order_relaxed );
  stop_requested.store( false, std::memory_order_relaxed );
  cuhyso::minimize< T >( objective,
                         Params{ -1.0, -1.0 },
                         Params{ 1.0, 1.0 },
                         2,
                         1.0e9,
                         stop_requested,
                         cuhyso::callback_t< Params >( []( Params, Params, Params ) {} ),
                         []( std::size_t, std::size_t ) {},
                         0,
                         1 );
  if ( evaluations.load( std::memory_order_relaxed ) != 4 )
  {
    return 17;
  }

  // The global counter is now namespaced and atomic, so concurrent increments
  // have a well-defined result.
  crack_growth::Hartman_Schijve::totalevals.store( 0 );
  std::vector< std::thread > workers;
  for ( int i = 0; i != 4; ++i )
  {
    workers.emplace_back( [] {
      for ( int j = 0; j != 1000; ++j )
      {
        crack_growth::Hartman_Schijve::totalevals.fetch_add( 1, std::memory_order_relaxed );
      }
    } );
  }
  for ( auto& worker : workers )
  {
    worker.join();
  }
  std::cerr << "total evaluations="
            << crack_growth::Hartman_Schijve::totalevals.load() << "\n";
  if ( crack_growth::Hartman_Schijve::totalevals.load() != 4000 )
  {
    return 12;
  }

  // Oracle generated from example_data/synthetic_benchmark.cg.json with the
  // predecessor (HEAD), using the same bounded two-round search below.
  const std::vector< T > delta_k{ 10.2,
                                  10.82061324,
                                  11.47898734,
                                  12.17741985,
                                  12.91834808,
                                  13.70435767,
                                  14.53819158,
                                  15.42275963,
                                  16.36114874,
                                  17.35663359,
                                  18.41268816,
                                  19.53299777,
                                  20.72147199,
                                  21.98225826,
                                  23.31975635,
                                  24.73863375,
                                  26.24384195,
                                  27.8406337,
                                  29.53458134,
                                  31.33159626,
                                  33.23794954,
                                  35.26029381,
                                  37.40568648,
                                  39.68161435,
                                  42.09601977,
                                  44.65732832,
                                  47.37447824,
                                  50.25695162,
                                  53.31480746,
                                  56.55871681 };
  const std::vector< T > dad_n{ 2.5911e-11,
                                4.74176e-10,
                                1.60836e-09,
                                3.60737e-09,
                                6.68176e-09,
                                1.10845e-08,
                                1.7122e-08,
                                2.51677e-08,
                                3.56788e-08,
                                4.92184e-08,
                                6.64836e-08,
                                8.83436e-08,
                                1.1589e-07,
                                1.50505e-07,
                                1.93958e-07,
                                2.48536e-07,
                                3.17236e-07,
                                4.04049e-07,
                                5.14374e-07,
                                6.55681e-07,
                                8.38554e-07,
                                1.07846e-06,
                                1.39881e-06,
                                1.83673e-06,
                                2.45462e-06,
                                3.36561e-06,
                                4.79768e-06,
                                7.28693e-06,
                                1.24565e-05,
                                2.85297e-05 };
  crack_growth::test_data_t< T > benchmark;
  benchmark.R = 0.8;
  for ( std::size_t i = 0; i != delta_k.size(); ++i )
  {
    benchmark.points.push_back( { delta_k[ i ], dad_n[ i ] } );
  }
  const std::vector< crack_growth::test_data_t< T > > benchmark_set{ benchmark };
  using HS = crack_growth::Hartman_Schijve::parameters< T >;
  const HS low{ 1.0e-12, 1.5, 0.01, 20.0 };
  const HS high{ 1.0e-8, 3.0, 10.0, 300.0 };
  // The no-stop overload must create independent cancellation state instead
  // of sharing a mutable process-wide default flag.
  const auto fitted = crack_growth::Hartman_Schijve::fit(
    low,
    high,
    benchmark_set,
    3,
    2.0,
    2,
    false,
    crack_growth::Hartman_Schijve::callback_t< T >( []( HS, HS, HS ) {} ),
    crack_growth::Hartman_Schijve::progress_callback_t( []( std::size_t, std::size_t ) {} ) );
  const auto benchmark_scale = crack_growth::computeAxesScale< T >( benchmark_set );
  const auto residual = crack_growth::Hartman_Schijve::objective_function(
    fitted, false, benchmark_set, benchmark_scale );
  const HS oracle{ 1.0e-10, 2.625, 7.5025, 300.0 };
  constexpr T oracle_residual = 0.323687;
  constexpr T parameter_tolerance = 0.002;
  const auto within_oracle = [parameter_tolerance]( T actual, T expected ) {
    return std::abs( actual - expected ) <= parameter_tolerance * std::max( T( 1 ), std::abs( expected ) );
  };
  if ( !within_oracle( fitted.D, oracle.D ) || !within_oracle( fitted.p, oracle.p )
       || !within_oracle( fitted.DeltaK_thr, oracle.DeltaK_thr )
       || !within_oracle( fitted.A, oracle.A ) || residual.distance > oracle_residual + 1.0e-6 )
  {
    std::cerr << "oracle mismatch: " << fitted.D << ' ' << fitted.p << ' '
              << fitted.DeltaK_thr << ' ' << fitted.A << " residual=" << residual.distance << "\n";
    return 14;
  }

  std::cout << "cgrow regression tests passed\n";
}
