#include "nelder_mead.hpp"

#include <spdlog/spdlog.h>
#include <nlopt.hpp>

#include <vector>

#define SCALE 1

#define LN std::cout << __LINE__ << std::endl;
double rosenbrock( double x, double y )
{
    using std::pow;
    return pow( 1.0 - SCALE*x, 2 ) + 100.0 * pow( y - pow( SCALE*x, 2 ), 2 );
}

int count =0;

double f_wrapper(const std::vector<double> &x, std::vector<double> &, void* )
{
    count ++;
    auto fv = rosenbrock( x[0], x[1] );
    return fv;
}


int main()
{
    using spdlog::info;

    nelder_mead::options_t<double> options;
    options.threshold       = 1e-30;
    options.size_threshold  = 1e-6;

    std::array x0       = { 50.5/SCALE, 200.0 };
    std::array Delta_x0 = {  4.0/SCALE,  10.0 };

    auto res = nelder_mead::searchmin(
                [](auto x)
    {
        return rosenbrock( x[0],x[1] );
    },
    x0,
    Delta_x0,
    options );

    info("Result: {}, {}", res[ 0 ], res[ 1 ] );


    nelder_mead::multi_index_t mi({2,2,2,2});


    for (; mi < mi.sizes(); mi++ )
    {
        std::cout << "mi: " << mi << std::endl;
    }


//    auto res2 = nelder_mead::search<double>(
//                [](auto x)
//    {
//        return rosenbrock( x[0],x[1] );
//    },
//    {1.0,0.0},
//    {-2.0,-2.0},
//    {2.0,2.0}
//    );

    try
    {
        nlopt::opt opt( nlopt::GN_ORIG_DIRECT_L, 2 );

        opt.set_lower_bounds( {-2.0, -2.0} );
        opt.set_upper_bounds( { 2.0, 2.0} );
        opt.set_min_objective( f_wrapper, nullptr );
        opt.set_maxeval( 5000 );

        opt.set_xtol_rel( 1.0e-5);

        info( "nlopt algorithm: {}. Number of dimensions: {}",
                      opt.get_algorithm_name(),
                      opt.get_dimension());

        std::vector<double> x_optim( opt.get_dimension() );
        double f_optim;
        auto res = opt.optimize( x_optim, f_optim );

        info("optim x: {} {}, fmin: {}, iterations: {}", x_optim[0], x_optim[1], f_optim, count );

    }
    catch ( const std::exception &e )
    {
        std::cout << e.what() << std::endl;
    }

    if ( std::abs( res[0] - 1.0 ) < options.size_threshold &&
         std::abs( res[0] - 1.0 ) < options.size_threshold )
    {
        return 0;
    }
}
