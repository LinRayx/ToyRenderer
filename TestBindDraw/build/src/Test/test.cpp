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
#include "RenderLoop.h"
#include "Camera.h"

using namespace std;

int WIDTH = 800;
int HEIGHT = 600;

class TestCube : public Draw::Model
{
public:

    TestCube(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::DescriptorPool> desc_pool_ptr, shared_ptr<Control::Camera> camera_ptr)
    {
        desc_ptr = make_shared<Graphics::DescriptorSetCore>(vulkan_ptr, desc_pool_ptr);
        this->vulkan_ptr = vulkan_ptr;
        this->desc_pool_ptr = desc_pool_ptr;
        this->camera_ptr = camera_ptr;
    }

    struct cube_t {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(cube_t);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
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

    const std::vector<cube_t> vertices = {
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

    void InitVar()
    {

        Dcb::RawLayout layout;

        layout.Add<Dcb::Float>("a1");
        layout.Add<Dcb::Float>("a2");
        layout.Add<Dcb::Float>("a3");
        layout.Add<Dcb::Float2>("a4");
        bufs["test1"] = make_shared<Dcb::Buffer>(std::move(layout));
        buffer_ptrs["test1"] = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, bufs["test1"]->GetSizeInBytes());

        desc_ptr->Add(Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX, buffer_ptrs["test1"]);

        Dcb::RawLayout layout2;
        layout2.Add<Dcb::Matrix>("model");
        layout2.Add<Dcb::Matrix>("view");
        layout2.Add<Dcb::Matrix>("proj");
        bufs["mvp"] = make_shared<Dcb::Buffer>(std::move(layout2));
        buffer_ptrs["mvp"] = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, bufs["mvp"]->GetSizeInBytes());

        desc_ptr->Add(Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX, buffer_ptrs["mvp"]);
    }

    void SetData()
    {
        (*bufs["test1"])["a1"] = 1.0f;
        (*bufs["test1"])["a3"] = 0.0f;
        (*bufs["test1"])["a2"] = 0.0f;
        (*bufs["test1"])["a4"] = glm::vec2(1., 2.);


    }

    // 通过 Model 继承
    void Compile()
    {
        desc_ptr->Compile();
    }

    size_t getCount()
    {
        return vertices.size();
    }

    size_t getSize()
    {
        return sizeof(vertices[0]) * vertices.size();
    }

    void* getData()
    {
        return (void*)vertices.data();
    }

    // 通过 Model 继承
    virtual void Update() override
    {
        (*bufs["mvp"])["model"] = this->getModelMatrix();
        (*bufs["mvp"])["view"] = camera_ptr->GetViewMatrix();
        (*bufs["mvp"])["proj"] = camera_ptr->GetViewMatrix();

        for (auto it : buffer_ptrs)
        {
            for (int i = 0; i < 2; ++i)
            {
                it.second->UpdateData(i, bufs[it.first]->GetSizeInBytes(), bufs[it.first]->GetData());
            }
        }
    }
};

//class TestTriangle : public Draw::Model
//{
//public:
//    TestTriangle(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::DescriptorPool> desc_pool_ptr)
//    {
//        desc_ptr = make_shared<Graphics::DescriptorSetCore>(vulkan_ptr, desc_pool_ptr);
//        this->vulkan_ptr = vulkan_ptr;
//        this->desc_pool_ptr = desc_pool_ptr;
//    }
//
//    struct Vertex {
//        glm::vec2 pos;
//        glm::vec3 color;
//    };
//
//    const std::vector<Vertex> vertices = {
//        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
//    };
//
//    VkVertexInputBindingDescription getBindingDescription() {
//        VkVertexInputBindingDescription bindingDescription{};
//        bindingDescription.binding = 0;
//        bindingDescription.stride = sizeof(Vertex);
//        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//        return bindingDescription;
//    }
//
//    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
//        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
//
//        attributeDescriptions[0].binding = 0;
//        attributeDescriptions[0].location = 0;
//        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
//        attributeDescriptions[0].offset = offsetof(Vertex, pos);
//
//        attributeDescriptions[1].binding = 0;
//        attributeDescriptions[1].location = 1;
//        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[1].offset = offsetof(Vertex, color);
//
//        return attributeDescriptions;
//    }
//
//    void InitVar()
//    {
//        layout.Add<Dcb::Float>("a1");
//        layout.Add<Dcb::Float>("a2");
//        layout.Add<Dcb::Float>("a3");
//        layout.Add<Dcb::Float2>("a4");
//    }
//
//    void SetData()
//    {
//        buf = make_shared<Dcb::Buffer>(std::move(layout));
//
//        buffer_ptr = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, buf->GetSizeInBytes());
//        desc_ptr->Add(Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX, buffer_ptr);
//
//        (*buf)["a1"] = 1.0f;
//        (*buf)["a3"] = 0.0f;
//        (*buf)["a2"] = 0.0f;
//        (*buf)["a4"] = glm::vec2(1., 2.);
//
//        buffer_ptr->UpdateData(0, buf->GetSizeInBytes(), buf->GetData());
//        buffer_ptr->UpdateData(1, buf->GetSizeInBytes(), buf->GetData());
//    }
//
//    shared_ptr<Graphics::Buffer> buffer_ptr;
//
//    void Compile()
//    {
//        desc_ptr->Compile();
//    }
//
//    size_t getCount()
//    {
//        return vertices.size();
//    }
//
//    size_t getSize()
//    {
//        return sizeof(vertices[0]) * vertices.size() ;
//    }
//
//    void* getData()
//    {
//        return (void*)vertices.data();
//    }
//
//    // 通过 Model 继承
//    virtual void Update() override
//    {
//    }
//};

class Test
{
	
public:

    Test()
    {
        vulkan_ptr = make_shared<Graphics::Vulkan>(WIDTH, HEIGHT);
        camera_ptr = make_shared<Control::Camera>(WIDTH, HEIGHT);

        image_ptr = make_shared<Graphics::Image>(vulkan_ptr);
        
        cmdPool_ptr = make_shared<Graphics::CommandPool>(vulkan_ptr);
        sync_ptr = make_shared<Graphics::Synchronization>(vulkan_ptr);
        cmdBuf_ptr = make_shared<Graphics::CommandBuffer>(vulkan_ptr, cmdPool_ptr);
        cmdQueue_ptr = make_shared<Graphics::CommandQueue>(vulkan_ptr, cmdBuf_ptr, sync_ptr);
        desc_pool_ptr = make_shared<Graphics::DescriptorPool>(vulkan_ptr);
        triangle_ptr = make_shared<TestCube>(vulkan_ptr, desc_pool_ptr, camera_ptr);
    }

    void test() {
        
        auto vertexShader_ptr = std::make_shared<Bind::VertexShader>(vulkan_ptr, "../src/shaders/desc_test.vert.glsl", "../src/shaders", "main");
        auto pixelShader_ptr = std::make_shared<Bind::PixelShader>(vulkan_ptr, "../src/shaders/desc_test.frag.glsl", "../src/shaders", "main");
        auto vertexBuffer_ptr = std::make_shared<Bind::VertexBuffer>(vulkan_ptr, triangle_ptr);

        auto pipeline_ptr = std::make_shared<Graphics::Pipeline>(vulkan_ptr);
        auto renderpass_ptr = std::make_shared<Graphics::RenderPass>(vulkan_ptr, image_ptr);
      
        triangle_ptr->InitVar();
        triangle_ptr->SetData();

        auto drawable = std::make_shared<Draw::Drawable>(vulkan_ptr);

        drawable->Register(triangle_ptr);
        drawable->Register<Draw::GraphicsType::Pipeline>(pipeline_ptr);
        drawable->Register<Draw::GraphicsType::RenderPass>(renderpass_ptr);
        drawable->Register<Draw::GraphicsType::CommandBuffer>(cmdBuf_ptr);
        drawable->Register<Draw::GraphicsType::CommandQueue>(cmdQueue_ptr);

        drawable->Register<Draw::BindType::VertexShader>(vertexShader_ptr);
        drawable->Register<Draw::BindType::PixelShader>(pixelShader_ptr);
        drawable->Register<Draw::BindType::VertexBuffer>(vertexBuffer_ptr);

        drawable->CompilePipeline();

        drawable->BuildCommandBuffer();

        renderLoop_ptr = std::make_shared<RenderSystem::RenderLoop>(vulkan_ptr, drawable, camera_ptr);
        renderLoop_ptr->Loop();
    }

    void TestDynamicConstant()
    {
        Dcb::RawLayout s;
        s.Add<Dcb::Float>("a1");
        s.Add<Dcb::Float>("a2");
        s.Add<Dcb::Float>("a3");
        s.Add<Dcb::Float2>("a4");

        cout << s.GetSignature() << endl;
        auto b = Dcb::Buffer(std::move(s));
        b["a3"] = 1.0f;
        b["a2"] = 2.0f;
        b["a4"] = glm::vec2(1., 2.);
        cout << b.GetSizeInBytes() << endl;
        cout << b.GetData() << endl;
    }

private:

	shared_ptr<Graphics::Vulkan> vulkan_ptr;
	shared_ptr<Graphics::Image> image_ptr;
    shared_ptr<TestCube> triangle_ptr;
    shared_ptr<Graphics::CommandPool> cmdPool_ptr;
    shared_ptr<Graphics::Synchronization> sync_ptr;
    shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr;
    shared_ptr<Graphics::CommandQueue> cmdQueue_ptr;
    shared_ptr<Graphics::DescriptorPool> desc_pool_ptr;
    shared_ptr<RenderSystem::RenderLoop> renderLoop_ptr;
    shared_ptr<Control::Camera> camera_ptr;
};


int main()
{
	Test test;
    test.test();
	cout << "hello" << endl;
	return 0;
}