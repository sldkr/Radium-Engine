#include <Engine/RadiumEngine.hpp>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>
#include <streambuf>
#include <string>
#include <thread>

#include <Core/Asset/FileData.hpp>
#include <Core/Asset/FileLoaderInterface.hpp>

#include <Engine/FrameInfo.hpp>
#include <Engine/System/System.hpp>
#include <Engine/Managers/EntityManager/EntityManager.hpp>
#include <Engine/Managers/SignalManager/SignalManager.hpp>
#include <Engine/Managers/ComponentMessenger/ComponentMessenger.hpp>
#include <Engine/Renderer/RenderObject/RenderObject.hpp>
#include <Engine/Renderer/RenderObject/RenderObjectManager.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgramManager.hpp>
#include <Engine/Renderer/Material/BlinnPhongMaterial.hpp>


namespace Ra {
namespace Engine {

RadiumEngine::RadiumEngine() {}

RadiumEngine::~RadiumEngine() {}

void RadiumEngine::initialize() {
    LOG( Core::Utils::logINFO ) << "*** Radium Engine ***";
    m_signalManager.reset( new SignalManager );
    m_entityManager.reset( new EntityManager );
    m_renderObjectManager.reset( new RenderObjectManager );
    m_loadedFile.reset();
    ComponentMessenger::createInstance();
    // Engine support some built-in materials. Register here
    BlinnPhongMaterial::registerMaterial();
    m_loadingState = false;
}

void RadiumEngine::cleanup() {
    BlinnPhongMaterial::unregisterMaterial();

    m_signalManager->setOn( false );
    m_entityManager.reset();
    m_renderObjectManager.reset();
    m_loadedFile.reset();

    for ( auto& system : m_systems )
    {
        system.second.reset();
    }

    ComponentMessenger::destroyInstance();
    ShaderProgramManager::destroyInstance();
    m_loadingState = false;
}

void RadiumEngine::endFrameSync() {
    m_entityManager->swapBuffers();
    m_signalManager->fireFrameEnded();
}

void RadiumEngine::getTasks( Ra::Core::Utils::TaskQueue* taskQueue, Scalar dt ) {
    static uint frameCounter = 0;
    FrameInfo frameInfo;
    frameInfo.m_dt = dt;
    frameInfo.m_numFrame = frameCounter++;
    for ( auto& syst : m_systems )
    {
        syst.second->generateTasks( taskQueue, frameInfo );
    }
}

void RadiumEngine::registerSystem( const std::string& name, System* system ) {
    CORE_ASSERT( m_systems.find( name ) == m_systems.end(), "Same system added multiple times." );

    m_systems[name] = std::shared_ptr<System>( system );
    LOG( Core::Utils::logINFO ) << "Loaded : " << name;
}

System* RadiumEngine::getSystem( const std::string& system ) const {
    System* sys = nullptr;
    auto it = m_systems.find( system );

    if ( it != m_systems.end() )
    {
        sys = it->second.get();
    }

    return sys;
}

Mesh* RadiumEngine::getMesh( const std::string& entityName, const std::string& componentName,
                             const std::string& roName ) const {

    // 1) Get entity
    if ( m_entityManager->entityExists( entityName ) )
    {
        Ra::Engine::Entity* e = m_entityManager->getEntity( entityName );

        // 2) Get component
        const Ra::Engine::Component* c = e->getComponent( componentName );

        if ( c != nullptr && !c->m_renderObjects.empty() )
        {
            // 3) Get RO
            if ( roName.empty() )
            {
                return m_renderObjectManager->getRenderObject( c->m_renderObjects.front() )
                    ->getMesh()
                    .get();
            } else
            {
                for ( const auto& idx : c->m_renderObjects )
                {
                    const auto& ro = m_renderObjectManager->getRenderObject( idx );
                    if ( ro->getName() == roName )
                    {
                        return ro->getMesh().get();
                    }
                }
            }
        }
    }
    return nullptr;
}

bool RadiumEngine::loadFile( const std::string& filename ) {
    std::string extension = Core::Utils::getFileExt( filename );

    for ( auto& l : m_fileLoaders )
    {
        if ( l->handleFileExtension( extension ) )
        {
            Core::Asset::FileData* data = l->loadFile( filename );
            if ( data != nullptr )
            {
                m_loadedFile.reset( data );
                break;
            }
        }
    }

    if ( m_loadedFile == nullptr )
    {
        LOG( Core::Utils::logERROR ) << "There is no loader to handle \"" << extension
                        << "\" extension ! File can't be loaded.";

        return false;
    }

    std::string entityName = Core::Utils::getBaseName( filename, false );

    Entity* entity = m_entityManager->createEntity( entityName );

    for ( auto& system : m_systems )
    {
        system.second->handleAssetLoading( entity, m_loadedFile.get() );
    }

    if ( entity->getComponents().size() > 0 )
    {
        for ( auto& comp : entity->getComponents() )
        {
            comp->initialize();
        }
    } else
    {
        LOG( Core::Utils::logWARNING ) << "File \"" << filename << "\" has no usable data. Deleting entity...";
        m_entityManager->removeEntity( entity );
    }
    m_loadingState = true;
    return true;
}

void RadiumEngine::releaseFile() {
    m_loadedFile.reset( nullptr );
    m_loadingState = false;
}

RenderObjectManager* RadiumEngine::getRenderObjectManager() const {
    return m_renderObjectManager.get();
}

EntityManager* RadiumEngine::getEntityManager() const {
    return m_entityManager.get();
}

SignalManager* RadiumEngine::getSignalManager() const {
    return m_signalManager.get();
}

void RadiumEngine::registerFileLoader( std::shared_ptr<Core::Asset::FileLoaderInterface> fileLoader ) {
    m_fileLoaders.push_back( fileLoader );
}

const std::vector<std::shared_ptr<Core::Asset::FileLoaderInterface>>&
RadiumEngine::getFileLoaders() const {
    return m_fileLoaders;
}

RA_SINGLETON_IMPLEMENTATION( RadiumEngine );

const Core::Asset::FileData& RadiumEngine::getFileData() const {
    CORE_ASSERT(m_loadingState, "Access to file content is only available at loading time.");
    return *( m_loadedFile.get() );
}

} // namespace Engine
} // namespace Ra
