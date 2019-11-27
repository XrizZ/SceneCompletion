#ifndef EIGEN_UMFPACKSUPPORT_MODULE_H
#define EIGEN_UMFPACKSUPPORT_MODULE_H

#include "SparseExtra"

#include "../../Eigen/src/Core/util/DisableStupidWarnings.h"

extern "C" {
#include "umfpack/include/umfpack.h"
}

namespace Eigen {

/** \ingroup Unsupported_modules
  * \defgroup UmfPackSupport_Module UmfPack support module
  *
  *
  *
  *
  * \code
  * #include <Eigen/UmfPackSupport>
  * \endcode
  */

struct UmfPack {};

#include "src/SparseExtra/UmfPackSupport.h"

} // namespace Eigen

#include "../../Eigen/src/Core/util/ReenableStupidWarnings.h"

#endif // EIGEN_UMFPACKSUPPORT_MODULE_H
