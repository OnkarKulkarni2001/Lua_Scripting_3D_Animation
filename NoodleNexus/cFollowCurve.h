#pragma once

#include "iCommand.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include "cPhysics.h"

class cFollowCurve : public iCommand
{
public:
    void Init(cPhysics::sPhysInfo* pObject,
        glm::vec3 start, glm::vec3 control1, glm::vec3 control2, glm::vec3 end, double duration);

    virtual void OnStart(void);
    virtual void Update(double deltaTime);
    virtual bool isFinished(void);
    virtual void OnFinished(void);

private:
    glm::vec3 m_start, m_control1, m_control2, m_end;
    double m_duration;
    cPhysics::sPhysInfo* m_pObject;
    double m_elapsedTime;

    glm::vec3 CalculateBezierPoint(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
    glm::vec3 CalculateBezierVelocity(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
};
