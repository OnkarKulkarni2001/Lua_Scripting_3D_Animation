#include "cSetCamera.h"
#include "sharedThings.h"

void cSetCamera::Init(cBasicFlyCamera* pFlyCam, glm::vec3 newEyeLocation, double timeToMove)
{
    this->pFlyCam = ::g_pFlyCamera;
    this->m_endXYZ = newEyeLocation;
    this->m_timeToMove = (timeToMove > 0.0) ? timeToMove : 0.01; // Avoid invalid or zero time
    this->m_elapsedTime = 0.0;
    this->m_initialPosition = pFlyCam->getEyeLocation(); // Store the starting position
}

void cSetCamera::OnStart(void)
{
    this->m_elapsedTime = 0.0;

    //glm::vec3 totalDistance = this->m_endXYZ - this->pFlyCam->getEyeLocation();
    //this->pFlyCam->velocity = totalDistance / static_cast<float>(this->m_timeToMove);

    // Store the initial position for reference
    //this->m_initialPosition = this->pFlyCam->getEyeLocation();
    //this->pFlyCam->setEyeLocation(m_endXYZ);

}

void cSetCamera::Update(double deltaTime)
{
    this->pFlyCam->setEyeLocation(m_endXYZ);
}

bool cSetCamera::isFinished(void)
{
    // Has the amount of time passed
    if (this->m_elapsedTime >= this->m_timeToMove)
    {
        // We've arrived
        return true;
    }

    // Keep going...
    return false;
}

void cSetCamera::OnFinished(void)
{
    this->pFlyCam->setEyeLocation(m_endXYZ);
}

