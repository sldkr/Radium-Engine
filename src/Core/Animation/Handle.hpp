#ifndef RADIUMENGINE_HANDLE_INCLUDE
#define RADIUMENGINE_HANDLE_INCLUDE

#include <Core/Animation/Pose.hpp>
#include <Core/Math/LinearAlgebra.hpp>
#include <Core/RaCore.hpp>

namespace Ra {
namespace Core {
namespace Animation {

/**
 * Type for transform names
 */
using Label = std::string;

/**
 * The Handle class describes the base handle structure used for deformation.
 */
class RA_CORE_API Handle {
  public:
    /**
     * The SpaceType specifies the space the transform lives in.
     */
    enum class SpaceType { LOCAL, MODEL };

    Handle();
    Handle( const uint n );
    Handle( const Handle& handle ) = default;
    Handle& operator=( const Handle& handle ) = default;

    virtual ~Handle();

    /**
     * @return the number of transformations
     */
    inline virtual uint size() const;

    /**
     * Clear the transformtions data
     */
    virtual void clear();

    /**
     * @return the current pose in the \p MODE space.
     */
    virtual const Pose& getPose( const SpaceType MODE ) const;

    /**
     * Set the current pose.
     * @param pose the new pose
     * @param MODE the space the psoe lives in
     */
    virtual void setPose( const Pose& pose, const SpaceType MODE );

    /**
     * @return the \p i-th transformation in \p MODE space.
     */
    virtual const Math::Transform& getTransform( const uint i, const SpaceType MODE ) const;

    /**
     * Set the i-th transform to \p T, which is given in the MODE space.
     */
    virtual void setTransform( const uint i, const Math::Transform& T, const SpaceType MODE );

    /**
     * @return the Handle's name
     */
    inline Label getName() const;

    /**
     * Set the Handle's name
     */
    inline void setName( const Label& name );

    /**
     * @return the name of the \p i-th transform.
     */
    inline Label getLabel( const uint i ) const;

    /**
     * Set the name of the \p i-th transform.
     */
    inline void setLabel( const uint i, const Label& text );

    /**
     * The current pose of the Handle
     */
    LocalPose m_pose;

  protected:
    /**
     * The Handle's name
     */
    Label m_name;

    /**
     * The names of the transforms.
     */
    std::vector<Label> m_label;
};

} // namespace Animation
} // Namespace Core
} // Namespace Ra

#include <Core/Animation/Handle.inl>

#endif // RADIUMENGINE_HANDLE_INCLUDE
