#include <cgrow.hpp>

#include <atomic>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
  using scalar_t = double;
  using hs_t     = crack_growth::Hartman_Schijve::parameters< scalar_t >;
  using test_t   = crack_growth::test_data_t< scalar_t >;

  // A small synthetic test. Replace these points with measurements from your
  // own experiment; each test has one R value and DeltaK/dadN pairs.
  constexpr scalar_t R = 0.8;
  const hs_t source_parameters{ 3.9e-10, 2.29, 3.04, 116.81 };
  const std::vector< scalar_t > delta_k{ 8.0, 10.0, 12.0, 16.0, 20.0, 22.0 };

  test_t test;
  test.R = R;
  for ( const auto delta_k_value : delta_k )
  {
    test.points.push_back( { delta_k_value,
                             crack_growth::Hartman_Schijve::evaluate(
                               source_parameters, R, delta_k_value ) } );
  }
  const std::vector< test_t > test_set{ test };

  // Parameter order: D, p, DeltaK_thr, A.
  const hs_t lower{ 1.0e-10, 1.8, 1.0, 80.0 };
  const hs_t upper{ 1.0e-9, 2.8, 6.0, 160.0 };
  std::atomic_bool stop_requested{ false };

  const auto fitted = crack_growth::Hartman_Schijve::fit(
    lower,
    upper,
    test_set,
    4,
    2.0,
    4,
    false,
    crack_growth::Hartman_Schijve::callback_t< scalar_t >( []( hs_t, hs_t, hs_t ) {} ),
    crack_growth::Hartman_Schijve::progress_callback_t( []( std::size_t, std::size_t ) {} ),
    stop_requested );

  const auto scale = crack_growth::computeAxesScale< scalar_t >( test_set );
  const auto residual
    = crack_growth::Hartman_Schijve::objective_function( fitted, false, test_set, scale );

  std::cout << "Fitted parameters\n"
            << "  D           = " << fitted.D << '\n'
            << "  p           = " << fitted.p << '\n'
            << "  DeltaK_thr  = " << fitted.DeltaK_thr << '\n'
            << "  A           = " << fitted.A << '\n'
            << "Residual distance = " << residual.distance << '\n'
            << "Utilization       = " << residual.utilization << '\n';

  return std::isfinite( fitted.D ) && std::isfinite( fitted.p )
                 && std::isfinite( fitted.DeltaK_thr ) && std::isfinite( fitted.A )
                 && std::isfinite( residual.distance ) && residual.utilization > 0.0
           ? 0
           : 1;
}
