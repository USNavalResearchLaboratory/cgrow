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

#include "test_series.hpp"

#include <cgrow.hpp>

#include <QMutex>
#include <QObject>

// Todo: move hs_parameters_t inside the fitting worker
using real_t          = long double;
using hs_parameters_t = crack_growth::Hartman_Schijve::parameters< real_t >;

struct Hartman_Schijve_autoRange
{
  bool DeltaK_thr_low  = true;
  bool DeltaK_thr_high = true;
  bool A_low           = true;
  bool A_high          = true;
};

namespace Hartman_Schijve
{
inline constexpr real_t min_DeltaK_thr = 0.000001;

real_t calc_max_DeltaK_thr( const test_data_t& test );
real_t calc_max_DeltaK_thr( const std::vector< test_data_t >& tests );

real_t calc_min_A( const test_data_t& test );
real_t calc_min_A( const std::vector< test_data_t >& tests );

real_t calc_max_A( const real_t& Amin );
real_t calc_max_A( const real_t& Amin );

inline constexpr real_t minD = 1e-12;
inline constexpr real_t maxD = 1e-4;

inline constexpr real_t minp = 0.01;
inline constexpr real_t maxp = 10.0;

}

Q_DECLARE_METATYPE( hs_parameters_t )

class fittingWorker : public QObject
{
  Q_OBJECT
public:
  const bool& stop_requested( ) const
  {
    return stop_requested_;
  };

  const bool& running( ) const
  {
    return running_;
  };

public slots:
  void run( hs_parameters_t                   params_low,
            hs_parameters_t                   params_high,
            int                               subdivisions,
            double                            amortization,
            bool                              use_geometric,
            const std::vector< test_data_t >& test_set,
            Hartman_Schijve_autoRange         autoRange,
            bool                              compute_individually = false );

  void stop( );

signals:
  void resultReady( const QString& result );

  void updatedResults( hs_parameters_t params,
                       hs_parameters_t params_lower,
                       hs_parameters_t params_upper );

  void individuallyUpdatedResults( hs_parameters_t params,
                                   hs_parameters_t params_lower,
                                   hs_parameters_t params_upper,
                                   int             dataSetId );

  void progressReport( int, int );

  void finished();

private:
  bool stop_requested_ = false;
  bool running_        = false; // todo: atomic bools

  QMutex calback_mutex;
};
