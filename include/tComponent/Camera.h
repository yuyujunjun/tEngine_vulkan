

#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include"ShaderVariable.h"
#include"imgui.h"
#include"Component.h"
namespace tEngine {

    enum class ProjectionType {
        Ortho,
        Perspective
    };
    
    struct CameraTransform {
        ProjectionType projection=ProjectionType::Perspective;
        CameraTransform() { update(); }
        glm::vec3 m_cameraPosition = glm::vec3(5, 5, 5);
        glm::vec3 m_centerPosition = glm::vec3(0, 0, 0);
        glm::vec3 m_upVector = glm::vec3(0, 1, 0);
        float     m_roll = 0;                      // Rotation around the Z axis in RAD
        glm::mat4 m_matrix = glm::mat4(1);
        glm::mat4 p_matrix = glm::mat4(1);
        glm::u32vec2 m_windowSize = glm::u32vec2(1080, 960);
        float nearPlane = 0.01;
        float farPlane = 1000;
        float fieldOfview = glm::radians(60.f);

        void update();
    };

    class CameraSystem:public System
    {
    public:
        enum class Action { None, Orbit, Dolly, Pan, LookAround };
        enum class Mode { Examine, Fly, Walk, Trackball };
        enum class MouseButton { None, Left, Middle, Right };
        enum class ModifierFlagBits : uint32_t { Shift = 1, Ctrl = 2, Alt = 4 };
        using ModifierFlags = uint32_t;

    public:
       // ImGuiIO io;
        void ExecuteAllComponents(float dt) override;

        CameraSystem();

        glm::vec3 const& getCameraPosition() const;
        glm::vec3 const& getCenterPosition() const;
        glm::mat4 const& getMatrix() const;
        Mode getMode() const;
        glm::ivec2 const& getMousePosition() const;
        float getRoll() const;
        float getSpeed() const;
        glm::vec3 const& getUpVector() const;
        glm::u32vec2 const& getWindowSize() const;

        Action mouseMove(glm::ivec2 const& position, MouseButton mouseButton, ModifierFlags& modifiers);
        void setLookat(const glm::vec3& cameraPosition, const glm::vec3& centerPosition, const glm::vec3& upVector);
        void setMode(Mode mode);
        void setMousePosition(glm::ivec2 const& position);
        void setRoll(float roll);   // roll in radians
        void setSpeed(float speed);
        void setWindowSize(glm::ivec2 const& size);
        void wheel(int value);
        void setCamera(CameraTransform* cam);
    private:
        void dolly(glm::vec2 const& delta);
        void motion(glm::ivec2 const& position, Action action = Action::None);
        void orbit(glm::vec2 const& delta, bool invert = false);
        void pan(glm::vec2 const& delta);
        double projectOntoTBSphere(const glm::vec2& p);
        void trackball(glm::ivec2 const& position);
        // void update();

    private:
        CameraTransform* cam;


        glm::u32vec2 m_windowSize = glm::u32vec2(1080, 960);

        float       m_speed = 30.0f;
        glm::ivec2  m_mousePosition = glm::ivec2(0, 0);

        Mode m_mode = Mode::Examine;



    };

    inline  glm::mat4 Perspective(vk::Extent2D extent, float nearPlane = 0.01, float farPlane = 1000, float fieldOfview = glm::radians(60.f)) {
        float _viewportAspectRatio = float(extent.width) / extent.height;
        return glm::perspective(fieldOfview, _viewportAspectRatio, nearPlane, farPlane);
    }
    inline glm::mat4 Ortho(float left, float right, float bottom, float top, float depth) {
        auto mat = glm::ortho(left, right, bottom, top, 1.f, 1000.f);
        return glm::ortho(left, right, bottom, top, -depth, depth);
    }
 


    class tShaderInterface;
    class tBuffer;
    using BufferHandle = std::shared_ptr<tBuffer>;
    class BufferRangeManager;
    class Device;
    BufferRangeManager* requestCameraBufferRange(const Device* device);
  
  void uploadCameraMatrix(const glm::mat4& view, const glm::mat4& projection, tShaderInterface* material);
  void uploadCameraMatrix(const glm::mat4& view, const glm::mat4& projection, tBuffer* buffer, unsigned rangeOffset);
 
}

