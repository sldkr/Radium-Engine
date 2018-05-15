#include <Core/Animation/SkeletonUtils.hpp>

#include <Core/Animation/Skeleton.hpp>
#include <iostream>

namespace Ra {
namespace Core {
namespace Animation {
/// Returns the start and end point of a bone in model space.
void getBonePoints( const Skeleton& skeleton, int boneIdx, Math::Vector3& startOut,
                    Math::Vector3& endOut ) {
    // Check bone index is valid
    CORE_ASSERT( boneIdx >= 0 && uint( boneIdx ) < skeleton.m_graph.size(), "invalid bone index" );

    startOut = skeleton.getTransform( boneIdx, Handle::SpaceType::MODEL ).translation();
    auto children = skeleton.m_graph.m_child.at( boneIdx );

    // A leaf bone has length 0
    if ( children.size() == 0 )
    {
        endOut = startOut;
    } else
    {
        // End point is the average of chidren start points.
        endOut = Math::Vector3::Zero();
        for ( auto child : children )
        {
            endOut += skeleton.getTransform( child, Handle::SpaceType::MODEL ).translation();
        }

        endOut *= ( 1.f / children.size() );
    }
}

/// Gives out the nearest point on a given bone.
inline Math::Vector3 projectOnBone( const Skeleton& skeleton, int boneIdx,
                              const Math::Vector3& pos ) {
    Math::Vector3 start, end;
    getBonePoints( skeleton, boneIdx, start, end );

    auto op = pos - start;
    auto dir = ( end - start );
    // Square length of bone
    const Scalar length_sq = dir.squaredNorm();
    CORE_ASSERT( length_sq != 0.f, "bone has lenght 0, cannot project." );

    // Project on the line segment
    const Scalar t = Math::clamp( op.dot( dir ) / length_sq, (Scalar)0.0, (Scalar)1.0 );
    return start + ( t * dir );
}

inline void to_string( const Skeleton& skeleton ) {
    std::cout << "Printing Skeleton Hierarchy..." << std::endl;

    for ( uint i = 0; i < skeleton.size(); ++i )
    {
        const uint id = i;
        const std::string name = skeleton.getLabel( i );
        const std::string type =
            skeleton.m_graph.isRoot( i )
                ? "ROOT"
                : skeleton.m_graph.isJoint( i )
                      ? "JOINT"
                      : skeleton.m_graph.isBranch( i )
                            ? "BRANCH"
                            : skeleton.m_graph.isLeaf( i ) ? "LEAF" : "UNKNOWN";
        const int pid = skeleton.m_graph.m_parent.at( i );
        const std::string pname =
            ( pid == -1 ) ? "" : ( "(" + std::to_string( pid ) + ") " + skeleton.getLabel( pid ) );

        std::cout << "Bone " << id << "\t: " << name << std::endl;
        std::cout << "Type\t: " << type << std::endl;
        std::cout << "Parent\t: " << pname << std::endl;
        std::cout << "Children#\t: " << skeleton.m_graph.m_child.at( i ).size() << std::endl;
        std::cout << "Children\t: ";
        for ( uint j = 0; j < skeleton.m_graph.m_child.at( i ).size(); ++j )
        {
            const uint cid = skeleton.m_graph.m_child.at( i ).at( j );
            const std::string cname = skeleton.getLabel( cid );
            std::cout << "(" << cid << ") " << cname << " | ";
        }
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
    }

    std::cout << "End of Skeleton Hierarchy" << std::endl;
}

} // namespace Animation
} // namespace Core
} // namespace Ra
