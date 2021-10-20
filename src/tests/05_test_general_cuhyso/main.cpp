#include <cgrow.hpp>
#include <cxxplot/cxxplot>
#include <nlopt.hpp>
#include <spdlog/spdlog.h>

#include <random>
#include <vector>

int main( )
{
  spdlog::info( "Starting." );

  using namespace std::chrono;
  using List = std::array< long double, 2 >;

  //-----------------------------------------------------------------
  List low   = { -2.0, -3.0 };
  List high  = { 2.0, 3.0 };

  std::vector< List > eval_set;
  std::atomic_bool stop;

  //-----------------------------------------------------------------
  auto objf = []( const List& v ) {
    return std::pow( ( long double )1.0 - v[ 0 ], 2 )
           + ( long double )100.0 * std::pow( v[ 1 ] - std::pow( v[ 0 ], 2 ), 2 );
  };

  //-----------------------------------------------------------------
  cuhyso::callback_t< List > cb = []( auto pmin, auto low, auto high ) {
    for ( auto e : pmin )
    {
      std::cout << e << " ";
    }
    std::cout << " | " << low[ 0 ] << ", " << high[ 0 ] << " | " << low[ 1 ] << ", " << high[ 1 ] <<  std::endl;
  };

  auto start = steady_clock::now( );

  //-----------------------------------------------------------------
  cuhyso::progress_callback_t pcb = [ &start ]( auto i, auto of ) {
    if ( steady_clock::now( ) - start > milliseconds( 250 ) )
    {
      start = steady_clock::now( );
      std::cout << i * 100 / of << "% (" << i << " of " << of << ")" << std::endl;
    }
  };

  //-----------------------------------------------------------------
  cuhyso::minimize< long double >( objf, low, high, 5000, 1.1024, stop, cb, pcb );

  return 0;
}
