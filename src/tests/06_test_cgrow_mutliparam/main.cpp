#include <cgrow.hpp>
#include <cxxplot/cxxplot>
#include <nlopt.hpp>
#include <spdlog/spdlog.h>

#include <random>
#include <vector>

using real_t      = double;
using test_data_t = crack_growth::test_data_t< real_t >;
using test_set_t  = std::vector< test_data_t >;
using params_t    = std::array< real_t, 4 >;

constexpr real_t tD     = 3.9e-10;
constexpr real_t tp     = 2.29;
constexpr real_t tDKThr = 3.04;
constexpr real_t tA     = 116.81;

using List = std::vector< real_t >;

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
auto generate_synthetic_data( T D, T p, T DKThr, T A, T R, std::size_t num_data_points = 50 )
{
  namespace hs = crack_growth::Hartman_Schijve;

  auto params     = hs::parameters< real_t > { D, p, DKThr, A };
  auto DeltaK_max = hs::calc_K_max( params, R );

  auto DekltaK_start  = params.DeltaK_thr * ( 1.0 + 0.01 );
  auto DekltaK_finish = DeltaK_max * ( 1.0 - 0.01 );
  auto DKs            = generate_sequence( DekltaK_start, DekltaK_finish, num_data_points );
  auto dadNs          = crack_growth::Hartman_Schijve::evaluate( params, R, DKs );

  return std::make_pair( DKs, dadNs );
}

template< class T >
std::string generate_legend_item( T DKthr, T A, T R )
{
  std::stringstream ss;
  ss << "ΔKthr = " << DKthr << ", A = " << A << ", R = " << R;

  return ss.str( );
}

template< class T, class ListType >
void append_to_test_set( test_set_t&     test_set,
                         const T&        R,
                         const ListType& DKs,
                         const ListType& dadNs )
{
  if ( DKs.size( ) != dadNs.size( ) )
  {
    throw std::out_of_range( "Incompatible DK and dadN list sizes." );
  }

  test_data_t test_data;
  test_data.R = R;

  for ( std::size_t i = 0; i != DKs.size( ); i++ )
  {
    test_data.points.push_back( test_data_t::point_type( DKs[ i ], dadNs[ i ] ) );
  }

  test_set.emplace_back( test_data );
}

int main( )
{
  return cxxplot::exec( 0, nullptr, [ & ]( ) {
    namespace plt = cxxplot;
    using namespace plt::named_parameters;
    // ----------------------------------------------------------------------
    spdlog::info( "Starting." );

    std::vector< crack_growth::test_data_t< real_t > > test_set;
    crack_growth::test_data_t< real_t >                test_data;

    real_t R = 0.8;

    auto [ DKs, dadNs ] = generate_synthetic_data( tD, tp, tDKThr, tA, R );

    append_to_test_set( test_set, R, DKs, dadNs );

    auto win0 = plt::plot( DKs,
                           dadNs,
                           line_style_          = plt::LineStyle::None,
                           marker_style_        = plt::MarkerStyle::Cross,
                           x_axis_scaling_type_ = plt::axis_scaling_type::logarithmic,
                           y_axis_scaling_type_ = plt::axis_scaling_type::logarithmic,
                           show_legend_         = true,
                           legend_alignment_
                           = plt::HorizontalAlignment::Right | plt::VerticalAlignment::Bottom,
                           name_ = generate_legend_item( tDKThr, tA, 0.8 ) );

    {
      real_t DKthr = 2.04;
      real_t A     = 130.0;
      real_t R     = 0.8;

      auto [ DKs, dadNs ] = generate_synthetic_data( tD, tp, DKthr, A, R );

      // append_to_test_set( test_set, R, DKs, dadNs );

      win0.add_graph( DKs,
                      dadNs,
                      line_style_   = plt::LineStyle::None,
                      marker_style_ = plt::MarkerStyle::Cross,
                      name_         = generate_legend_item( DKthr, A, R ) );
    }

    {
      real_t DKthr = 1.8;
      real_t A     = 110.0;
      real_t R     = 0.8;

      auto [ DKs, dadNs ] = generate_synthetic_data( tD, tp, DKthr, A, R );

      // append_to_test_set( test_set, R, DKs, dadNs );

      win0.add_graph( DKs,
                      dadNs,
                      line_style_   = plt::LineStyle::None,
                      marker_style_ = plt::MarkerStyle::Cross,
                      name_         = generate_legend_item( DKthr, A, R ) );
    }

    {
      real_t DKthr = 1.92;
      real_t A     = 114.0;
      real_t R     = 0.5;

      auto [ DKs, dadNs ] = generate_synthetic_data( tD, tp, DKthr, A, R );

      // append_to_test_set( test_set, R, DKs, dadNs );

      win0.add_graph( DKs,
                      dadNs,
                      line_style_   = plt::LineStyle::None,
                      marker_style_ = plt::MarkerStyle::Cross,
                      name_         = generate_legend_item( DKthr, A, R ) );
    }

    auto scale = crack_growth::computeAxesScale< real_t >( test_set );

    //------------------------------
    auto objf = [ scale, &test_set ]( const params_t& v ) {
      auto D     = std::pow( 10.0, v[ 0 ] );
      auto p     = v[ 1 ];
      auto DKthr = v[ 2 ];
      auto A     = v[ 3 ];

      crack_growth::Hartman_Schijve::parameters< real_t > params { D, p, DKthr, A };

      auto md = crack_growth::Hartman_Schijve::objective_function( params, true, test_set, scale );

      return md.distance;
    };

    params_t atmin;
    //-----------------------------------------------------------------
    cuhyso::callback_t< params_t > cb = [ &atmin ]( auto pmin, auto low, auto high ) {
      atmin = pmin;
      for ( auto e : pmin )
      {
        std::cout << e << " ";
      }
      std::cout << " | " << low[ 0 ] << ", " << high[ 0 ] << " | " << low[ 1 ] << ", " << high[ 1 ]
                << std::endl;
    };

    using namespace std::chrono;

    auto start = steady_clock::now( );

    //-----------------------------------------------------------------
    cuhyso::progress_callback_t pcb = [ &start ]( auto i, auto of ) {
      if ( steady_clock::now( ) - start > milliseconds( 250 ) )
      {
        start = steady_clock::now( );
        std::cout << i * 100 / of << "% (" << i << " of " << of << ")" << std::endl;
      }
    };

    params_t low  = { -12.0, 1.5, 0.001, 60.0 };
    params_t high = { -9.0, 2.5, 20.0, 200.0 };

    std::atomic_bool stop = false;

    cuhyso::minimize( objf, low, high, 7, 2.0, stop, cb, pcb );

    {
      auto [ DKs, dadNs ] = generate_synthetic_data(
        std::pow( 10.0, atmin[ 0 ] ), atmin[ 1 ], atmin[ 2 ], atmin[ 3 ], 0.8, 500 );

      win0.add_graph( DKs, dadNs, name_ = "Identified" );
    }

    return 0;
  } );
}
