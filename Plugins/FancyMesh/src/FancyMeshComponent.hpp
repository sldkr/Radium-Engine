#ifndef FANCYMESHPLUGIN_FANCYMESHCOMPONENT_HPP
#define FANCYMESHPLUGIN_FANCYMESHCOMPONENT_HPP

#include <FancyMeshPluginMacros.hpp>

#include <Core/Geometry/MeshTypes.hpp>
#include <Core/Geometry/TriangleMesh.hpp>

#include <Core/Asset/GeometryData.hpp>

#include <Engine/Component/Component.hpp>

namespace Ra {
namespace Engine {
class RenderTechnique;
class Mesh;
} // namespace Engine
} // namespace Ra

namespace FancyMeshPlugin {
/*!
 * \brief The FancyMeshComponent class
 *
 * Exports access to the mesh geometry:
 *  - TriangleMesh: get, rw (set vertices, normals and triangles dirty)
 *  - Vertices: rw (if deformable)
 *  - normals: rw (if deformable)
 *  - triangles: rw (if deformable)
 */
class FM_PLUGIN_API FancyMeshComponent : public Ra::Engine::Component {
  public:
    using DuplicateTable = Ra::Core::Asset::GeometryData::DuplicateTable;

    FancyMeshComponent( const std::string& name, bool deformable, Ra::Engine::Entity* entity );
    virtual ~FancyMeshComponent();

    void initialize() override;

    void addMeshRenderObject( const Ra::Core::Geometry::TriangleMesh& mesh, const std::string& name );
    void handleMeshLoading( const Ra::Core::Asset::GeometryData* data );

    /// Returns the index of the associated RO (the display mesh)
    Ra::Core::Container::Index getRenderObjectIndex() const;

    /// Returns the current display geometry.
    const Ra::Core::Geometry::TriangleMesh& getMesh() const;

  public:
    // Component communication management
    void setupIO( const std::string& id );
    void setContentName( const std::string name );
    void setDeformable( const bool b );

  private:
    const Ra::Engine::Mesh& getDisplayMesh() const;
    Ra::Engine::Mesh& getDisplayMesh();

    // Fancy mesh accepts to give its mesh and (if deformable) to update it
    const Ra::Core::Geometry::TriangleMesh* getMeshOutput() const;
    const DuplicateTable* getDuplicateTableOutput() const;
    Ra::Core::Geometry::TriangleMesh* getMeshRw();
    void setMeshInput( const Ra::Core::Geometry::TriangleMesh* mesh );
    Ra::Core::Container::Vector3Array* getVerticesRw();
    Ra::Core::Container::Vector3Array* getNormalsRw();
    Ra::Core::Container::VectorArray<Ra::Core::Geometry::Triangle>* getTrianglesRw();

    const Ra::Core::Container::Index* roIndexRead() const;

  private:
    // The duplicate table for vertices, according to vertices position and normals in the mesh
    // data. Let M be a mesh with a total of n vertices, of which m are duplicates of others (same
    // position but different attributes). Then, the duplicate table for M has n entries such that
    // m_duplicateTable[i] is the index for the duplicated attributes of vertice i in M data
    // (position, one-ring normal, ...) while i is the index for its non duplicated attributes
    // (texture coordinates, face normal, tangent vector, ...). Note: if duplicates have NOT been
    // loaded, then m_duplicateTable[i] == i.
    DuplicateTable m_duplicateTable;

    Ra::Core::Container::Index m_meshIndex;
    Ra::Core::Container::Index m_aabbIndex;
    std::string m_contentName;
    bool m_deformable;
};

} // namespace FancyMeshPlugin

#endif // FANCYMESHPLUGIN_FANCYMESHCOMPONENT_HPP
