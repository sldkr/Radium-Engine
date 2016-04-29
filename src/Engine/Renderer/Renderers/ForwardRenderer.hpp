#ifndef RADIUMENGINE_FORWARDRENDERER_HPP
#define RADIUMENGINE_FORWARDRENDERER_HPP

#include <Engine/RadiumEngine.hpp>
#include <Engine/Renderer/Renderer.hpp>

#include <Engine/Renderer/Passes/Passes.hpp> // include every Pass*.hpp, TODO(Hugo) remove it after debug
#include <Core/GraphStructures/MultiGraph.hpp>

namespace Ra
{
    namespace Engine
    {
        class RA_ENGINE_API ForwardRenderer : public Renderer
        {
        public:
            ForwardRenderer( uint width, uint height );
            virtual ~ForwardRenderer();

            virtual std::string getRendererName() const override { return "Forward Renderer"; }

        protected:

            virtual void initializeInternal() override;
            virtual void resizeInternal() override;

            virtual void updateStepInternal( const RenderData& renderData ) override;
            virtual void renderInternal( const RenderData& renderData ) override;
            virtual void postProcessInternal( const RenderData& renderData ) override;

        private:
            void initShaders();
            void initBuffers();
            void initPasses();
            void initGraph();

        private:
            enum RendererTextures
            {
                TEX_DEPTH = 0,
                TEX_NORMAL,
                TEX_LIT,
                TEX_LUMINANCE,
                TEX_TONEMAPPED,
                TEX_TONEMAP_PING,
                TEX_TONEMAP_PONG,
                TEX_BLOOM_PING,
                TEX_BLOOM_PONG,
                TEX_COUNT
            };

            // Default renderer logic here, no need to be accessed by overriding renderers.
            std::unique_ptr<FBO> m_fbo;
            std::unique_ptr<FBO> m_postprocessFbo;
            std::unique_ptr<FBO> m_pingPongFbo;
            std::unique_ptr<FBO> m_bloomFbo;

            uint m_pingPongSize;

            std::array<std::unique_ptr<Texture>, TEX_COUNT> m_textures;

            std::map<std::string, Pass*>       m_passmap;
            std::vector<std::unique_ptr<Pass>> m_passes;

            Core::MultiGraph<Pass> m_passgraph;
        };

    } // namespace Engine
} // namespace Ra

#endif // RADIUMENGINE_FORWARDRENDERER_HPP
