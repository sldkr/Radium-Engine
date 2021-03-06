#ifndef AREA_DEFINITION
#define AREA_DEFINITION

#include <Core/Container/VectorArray.hpp>
#include <Core/Math/LinearAlgebra.hpp>
#include <Core/Geometry/MeshTypes.hpp>

namespace Ra {
namespace Core {
namespace Geometry {

/////////////////////
/// GLOBAL MATRIX ///
/////////////////////

// Defining the AreaMatrix as the diagonal matrix such that:
//    A( i, j ) = area(Pi) , if i == j
//    A( i, j ) = 0        , if i != j
// where Pi is the i-th point of the mesh.
using AreaMatrix = Math::Diagonal;

/*
 * Return the AreaMatrix for the given set of points and triangles.
 * The values correspond to the one-ring area of each point in p.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
AreaMatrix RA_CORE_API oneRingArea( const Container::VectorArray<Math::Vector3>& p, const Container::VectorArray<Triangle>& T );

/*
 * Return the AreaMatrix for the given set of points and triangles.
 * The values correspond to the one-ring area of each point in p.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
void RA_CORE_API oneRingArea( const Container::VectorArray<Math::Vector3>& p, const Container::VectorArray<Triangle>& T,
                              AreaMatrix& A );

/*
 * Return the AreaMatrix for the given set of points and triangles.
 * The values correspond to the barycentric area of each point in p.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
AreaMatrix RA_CORE_API barycentricArea( const Container::VectorArray<Math::Vector3>& p,
                                        const Container::VectorArray<Triangle>& T );

/*
 * Return the AreaMatrix for the given set of points and triangles.
 * The values correspond to the barycentric area of each point in p.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
void RA_CORE_API barycentricArea( const Container::VectorArray<Math::Vector3>& p, const Container::VectorArray<Triangle>& T,
                                  AreaMatrix& A );

/*
 * Return the AreaMatrix for the given set of points and triangles.
 * The values correspond to the Voronoi area of each point in p.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
AreaMatrix RA_CORE_API voronoiArea( const Container::VectorArray<Math::Vector3>& p, const Container::VectorArray<Triangle>& T );

/*
 * Return the AreaMatrix for the given set of points and triangles.
 * The values correspond to the mixed area of each point in p.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
AreaMatrix RA_CORE_API mixedArea( const Container::VectorArray<Math::Vector3>& p, const Container::VectorArray<Triangle>& T );

////////////////
/// ONE RING ///
////////////////

/*
 * Return the area for the given point v and its one-ring.
 * The values correspond to the one-ring area of v.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
Scalar RA_CORE_API oneRingArea( const Math::Vector3& v, const Container::VectorArray<Math::Vector3>& p );

/*
 * Return the area for the given point v and its one-ring.
 * The values correspond to the barycentric area of v.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
Scalar RA_CORE_API barycentricArea( const Math::Vector3& v, const Container::VectorArray<Math::Vector3>& p );

/*
 * Return the area for the given point v and its one-ring.
 * The values correspond to the Voronoi area of v.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
Scalar voronoiArea( const Math::Vector3& v, const Container::VectorArray<Math::Vector3>& p );

/*
 * Return the area for the given point v and its one-ring.
 * The values correspond to the mixed area of v.
 *
 * The definition was taken from:
 * "Discrete Differential-Geometry Operators for Triangulated 2-Manifolds"
 * [ Mark Meyer, Mathieu Desbrun, Peter Schoder, Alan H. Barr ]
 * Visualization and Mathematics III 2003
 */
Scalar mixedArea( const Math::Vector3& v, const Container::VectorArray<Math::Vector3>& p );

} // namespace Geometry
} // namespace Core
} // namespace Ra

#endif // AREA_DEFINITION
