#include "vulkan_core_headers.h"
#include "Bindable_heads.h"
#include "test.h"
#include <iostream>
#include "Drawable.h"
#include <glm/glm.hpp>
#include <array>

using namespace std;

class TestCube
{
public:
    struct cube_t {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(cube_t);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(cube_t, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(cube_t, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(cube_t, uv);

        return attributeDescriptions;
    }

    const std::vector<cube_t> cube_vertices = {
        // positions          // normals           // texture coords
        {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f} },
        { {0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f},  {1.0f,  0.0f} },
        {{0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f }},
        {{0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f} },
        { {-0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f }, { 0.0f,  1.0f }},
        { {-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f }, { 0.0f,  0.0f }},

        { {-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f,  1.0f},  {0.0f,  0.0f} },
        {{0.5f, -0.5f,  0.5f},  {0.0f,  0.0f,  1.0f},  {1.0f,  0.0f}},
        {{0.5f,  0.5f,  0.5f},  {0.0f,  0.0f,  1.0f},  {1.0f,  1.0f}},
        {{0.5f,  0.5f,  0.5f},  {0.0f,  0.0f,  1.0f},  {1.0f,  1.0f}},
        {{-0.5f,  0.5f,  0.5f},  {0.0f,  0.0f,  1.0f}, { 0.0f,  1.0f}},
        {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f,  1.0f},  {0.0f,  0.0f}},

        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f},  {1.0f,  1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, { 0.0f,  1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},

        {{0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, { 1.0f,  0.0f}},
        {{0.5f,  0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, { 1.0f,  1.0f}},
        {{0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, { 0.0f,  1.0f}},
        {{0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, { 0.0f,  1.0f}},
        {{0.5f, -0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
        {{0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, { 1.0f,  0.0f}},

        {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, { 0.0f,  1.0f}},
        {{0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, { 1.0f,  1.0f}},
        {{0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, { 1.0f,  0.0f}},
        {{0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, { 1.0f,  0.0f}},
        {{-0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, { 0.0f,  0.0f}},
        {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, { 0.0f,  1.0f}},

        {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}, { 0.0f,  1.0f}},
        {{0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f},  {1.0f,  1.0f}},
        {{0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
        {{0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}, { 0.0f,  0.0f}},
        {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f},{  0.0f,  1.0f}}
    };
};

class Test
{
	using nameAndBindable = std::pair<Draw::BindType, std::shared_ptr<Bind::Bindable> >;
	
public:
	void init()
	{
		vulkan_ptr = make_shared<Graphics::Vulkan>();
		image_ptr = make_shared<Graphics::Image>(vulkan_ptr);
	}
	void test() {
		init();
		
		auto pipelineLayout_ptr = std::make_shared<Bind::PipelineLayout>();
		pipelineLayout_ptr->AddLayout(Bind::DESCRIPTOR_TYPE::UNFIORM, 0);
		pipelineLayout_ptr->AddLayout(Bind::DESCRIPTOR_TYPE::UNFIORM, 1);
        
		auto vertexShader_ptr = std::make_shared<Bind::VertexShader>(vulkan_ptr, "../src/shaders/cube.vert.glsl", "../src/shaders", "main");
		auto pixelShader_ptr = std::make_shared<Bind::PixelShader>(vulkan_ptr, "../src/shaders/cube.frag.glsl", "../src/shaders", "main");
		auto vertexBuffer_ptr = std::make_shared<Bind::VertexBuffer>(TestCube::getBindingDescription(), TestCube::getAttributeDescriptions());
		
		auto pipeline_ptr = std::make_shared<Graphics::Pipeline>(vulkan_ptr);
		auto renderpass_ptr = std::make_shared<Graphics::RenderPass>(vulkan_ptr, image_ptr);

		auto drawable = std::make_shared<Draw::Drawable>(vulkan_ptr);

		drawable->Register<Draw::GraphicsType::Pipeline>(pipeline_ptr);
		drawable->Register<Draw::GraphicsType::RenderPass>(renderpass_ptr);

		drawable->Register<Draw::BindType::PipelineLayout>(pipelineLayout_ptr);
		drawable->Register<Draw::BindType::VertexShader>(vertexShader_ptr);
		drawable->Register<Draw::BindType::PixelShader>(pixelShader_ptr);
		drawable->Register<Draw::BindType::VertexBuffer>(vertexBuffer_ptr);

		drawable->CompilePipeline();
	}

private:

	shared_ptr<Graphics::Vulkan> vulkan_ptr;
	shared_ptr<Graphics::Image> image_ptr;
	
};


int main()
{
	Test test;
	test.test();
	cout << "hello" << endl;
	return 0;
}