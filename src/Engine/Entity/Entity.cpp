#include <Engine/Entity/Entity.hpp>

#include <Core/Utils/StringUtils.hpp>

#include <Engine/Component/Component.hpp>
#include <Engine/Managers/SignalManager/SignalManager.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/System/System.hpp>

namespace Ra {
namespace Engine {

Entity::Entity( const std::string& name ) :
    Core::Container::IndexedObject(),
    m_transform( Core::Math::Transform::Identity() ),
    m_doubleBufferedTransform( Core::Math::Transform::Identity() ),
    m_name( name ),
    m_transformChanged( false ) {}

Entity::~Entity() {
    // Ensure components are deleted before the entity for consistent
    // ordering of signals.
    m_components.clear();
    RadiumEngine::getInstance()->getSignalManager()->fireEntityDestroyed( ItemEntry( this ) );
}

void Entity::addComponent( Engine::Component* component ) {
    CORE_ASSERT( getComponent( component->getName() ) == nullptr,
                 "Component \"" << component->getName()
                                << "\" has already been added to the entity." );

    m_components.emplace_back( std::unique_ptr<Component>( component ) );
    component->setEntity( this );

    RadiumEngine::getInstance()->getSignalManager()->fireComponentAdded(
        ItemEntry( this, component ) );
}

Component* Entity::getComponent( const std::string& name ) {
    const auto& pos = std::find_if( m_components.begin(), m_components.end(),
                                    [name]( const auto& c ) { return c->getName() == name; } );

    return pos != m_components.end() ? pos->get() : nullptr;
}

const Component* Entity::getComponent( const std::string& name ) const {
    const auto& pos = std::find_if( m_components.begin(), m_components.end(),
                                    [name]( const auto& c ) { return c->getName() == name; } );

    return pos != m_components.end() ? pos->get() : nullptr;
}
const std::vector<std::unique_ptr<Component>>& Entity::getComponents() const {
    return m_components;
}

void Entity::removeComponent( const std::string& name ) {
    const auto& pos = std::find_if( m_components.begin(), m_components.end(),
                                    [name]( const auto& c ) { return c->getName() == name; } );

    CORE_ASSERT( pos != m_components.end(), "Component not found in entity" );
    m_components.erase( pos );
}

void Entity::swapTransformBuffers() {
    if ( m_transformChanged )
    {
        m_transform = m_doubleBufferedTransform;
        m_transformChanged = false;
    }
}

EntityManager* Entity::getEntityMgr(){
    return RadiumEngine::getInstance()->getEntityManager();
}

void Entity::rayCastQuery( const Core::Math::Ray& r ) const {
    // put ray in local frame.
    Core::Math::Ray transformedRay = Ra::Core::Math::transformRay( r, m_transform.inverse() );
    for ( const auto& c : m_components )
    {
        c->rayCastQuery( transformedRay );
    }
}

} // namespace Engine

} // namespace Ra
