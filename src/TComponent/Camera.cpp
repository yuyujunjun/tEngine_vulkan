/*
 tdogl::Camera

 Copyright 2012 Thomas Dalling - http://tomdalling.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#define _USE_MATH_DEFINES
#define GLM_FORCE_RADIANS 
#include <cmath>
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include"Shader.h"
#include"ShaderInterface.h"
using namespace tEngine;

static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock

const float trackballSize = 0.8f;

//-----------------------------------------------------------------------------
// MATH functions
//
template <typename T>
bool isZero(const T& _a)
{
    return fabs(_a) < std::numeric_limits<T>::epsilon();
}
template <typename T>
bool isOne(const T& _a)
{
    return areEqual(_a, (T)1);
}
inline float sign(float s)
{
    return (s < 0.f) ? -1.f : 1.f;
}

CameraSystem::CameraSystem()
{
   
}

glm::vec3 const& CameraSystem::getCameraPosition() const
{
    return cam->m_cameraPosition;
}

glm::vec3 const& CameraSystem::getCenterPosition() const
{
    return cam->m_centerPosition;
}

glm::mat4 const& CameraSystem::getMatrix() const
{
    return cam->m_matrix;
}

CameraSystem::Mode CameraSystem::getMode() const
{
    return m_mode;
}

glm::ivec2 const& CameraSystem::getMousePosition() const
{
    return m_mousePosition;
}

float CameraSystem::getRoll() const
{
    return cam->m_roll;
}

float CameraSystem::getSpeed() const
{
    return m_speed;
}

glm::vec3 const& CameraSystem::getUpVector() const
{
    return cam->m_upVector;
}

glm::u32vec2 const& CameraSystem::getWindowSize() const
{
    return m_windowSize;
}

CameraSystem::Action
CameraSystem::mouseMove(glm::ivec2 const& position, MouseButton mouseButton, ModifierFlags& modifiers)
{
    Action curAction = Action::None;
    switch (mouseButton)
    {
    case MouseButton::Left:
        if (((modifiers & static_cast<uint32_t>(ModifierFlagBits::Ctrl)) && (modifiers & static_cast<uint32_t>(ModifierFlagBits::Shift))) ||
            (modifiers & static_cast<uint32_t>(ModifierFlagBits::Alt)))
        {
            curAction = m_mode == Mode::Examine ? Action::LookAround : Action::Orbit;
        }
        else if (modifiers & static_cast<uint32_t>(ModifierFlagBits::Shift))
        {
            curAction = Action::Dolly;
        }
        else if (modifiers & static_cast<uint32_t>(ModifierFlagBits::Ctrl))
        {
            curAction = Action::Pan;
        }
        else
        {
            curAction = m_mode == Mode::Examine ? Action::Orbit : Action::LookAround;
        }
        break;
    case MouseButton::Middle: curAction = Action::Pan; break;
    case MouseButton::Right: curAction = Action::Dolly; break;
    default: assert(false);
    }
    assert(curAction != Action::None);
    motion(position, curAction);

    return curAction;
}

void CameraSystem::setLookat(const glm::vec3& cameraPosition,
    const glm::vec3& centerPosition,
    const glm::vec3& upVector)
{
    cam->m_cameraPosition = cameraPosition;
    cam->m_centerPosition = centerPosition;
    cam->m_upVector = upVector;
    cam->update();
}

void CameraSystem::setMode(Mode mode)
{
    m_mode = mode;
}

void CameraSystem::setMousePosition(glm::ivec2 const& position)
{
    m_mousePosition = position;
}

void CameraSystem::setRoll(float roll)
{
    cam->m_roll = roll;
    cam->update();
}
void CameraSystem::setCamera(CameraComponent* cam) {
    this->cam = cam;
    cam->update();

}
void CameraSystem::setSpeed(float speed)
{
    m_speed = speed;
}

void CameraSystem::setWindowSize(glm::ivec2 const& size)
{
    m_windowSize = size;
}

void CameraSystem::wheel(int value)
{
    float fValue = static_cast<float>(value);
    float dx = (fValue * std::abs(fValue)) / static_cast<float>(m_windowSize[0]);

    glm::vec3 z = cam->m_cameraPosition - cam->m_centerPosition;
    float     length = z.length() * 0.1f;
    length = length < 0.001f ? 0.001f : length;

    dx *= m_speed;
    dolly(glm::vec2(dx, dx));
    cam->update();
}

void CameraSystem::dolly(glm::vec2 const& delta)
{
    glm::vec3 z = cam->m_centerPosition - cam->m_cameraPosition;
    float     length = glm::length(z);

    // We are at the point of interest, and don't know any direction, so do nothing!
    if (isZero(length))
    {
        return;
    }

    // Use the larger movement.
    float dd;
    if (m_mode != Mode::Examine)
    {
        dd = -delta[1];
    }
    else
    {
        dd = fabs(delta[0]) > fabs(delta[1]) ? delta[0] : -delta[1];
    }

    float factor = m_speed * dd / length;

    // Adjust speed based on distance.
    length /= 10;
    length = length < 0.001f ? 0.001f : length;
    factor *= length;

    // Don't move to or through the point of interest.
    if (1.0f <= factor)
    {
        return;
    }

    z *= factor;

    // Not going up
    if (m_mode == Mode::Walk)
    {
        if (cam->m_upVector.y > cam->m_upVector.z)
        {
            z.y = 0;
        }
        else
        {
            z.z = 0;
        }
    }

    cam->m_cameraPosition += z;

    // In fly mode, the interest moves with us.
    if (m_mode != Mode::Examine)
    {
        cam->m_centerPosition += z;
    }
}

void CameraSystem::motion(glm::ivec2 const& position, Action action)
{
    glm::vec2 delta(float(position[0] - m_mousePosition[0]) / float(m_windowSize[0]),
        float(position[1] - m_mousePosition[1]) / float(m_windowSize[1]));

    switch (action)
    {
    case Action::Orbit:
        if (m_mode == Mode::Trackball)
        {
            orbit(delta, true);  // trackball(position);
        }
        else
        {
            orbit(delta, false);
        }
        break;
    case Action::Dolly: dolly(delta); break;
    case Action::Pan: pan(delta); break;
    case Action::LookAround:
        if (m_mode == Mode::Trackball)
        {
            trackball(position);
        }
        else
        {
            orbit(glm::vec2(delta[0], -delta[1]), true);
        }
        break;
    default: break;
    }

    cam->update();

    m_mousePosition = position;
}

void CameraSystem::orbit(glm::vec2 const& delta, bool invert)
{
    if (isZero(delta[0]) && isZero(delta[1]))
    {
        return;
    }

    // Full width will do a full turn
    float dx = delta[0] * float(glm::two_pi<float>());
    float dy = delta[1] * float(glm::two_pi<float>());

    // Get the camera
    glm::vec3 origin(invert ? cam->m_cameraPosition : cam->m_centerPosition);
    glm::vec3 position(invert ? cam->m_centerPosition : cam->m_cameraPosition);

    // Get the length of sight
    glm::vec3 centerToEye(position - origin);
    float     radius = glm::length(centerToEye);
    centerToEye = glm::normalize(centerToEye);

    // Find the rotation around the UP axis (Y)
    glm::vec3 zAxis(centerToEye);
    glm::mat4 yRotation = glm::rotate(-dx, cam->m_upVector);

    // Apply the (Y) rotation to the eye-center vector
    glm::vec4 tmpVector = yRotation * glm::vec4(centerToEye.x, centerToEye.y, centerToEye.z, 0.0f);
    centerToEye = glm::vec3(tmpVector.x, tmpVector.y, tmpVector.z);

    // Find the rotation around the X vector: cross between eye-center and up (X)
    glm::vec3 xAxis = glm::cross(cam->m_upVector, zAxis);
    xAxis = glm::normalize(xAxis);
    glm::mat4 xRotation = glm::rotate(-dy, xAxis);

    // Apply the (X) rotation to the eye-center vector
    tmpVector = xRotation * glm::vec4(centerToEye.x, centerToEye.y, centerToEye.z, 0);
    glm::vec3 rotatedVector(tmpVector.x, tmpVector.y, tmpVector.z);
    if (sign(rotatedVector.x) == sign(centerToEye.x))
    {
        centerToEye = rotatedVector;
    }

    // Make the vector as long as it was originally
    centerToEye *= radius;

    // Finding the new position
    glm::vec3 newPosition = centerToEye + origin;

    if (!invert)
    {
        cam->m_cameraPosition = newPosition;  // Normal: change the position of the camera
    }
    else
    {
        cam->m_centerPosition = newPosition;  // Inverted: change the interest point
    }
}

void CameraSystem::pan(glm::vec2 const& delta)
{
    glm::vec3 z(cam->m_cameraPosition - cam->m_centerPosition);
    float     length = static_cast<float>(glm::length(z)) / 0.785f;  // 45 degrees
    z = glm::normalize(z);
    glm::vec3 x = glm::normalize(glm::cross(cam->m_upVector, z));
    glm::vec3 y = glm::normalize(glm::cross(z, x));
    x *= -delta[0] * length;
    y *= delta[1] * length;

    if (m_mode == Mode::Fly)
    {
        x = -x;
        y = -y;
    }

    cam->m_cameraPosition += x + y;
    cam->m_centerPosition += x + y;
}

double CameraSystem::projectOntoTBSphere(const glm::vec2& p)
{
    double z;
    double d = length(p);
    if (d < trackballSize * 0.70710678118654752440)
    {
        // inside sphere
        z = sqrt(trackballSize * trackballSize - d * d);
    }
    else
    {
        // on hyperbola
        double t = trackballSize / 1.41421356237309504880;
        z = t * t / d;
    }

    return z;
}

void CameraSystem::trackball(glm::ivec2 const& position)
{
    glm::vec2 p0(2 * (m_mousePosition[0] - m_windowSize[0] / 2) / double(m_windowSize[0]),
        2 * (m_windowSize[1] / 2 - m_mousePosition[1]) / double(m_windowSize[1]));
    glm::vec2 p1(2 * (position[0] - m_windowSize[0] / 2) / double(m_windowSize[0]),
        2 * (m_windowSize[1] / 2 - position[1]) / double(m_windowSize[1]));

    // determine the z coordinate on the sphere
    glm::vec3 pTB0(p0[0], p0[1], projectOntoTBSphere(p0));
    glm::vec3 pTB1(p1[0], p1[1], projectOntoTBSphere(p1));

    // calculate the rotation axis via cross product between p0 and p1
    glm::vec3 axis = glm::cross(pTB0, pTB1);
    axis = glm::normalize(axis);

    // calculate the angle
    float t = glm::length(pTB0 - pTB1) / (2.f * trackballSize);

    // clamp between -1 and 1
    if (t > 1.0f)
    {
        t = 1.0f;
    }
    else if (t < -1.0f)
    {
        t = -1.0f;
    }

    float rad = 2.0f * asin(t);

    {
        glm::vec4 rot_axis = cam->m_matrix * glm::vec4(axis, 0);
        glm::mat4 rot_mat = glm::rotate(rad, glm::vec3(rot_axis.x, rot_axis.y, rot_axis.z));

        glm::vec3 pnt = cam->m_cameraPosition - cam->m_centerPosition;
        glm::vec4 pnt2 = rot_mat * glm::vec4(pnt.x, pnt.y, pnt.z, 1);
        cam->m_cameraPosition = cam->m_centerPosition + glm::vec3(pnt2.x, pnt2.y, pnt2.z);
        glm::vec4 up2 = rot_mat * glm::vec4(cam->m_upVector.x, cam->m_upVector.y, cam->m_upVector.z, 0);
        cam->m_upVector = glm::vec3(up2.x, up2.y, up2.z);
    }
}

void CameraComponent::update()
{
    auto cam = this;
    cam->m_matrix = glm::lookAt(cam->m_cameraPosition, cam->m_centerPosition, cam->m_upVector);

    if (!isZero(cam->m_roll))
    {
        glm::mat4 rot = glm::rotate(cam->m_roll, glm::vec3(0, 0, 1));
        cam->m_matrix = cam->m_matrix * rot;
    }
}
void tEngine::updateCameraBehavior(ImGuiIO& io,CameraSystem& cam) {
    if (io.WantCaptureMouse)return;
    CameraSystem::MouseButton mouseButton =
        (io.MouseDown[0]) ? CameraSystem::MouseButton::Left:
        (io.MouseDown[2])? CameraSystem::MouseButton::Middle
        : (io.MouseDown[1]) ? CameraSystem::MouseButton::Right
        : CameraSystem::MouseButton::None;
    if (mouseButton != CameraSystem::MouseButton::None)
    {
        CameraSystem::ModifierFlags modifiers = 0;
        
        if (io.KeyAlt)
        {
            modifiers |= static_cast<uint32_t>(CameraSystem::ModifierFlagBits::Alt);
        }
        if (io.KeyCtrl)
        {
            modifiers |= static_cast<uint32_t>(CameraSystem::ModifierFlagBits::Ctrl);
        }
        if (io.KeyShift)
        {
            modifiers |= static_cast<uint32_t>(CameraSystem::ModifierFlagBits::Shift);
        }
        cam.mouseMove(
            glm::ivec2(static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y)), mouseButton, modifiers);
    }
    cam.setMousePosition(glm::ivec2(static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y)));
    
    cam.wheel(io.MouseWheel);
}
namespace tEngine {
  
      void uploadCameraMatrix(const glm::mat4& view, const glm::mat4& projection, tShaderInterface* material) {
        material->SetValueOnBuffer(ShaderString(SV::_MATRIX_V), view);
        material->SetValueOnBuffer(ShaderString(SV::_MATRIX_P), projection);
        material->SetValueOnBuffer(ShaderString(SV::_MATRIX_VP), projection * view);
        material->SetValueOnBuffer(ShaderString(SV::_INV_MATRIX_VP), glm::inverse(projection * view));
    }
}