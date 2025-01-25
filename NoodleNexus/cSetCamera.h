#pragma once
#include "cBasicFlyCamera/cBasicFlyCamera.h"
#include "cPhysics.h"
#include "iCommand.h"

class cSetCamera : public iCommand
{
public:
    // Call this to set the values specific to this particular command
    void Init(cBasicFlyCamera* pFlyCam, glm::vec3 newEyeLocation, double timeToMove);

    // From iCommand interface
    virtual void OnStart(void);
    virtual void Update(double deltaTime);
    virtual bool isFinished(void);
    virtual void OnFinished(void);
    //void Stop(); // Optional: Stop the command prematurely

private:
    glm::vec3 m_endXYZ;
    double m_timeToMove;
    cBasicFlyCamera* pFlyCam;
    cPhysics::sPhysInfo* m_pObject;

    // Calculated values
    double m_elapsedTime;
    glm::vec3 m_initialPosition;
};

