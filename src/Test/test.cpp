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
#include "Scene.h"
#include <glm/gtx/string_cast.hpp>


using namespace std;

int WIDTH = 800;
int HEIGHT = 600;

class TestCube : public Draw::ModelBase
{
public:

    TestCube(shared_ptr<Graphics::Vulkan> vulkan_ptr, shared_ptr<Graphics::DescriptorSetCore> desc_ptr)
    {
        this->vulkan_ptr = vulkan_ptr;
        this->desc_ptr = desc_ptr;
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

        layout.Add<Dcb::Matrix>("model");

        bufs["test1"] = make_shared<Dcb::Buffer>(std::move(layout));
        buffer_ptrs["test1"] = make_shared<Graphics::Buffer>(vulkan_ptr, Graphics::BufferUsage::UNIFORM, bufs["test1"]->GetSizeInBytes());

        desc_ptr->Add(Graphics::LayoutType::MODEL, Graphics::DescriptorType::UNIFORM, Graphics::StageFlag::VERTEX, buffer_ptrs["test1"]);
    
        (*bufs["test1"])["model"] = this->getModelMatrix();

        for (auto it : buffer_ptrs)
        {
            for (int i = 0; i < 2; ++i)
            {
                it.second->UpdateData(i, bufs[it.first]->GetSizeInBytes(), bufs[it.first]->GetData());
            }
        }
    }

    void SetData()
    {
    }

    // 通过 Model 继承
    void Compile()
    {

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
        (*bufs["test1"])["model"] = this->getModelMatrix();
       
        for (auto it : buffer_ptrs)
        {
            for (int i = 0; i < 2; ++i)
            {
                it.second->UpdateData(i, bufs[it.first]->GetSizeInBytes(), bufs[it.first]->GetData());
            }
        }
    }
};


class Test
{
	
public:

    Test()
    {
        
        vulkan_ptr = make_shared<Graphics::Vulkan>(WIDTH, HEIGHT);


        image_ptr = make_shared<Graphics::Image>(vulkan_ptr);
        
        cmdPool_ptr = make_shared<Graphics::CommandPool>(vulkan_ptr);
        sync_ptr = make_shared<Graphics::Synchronization>(vulkan_ptr);
        cmdBuf_ptr = make_shared<Graphics::CommandBuffer>(vulkan_ptr, cmdPool_ptr);
        cmdQueue_ptr = make_shared<Graphics::CommandQueue>(vulkan_ptr, cmdBuf_ptr, sync_ptr);
        desc_pool_ptr = make_shared<Graphics::DescriptorPool>(vulkan_ptr);
        desc_ptr = make_shared<Graphics::DescriptorSetCore>(vulkan_ptr, desc_pool_ptr);

        // camera_ptr = make_shared<Control::Camera>(WIDTH, HEIGHT);
        scene_ptr = make_shared<Control::Scene>(vulkan_ptr, desc_ptr, WIDTH, HEIGHT);
        triangle_ptr = make_shared<TestCube>(vulkan_ptr, desc_ptr);
    }

    void test() {
        //auto mm = make_shared<Draw::Model>(vulkan_ptr, "../assets/bunny/bunny.obj");
        //
        //auto vertexShader_ptr = std::make_shared<Bind::VertexShader>(vulkan_ptr, "../src/shaders/Phone.vert.glsl", "../src/shaders", "main");
        //auto pixelShader_ptr = std::make_shared<Bind::PixelShader>(vulkan_ptr, "../src/shaders/Phone.frag.glsl", "../src/shaders", "main");
        //// auto vertexBuffer_ptr = std::make_shared<Bind::VertexBuffer>(vulkan_ptr, triangle_ptr);

        //auto pipeline_ptr = std::make_shared<Graphics::Pipeline>(vulkan_ptr);
        //auto renderpass_ptr = std::make_shared<Graphics::RenderPass>(vulkan_ptr, image_ptr);
      
        //scene_ptr->InitSceneData();
        //triangle_ptr->InitVar();
        //triangle_ptr->SetData();

        //auto drawable = std::make_shared<Draw::Drawable>(vulkan_ptr);

        //drawable->Register<Draw::GraphicsType::DescriptorSet>(desc_ptr);
        //drawable->Register<Draw::GraphicsType::Pipeline>(pipeline_ptr);
        //drawable->Register<Draw::GraphicsType::RenderPass>(renderpass_ptr);

        //drawable->Register<Draw::GraphicsType::CommandBuffer>(cmdBuf_ptr);
        //drawable->Register<Draw::GraphicsType::CommandQueue>(cmdQueue_ptr);

        //drawable->Register<Draw::BindType::VertexShader>(vertexShader_ptr);
        //drawable->Register<Draw::BindType::PixelShader>(pixelShader_ptr);
        //// drawable->Register<Draw::BindType::VertexBuffer>(vertexBuffer_ptr);

        //drawable->CompilePipeline();

        //drawable->BuildCommandBuffer();

        //renderLoop_ptr = std::make_shared<RenderSystem::RenderLoop>(vulkan_ptr, drawable, scene_ptr);
        //renderLoop_ptr->Loop();
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
    shared_ptr<Control::Scene> scene_ptr;
    shared_ptr<Graphics::DescriptorSetCore> desc_ptr;
    
};


int main()
{
	Test test;
    test.test();
	cout << "hello" << endl;
	return 0;
}