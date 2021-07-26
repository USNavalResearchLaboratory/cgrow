#include <cgrow.hpp>
#include <cxxplot/cxxplot>
#include <nlopt.hpp>
#include <spdlog/spdlog.h>

#include <vector>

template< class T >
std::vector< T > generate_sequence( T from, T to, std::size_t count )
{
  spdlog::info( "{},{},{}", from, to, count );

  std::vector< T > seq;

  T step = ( to - from ) / ( count - 1 );

  for ( std::size_t i = 0; i != count; i++ )
  {
    seq.push_back( from + step * i );
  }

  return seq;
}

using real_t = long double;

template< class TestSet_t >
void test_nelder_mead( TestSet_t& test_set )
{
  using namespace crack_growth::Hartman_Schijve;

  auto obj = [ &test_set ]( std::array< real_t, 4 > vals ) {
    parameters< real_t > params;
    params.D          = vals[ 0 ];
    params.p          = vals[ 1 ];
    params.DeltaK_thr = vals[ 2 ];
    params.A          = vals[ 3 ];

    auto d = crack_growth::Hartman_Schijve::objective_function( params, false, test_set );
    return d.distance;
  };

  nelder_mead::iteration_callback_t< real_t, 4 > callback
    = []( std::array< real_t, 5 >                  func_val,
          std::array< std::array< real_t, 4 >, 5 > vertices,
          nelder_mead::step_kind_t                 step_kind ) {
        spdlog::info( "fmin:{}, D:{} , p: {}, DeltaK_thr: {}, A: {} | Kind: {}",
                      func_val[ 0 ],
                      vertices[ 0 ][ 0 ],
                      vertices[ 0 ][ 1 ],
                      vertices[ 0 ][ 2 ],
                      vertices[ 0 ][ 3 ],
                      nelder_mead::to_string( step_kind ) );
      };

  auto options           = nelder_mead::options_t< real_t >( );
  options.size_threshold = 1e-5;

  std::array< real_t, 4 > x0      = { 2.0e-10, 2.0, 2.5, 100.0 };
  std::array< real_t, 4 > Deltax0 = { 0.1e-10, 0.1, 0.2, 20.0 };

  nelder_mead::searchmin( obj, x0, Deltax0, callback, options );
}

double      fmin1 = 1000000000;
std::size_t evals = 0;
template< class TestSet_t >
void test_direct( TestSet_t& test_set, cxxplot::figure& f1 )
{
  namespace plt = cxxplot;

  using namespace crack_growth::Hartman_Schijve;

  std::vector< plt::point2d > data;

  for ( double x = 0; x < 1; x++ )
    data.push_back( { x, x * x } );

  using namespace cxxplot::named_parameters;

  using data_t = std::tuple< TestSet_t*, plt::graph* >;

  auto obj = []( const std::vector< double >& vals, std::vector< double >&, void* data ) {
    data_t& data_cast = *static_cast< data_t* >( data );

    TestSet_t&  test_set = *std::get< 0 >( data_cast );
    plt::graph& graph    = *std::get< 1 >( data_cast );

    //  std::cout << test_set[0].points.size() << std::endl;
    using real_t = long double;

    parameters< real_t > params;
    params.D          = vals[ 0 ] * 1e-10;
    params.p          = vals[ 1 ];
    params.DeltaK_thr = vals[ 2 ];
    params.A          = vals[ 3 ];

    // graph.append_data( params.DeltaK_thr, params.A );

    evals++;
    auto d = crack_growth::Hartman_Schijve::objective_function( params, true, test_set );
    if ( fmin1 > double( d.distance ) )
    {
      fmin1 = double( d.distance );
      graph.append_data( evals, params.A );
    }

    return double( d.distance );
  };

  try
  {
    std::vector< double > dummy;

    using namespace nlopt;

    std::array solvers
      = { GN_DIRECT_L, GN_CRS2_LM, GN_MLSL, GN_MLSL_LDS, GN_ISRES, GN_ESCH, GN_AGS };

    for ( auto optimizer : solvers )
    {
      nlopt::opt opt( optimizer, 4 );

      auto& g1        = f1.add_graph( );
      g1.name = opt.get_algorithm_name();

     // g1.marker_style = plt::MarkerStyle::Cross;
     // g1.lineStyle    = plt::LineStyle::None;

      auto plot_window = plt::plot( data,
                                    window_title_ = opt.get_algorithm_name( ),
                                    marker_style_ = plt::MarkerStyle::Cross,
                                    marker_size_  = 0.3,
                                    line_style_   = plt::LineStyle::None,
                                    xlim_         = { 0, 5 },
                                    ylim_         = { 10, 300 },
                                    xlabel_       = "DKthr",
                                    ylabel_       = "A" );

      auto& f = plot_window.figure( 0 );
      auto& g = f.graph( 0 );

      data_t data_tuple( &test_set, &g1 );

      opt.set_lower_bounds( { 1.0, 1.5, 0.0001, 53.0 } );
      opt.set_upper_bounds( { 5.0, 2.5, 3.15, 450.0 } );
      opt.set_min_objective( obj, static_cast< void* >( &data_tuple ) );
      opt.set_maxtime( 10 ); // SECONDS

      std::vector< double > x_optim = { 1.2, 1.6, 0.01, 100.0 };

      double f_optim;

      fmin1    = 10000000;
      evals    = 0;
      auto res = opt.optimize( x_optim, f_optim );

      std::string name = opt.get_algorithm_name( );

      spdlog::info( "{:<75}, obj:{:.4f}, D: {:.2g}, p: {:.2f}, DKthr: {:.2f}, A:{:.2f}",
                    name,
                    f_optim,
                    x_optim[ 0 ] * 1e-10,
                    x_optim[ 1 ],
                    x_optim[ 2 ],
                    x_optim[ 3 ] );
    }
  }
  catch ( const std::exception& e )
  {
    std::cout << e.what( ) << std::endl;
  }
}
int main( )
{

  return cxxplot::exec( 0, nullptr, [ & ]( ) {
    namespace plt = cxxplot;
    using namespace plt::named_parameters;

    spdlog::info( "cxxplot version: {} ", plt::version( ) );

    namespace hs = crack_growth::Hartman_Schijve;

    hs::parameters< real_t > params { 2.73e-10, 2.03, 3.1, 59.8 };

    real_t R = 0.8;

    auto DeltaK_max = hs::calc_K_max( params, R );

    spdlog::info( "{}", hs::evaluate( params, R, real_t( 6.0 ) ) );

    auto DKs = generate_sequence( params.DeltaK_thr * ( 1.0 + 0.001 ), // From
                                  DeltaK_max * ( 1.0 - 0.001 ),        // To
                                  30                                  // Number of points
    );

    auto dadNs = hs::evaluate( params, R, DKs );

    plt::plot( DKs,
               dadNs,
               line_style_          = plt::LineStyle::None,
               marker_style_        = plt::MarkerStyle::Cross,
               x_axis_scaling_type_ = plt::axis_scaling_type::logarithmic,
               y_axis_scaling_type_ = plt::axis_scaling_type::logarithmic );

    std::vector< crack_growth::test_data_t< real_t > > test_set;
    crack_growth::test_data_t< real_t >                test_data;

    {
      test_data.R = 0.8;
      for ( std::size_t i = 0; i != dadNs.size( ); i++ )
      {
        test_data.points.push_back( { DKs[ i ], dadNs[ i ] } );
      }
      test_set.emplace_back( test_data );
    }

    for ( std::size_t i = 0; i != dadNs.size( ); i++ )
    {
      spdlog::info( "{},{}", DKs[ i ], dadNs[ i ] );
    }

    auto model_distance = hs::distance( params, R, DKs, dadNs );

    spdlog::info( "Distance: {}, Util: {}", model_distance.distance, model_distance.utilization );

    params.p       = 2;
    model_distance = hs::distance( params, R, DKs, dadNs );
    spdlog::info( "Distance: {}, Util: {}", model_distance.distance, model_distance.utilization );

    params.DeltaK_thr = 4;
    model_distance    = hs::distance( params, R, DKs, dadNs );
    spdlog::info( "Distance: {}, Util: {}", model_distance.distance, model_distance.utilization );

    // test_nelder_mead( test_set );

    auto plot_window1 = plt::plot( std::array< double, 0 > { },
                                   std::array< double, 0 > { },
                                   window_title_ = "CUHYSO",
                                   //marker_style_ = plt::MarkerStyle::Cross,
                                   marker_size_  = 4,
                                   //line_style_   = plt::LineStyle::None,
                                   xlabel_       = "DKthr",
                                   ylabel_       = "A",
                                   show_legend_ = true );

    auto& f1               = plot_window1.figure( 0 );

    auto& g1               = f1.graph( 0 );
    g1.name = "CUHYSO";
    f1.x_axis_scaling_type = plt::axis_scaling_type::logarithmic;

    auto plot_window2 = plt::plot( { 0 },
                                   { 0 },
                                   window_title_ = "CUHYSO2",
                                   //marker_style_ = plt::MarkerStyle::Cross,
                                   //marker_size_  = 3,
                                   //line_style_   = plt::LineStyle::None,

                                   xlabel_ = "D x 1e-10",
                                   ylabel_ = "p" );

    auto& f2 = plot_window2.figure( 0 );
    auto& g2 = f2.graph( 0 );

    std::size_t i = 0;
    totalevals = 0;
    auto        p = hs::fit< real_t >(
      test_set,
      [ &i, &g1 ](
        hs::parameters< real_t > p, hs::parameters< real_t > l, hs::parameters< real_t > h ) {
        //   spdlog::info( "D: {}, p: {}, A: {}, DeltaK_thr: {}", p.D, p.p, p.DeltaK_thr, p.A );
        //   spdlog::info( " D: {}, p: {}, A: {}, DeltaK_thr: {}", l.D, l.p, l.DeltaK_thr, l.A );
        //   spdlog::info( " D: {}, p: {}, A: {}, DeltaK_thr: {}", h.D, h.p, h.DeltaK_thr, h.A );
        spdlog::info( "{},{}", i, p.A );
        g1.append_data( totalevals, p.A );
      },
      [ &i ]( std::size_t t, std::size_t ) { i = t; },
      false,
      [ &g1, &g2 ]( hs::parameters< real_t > p ) {
        //  g1.append_data( i++, p.DeltaK_thr, );
        //   g2.append_data( p.D*1e10, p.p );
      } );

    {
      auto model_distance = hs::distance( p, R, DKs, dadNs );

      spdlog::info( "{:<75}, obj:{:.4f}, D: {:.2g}, p: {:.2f}, DKthr: {:.2f}, A:{:.2f}",
                    "CUHYSO:",
                    model_distance.distance,
                    p.D,
                    p.p,
                    p.DeltaK_thr,
                    p.A );
    }

    test_direct( test_set, f1 );

    return 0;
  } );
}
