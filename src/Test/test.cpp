#include "vulkan_core_headers.h"
#include "Bindable_heads.h"
#include "test.h"
#include <iostream>
#include "Drawable.h"
#include <glm/glm.hpp>
#include <array>
#include "Model.h"
#include "DynamicConstant.h"
#include "LayoutCodex.h"
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

class TestTriangle : public Draw::Model
{
public:
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
    };

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }

    size_t getCount()
    {
        return vertices.size();
    }

    size_t getSize()
    {
        return sizeof(vertices.size());
    }

    void* getData()
    {
        return (void*)vertices.data();
    }
};

class Test
{
	using nameAndBindable = std::pair<Draw::BindType, std::shared_ptr<Bind::Bindable> >;
	
public:

    Test()
    {
        vulkan_ptr = make_shared<Graphics::Vulkan>();
        image_ptr = make_shared<Graphics::Image>(vulkan_ptr);
        triangle_ptr = make_shared<TestTriangle>();
        cmdPool_ptr = make_shared<Graphics::CommandPool>(vulkan_ptr);
        sync_ptr = make_shared<Graphics::Synchronization>(vulkan_ptr);
        cmdBuf_ptr = make_shared<Graphics::CommandBuffer>(vulkan_ptr, cmdPool_ptr);
        cmdQueue_ptr = make_shared<Graphics::CommandQueue>(vulkan_ptr, cmdBuf_ptr, sync_ptr);
    }

	void test() {
		
		auto pipelineLayout_ptr = std::make_shared<Bind::PipelineLayout>();
		pipelineLayout_ptr->AddLayout(Bind::DESCRIPTOR_TYPE::UNFIORM, 0);
		pipelineLayout_ptr->AddLayout(Bind::DESCRIPTOR_TYPE::UNFIORM, 1);
        
		auto vertexShader_ptr = std::make_shared<Bind::VertexShader>(vulkan_ptr, "../src/shaders/triangle.vert.glsl", "../src/shaders", "main");
		auto pixelShader_ptr = std::make_shared<Bind::PixelShader>(vulkan_ptr, "../src/shaders/triangle.frag.glsl", "../src/shaders", "main");
		auto vertexBuffer_ptr = std::make_shared<Bind::VertexBuffer>(vulkan_ptr, triangle_ptr);
		
		auto pipeline_ptr = std::make_shared<Graphics::Pipeline>(vulkan_ptr);
		auto renderpass_ptr = std::make_shared<Graphics::RenderPass>(vulkan_ptr, image_ptr);

		auto drawable = std::make_shared<Draw::Drawable>(vulkan_ptr);

		drawable->Register<Draw::GraphicsType::Pipeline>(pipeline_ptr);
		drawable->Register<Draw::GraphicsType::RenderPass>(renderpass_ptr);
        drawable->Register<Draw::GraphicsType::CommandBuffer>(cmdBuf_ptr);
        drawable->Register<Draw::GraphicsType::CommandQueue>(cmdQueue_ptr);

		drawable->Register<Draw::BindType::PipelineLayout>(pipelineLayout_ptr);
		drawable->Register<Draw::BindType::VertexShader>(vertexShader_ptr);
		drawable->Register<Draw::BindType::PixelShader>(pixelShader_ptr);
		drawable->Register<Draw::BindType::VertexBuffer>(vertexBuffer_ptr);

		drawable->CompilePipeline();

        drawable->BuildCommandBuffer();

        drawable->Submit();
	}

    void TestDynamicConstant()
    {
        Dcb::RawLayout s;
        s.Add<Dcb::Struct>("butts"s);
        //s["butts"s].Add<Dcb::Float3>("pubes"s);
        s["butts"s].Add<Dcb::Float>("dank"s);
        //s.Add<Dcb::Float>("woot"s);
        //s.Add<Dcb::Array>("arr"s);
        //s["arr"s].Set<Dcb::Struct>(4);
        //s["arr"s].T().Add<Dcb::Float3>("twerk"s);
        //s["arr"s].T().Add<Dcb::Array>("werk"s);
        //s["arr"s].T()["werk"s].Set<Dcb::Float>(6);
        //s["arr"s].T().Add<Dcb::Array>("meta"s);
        //s["arr"s].T()["meta"s].Set<Dcb::Array>(6);
        //s["arr"s].T()["meta"s].T().Set<Dcb::Matrix>(4);
        //s["arr"s].T().Add<Dcb::Bool>("booler");

        
        cout << s.GetSignature() << endl;
        auto b = Dcb::Buffer(std::move(s));
        cout << b.GetSizeInBytes() << endl;
        cout << b.GetData() << endl;
    }

private:

	shared_ptr<Graphics::Vulkan> vulkan_ptr;
	shared_ptr<Graphics::Image> image_ptr;
    shared_ptr<TestTriangle> triangle_ptr;
    shared_ptr<Graphics::CommandPool> cmdPool_ptr;
    shared_ptr<Graphics::Synchronization> sync_ptr;
    shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
    shared_ptr<Graphics::CommandQueue> cmdQueue_ptr;
};


int main()
{
	Test test;
    test.test();
	cout << "hello" << endl;
	return 0;
}