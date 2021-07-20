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
#include "linalg.h"

#include <array>
#include <numeric>
#include <algorithm>
#include <iostream>


template<typename OStream, typename T, std::size_t N>
OStream &operator<<(OStream &os, const std::array<T,N> &a)
{
    for ( std::size_t i = 0 ; i != N; i++)
    {
        os << a[i] << ' ';
    }
    return os ;
}

namespace nelder_mead
{
namespace detail
{
template <class T, std::size_t N>
std::array<T,N> &operator+=( std::array<T,N> &ar1, const std::array<T,N> &ar2 )
{
    for ( std::size_t i = 0; i != N; i++ )
    {
        ar1[i] += ar2[i];
    }
    return ar1;
}

template <class T, std::size_t N>
std::array<T,N> operator+( std::array<T,N> ar1, const std::array<T,N> &ar2 )
{
    for ( std::size_t i = 0; i != N; i++ )
    {
        ar1[i] += ar2[i];
    }
    return ar1;
}

template <class T, std::size_t N>
std::array<T,N> operator-( std::array<T,N> ar1, const std::array<T,N> &ar2 )
{
    for ( std::size_t i = 0; i != N; i++ )
    {
        ar1[i] -= ar2[i];
    }
    return ar1;
}

template <class T, std::size_t N>
std::array<T,N> operator+( std::array<T,N> ar, const T&v )
{
    for ( std::size_t i = 0; i != N; i++ )
    {
        ar[i] += v;
    }
    return ar;
}

template <class T, std::size_t N>
std::array<T,N> operator*( std::array<T,N> ar, const T&v )
{
    for ( std::size_t i = 0; i != N; i++ )
    {
        ar[i] *= v;
    }
    return ar;
}

template <class T, std::size_t N>
std::array<T,N> operator/( std::array<T,N> ar, const T&v )
{
    for ( std::size_t i = 0; i != N; i++ )
    {
        ar[i] /= v;
    }
    return ar;
}

template <typename T>
std::vector<std::size_t> sort_indexes( const std::vector<T> &v )
{

    std::vector<std::size_t> idx( v.size() );
    std::iota(idx.begin(), idx.end(), 0);

    std::stable_sort(idx.begin(), idx.end(),
                     [&v](size_t i1, size_t i2)
    {
        return v[i1] < v[i2];
    });

    return idx;
}

template <typename T, std::size_t N>
std::array<std::size_t ,N> sort_indexes(const std::array<T, N> &v)
{

    std::array<std::size_t, N> idx;
    std::iota(idx.begin(), idx.end(), 0);

    std::stable_sort(idx.begin(), idx.end(),
                     [&v](size_t i1, size_t i2)
    {
        return v[i1] < v[i2];
    });

    return idx;
}

template <typename T, std::size_t N>
std::array<T ,N> reorder(
        const std::array<T, N> &v,
        const std::array<std::size_t, N> &idx)
{
    std::array<T, N> v_ret;

    for ( std::size_t i = 0; i != N; i++ )
    {
        v_ret[i] = v[idx[i]];
    }
    return v_ret;
}

template <class T, std::size_t N, std::size_t M>
T simplex_size( const std::array<std::array< T, N>, M> simplex )
{
    std::array<T,N> mins; mins.fill( std::numeric_limits<T>::max() );
    std::array<T,N> maxs; maxs.fill( std::numeric_limits<T>::lowest() );

    for ( const auto &vertex : simplex )
    {
        for ( std::size_t i = 0; i != vertex.size(); i++ )
        {
            mins[i] = std::min( mins[i], vertex[i] );
            maxs[i] = std::max( maxs[i], vertex[i] );
        }
    }

    T volume = 1.0;
    for ( std::size_t i = 0; i != mins.size(); i++ )
    {
        volume *= (maxs[i] - mins[i]);
    }

    return std::pow( volume, (1.0 / N ) );
}

template <class T, class U>
void sort_vertices( T &func_val, U &vertices )
{
    auto j = sort_indexes( func_val );

    func_val = reorder( func_val, j );
    vertices = reorder( vertices, j );
};

}

template <class T = double>
struct options_t
{
    options_t() = default;

    std::size_t max_iterations  = 10000;
    T threshold                 = 1e-4;
    T size_threshold            = 1e-4; // Characteristic length. If simplex is enclosed in a 3D box this would be: volume^(1/3). In a rectangle: area^(1/2) and so on
};

enum class step_kind_t
{
    initial,
    reflection,
    expansion,
    contraction,
    shrink
};

inline const char *to_string( const step_kind_t &step_kind )
{
    switch ( step_kind )
    {
    case step_kind_t::initial:      return "initial";
    case step_kind_t::reflection:   return "reflection";
    case step_kind_t::expansion:    return "expansion";
    case step_kind_t::contraction:  return "contraction";
    case step_kind_t::shrink:       return "shrink";

    }

    return "unknown";
}

template <class T, std::size_t N>
using iteration_callback_t = std::function<
void( const std::array<T,N+1> , const std::array<std::array<T,N>,N+1>, step_kind_t) >;

// Delta_x0 to setup the initial simplex
template <class T, class F, std::size_t N>
std::array<T, N> searchmin(
        F               &&func,
        std::array<T,N> x0,
        std::array<T,N> Delta_x0,
        options_t<T>    options = options_t<T>{},
        iteration_callback_t<T,N> callback=
        []( std::array<T,N+1>, std::array<std::array<T,N>,N+1>, step_kind_t ){ })
{
using namespace detail;

using params_t = std::array<T, N>;

using st = std::size_t;

constexpr st first_vertex       = 0;
constexpr st penultimate_vertex = N-1;
constexpr st ultimate_vertex    = N;

// Initial simplex
std::array< params_t, N+1> vertices;
{
vertices[ first_vertex ] = x0;

for ( st i = 1; i!= vertices.size(); i++ )
{
    vertices[i]         = x0;
    vertices[i][i-1]    = x0[i - 1] + Delta_x0[ i - 1 ];
}
}

std::array<T,N+1> func_val{0.0};
{
for ( st i = 0; i != func_val.size(); i++ )
{
    func_val[i]    = func( vertices[ i ] );
}
}

// Regular Nelder-Mead parameters
T alpha = 1.0;
T beta  = 0.5;
T gamma = 2.0;
T sigma = 0.5;

step_kind_t step_kind = step_kind_t::initial;

st iteration = 0;
for ( ;
iteration                 < options.max_iterations    &&
func_val[0]               > options.threshold         &&
simplex_size( vertices )  > options.size_threshold    ;

iteration++ )
{
    sort_vertices( func_val, vertices );


    callback( func_val, vertices, step_kind );

    params_t mean_vertex{ 0.0 };

    for ( st i = 0; i != vertices.size() - 1; i++)
    {
        mean_vertex += vertices[i] / T{ vertices.size() - 1 };
    }

    auto reflected_vertex = mean_vertex * ( 1.0 + alpha ) - vertices[ ultimate_vertex ] * alpha;
    auto reflected_func_val = func( reflected_vertex );


    if ( reflected_func_val < func_val[ penultimate_vertex ] &&
         reflected_func_val > func_val[ first_vertex ] )
    {
        vertices[ ultimate_vertex ] = reflected_vertex;
        func_val[ ultimate_vertex ] = reflected_func_val;
        step_kind = step_kind_t::reflection;
    }
    else
    {
        if ( reflected_func_val < func_val[ first_vertex ] )
        {
            auto expanded_vertex = mean_vertex + (reflected_vertex - mean_vertex) * gamma;
            auto expanded_func_val = func( expanded_vertex );

            if ( expanded_func_val < reflected_func_val )
            {
                vertices[ ultimate_vertex ] = expanded_vertex;
                func_val[ ultimate_vertex ] = expanded_func_val;
                step_kind = step_kind_t::expansion;
            }
            else
            {
                vertices[ ultimate_vertex ] = reflected_vertex;
                func_val[ ultimate_vertex ] = reflected_func_val;
                step_kind = step_kind_t::reflection;
            }
        }
        else
        {   // Modified the wikipedia variation to account for outside contraction.
            auto contracted_vertex = ( reflected_func_val < func_val[ ultimate_vertex ]) ?
                        mean_vertex * ( 1.0 + beta ) - vertices[ ultimate_vertex ] * beta:  //outside
                        mean_vertex + ( vertices[ ultimate_vertex ] - mean_vertex ) * beta; //inside

            auto contracted_func_val = func( contracted_vertex );

            if ( contracted_func_val < func_val[ ultimate_vertex ])
            {
                vertices[ ultimate_vertex ] = contracted_vertex;
                func_val[ ultimate_vertex ] = contracted_func_val;
                step_kind = step_kind_t::contraction;
            }
            else
            {   // shrink
                for ( st i = 1 ; i != vertices.size( ); i++ )
                {
                    vertices[ i ] = vertices[ 0 ] + ( vertices[ i ] - vertices[ 0 ]) * sigma;
                    func_val[ i ] = func( vertices[ i ] );
                }
                step_kind = step_kind_t::shrink;
            }
        }
    }

    //std::cout << vertices << std::endl;
}

sort_vertices( func_val, vertices );

return vertices[ first_vertex ];
}

template <class T, class F, std::size_t N>
std::array<T, N> searchmin(
        F               &&func,
        std::array<T,N> x0,
        std::array<T,N> Delta_x0,
        iteration_callback_t<T,N> callback,
        options_t<T>    options = options_t<T>{})
{
    return searchmin( func, x0, Delta_x0, options, callback );
}

template <class T>
class bounds
{
public:
    void expand( const T &p )
    {
        lower = std::min( lower, p );
        upper = std::max( upper, p );
    }

    T lower = std::numeric_limits<T>::max();
    T upper = std::numeric_limits<T>::lowest();
};


struct multi_index_t
{
    multi_index_t( std::vector<std::size_t> sizes ):
        sizes_( sizes ),
        location_( sizes.size() )
    {

    }

    std::size_t rank( ) const { return sizes_.size(); }


    inline bool operator<( const std::vector<std::size_t>& other_sizes ) const
    {
        if ( rank() != other_sizes.size() )
        {
            throw std::runtime_error("Incompatible sizes.");
        }

        return compare( other_sizes, 0 );
    }

    inline bool operator<( const multi_index_t& other ) const
    {
        if ( other.rank() != rank() )
        {
            throw std::runtime_error("Incompatible size multi_indexes.");
        }

        return compare( other, 0 );
    }

    multi_index_t &operator++( int)
    {
        advance( rank() - 1 );
        return *this;
    }

    const std::vector<std::size_t> &sizes() const { return sizes_; }

    const std::vector<std::size_t> &location() const { return location_; }

    const std::size_t operator[]( std::size_t index ) const { return location()[index]; }

private:

    bool compare( const multi_index_t& other, std::size_t index ) const
    {

        if ( index == sizes_.size() )
        {
            return true;
        }

        if ( location_[ index ] >= other.location_[index] )
        {
            return false;
        }

        return compare( other, index + 1);
    }

    bool compare( const std::vector<std::size_t>& other_sizes, std::size_t index ) const
    {

        if ( index == sizes_.size() )
        {
            return true;
        }

        if ( location_[ index ] >= other_sizes[index] )
        {
            return false;
        }

        return compare( other_sizes, index + 1);
    }

    void advance( std::size_t index )
    {
        if ( location_[ index ] < sizes_[ index ] - 1 )
        {
            location_[index]++;
            return;
        }

        if ( index != 0 )
        {
            for ( auto i = index ; i != rank(); i++ )
            {
                location_[index] = 0;
            }
            advance( index - 1 );
            return;
        }

        location_[ rank() - 1 ] = sizes_[rank() - 1];
    }


    std::vector< std::size_t > sizes_;
    std::vector< std::size_t > location_;
};

template<typename OStream>
OStream &operator<<(OStream &os, const multi_index_t &mi)
{
    for ( std::size_t i = 0 ; i != mi.rank(); i++)
    {
        os << mi[i] << ' ';
    }
    return os ;
}

template<typename OStream, typename T>
OStream &operator<<(OStream &os, const std::vector<T> &v)
{
    for ( std::size_t i = 0 ; i != v.size(); i++)
    {
        os << v[i] << ' ';
    }
    return os ;
}

namespace detail
{

template <class T>
std::vector<T> operator-( std::vector<T> a, const std::vector<T> &b )
{
    for ( std::size_t i = 0 ; i != a.size(); i ++ )
    {
        a[i] -= b[i];
    }
    return a;
}

template <class T>
std::vector<T> operator+( std::vector<T> a, const std::vector<T> &b )
{
    for ( std::size_t i = 0 ; i != a.size(); i ++ )
    {
        a[i] += b[i];
    }
    return a;
}

template <class T>
std::vector<T> operator*( const T a, std::vector<T> v )
{
    for ( std::size_t i = 0 ; i != v.size(); i ++ )
    {
        v[i] *= a;
    }
    return v;
}

}

template <class T>
class node
{
    int dimensions = 2;
    std::vector< node< T > > subnodes;
    std::vector< bounds<T> > bounds_;

    bounds<T> get_bounds_of_subdiv( int index )
    {
        std::vector< bounds<T>> subdiv_bounds;

        for ( auto dim = 0; dim != bounds_.size(); dim++ )
        {
            auto middle = ( bounds_[index].lower + bounds_[index].upper ) / 2.0;

            auto is_the_left = index & (1 << dim); //Binary representation of left and right

            if ( is_the_left )
            {
                subdiv_bounds.emplace_back( bounds<T>{ bounds_[index].lower, middle } );
            }
            else
            {
                subdiv_bounds.emplace_back( bounds<T>{ middle, bounds_[index].upper } );
            }
        }
    }
};

template <class T, class F>
std::vector<T> search(
        F               &&f,
        std::vector<T>  x0,
        std::vector<T>  low,
        std::vector<T>  high )
{
    using namespace detail;

    if ( !( x0.size() == low.size() && x0.size() == high.size())  )
    {
        throw std::runtime_error("x0 and r0 lists should be the same length.");
    }

    auto n = x0.size();

    std::vector< bounds< T > > bounding_box;
    bounding_box.reserve( n );

    for ( std::size_t i = 0 ; i != n; i++)
    {
        bounds< T > b;
        b.expand( low[ i ] );
        b.expand( high[ i ] );

        bounding_box.emplace_back( b );
    }

    auto x = x0;
    auto fmin = f( x );




    for ( auto k = 0; k != 10; k++)
    {
        for ( std::size_t j = 0 ; j != n ; j++)
        {
            std::cout << "[ " << bounding_box[j].lower << " " << bounding_box[j].upper << " ]" << std::endl;
        }
        std::cout << "New x: " << x << std::endl;

        std::vector<T> func_vals;
        //func_vals.push_back( fmin );

        std::vector<std::vector<double>> xs;
        //xs.push_back( x );

        multi_index_t mi( std::vector<std::size_t>( n, 2) );

        std::vector<bounds<T>> new_bounds( n );
        std::vector<T> newx(n);

        std::size_t count = 0;
        for ( ; mi < mi.sizes(); mi++ )
        {
            auto l = mi.location();

            std::vector<double> xcorner;

            for ( std::size_t i = 0 ; i != l.size(); i++ )
            {
                if ( l[i] == 0 )
                {
                    xcorner.push_back( bounding_box[i].lower );
                }
                else
                {
                    xcorner.push_back( bounding_box[i].upper );
                }
            }


            for ( auto i = 0; i !=50; i++)
            {
                auto fcorner = f( xcorner ) ;

                auto test = 0.5*(xcorner - x);
                auto ftest = f( test );
                std::cout << fcorner << ", " << ftest << std::endl;

                if ( f( test ) > f( xcorner ) )
                {
                    test = 1.5*(xcorner - x );

                }
                xcorner = test;
            }

            for ( std::size_t i = 0; i !=n; i++ )
            {
                new_bounds[i].expand( xcorner[i] );
            }

            newx = newx + xcorner;

            count++;

            std::cout << std::endl;

            //            xs.push_back( x );
            //            func_vals.push_back( f(xcorner) );

            //            xs.push_back( mid );
            //            func_vals.push_back( f(mid) );

            //                   std::cout << "xcorner: " << xcorner << " " << *func_vals.rbegin() << std::endl;
            //                   std::cout << "mid: " << mid << " " << *func_vals.rbegin() << std::endl;
        }

        //  auto idx = detail::sort_indexes( func_vals );

        //   std::vector<bounds<T>> new_bounds( n );


        newx = (1.0/count) * newx;
        //        for ( std::size_t i = 0 ; i !=idx.size() / 2 ; i ++)
        //        {
        //            for ( std::size_t j = 0 ; j != n ; j++)
        //            {
        //                new_bounds[j].expand( xs[idx[i]][j] );
        //            }
        //        }



        //        for ( std::size_t i = 0; i !=idx.size() / 2; i++ )
        //        {
        //            std::cout << xs[idx[i]] << "aa" << std::endl;
        //            newx = newx + (1.0/(idx.size() / 2)) * xs[idx[i]];
        //        }

        auto fnewx = f(newx);

        if ( fnewx < fmin )
        {
            x=newx;
            fmin =fnewx;
        }



        bounding_box = new_bounds;
    }
}
}




