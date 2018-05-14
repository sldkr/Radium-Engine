#ifndef RADIUMENGINE_MESH_CONVERT_HPP
#define RADIUMENGINE_MESH_CONVERT_HPP

#include <Core/RaCore.hpp>

namespace Ra {
namespace Core {

// Forward declaration
namespace Container {
class Index;
} // namespace Container

//namespace Geometry {

struct TriangleMesh;
class Dcel;

struct Twin {
    Twin();
    Twin( const uint i, const uint j );

    bool operator==( const Twin& twin ) const;
    bool operator<( const Twin& twin ) const;

    uint m_id[2];
};

RA_CORE_API void convert( const TriangleMesh& mesh, Dcel& dcel );
RA_CORE_API void convert( const Dcel& dcel, TriangleMesh& mesh );

//} //namespace Geometry
} // namespace Core
} // namespace Ra

#endif // RADIUMENGINE_MESH_CONVERT_HPP
