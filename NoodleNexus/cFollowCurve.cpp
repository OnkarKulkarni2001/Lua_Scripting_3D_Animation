#include "cFollowCurve.h"
#include <iostream>

void cFollowCurve::Init(cPhysics::sPhysInfo* pObject,
    glm::vec3 start, glm::vec3 control1, glm::vec3 control2, glm::vec3 end, double duration)
{
    this->m_pObject = pObject;
    this->m_start = start;
    this->m_control1 = control1;
    this->m_control2 = control2;
    this->m_end = end;
    this->m_duration = duration;
    this->m_elapsedTime = 0.0;
}

void cFollowCurve::OnStart(void)
{
    this->m_elapsedTime = 0.0;
    this->m_pObject->position = m_start; // Start at the beginning of the curve
}

void cFollowCurve::Update(double deltaTime)
{
    this->m_elapsedTime += deltaTime;
    float t = glm::clamp(static_cast<float>(this->m_elapsedTime / this->m_duration), 0.0f, 1.0f);

    // Set velocity based on the derivative of the Bézier curve
    this->m_pObject->velocity = CalculateBezierVelocity(t, m_start, m_control1, m_control2, m_end);
}

bool cFollowCurve::isFinished(void)
{
    return this->m_elapsedTime >= this->m_duration;
}

void cFollowCurve::OnFinished(void)
{
    this->m_pObject->velocity = glm::vec3(0.0f); // Stop the object
}

glm::vec3 cFollowCurve::CalculateBezierPoint(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 point = uuu * p0;      // (1 - t)^3 * p0
    point += 3 * uu * t * p1;        // 3 * (1 - t)^2 * t * p1
    point += 3 * u * tt * p2;        // 3 * (1 - t) * t^2 * p2
    point += ttt * p3;               // t^3 * p3
    return point;
}

glm::vec3 cFollowCurve::CalculateBezierVelocity(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
    float u = 1.0f - t;

    glm::vec3 velocity = 3 * u * u * (p1 - p0);  // 3 * (1 - t)^2 * (p1 - p0)
    velocity += 6 * u * t * (p2 - p1);          // 6 * (1 - t) * t * (p2 - p1)
    velocity += 3 * t * t * (p3 - p2);          // 3 * t^2 * (p3 - p2)

    return velocity / static_cast<float>(m_duration); // Normalize velocity by duration
}
