#ifndef RADIUMENGINE_TRANSFORM_EDITOR_HPP_
#define RADIUMENGINE_TRANSFORM_EDITOR_HPP_

#include <Core/Math/LinearAlgebra.hpp>
#include <Engine/ItemModel/ItemEntry.hpp>
#include <GuiBase/RaGuiBase.hpp>
namespace Ra {
namespace GuiBase {
class RA_GUIBASE_API TransformEditor {
  public:
    RA_CORE_ALIGNED_NEW

    TransformEditor() : m_transform( Core::Math::Transform::Identity() ) {}
    virtual ~TransformEditor();

    /// Change the current editable object,
    virtual void setEditable( const Engine::ItemEntry& entry );

    /// Retrieve the transform from the editable and update the editor.
    virtual void updateValues() = 0;

  protected:
    // Helper to get the transform property from the editable.
    void getTransform();

    /// Helper to set the transform to the editable.
    void setTransform( const Ra::Core::Math::Transform& tr );

    bool canEdit() const;

    Core::Math::Transform getWorldTransform() const;

  protected:
    Core::Math::Transform m_transform;     //! The transform being edited.
    Engine::ItemEntry m_currentEdit; //! The current item being edited.
};
} // namespace GuiBase
} // namespace Ra
#endif // RADIUMENGINE_TRANSFORM_EDITOR_HPP_
