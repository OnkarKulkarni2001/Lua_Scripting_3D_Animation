//#define GLAD_GL_IMPLEMENTATION
//#include <glad/glad.h>
//
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>
#include "GLCommon.h"

//#include "linmath.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <iostream>     // "input output" stream
#include <fstream>      // "file" stream
#include <sstream>      // "string" stream ("string builder" in Java c#, etc.)
#include <string>
#include <vector>

//void ReadPlyModelFromFile(std::string plyFileName);
#include "PlyFileLoaders.h"
#include "Basic_Shader_Manager/cShaderManager.h"
#include "sMesh.h"
#include "cVAOManager/cVAOManager.h"
#include "sharedThings.h"       // Fly camera
#include "cPhysics.h"
#include "cLightManager.h"
#include <windows.h>    // Includes ALL of windows... MessageBox
#include "cLightHelper/cLightHelper.h"
//
#include "cBasicTextureManager/cBasicTextureManager.h"

#include "cLowPassFilter.h"
#include "cFollowCurve.h"

//
//const unsigned int MAX_NUMBER_OF_MESHES = 1000;
//unsigned int g_NumberOfMeshesToDraw;
//sMesh* g_myMeshes[MAX_NUMBER_OF_MESHES] = { 0 };    // All zeros

std::vector<sMesh*> g_vecMeshesToDraw;

cPhysics* g_pPhysicEngine = NULL;
// This loads the 3D models for drawing, etc.
cVAOManager* g_pMeshManager = NULL;

cBasicTextureManager* g_pTextures = NULL;

cCommandGroup* g_pCommandDirector = NULL;
cCommandFactory* g_pCommandFactory = NULL;

//enum eCommands
//{
//    MOVE_TO,
//    ORIENT_TO
//};



//cLightManager* g_pLightManager = NULL;

void AddModelsToScene(cVAOManager* pMeshManager, GLuint shaderProgram);

void DrawMesh(sMesh* pCurMesh, GLuint program);

//glm::vec3 cameraEye = glm::vec3(0.0, 0.0, 4.0f);




// This is the function that Lua will call when 
//void g_Lua_AddSerialCommand(std::string theCommandText)
int g_Lua_AddSerialCommand(lua_State* L)
{
//    std::cout << "**************************" << std::endl;
//    std::cout << "g_Lua_AddSerialCommand() called" << std::endl;
//    std::cout << "**************************" << std::endl;
    // AddSerialCommand() has been called
    // eg: AddSerialCommand('New_Viper_Player', -50.0, 15.0, 30.0, 5.0)

    int argCount = lua_gettop(L); // Get the number of arguments on the stack
    if (argCount < 6) {
        std::cerr << "Error: Expected 6 arguments, got " << argCount << "." << std::endl;
        return 0;
    }
    std::string commandType = lua_tostring(L, 1);
    std::vector<std::string> vecCommandDetails;
    vecCommandDetails.push_back(commandType);

    if (commandType == "SET_CAMERA")
    {

        std::string objectFriendlyName = lua_tostring(L, 2);      // 'New_Viper_Player'
        float x = (float)lua_tonumber(L, 3);                   // -50.0
        float y = (float)lua_tonumber(L, 4);                   // 15.0
        float z = (float)lua_tonumber(L, 5);                   // 30.0
        float timeSeconds = (float)lua_tonumber(L, 6);


        vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
        vecCommandDetails.push_back(::g_floatToString(x));
        vecCommandDetails.push_back(::g_floatToString(y));
        vecCommandDetails.push_back(::g_floatToString(z));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pSetCamera = ::g_pCommandFactory->pCreateCommandObject(
            "Set Camera", vecCommandDetails);

        ::g_pCommandDirector->addSerial(pSetCamera);
    }
    else if (commandType == "MOVE_TO")
    {

        std::string objectFriendlyName = lua_tostring(L, 2);      // 'New_Viper_Player'
        float x = (float)lua_tonumber(L, 3);                   // -50.0
        float y = (float)lua_tonumber(L, 4);                   // 15.0
        float z = (float)lua_tonumber(L, 5);                   // 30.0
        float timeSeconds = (float)lua_tonumber(L, 6);


        vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
        vecCommandDetails.push_back(::g_floatToString(x));
        vecCommandDetails.push_back(::g_floatToString(y));
        vecCommandDetails.push_back(::g_floatToString(z));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pMoveViper = ::g_pCommandFactory->pCreateCommandObject(
            "Move Relative ConstVelocity+Time", vecCommandDetails);

        ::g_pCommandDirector->addSerial(pMoveViper);
    }
    else if (commandType == "ORIENT_TO")
    {

        std::string objectFriendlyName = lua_tostring(L, 2);      // 'New_Viper_Player'
        float x = (float)lua_tonumber(L, 3);                   // -50.0
        float y = (float)lua_tonumber(L, 4);                   // 15.0
        float z = (float)lua_tonumber(L, 5);                   // 30.0
        float timeSeconds = (float)lua_tonumber(L, 6);


        vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
        vecCommandDetails.push_back(::g_floatToString(x));
        vecCommandDetails.push_back(::g_floatToString(y));
        vecCommandDetails.push_back(::g_floatToString(z));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pOrientViper = ::g_pCommandFactory->pCreateCommandObject(
            "Orient Relative Time", vecCommandDetails);
        ::g_pCommandDirector->addSerial(pOrientViper);
    }
    else if (commandType == "FOLLOW_CURVE")
    {
        std::string objectFriendlyName = lua_tostring(L, 2);

        float startPosX = (float)lua_tonumber(L, 3);
        float startPosY = (float)lua_tonumber(L, 4);
        float startPosZ = (float)lua_tonumber(L, 5);

        float control1X = (float)lua_tonumber(L, 6);
        float control1Y = (float)lua_tonumber(L, 7);
        float control1Z = (float)lua_tonumber(L, 11);

        float control2X = (float)lua_tonumber(L, 9);
        float control2Y = (float)lua_tonumber(L, 10);
        float control2Z = (float)lua_tonumber(L, 11);

        float endPosX = (float)lua_tonumber(L, 12);
        float endPosY = (float)lua_tonumber(L, 13);
        float endPosZ = (float)lua_tonumber(L, 14);

        float timeSeconds = (float)lua_tonumber(L, 15);

        vecCommandDetails.push_back(objectFriendlyName);
        vecCommandDetails.push_back(::g_floatToString(startPosX)); 
        vecCommandDetails.push_back(::g_floatToString(startPosY)); 
        vecCommandDetails.push_back(::g_floatToString(startPosZ));

        vecCommandDetails.push_back(::g_floatToString(control1X));
        vecCommandDetails.push_back(::g_floatToString(control1Y)); 
        vecCommandDetails.push_back(::g_floatToString(control1Z));

        vecCommandDetails.push_back(::g_floatToString(control2X)); 
        vecCommandDetails.push_back(::g_floatToString(control2Y)); 
        vecCommandDetails.push_back(::g_floatToString(control2Z));

        vecCommandDetails.push_back(::g_floatToString(endPosX)); 
        vecCommandDetails.push_back(::g_floatToString(endPosY)); 
        vecCommandDetails.push_back(::g_floatToString(endPosZ));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pFollowCurveViper = ::g_pCommandFactory->pCreateCommandObject(
            "Follow Curve", vecCommandDetails);
        ::g_pCommandDirector->addSerial(pFollowCurveViper);
    }
    else if (commandType == "FOLLOW_OBJECT")
    {

        std::string objectFriendlyNameFollower = lua_tostring(L, 2);
        std::string objectFriendlyNameToFollow = lua_tostring(L, 3);

        float offsetX = (float)lua_tonumber(L, 4);
        float offsetY = (float)lua_tonumber(L, 5);
        float offsetZ = (float)lua_tonumber(L, 6);

        float timeSeconds = (float)lua_tonumber(L, 7);

        vecCommandDetails.push_back(objectFriendlyNameFollower);
        vecCommandDetails.push_back(objectFriendlyNameToFollow);
        vecCommandDetails.push_back(::g_floatToString(offsetX));
        vecCommandDetails.push_back(::g_floatToString(offsetY));
        vecCommandDetails.push_back(::g_floatToString(offsetZ));

        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pFollowObjectViper = ::g_pCommandFactory->pCreateCommandObject(
            "Follow Object", vecCommandDetails);
        ::g_pCommandDirector->addSerial(pFollowObjectViper);
    }


    // We'll return some value to indicate if the command worked or not
    // Here, we'll push "true" if it worked
    lua_pushboolean(L, true);
    // return 1 because we pushed 1 thing onto the stack
    return 1;
}




static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

bool isControlDown(GLFWwindow* window);
#include "cTankBuilder.h"
#include "cArena.h"
void SetUpTankGame(void);
void TankStepFrame(double timeStep);
std::vector< iTank* > g_vecTheTanks;
cArena* g_pTankArena = NULL;
sMesh* g_pTankModel = NULL;

// END OF: TANK GAME


bool IsMeshInsideTriggerBox(sMesh* pMesh, sMesh* pTriggerBox) 
{
    // Calculate the half-extents of the trigger box
    glm::vec3 halfExtents = glm::vec3(pTriggerBox->uniformScale * 0.5f);

    // Calculate the world-space bounds of the trigger box
    glm::vec3 minBounds = pTriggerBox->positionXYZ - halfExtents;
    glm::vec3 maxBounds = pTriggerBox->positionXYZ + halfExtents;

    // Get the position of the mesh (assumed to be its center point)
    glm::vec3 meshPosition = pMesh->positionXYZ;

    // Check if the mesh is inside the trigger box
    return (meshPosition.x >= minBounds.x && meshPosition.x <= maxBounds.x &&
        meshPosition.y >= minBounds.y && meshPosition.y <= maxBounds.y &&
        meshPosition.z >= minBounds.z && meshPosition.z <= maxBounds.z);
}







void ConsoleStuff(void);

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float getRandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

glm::vec3 getRandom_vec3(glm::vec3 min, glm::vec3 max)
{
    return glm::vec3(
        getRandomFloat(min.x, max.x),
        getRandomFloat(min.y, max.y),
        getRandomFloat(min.z, max.z));
}

std::string getStringVec3(glm::vec3 theVec3)
{
    std::stringstream ssVec;
    ssVec << "(" << theVec3.x << ", " << theVec3.y << ", " << theVec3.z << ")";
    return ssVec.str();
}

// Returns NULL if NOT found
sMesh* pFindMeshByFriendlyName(std::string theNameToFind)
{
    for (unsigned int index = 0; index != ::g_vecMeshesToDraw.size(); index++)
    {
        if (::g_vecMeshesToDraw[index]->uniqueFriendlyName == theNameToFind)
        {
            return ::g_vecMeshesToDraw[index];
        }
    }
    // Didn't find it
    return NULL;
}

sMesh* GiveRandomAsteroid() {
    float randomAsteroidInt = (int)getRandomFloat(0, 6);
    sMesh* asteroid = new sMesh();
    //sMesh* pAsteroid1 = pFindMeshByFriendlyName("asteroid1");
    //sMesh* pAsteroid2 = pFindMeshByFriendlyName("asteroid2");
    //sMesh* pAsteroid3 = pFindMeshByFriendlyName("asteroid3");
    //sMesh* pAsteroid4 = pFindMeshByFriendlyName("asteroid4");
    //sMesh* pAsteroid5 = pFindMeshByFriendlyName("asteroid5");
    //sMesh* pAsteroidGroup = pFindMeshByFriendlyName("asteroidGroup");

    if (randomAsteroidInt == 0) {
        asteroid = pFindMeshByFriendlyName("asteroid1");
        return asteroid;
    }
    if (randomAsteroidInt == 1) {
        asteroid = pFindMeshByFriendlyName("asteroid2");
        return asteroid;
    }
    if (randomAsteroidInt == 2) {
        asteroid = pFindMeshByFriendlyName("asteroid3");
        return asteroid;
    }
    if (randomAsteroidInt == 3) {
        asteroid = pFindMeshByFriendlyName("asteroid4");
        return asteroid;
    }
    if (randomAsteroidInt == 4) {
        asteroid = pFindMeshByFriendlyName("asteroid5");
        return asteroid;
    }
    //if (randomAsteroidInt == 5) {
    //    asteroid = pFindMeshByFriendlyName("asteroidGroup");
    //    return asteroid;
    //}
}


void GenerateRandomCoordinates(float& x, float& y, float& z) {
    x = getRandomFloat(3000.0f, 4000.0f);
    y = getRandomFloat(-1000.0f, 1000.0f);
    z = getRandomFloat(-800.0f, 800.0f);       // Range taken from screenshots of camLocation
}

void AABBOctTree(void);

int main(void)
{
    
    AABBOctTree();

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Callback for keyboard, but for "typing"
    // Like it captures the press and release and repeat
    glfwSetKeyCallback(window, key_callback);

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowFocusCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);


    cShaderManager* pShaderManager = new cShaderManager();

    cShaderManager::cShader vertexShader;
    vertexShader.fileName = "assets/shaders/vertex01.glsl";

    cShaderManager::cShader fragmentShader;
    fragmentShader.fileName = "assets/shaders/fragment01.glsl";

    if ( ! pShaderManager->createProgramFromFile("shader01",
                                                 vertexShader, fragmentShader))
    {
        std::cout << "Error: " << pShaderManager->getLastError() << std::endl;
    }
    else
    {
        std::cout << "Shader built OK" << std::endl;
    }

    const GLuint program = pShaderManager->getIDFromFriendlyName("shader01");

    glUseProgram(program);


    ::g_pMyLuaMasterBrain = new cLuaBrain();


//    cVAOManager* pMeshManager = new cVAOManager();
    ::g_pMeshManager = new cVAOManager();

    ::g_pPhysicEngine = new cPhysics();
    // For triangle meshes, let the physics object "know" about the VAO manager
    ::g_pPhysicEngine->setVAOManager(::g_pMeshManager);


    ::g_pCommandDirector = new cCommandGroup();
    ::g_pCommandFactory = new cCommandFactory();
    // 
    // Tell the command factory about the phsyics and mesh stuff
    ::g_pCommandFactory->setPhysics(::g_pPhysicEngine);
    // (We are passing the address of this...)
    ::g_pCommandFactory->setVectorOfMeshes(&g_vecMeshesToDraw);

    // This also adds physics objects to the phsyics system
    AddModelsToScene(::g_pMeshManager, program);
     
    ::g_pFlyCamera = new cBasicFlyCamera();
    ::g_pFlyCamera->setEyeLocation(glm::vec3(450.0f, 0.0f, -900.0f));

    glUseProgram(program);

    // Enable depth buffering (z buffering)
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
    glEnable(GL_DEPTH_TEST);

    cLowPassFilter frameTimeFilter;
//    frameTimeFilter.setNumSamples(30000);

    double currentFrameTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();

    // Set up the lights
    ::g_pLightManager = new cLightManager();
    // Called only once
    ::g_pLightManager->loadUniformLocations(program);

    // Set up one of the lights in the scene
    ::g_pLightManager->theLights[0].position = glm::vec4(2'000.0f, 100'000.0f, 10'000.0f, 1.0f);
    ::g_pLightManager->theLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[0].atten.y = 0.000006877f;
    ::g_pLightManager->theLights[0].atten.z = 0.0000000001184f;

    ::g_pLightManager->theLights[0].param1.x = 0.0f;    // Point light (see shader)
    ::g_pLightManager->theLights[0].param2.x = 1.0f;    // Turn on (see shader)

    // Set up one of the lights in the scene
    ::g_pLightManager->theLights[1].position = glm::vec4(0.0f, 20.0f, 0.0f, 1.0f);
    ::g_pLightManager->theLights[1].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[1].atten.y = 0.01f;
    ::g_pLightManager->theLights[1].atten.z = 0.001f;

    ::g_pLightManager->theLights[1].param1.x = 1.0f;    // Spot light (see shader)
    ::g_pLightManager->theLights[1].direction = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    ::g_pLightManager->theLights[1].param1.y = 5.0f;   //  y = inner angle
    ::g_pLightManager->theLights[1].param1.z = 10.0f;  //  z = outer angle

    ::g_pLightManager->theLights[1].param2.x = 1.0f;    // Turn on (see shader)

    ::g_pTextures = new cBasicTextureManager();

    ::g_pTextures->SetBasePath("assets/textures");
    ::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("dua-lipa-promo.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Puzzle_parts.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Non-uniform concrete wall 0512-3-1024x1024.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("UV_Test_750x750.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("shape-element-splattered-texture-stroke_1194-8223.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Grey_Brick_Wall_Texture.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("dirty-metal-texture_1048-4784.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080_24bit_black_and_white.bmp");
    //
    ::g_pTextures->Create2DTextureFromBMPFile("SurprisedChildFace.bmp");

    // Load the space skybox
    std::string errorString;
    ::g_pTextures->SetBasePath("assets/textures/CubeMaps");
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("Space",
        "SpaceBox_right1_posX.bmp", 
        "SpaceBox_left2_negX.bmp",
        "SpaceBox_top3_posY.bmp", 
        "SpaceBox_bottom4_negY.bmp",
        "SpaceBox_front5_posZ.bmp", 
        "SpaceBox_back6_negZ.bmp", true, errorString))
    {
        std::cout << "Loaded space skybox" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space skybox because: " << errorString << std::endl;
    }

    // Load the sunny day cube map
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("SunnyDay",
        "TropicalSunnyDayLeft2048.bmp",
        "TropicalSunnyDayRight2048.bmp",
        "TropicalSunnyDayUp2048.bmp",
        "TropicalSunnyDayDown2048.bmp",
        "TropicalSunnyDayFront2048.bmp",
        "TropicalSunnyDayBack2048.bmp",
        true, errorString))
    {
        std::cout << "Loaded space SunnyDay" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space SunnyDay because: " << errorString << std::endl;
    }

    GLint iMaxCombinedTextureInmageUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxCombinedTextureInmageUnits);
    std::cout << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = " << iMaxCombinedTextureInmageUnits << std::endl;

    // data returns one value, the maximum number of components of the inputs read by the fragment shader, 
    // which must be at least 128.
    GLint iMaxFragmentInputComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &iMaxFragmentInputComponents);
    std::cout << "GL_MAX_FRAGMENT_INPUT_COMPONENTS = " << iMaxFragmentInputComponents << std::endl;
    

    // data returns one value, the maximum number of individual floating - point, integer, or boolean values 
    // that can be held in uniform variable storage for a fragment shader.The value must be at least 1024. 
    GLint iMaxFragmentUniformComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &iMaxFragmentUniformComponents);
    std::cout << "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = " << iMaxFragmentUniformComponents << std::endl;
        

    //  Turn on the blend operation
    glEnable(GL_BLEND);
    // Do alpha channel transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    cLightHelper TheLightHelper;

    // Is the default (cull back facing polygons)
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    sMesh* pCube1 = pFindMeshByFriendlyName("Cube_1");
    sMesh* pCube2 = pFindMeshByFriendlyName("Cube_2");
    sMesh* pCube3 = pFindMeshByFriendlyName("Cube_3");
    sMesh* pCube4 = pFindMeshByFriendlyName("Cube_4");
    sMesh* pCube5 = pFindMeshByFriendlyName("Cube_5");

    bool hasCarTriggeredCube1 = false;
    bool hasCarTriggeredCube2 = false;
    bool hasCarTriggeredCube3 = false;
    bool hasCarTriggeredCube4 = true;
    bool hasCarTriggeredCube5 = false;

    bool bIsF11Pressed = false;
    bool bIsYawSet = false;
    double timer = 0.0f;
    double totalTimeElapsed = 0.0f;

    std::vector<sMesh> vecAsteroids;
    for (int i = 0; i != 50; i++) {             // 50 stars
        vecAsteroids.push_back(*GiveRandomAsteroid());
        GenerateRandomCoordinates(vecAsteroids[i].positionXYZ.x, vecAsteroids[i].positionXYZ.y, vecAsteroids[i].positionXYZ.z);
    }

    float generationDelay = 0.0f;

    //sMesh* pCar = pFindMeshByFriendlyName("Car");
    //sMesh* pFalcon = pFindMeshByFriendlyName("falcon");
    //sMesh* pTieFighter1 = pFindMeshByFriendlyName("tieFighter");
    ////cPhysics::sPhysInfo* pTieFighter1Phys = new cPhysics::sPhysInfo();
    ////pTieFighter1Phys = g_pPhysicEngine->pFindAssociateMeshByFriendlyName("tieFighter");

    //sMesh* pTieFighter2 = new sMesh();
    //pTieFighter2 = pFindMeshByFriendlyName("tieFighter");
    //pTieFighter2->uniqueFriendlyName = "tieFighter2";
    //g_vecMeshesToDraw.push_back(pTieFighter2);

    //cPhysics::sPhysInfo* pTieFighter2Phys = new  cPhysics::sPhysInfo();
    //pTieFighter2Phys->bDoesntMove = true;
    //pTieFighter2Phys->position = pTieFighter2->positionXYZ;
    //pTieFighter2Phys->rotation = pTieFighter2->rotationEulerXYZ;
    //pTieFighter2Phys->velocity = glm::vec3(0.0f);
    //pTieFighter2Phys->pAssociatedDrawingMeshInstance = pTieFighter2;
    //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTieFighter2Phys);
    
    //sMesh* pTieFighter3 = new sMesh();
    //pTieFighter3 = pFindMeshByFriendlyName("tieFighter");
    //pTieFighter3->uniqueFriendlyName = "tieFighter3";
    //g_vecMeshesToDraw.push_back(pTieFighter3);

    //cPhysics::sPhysInfo* pTieFighter3Phys = new  cPhysics::sPhysInfo();
    //pTieFighter3Phys->bDoesntMove = true;
    //pTieFighter3Phys->position = pTieFighter3->positionXYZ;
    //pTieFighter3Phys->rotation = pTieFighter3->rotationEulerXYZ;
    //pTieFighter3Phys->velocity = glm::vec3(0.0f);
    //pTieFighter3Phys->pAssociatedDrawingMeshInstance = pTieFighter3;
    //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTieFighter3Phys);
    //
    //sMesh* pTieFighter4 = new sMesh();
    //pTieFighter4 = pFindMeshByFriendlyName("tieFighter");
    //pTieFighter4->uniqueFriendlyName = "tieFighter4";
    //g_vecMeshesToDraw.push_back(pTieFighter4);

    //cPhysics::sPhysInfo* pTieFighter4Phys = new  cPhysics::sPhysInfo();
    //pTieFighter4Phys->bDoesntMove = true;
    //pTieFighter4Phys->position = pTieFighter4->positionXYZ;
    //pTieFighter4Phys->rotation = pTieFighter4->rotationEulerXYZ;
    //pTieFighter4Phys->velocity = glm::vec3(0.0f);
    //pTieFighter4Phys->pAssociatedDrawingMeshInstance = pTieFighter4;
    //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTieFighter4Phys);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //        glm::mat4 m, p, v, mvp;
        glm::mat4 matProjection = glm::mat4(1.0f);

        matProjection = glm::perspective(0.6f,           // FOV
            ratio,          // Aspect ratio of screen
            0.1f,           // Near plane (as far from the camera as possible)
            1'000.0f);       // Far plane (as near to the camera as possible)
// For a "far" view of the large Galactica
//            1'000.1f,           // Near plane (as far from the camera as possible)
//            1'000'000.0f);       // Far plane (as near to the camera as possible)

        // View or "camera"
        glm::mat4 matView = glm::mat4(1.0f);

        //        glm::vec3 cameraEye = glm::vec3(0.0, 0.0, 4.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        matView = glm::lookAt(::g_pFlyCamera->getEyeLocation(),
            ::g_pFlyCamera->getTargetLocation(),
            upVector);
        //        matView = glm::lookAt( cameraEye,
        //                               cameraTarget,
        //                               upVector);


        const GLint matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        const GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);


        // Calculate elapsed time
        // We'll enhance this
        currentFrameTime = glfwGetTime();
        double tempDeltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Set a limit on the maximum frame time
        const double MAX_FRAME_TIME = 1.0 / 60.0;   // 60Hz (16 ms)
        if (tempDeltaTime > MAX_FRAME_TIME)
        {
            tempDeltaTime = MAX_FRAME_TIME;
        }

        // Add this sample to the low pass filer ("averager")
        frameTimeFilter.addSample(tempDeltaTime);
        // 
        double deltaTime = frameTimeFilter.getAverage();


        // **************************************************************
// Sky box
// Move the sky sphere with the camera
        sMesh* pSkySphere = pFindMeshByFriendlyName("SkySphere");
        pSkySphere->positionXYZ = ::g_pFlyCamera->getEyeLocation();

        // Disable backface culling (so BOTH sides are drawn)
        glDisable(GL_CULL_FACE);
        // Don't perform depth buffer testing
        glDisable(GL_DEPTH_TEST);
        // Don't write to the depth buffer when drawing to colour (back) buffer
//        glDepthMask(GL_FALSE);
//        glDepthFunc(GL_ALWAYS);// or GL_LESS (default)
        // GL_DEPTH_TEST : do or not do the test against what's already on the depth buffer

        pSkySphere->bIsVisible = true;
        //        pSkySphere->bDoNotLight = true;

        pSkySphere->uniformScale = 1.0f;

        // Tell the shader this is the skybox, so use the cube map
        // uniform samplerCube skyBoxTexture;
        // uniform bool bIsSkyBoxObject;
        GLuint bIsSkyBoxObject_UL = glGetUniformLocation(program, "bIsSkyBoxObject");
        glUniform1f(bIsSkyBoxObject_UL, (GLfloat)GL_TRUE);
        
        // Set the cube map texture, just like we do with the 2D
        GLuint cubeSamplerID = ::g_pTextures->getTextureIDFromName("Space");
//        GLuint cubeSamplerID = ::g_pTextures->getTextureIDFromName("SunnyDay");
        // Make sure this is an unused texture unit
        glActiveTexture(GL_TEXTURE0 + 40);
        // *****************************************
        // NOTE: This is a CUBE_MAP, not a 2D
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeSamplerID);
//        glBindTexture(GL_TEXTURE_2D, cubeSamplerID);
        // *****************************************
        GLint skyBoxTextureSampler_UL = glGetUniformLocation(program, "skyBoxTextureSampler");
        glUniform1i(skyBoxTextureSampler_UL, 40);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here


        DrawMesh(pSkySphere, program);

        pSkySphere->bIsVisible = false;

        glUniform1f(bIsSkyBoxObject_UL, (GLfloat)GL_FALSE);

        glEnable(GL_CULL_FACE);
        // Enable depth test and write to depth buffer (normal rendering)
        glEnable(GL_DEPTH_TEST);
        //        glDepthMask(GL_FALSE);
        //        glDepthFunc(GL_LESS);
                // **************************************************************

        ::g_pLightManager->updateShaderWithLightInfo();

        // *******************************************************************
        //    ____                       _                      
        //   |  _ \ _ __ __ ___      __ | |    ___   ___  _ __  
        //   | | | | '__/ _` \ \ /\ / / | |   / _ \ / _ \| '_ \ 
        //   | |_| | | | (_| |\ V  V /  | |__| (_) | (_) | |_) |
        //   |____/|_|  \__,_| \_/\_/   |_____\___/ \___/| .__/ 
        //                                               |_|            
        // // Will do two passes, one with "close" projection (clipping)
        // and one with "far away"

        matProjection = glm::perspective(0.6f,           // FOV
            ratio,          // Aspect ratio of screen
            0.1f,           // Near plane (as far from the camera as possible)
            50000.0f);       // Far plane (as near to the camera as possible)
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        // Draw all the objects
        for (unsigned int meshIndex = 0; meshIndex != ::g_vecMeshesToDraw.size(); meshIndex++)
        {
            //            sMesh* pCurMesh = ::g_myMeshes[meshIndex];
           sMesh* pCurMesh = ::g_vecMeshesToDraw[meshIndex];
//            pCurMesh->bDoNotLight = true;
            DrawMesh(pCurMesh, program);

        }//for (unsigned int meshIndex..

        // Draw the LASER beam
        cPhysics::sLine LASERbeam;
        glm::vec3 LASERbeam_Offset = glm::vec3(0.0f, -2.0f, 0.0f);

        if (::g_bShowLASERBeam)
        {
            // Draw a bunch of little balls along a line from the camera
            //  to some place in the distance

            // The fly camera is always "looking at" something 1.0 unit away
            glm::vec3 cameraDirection = ::g_pFlyCamera->getTargetRelativeToCamera();     //0,0.1,0.9

            LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();

            // Move the LASER below the camera
            LASERbeam.startXYZ += LASERbeam_Offset;
            glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

            // Is the LASER less than 500 units long?
            // (is the last LAZER ball we drew beyond 500 units form the camera?)
            while ( glm::distance(::g_pFlyCamera->getEyeLocation(), LASER_ball_location) < 150.0f )
            {
                // Move the next ball 0.1 times the normalized camera direction
                LASER_ball_location += (cameraDirection * 0.10f);  
                //DrawDebugSphere(LASER_ball_location, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.05f, program);
            }

            // Set the end of the LASER to the last location of the beam
            LASERbeam.endXYZ = LASER_ball_location;

        }//if (::g_bShowLASERBeam)

        // Draw the end of this LASER beam
        //DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

        // Now draw a different coloured ball wherever we get a collision with a triangle
        std::vector<cPhysics::sCollision_RayTriangleInMesh> vec_RayTriangle_Collisions;
        ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, vec_RayTriangle_Collisions, false);

        glm::vec4 triColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        float triangleSize = 0.25f;

        for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions.begin();
            itTriList != vec_RayTriangle_Collisions.end(); itTriList++)
        {
            for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles.begin();
                itTri != itTriList->vecTriangles.end(); itTri++)
            {
                // Draw a sphere at the centre of the triangle
//                glm::vec3 triCentre = (itTri->vertices[0] + itTri->vertices[1] + itTri->vertices[2]) / 3.0f;
//                DrawDebugSphere(triCentre, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.5f, program);

//                DrawDebugSphere(itTri->intersectionPoint, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 0.25f, program);
                //DrawDebugSphere(itTri->intersectionPoint, triColour, triangleSize, program);
                triColour.r -= 0.1f;
                triColour.g -= 0.1f;
                triColour.b += 0.2f;
                triangleSize *= 1.25f;


            }//for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles

        }//for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions


        //-------------------------------------------------------------------Asteroid Generation-----------------------------------------------------
        if (deltaTime <= 1.0f) {
            generationDelay += deltaTime;
        }
        if (generationDelay > 0.2f) {
            GenerateRandomCoordinates(GiveRandomAsteroid()->positionXYZ.x, GiveRandomAsteroid()->positionXYZ.y, GiveRandomAsteroid()->positionXYZ.z);
            vecAsteroids.push_back(*GiveRandomAsteroid());
            generationDelay = 0.0f;
        }

        for (int i = 0; i != vecAsteroids.size(); i++) {             // 50 stars
            //GenerateRandomCoordinates(vecAsteroids[i]->positionXYZ.x, vecAsteroids[i]->positionXYZ.y, vecAsteroids[i]->positionXYZ.z);
            vecAsteroids[i].positionXYZ.x -= 350 * deltaTime;
            DrawMesh(&vecAsteroids[i], program);
        }
        //-------------------------------------------------------------------Asteroid Generation-----------------------------------------------------
       



        //----------------------------------------------------------Animation--------------------------------------------------------------------
        // Scene 1
        if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
            bIsF11Pressed = true;
        }
        if (bIsF11Pressed) {
            //::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('SET_CAMERA', 'camera', 1200.0, 0.0, -900.0, 1)");
            //::g_pFlyCamera->setEyeLocation(1200.0, 0.0, -900.0);
            timer += deltaTime;
        }
        if (timer > 0.0f && timer < 1.0f ) {
            ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'falcon', -115.0, 0.0, 90.0, 5)");
        }
        if (timer >= 1.0f && timer < 2.0f) {
            ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'falcon', -75.0, 0.0, 90.0, 5)");
        }
        if (timer >= 2.0f && timer < 3.0f) {
            ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'falcon', -90.0, 0.0, 90.0, 5)");
        }
        if (bIsF11Pressed && timer >= 3.0f) {
            totalTimeElapsed += timer;
            timer = totalTimeElapsed;
            bIsF11Pressed = false;
            std::cout << "totalTimeElapsed: " << totalTimeElapsed << std::endl;
        }

        // Scene 2
        if (totalTimeElapsed >= 3.0f && totalTimeElapsed < 8.0f) {
            std::ifstream luaCommandFile("Scene2.txt");
            // Read this file and send each command to lua
            if (luaCommandFile.is_open())
            {
                char lua_commandLine[2048] = { 0 }; // buffer of 2048 chars
                while (luaCommandFile.getline(lua_commandLine, 2048))
                {
                    // If the 1st character is a "~" then skip
                    // So we can 'comment out' certain commands
                    if (lua_commandLine[0] != '~')
                    {
                        ::g_pMyLuaMasterBrain->RunScriptImmediately(std::string(lua_commandLine));
                    }
                }
            }
            timer += deltaTime;
            if (!bIsYawSet) {
                g_pFlyCamera->rotateLeftRight_Yaw_NoScaling(-1.56);
                bIsYawSet = true;
            }
            if (timer >= 3.0f && timer < 4.25f) {
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'falcon', -165.0, 0.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter', -165.0, -165.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter2', -135.0, -135.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter3', -145.0, -145.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter4', -185.0, -185.0, 90.0, 5)");
            }
            if (timer >= 4.25f && timer < 5.5f) {
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'falcon', -90.0, 0.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter', -90.0, -90.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter2', -95.0, -95.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter3', -80.0, -80.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter4', -105.0, -105.0, 90.0, 5)");
            }
            if (timer >= 5.5f && timer < 8.0f) {
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'falcon', -55.0, 0.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter', -55.0, -55.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter2', -65.0, -65.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter3', -45.0, -45.0, 90.0, 5)");
                ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'tieFighter4', -50.0, -50.0, 90.0, 5)");
            }
            if (timer >= 8.0f && totalTimeElapsed >= 3.0f) {
                totalTimeElapsed += timer;
                timer = totalTimeElapsed;
                bIsYawSet = false;
            }
            std::cout << "totalTimeElapsed: " << totalTimeElapsed << std::endl;
        }
        
        // Scene 3
        if (totalTimeElapsed >= 11.0f && totalTimeElapsed < 16.0f) {
            std::ifstream luaCommandFile("Scene3.txt");
            // Read this file and send each command to lua
            if (luaCommandFile.is_open())
            {
                char lua_commandLine[2048] = { 0 }; // buffer of 2048 chars
                while (luaCommandFile.getline(lua_commandLine, 2048))
                {
                    // If the 1st character is a "~" then skip
                    // So we can 'comment out' certain commands
                    if (lua_commandLine[0] != '~')
                    {
                        ::g_pMyLuaMasterBrain->RunScriptImmediately(std::string(lua_commandLine));
                    }
                }
            }
            timer += deltaTime;
            if (!bIsYawSet) {
                g_pFlyCamera->rotateLeftRight_Yaw_NoScaling(2.15);
                bIsYawSet = true;
            }
            // 2000, -90, 742
            std::cout << "timer: " << timer << std::endl;

        }

        //----------------------------------------------------------Animation--------------------------------------------------------------------



                // *******************************************************************
        // Have camera follow the viper at a set distance
        //{
        //    cPhysics::sPhysInfo* pEntityPhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("New_Viper_Player");
        //    if (pEntityPhys)
        //    {
        //        const float MAX_DISTANCE_FROM_ENTITY = 50.0f;
        //        const float MAX_CAMERA_SPEED = 10.0f;
        //        // How far away are we?
        //        if (glm::distance(pEntityPhys->position, ::g_pFlyCamera->getEyeLocation()) > MAX_DISTANCE_FROM_ENTITY)
        //        {
        //            // Too far from viper.
        //            glm::vec3 vecCameraToEntity = pEntityPhys->position - ::g_pFlyCamera->getEyeLocation();

        //            glm::vec3 cameraDirection = glm::normalize(vecCameraToEntity);

        //            glm::vec3 cameraSpeed = cameraDirection * MAX_CAMERA_SPEED;

        //            ::g_pFlyCamera->moveForward(cameraSpeed.z);
        //            ::g_pFlyCamera->moveLeftRight(cameraSpeed.x);
        //            ::g_pFlyCamera->moveUpDown(cameraSpeed.y);
        //        }
        //    }//if (pViperPhys)
        //}
        //        // *******************************************************************



        //if (IsMeshInsideTriggerBox(pCar, pCube1))
        //{
        //    if (!hasCarTriggeredCube1)
        //    {
        //        std::cout << "Car Hit Cube 1\n";
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'Car', -90.0, 0.0, 0.0, 1)");
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddSerialCommand('MOVE_TO', 'Car', -59.0, 2.0, 18.0, 5)");
        //        hasCarTriggeredCube1 = true;
        //        hasCarTriggeredCube4 = false;
        //    }
        //}
        //else
        //{
        //    hasCarTriggeredCube1 = false;
        //}

        //if (IsMeshInsideTriggerBox(pCar, pCube2))
        //{
        //    if (!hasCarTriggeredCube2)
        //    {
        //        std::cout << "Car Hit Cube 2\n";
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'Car', -90.0, 0.0, -90.0, 1)");
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddSerialCommand('MOVE_TO', 'Car', -59.0, 2.0, -49.0, 3)");
        //        hasCarTriggeredCube2 = true;
        //    }
        //}
        //else
        //{
        //    hasCarTriggeredCube2 = false;
        //}

        //if (IsMeshInsideTriggerBox(pCar, pCube3))
        //{
        //    if (!hasCarTriggeredCube3)
        //    {
        //        std::cout << "Car Hit Cube 3\n";
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'Car', -90.0, 0.0, 180.0, 1)");
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddSerialCommand('MOVE_TO', 'Car', 21.0, 2.0, -50.0, 5)");
        //        hasCarTriggeredCube3 = true;
        //    }
        //}
        //else
        //{
        //    hasCarTriggeredCube3 = false;
        //}

        //if (IsMeshInsideTriggerBox(pCar, pCube4))
        //{
        //    if (!hasCarTriggeredCube4)
        //    {
        //        std::cout << "Car Hit Cube 4\n";
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO','Car', -90.0, 0.0, 90.0, 1)");
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddSerialCommand('MOVE_TO', 'Car', 21.0, 2.0, 18.0, 8.0)");
        //        hasCarTriggeredCube4 = true;
        //    }
        //}
        //else
        //{
        //    hasCarTriggeredCube4 = false;
        //}


        //if (IsMeshInsideTriggerBox(pCar, pCube5))
        //{
        //    if (!hasCarTriggeredCube5)
        //    {
        //        std::cout << "Car Hit Cube 5\n";
        //        //::g_pMyLuaMasterBrain->RunScriptImmediately("AddParallelCommand('ORIENT_TO', 'Car', -90.0, 0.0, -90.0, 1)");
        //        //::g_pMyLuaMasterBrain->RunScriptImmediately("AddSerialCommand('MOVE_TO', 'Car', -59.0, 2.0, -49.0, 3)");
        //        ::g_pMyLuaMasterBrain->RunScriptImmediately("AddSerialCommand('FOLLOW_OBJECT', 'Car_Enemy', 'Car', 1,0,1, 15)");
        //    
        //        hasCarTriggeredCube5 = true;
        //    }
        //}
        //else
        //{
        //    hasCarTriggeredCube5 = false;
        //}


        // **********************************************************************************
        if (::g_bShowDebugSpheres)
        {

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            const float DEBUG_LIGHT_BRIGHTNESS = 0.3f;

            const float ACCURACY = 0.1f;       // How many units distance
            float distance_75_percent =
                TheLightHelper.calcApproxDistFromAtten(0.75f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(DEBUG_LIGHT_BRIGHTNESS, 0.0f, 0.0f, 1.0f),
                distance_75_percent,
                program);


            float distance_50_percent =
                TheLightHelper.calcApproxDistFromAtten(0.5f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(0.0f, DEBUG_LIGHT_BRIGHTNESS, 0.0f, 1.0f),
                distance_50_percent,
                program);

            float distance_25_percent =
                TheLightHelper.calcApproxDistFromAtten(0.25f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(0.0f, 0.0f, DEBUG_LIGHT_BRIGHTNESS, 1.0f),
                distance_25_percent,
                program);

            float distance_05_percent =
                TheLightHelper.calcApproxDistFromAtten(0.05f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(DEBUG_LIGHT_BRIGHTNESS, DEBUG_LIGHT_BRIGHTNESS, 0.0f, 1.0f),
                distance_05_percent,
                program);

        }
        // **********************************************************************************

        sMesh* pDebugAABB = pFindMeshByFriendlyName("AABB_MinXYZ_At_Origin");
        if (pDebugAABB)
        {
            pDebugAABB->bIsVisible = true;
            pDebugAABB->uniformScale = 1'000.0f;

            for (std::map< unsigned long long, cPhysics::cBroad_Cube* >::iterator
                it_pCube = ::g_pPhysicEngine->map_BP_CubeGrid.begin();
                it_pCube != ::g_pPhysicEngine->map_BP_CubeGrid.end();
                it_pCube++)
            {

                // Draw a cube at that location
                pDebugAABB->positionXYZ = it_pCube->second->getMinXYZ();
                DrawMesh(pDebugAABB, program);

            }

            pDebugAABB->bIsVisible = false;
        }//if (pDebugAABB)



        //// HACK: Update "shadow" of ball to be where the ball hits the large block ground
        //sMesh* pBallShadow = pFindMeshByFriendlyName("Ball_Shadow");
        //sMesh* pBall = pFindMeshByFriendlyName("Ball");
        //pBallShadow->positionXYZ.x = pBall->positionXYZ.x;
        //pBallShadow->positionXYZ.z = pBall->positionXYZ.z;
        //// Don't update the y - keep the shadow near the plane

        // Physic update and test 
        ::g_pPhysicEngine->StepTick(deltaTime);

        // Update the commands, too
        ::g_pCommandDirector->Update(deltaTime);

        // Handle any collisions
        if (::g_pPhysicEngine->vec_SphereAABB_Collisions.size() > 0 )
        {
            // Yes, there were collisions

            for (unsigned int index = 0; index != ::g_pPhysicEngine->vec_SphereAABB_Collisions.size(); index++)
            {
                cPhysics::sCollision_SphereAABB thisCollisionEvent = ::g_pPhysicEngine->vec_SphereAABB_Collisions[index];

                if (thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y  < 0.0f)
                {
                    // Yes, it's heading down
                    // So reverse the direction of velocity
                    thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y = fabs(thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y);
                }

            }//for (unsigned int index
 
        }//if (::g_pPhysicEngine->vec_SphereAABB_Collisions

        // Point the spot light to the ball
        sMesh* pBouncy_5_Ball = pFindMeshByFriendlyName("Bouncy_5");
        if (pBouncy_5_Ball)
        {
            glm::vec3 directionToBal
                = pBouncy_5_Ball->positionXYZ - glm::vec3(::g_pLightManager->theLights[1].position);
    
            // Normalize to get the direction only
            directionToBal = glm::normalize(directionToBal);

            // Point the spot light at the bouncy ball
            ::g_pLightManager->theLights[1].direction = glm::vec4(directionToBal, 1.0f);
        }

        // Handle async IO stuff
        handleKeyboardAsync(window);
        handleMouseAsync(window);

        glfwSwapBuffers(window);
        glfwPollEvents();

        //std::cout << "Camera: "
        std::stringstream ssTitle;
        ssTitle << "Camera: "
            << ::g_pFlyCamera->getEyeLocation().x << ", "
            << ::g_pFlyCamera->getEyeLocation().y << ", "
            << ::g_pFlyCamera->getEyeLocation().z 
            << "   ";

        ssTitle << "Yaw: " << g_pFlyCamera->getYaw() << " Pitch: " << g_pFlyCamera->getPitch() << " ";
        //ssTitle << "light[" << g_selectedLightIndex << "] "
        //    << ::g_pLightManager->theLights[g_selectedLightIndex].position.x << ", "
        //    << ::g_pLightManager->theLights[g_selectedLightIndex].position.y << ", "
        //    << ::g_pLightManager->theLights[g_selectedLightIndex].position.z
        //    << "   "
        //    << "linear: " << ::g_pLightManager->theLights[0].atten.y
        //    << "   "
        //    << "quad: " << ::g_pLightManager->theLights[0].atten.z;

        // Add the viper info, too
        //cPhysics::sPhysInfo* pCarPhysics = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("Car");
        cPhysics::sPhysInfo* pFalconPhysics = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("falcon");
        if (pFalconPhysics)
        {
            ssTitle
                << " Falcon XYZ:" << getStringVec3(pFalconPhysics->position)
                << " vel:" << getStringVec3(pFalconPhysics->velocity)
                << " acc:" << getStringVec3(pFalconPhysics->acceleration)
                << " ";

        }//if (pViperPhys)

        //cPhysics::sPhysInfo* pViperPhys1 = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("New_Viper_Player_1");
        //if (pViperPhys1)
        //{
        //    ssTitle
        //        << " Viper1 XYZ:" << getStringVec3(pViperPhys1->position)
        //        << " vel:" << getStringVec3(pViperPhys1->velocity)
        //        << " acc:" << getStringVec3(pViperPhys1->acceleration);

        //}//if (pViperPhys)

        // Show frame time
        //ssTitle << " deltaTime = " << deltaTime
        //    << " FPS: " << 1.0 / deltaTime;

 //       std::cout << " deltaTime = " << deltaTime << " FPS: " << 1.0 / deltaTime << std::endl;


//        glfwSetWindowTitle(window, "Hey!");
        glfwSetWindowTitle(window, ssTitle.str().c_str());


    }// End of the draw loop


    // Delete everything
    delete ::g_pFlyCamera;
    delete ::g_pPhysicEngine;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void AddModelsToScene(cVAOManager* pMeshManager, GLuint program)
{
    {
        sModelDrawInfo falcon;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/Millennium Falcon/Falcon_Body_553867_verts_uv.ply",falcon, program, true);
        std::cout << falcon.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pFalcon = new sMesh();
        pFalcon->modelFileName = "Model_assets/assets/Ply_files/Millennium Falcon/Falcon_Body_553867_verts_uv.ply";
        pFalcon->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pFalcon->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pFalcon->uniformScale = 1.0f;
        pFalcon->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pFalcon->bOverrideObjectColour = true;
        pFalcon->uniqueFriendlyName = "falcon";
        pFalcon->bIsVisible = true;
        pFalcon->bDoNotLight = false;
        pFalcon->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pFalcon->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pFalcon);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pFalconPhys = new  cPhysics::sPhysInfo();
        pFalconPhys->bDoesntMove = false;
        pFalconPhys->position = pFalcon->positionXYZ;
        pFalconPhys->rotation = pFalcon->rotationEulerXYZ;
        pFalconPhys->velocity = glm::vec3(0.0f);
        pFalconPhys->pAssociatedDrawingMeshInstance = pFalcon;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pFalconPhys);
    }

    {
        sModelDrawInfo asteroid1;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/Asteroids/Asteroid_001_rgba_n_uv.ply", asteroid1, program, true);
        std::cout << asteroid1.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pAsteroid1 = new sMesh();
        pAsteroid1->modelFileName = "Model_assets/assets/Ply_files/Asteroids/Asteroid_001_rgba_n_uv.ply";
        pAsteroid1->positionXYZ = glm::vec3(10000.0f, 10000.0f, 10000.0f);
        pAsteroid1->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pAsteroid1->uniformScale = 0.2f;
        pAsteroid1->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pAsteroid1->bOverrideObjectColour = true;
        pAsteroid1->uniqueFriendlyName = "asteroid1";
        pAsteroid1->bIsVisible = true;
        pAsteroid1->bDoNotLight = false;
        pAsteroid1->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pAsteroid1->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pAsteroid1);

        // Add a associated physics object to have the phsyics "move" this
        //cPhysics::sPhysInfo* pAsteroid1Phys = new  cPhysics::sPhysInfo();
        //pAsteroid1Phys->bDoesntMove = true;
        //pAsteroid1Phys->position = pAsteroid1->positionXYZ;
        //pAsteroid1Phys->rotation = pAsteroid1->rotationEulerXYZ;
        //pAsteroid1Phys->velocity = glm::vec3(0.0f);
        //pAsteroid1Phys->pAssociatedDrawingMeshInstance = pAsteroid1;
        //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pAsteroid1Phys);
    }

    {
        sModelDrawInfo asteroid2;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/Asteroids/Asteroid_002_rgba_n_uv.ply", asteroid2, program, true);
        std::cout << asteroid2.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pAsteroid2 = new sMesh();
        pAsteroid2->modelFileName = "Model_assets/assets/Ply_files/Asteroids/Asteroid_002_rgba_n_uv.ply";
        pAsteroid2->positionXYZ = glm::vec3(10000.0f, 10000.0f, 10000.0f);
        pAsteroid2->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pAsteroid2->uniformScale = 0.2f;
        pAsteroid2->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pAsteroid2->bOverrideObjectColour = true;
        pAsteroid2->uniqueFriendlyName = "asteroid2";
        pAsteroid2->bIsVisible = true;
        pAsteroid2->bDoNotLight = false;
        pAsteroid2->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pAsteroid2->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pAsteroid2);

        // Add a associated physics object to have the phsyics "move" this
        //cPhysics::sPhysInfo* pAsteroid1Phys = new  cPhysics::sPhysInfo();
        //pAsteroid1Phys->bDoesntMove = true;
        //pAsteroid1Phys->position = pAsteroid1->positionXYZ;
        //pAsteroid1Phys->rotation = pAsteroid1->rotationEulerXYZ;
        //pAsteroid1Phys->velocity = glm::vec3(0.0f);
        //pAsteroid1Phys->pAssociatedDrawingMeshInstance = pAsteroid1;
        //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pAsteroid1Phys);
    }

    {
        sModelDrawInfo asteroid3;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/Asteroids/Asteroid_003_rgba_n_uv.ply", asteroid3, program, true);
        std::cout << asteroid3.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pAsteroid3 = new sMesh();
        pAsteroid3->modelFileName = "Model_assets/assets/Ply_files/Asteroids/Asteroid_003_rgba_n_uv.ply";
        pAsteroid3->positionXYZ = glm::vec3(10000.0f, 10000.0f, 10000.0f);
        pAsteroid3->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pAsteroid3->uniformScale = 0.2f;
        pAsteroid3->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pAsteroid3->bOverrideObjectColour = true;
        pAsteroid3->uniqueFriendlyName = "asteroid3";
        pAsteroid3->bIsVisible = true;
        pAsteroid3->bDoNotLight = false;
        pAsteroid3->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pAsteroid3->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pAsteroid3);

        // Add a associated physics object to have the phsyics "move" this
        //cPhysics::sPhysInfo* pAsteroid1Phys = new  cPhysics::sPhysInfo();
        //pAsteroid1Phys->bDoesntMove = true;
        //pAsteroid1Phys->position = pAsteroid1->positionXYZ;
        //pAsteroid1Phys->rotation = pAsteroid1->rotationEulerXYZ;
        //pAsteroid1Phys->velocity = glm::vec3(0.0f);
        //pAsteroid1Phys->pAssociatedDrawingMeshInstance = pAsteroid1;
        //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pAsteroid1Phys);
    }

    {
        sModelDrawInfo asteroid4;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/Asteroids/Asteroid_004_rgba_n_uv.ply", asteroid4, program, true);
        std::cout << asteroid4.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pAsteroid4 = new sMesh();
        pAsteroid4->modelFileName = "Model_assets/assets/Ply_files/Asteroids/Asteroid_004_rgba_n_uv.ply";
        pAsteroid4->positionXYZ = glm::vec3(10000.0f, 10000.0f, 10000.0f);
        pAsteroid4->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pAsteroid4->uniformScale = 0.2f;
        pAsteroid4->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pAsteroid4->bOverrideObjectColour = true;
        pAsteroid4->uniqueFriendlyName = "asteroid4";
        pAsteroid4->bIsVisible = true;
        pAsteroid4->bDoNotLight = false;
        pAsteroid4->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pAsteroid4->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pAsteroid4);

        // Add a associated physics object to have the phsyics "move" this
        //cPhysics::sPhysInfo* pAsteroid1Phys = new  cPhysics::sPhysInfo();
        //pAsteroid1Phys->bDoesntMove = true;
        //pAsteroid1Phys->position = pAsteroid1->positionXYZ;
        //pAsteroid1Phys->rotation = pAsteroid1->rotationEulerXYZ;
        //pAsteroid1Phys->velocity = glm::vec3(0.0f);
        //pAsteroid1Phys->pAssociatedDrawingMeshInstance = pAsteroid1;
        //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pAsteroid1Phys);
    }

    {
        sModelDrawInfo asteroid5;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/Asteroids/Asteroid_005_rgba_n_uv.ply", asteroid5, program, true);
        std::cout << asteroid5.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pAsteroid5 = new sMesh();
        pAsteroid5->modelFileName = "Model_assets/assets/Ply_files/Asteroids/Asteroid_005_rgba_n_uv.ply";
        pAsteroid5->positionXYZ = glm::vec3(10000.0f, 10000.0f, 10000.0f);
        pAsteroid5->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pAsteroid5->uniformScale = 0.2f;
        pAsteroid5->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pAsteroid5->bOverrideObjectColour = true;
        pAsteroid5->uniqueFriendlyName = "asteroid5";
        pAsteroid5->bIsVisible = true;
        pAsteroid5->bDoNotLight = false;
        pAsteroid5->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pAsteroid5->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pAsteroid5);

        // Add a associated physics object to have the phsyics "move" this
        //cPhysics::sPhysInfo* pAsteroid1Phys = new  cPhysics::sPhysInfo();
        //pAsteroid1Phys->bDoesntMove = true;
        //pAsteroid1Phys->position = pAsteroid1->positionXYZ;
        //pAsteroid1Phys->rotation = pAsteroid1->rotationEulerXYZ;
        //pAsteroid1Phys->velocity = glm::vec3(0.0f);
        //pAsteroid1Phys->pAssociatedDrawingMeshInstance = pAsteroid1;
        //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pAsteroid1Phys);
    }

    {
        sModelDrawInfo asteroidGroup;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/Asteroids/asteroid_group_003830_verts_rgba_n_uv.ply", asteroidGroup, program, true);
        std::cout << asteroidGroup.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pAsteroidGroup = new sMesh();
        pAsteroidGroup->modelFileName = "Model_assets/assets/Ply_files/Asteroids/asteroid_group_003830_verts_rgba_n_uv.ply";
        pAsteroidGroup->positionXYZ = glm::vec3(10000.0f, 10000.0f, 10000.0f);
        pAsteroidGroup->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pAsteroidGroup->uniformScale = 0.2f;
        pAsteroidGroup->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pAsteroidGroup->bOverrideObjectColour = true;
        pAsteroidGroup->uniqueFriendlyName = "asteroidGroup";
        pAsteroidGroup->bIsVisible = true;
        pAsteroidGroup->bDoNotLight = false;
        pAsteroidGroup->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pAsteroidGroup->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pAsteroidGroup);

        // Add a associated physics object to have the phsyics "move" this
        //cPhysics::sPhysInfo* pAsteroid1Phys = new  cPhysics::sPhysInfo();
        //pAsteroid1Phys->bDoesntMove = true;
        //pAsteroid1Phys->position = pAsteroid1->positionXYZ;
        //pAsteroid1Phys->rotation = pAsteroid1->rotationEulerXYZ;
        //pAsteroid1Phys->velocity = glm::vec3(0.0f);
        //pAsteroid1Phys->pAssociatedDrawingMeshInstance = pAsteroid1;
        //g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pAsteroid1Phys);
    }

    {
        sModelDrawInfo tieFighter;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply", tieFighter, program, true);
        std::cout << tieFighter.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pTieFighter = new sMesh();
        pTieFighter->modelFileName = "Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply";
        pTieFighter->positionXYZ = glm::vec3(-6800.0, 0.0, -220.0);
        pTieFighter->rotationEulerXYZ = glm::vec3(0.0f, 90.0f, 0.0f);
        pTieFighter->uniformScale = 1.0f;
        pTieFighter->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTieFighter->bOverrideObjectColour = true;
        pTieFighter->uniqueFriendlyName = "tieFighter";
        pTieFighter->bIsVisible = true;
        pTieFighter->bDoNotLight = false;
        pTieFighter->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pTieFighter->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTieFighter);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTieFighterPhys = new cPhysics::sPhysInfo();
        pTieFighterPhys->bDoesntMove = false;
        pTieFighterPhys->position = pTieFighter->positionXYZ;
        pTieFighterPhys->rotation = pTieFighter->rotationEulerXYZ;
        pTieFighterPhys->velocity = glm::vec3(0.0f);
        pTieFighterPhys->pAssociatedDrawingMeshInstance = pTieFighter;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTieFighterPhys);
    }

    {
        sModelDrawInfo tieFighter2;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply", tieFighter2, program, true);
        std::cout << tieFighter2.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pTieFighter2 = new sMesh();
        pTieFighter2->modelFileName = "Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply";
        pTieFighter2->positionXYZ = glm::vec3(-6800.0, 0.0, -420.0);
        pTieFighter2->rotationEulerXYZ = glm::vec3(0.0f, 90.0f, 0.0f);
        pTieFighter2->uniformScale = 1.0f;
        pTieFighter2->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTieFighter2->bOverrideObjectColour = true;
        pTieFighter2->uniqueFriendlyName = "tieFighter2";
        pTieFighter2->bIsVisible = true;
        pTieFighter2->bDoNotLight = false;
        pTieFighter2->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pTieFighter2->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTieFighter2);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTieFighter2Phys = new cPhysics::sPhysInfo();
        pTieFighter2Phys->bDoesntMove = false;
        pTieFighter2Phys->position = pTieFighter2->positionXYZ;
        pTieFighter2Phys->rotation = pTieFighter2->rotationEulerXYZ;
        pTieFighter2Phys->velocity = glm::vec3(0.0f);
        pTieFighter2Phys->pAssociatedDrawingMeshInstance = pTieFighter2;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTieFighter2Phys);
    }

    {
        sModelDrawInfo tieFighter3;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply", tieFighter3, program, true);
        std::cout << tieFighter3.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pTieFighter3 = new sMesh();
        pTieFighter3->modelFileName = "Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply";
        pTieFighter3->positionXYZ = glm::vec3(-6800.0, 300.0, -220.0);
        pTieFighter3->rotationEulerXYZ = glm::vec3(0.0f, 90.0f, 0.0f);
        pTieFighter3->uniformScale = 1.0f;
        pTieFighter3->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTieFighter3->bOverrideObjectColour = true;
        pTieFighter3->uniqueFriendlyName = "tieFighter3";
        pTieFighter3->bIsVisible = true;
        pTieFighter3->bDoNotLight = false;
        pTieFighter3->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pTieFighter3->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTieFighter3);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTieFighter3Phys = new cPhysics::sPhysInfo();
        pTieFighter3Phys->bDoesntMove = false;
        pTieFighter3Phys->position = pTieFighter3->positionXYZ;
        pTieFighter3Phys->rotation = pTieFighter3->rotationEulerXYZ;
        pTieFighter3Phys->velocity = glm::vec3(0.0f);
        pTieFighter3Phys->pAssociatedDrawingMeshInstance = pTieFighter3;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTieFighter3Phys);
    }

    {
        sModelDrawInfo tieFighter4;
        ::g_pMeshManager->LoadModelIntoVAO("Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply", tieFighter4, program, true);
        std::cout << tieFighter4.numberOfVertices << " vertices loaded" << std::endl;
    }
    {
        sMesh* pTieFighter4 = new sMesh();
        pTieFighter4->modelFileName = "Model_assets/assets/Ply_files/TieFighter/TIE-fighter_xyz_n_rgba_130744_verts_uv.ply";
        pTieFighter4->positionXYZ = glm::vec3(-6800.0, -350.0, -220.0);
        pTieFighter4->rotationEulerXYZ = glm::vec3(0.0f, 90.0f, 0.0f);
        pTieFighter4->uniformScale = 1.0f;
        pTieFighter4->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTieFighter4->bOverrideObjectColour = true;
        pTieFighter4->uniqueFriendlyName = "tieFighter4";
        pTieFighter4->bIsVisible = true;
        pTieFighter4->bDoNotLight = false;
        pTieFighter4->bIsWireframe = false;
        //pFalcon->textures[0] = "Cube.bmp";
        pTieFighter4->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTieFighter4);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTieFighter4Phys = new cPhysics::sPhysInfo();
        pTieFighter4Phys->bDoesntMove = false;
        pTieFighter4Phys->position = pTieFighter4->positionXYZ;
        pTieFighter4Phys->rotation = pTieFighter4->rotationEulerXYZ;
        pTieFighter4Phys->velocity = glm::vec3(0.0f);
        pTieFighter4Phys->pAssociatedDrawingMeshInstance = pTieFighter4;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTieFighter4Phys);
    }

    //{
    //    sModelDrawInfo cubeMinXYZ_at_OriginInfo;
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Cube_MinXYZ_at_Origin_xyz_n_uv.ply",
    //        cubeMinXYZ_at_OriginInfo, program);
    //    std::cout << cubeMinXYZ_at_OriginInfo.meshName << ": " << cubeMinXYZ_at_OriginInfo.numberOfVertices << " vertices loaded" << std::endl;
    //}

    //Cube Centre
    {
        sModelDrawInfo cubeCentre;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Cube_Centre.ply",
            cubeCentre, program, false);
        std::cout << cubeCentre.meshName << ": " << cubeCentre.numberOfVertices << " vertices loaded" << std::endl;
    }

    //Trigger Cube 1
    {
        sMesh* pTriggerCube1 = new sMesh();
        pTriggerCube1->modelFileName = "assets/models/Cube_Centre.ply";
        pTriggerCube1->positionXYZ = glm::vec3(20.7f, 2.0f, 18.7f);
        pTriggerCube1->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pTriggerCube1->uniformScale = 5.0f;
        pTriggerCube1->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTriggerCube1->bOverrideObjectColour = true;
        pTriggerCube1->uniqueFriendlyName = "Cube_1";
        pTriggerCube1->bIsVisible = true;
        pTriggerCube1->bDoNotLight = true;
        pTriggerCube1->bIsWireframe = true;
        pTriggerCube1->textures[0] = "Cube.bmp";
        pTriggerCube1->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTriggerCube1);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTriggerCube1PhysicsObject = new  cPhysics::sPhysInfo();
        pTriggerCube1PhysicsObject->bDoesntMove = true;
        pTriggerCube1PhysicsObject->position = pTriggerCube1->positionXYZ;
        pTriggerCube1PhysicsObject->rotation = pTriggerCube1->rotationEulerXYZ;
        pTriggerCube1PhysicsObject->velocity = glm::vec3(0.0f);
        pTriggerCube1PhysicsObject->pAssociatedDrawingMeshInstance = pTriggerCube1;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTriggerCube1PhysicsObject);
    }

    //Trigger Cube 2
    {
        sMesh* pTriggerCube2 = new sMesh();
        pTriggerCube2->modelFileName = "assets/models/Cube_Centre.ply";
        pTriggerCube2->positionXYZ = glm::vec3(-57.5f, 2.0f, 18.7f);
        pTriggerCube2->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pTriggerCube2->uniformScale = 5.0f;
        pTriggerCube2->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTriggerCube2->bOverrideObjectColour = true;
        pTriggerCube2->uniqueFriendlyName = "Cube_2";
        pTriggerCube2->bIsVisible = true;
        pTriggerCube2->bDoNotLight = true;
        pTriggerCube2->bIsWireframe = true;
        pTriggerCube2->textures[0] = "Cube.bmp";
        pTriggerCube2->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTriggerCube2);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTriggerCube2PhysicsObject = new  cPhysics::sPhysInfo();
        pTriggerCube2PhysicsObject->bDoesntMove = true;
        pTriggerCube2PhysicsObject->position = pTriggerCube2->positionXYZ;
        pTriggerCube2PhysicsObject->rotation = pTriggerCube2->rotationEulerXYZ;
        pTriggerCube2PhysicsObject->velocity = glm::vec3(0.0f);
        pTriggerCube2PhysicsObject->pAssociatedDrawingMeshInstance = pTriggerCube2;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTriggerCube2PhysicsObject);
    }

    //Trigger Cube 3
    {
        sMesh* pTriggerCube3 = new sMesh();
        pTriggerCube3->modelFileName = "assets/models/Cube_Centre.ply";
        pTriggerCube3->positionXYZ = glm::vec3(-59.0f, 2.0f, -49.0f);
        pTriggerCube3->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pTriggerCube3->uniformScale = 5.0f;
        pTriggerCube3->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTriggerCube3->bOverrideObjectColour = true;
        pTriggerCube3->uniqueFriendlyName = "Cube_3";
        pTriggerCube3->bIsVisible = true;
        pTriggerCube3->bDoNotLight = true;
        pTriggerCube3->bIsWireframe = true;
        pTriggerCube3->textures[0] = "Cube.bmp";
        pTriggerCube3->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTriggerCube3);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTriggerCube3PhysicsObject = new  cPhysics::sPhysInfo();
        pTriggerCube3PhysicsObject->bDoesntMove = true;
        pTriggerCube3PhysicsObject->position = pTriggerCube3->positionXYZ;
        pTriggerCube3PhysicsObject->rotation = pTriggerCube3->rotationEulerXYZ;
        pTriggerCube3PhysicsObject->velocity = glm::vec3(0.0f);
        pTriggerCube3PhysicsObject->pAssociatedDrawingMeshInstance = pTriggerCube3;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTriggerCube3PhysicsObject);
    }

    //Trigger Cube 4
    {
        sMesh* pTriggerCube4 = new sMesh();
        pTriggerCube4->modelFileName = "assets/models/Cube_Centre.ply";
        pTriggerCube4->positionXYZ = glm::vec3(21.0f, 2.0f, -50.0f);
        pTriggerCube4->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pTriggerCube4->uniformScale = 5.0f;
        pTriggerCube4->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTriggerCube4->bOverrideObjectColour = true;
        pTriggerCube4->uniqueFriendlyName = "Cube_4";
        pTriggerCube4->bIsVisible = true;
        pTriggerCube4->bDoNotLight = true;
        pTriggerCube4->bIsWireframe = true;
        pTriggerCube4->textures[0] = "Cube.bmp";
        pTriggerCube4->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTriggerCube4);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTriggerCube4PhysicsObject = new  cPhysics::sPhysInfo();
        pTriggerCube4PhysicsObject->bDoesntMove = true;
        pTriggerCube4PhysicsObject->position = pTriggerCube4->positionXYZ;
        pTriggerCube4PhysicsObject->rotation = pTriggerCube4->rotationEulerXYZ;
        pTriggerCube4PhysicsObject->velocity = glm::vec3(0.0f);
        pTriggerCube4PhysicsObject->pAssociatedDrawingMeshInstance = pTriggerCube4;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTriggerCube4PhysicsObject);
    }

    //Trigger Cube 5
    {
        sMesh* pTriggerCube5 = new sMesh();
        pTriggerCube5->modelFileName = "assets/models/Cube_Centre.ply";
        pTriggerCube5->positionXYZ = glm::vec3(-57.0f, 2.0f, -2.8f);
        pTriggerCube5->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pTriggerCube5->uniformScale = 5.0f;
        pTriggerCube5->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pTriggerCube5->bOverrideObjectColour = true;
        pTriggerCube5->uniqueFriendlyName = "Cube_5";
        pTriggerCube5->bIsVisible = true;
        pTriggerCube5->bDoNotLight = true;
        pTriggerCube5->bIsWireframe = true;
        pTriggerCube5->textures[0] = "Cube.bmp";
        pTriggerCube5->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pTriggerCube5);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pTriggerCube5PhysicsObject = new  cPhysics::sPhysInfo();
        pTriggerCube5PhysicsObject->bDoesntMove = true;
        pTriggerCube5PhysicsObject->position = pTriggerCube5->positionXYZ;
        pTriggerCube5PhysicsObject->rotation = pTriggerCube5->rotationEulerXYZ;
        pTriggerCube5PhysicsObject->velocity = glm::vec3(0.0f);
        pTriggerCube5PhysicsObject->pAssociatedDrawingMeshInstance = pTriggerCube5;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pTriggerCube5PhysicsObject);
    }

    //{
    //    sModelDrawInfo cityModel;
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/city.ply",
    //        cityModel, program);
    //    std::cout << cityModel.meshName << ": " << cityModel.numberOfVertices << " vertices loaded" << std::endl;
    //}

    //City
    {
        sMesh* pCity = new sMesh();
        pCity->modelFileName = "assets/models/city.ply";
        pCity->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pCity->rotationEulerXYZ = glm::vec3(-90.0f, 0.0f,0.0f);
        pCity->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pCity->bOverrideObjectColour = true;
        pCity->uniqueFriendlyName = "City";
        pCity->bIsVisible = true;
        pCity->textures[0] = "city.bmp";
        pCity->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pCity);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pCityPhysicsObject = new  cPhysics::sPhysInfo();
        pCityPhysicsObject->bDoesntMove = true;
        pCityPhysicsObject->position = pCity->positionXYZ;
        pCityPhysicsObject->rotation = pCity->rotationEulerXYZ;
        pCityPhysicsObject->velocity = glm::vec3(0.0f);
        pCityPhysicsObject->pAssociatedDrawingMeshInstance = pCity;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pCityPhysicsObject);
    }

    //{
    //    sModelDrawInfo carModel;
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/bugatti.ply",
    //        carModel, program);
    //    std::cout << carModel.meshName << ": " << carModel.numberOfVertices << " vertices loaded" << std::endl;
    //}

    //Car
    {
        sMesh* pCar = new sMesh();
        pCar->modelFileName = "assets/models/bugatti.ply";
        pCar->positionXYZ = glm::vec3(21.0f, 2.0f, -50.0f);
        pCar->rotationEulerXYZ = glm::vec3(-90.0f, 0.0f, 0.0f);
        pCar->uniformScale = 0.44f;
        pCar->objectColourRGBA = glm::vec4(0.9f, 0.6f, 0.6f, 1.0f);
        pCar->bOverrideObjectColour = true;
        pCar->uniqueFriendlyName = "Car";
        pCar->bIsVisible = true;
        pCar->textures[0] = "dua-lipa-promo.bmp";
        pCar->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pCar);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pCarPhysicsObject = new  cPhysics::sPhysInfo();
        pCarPhysicsObject->bDoesntMove = false;
        pCarPhysicsObject->position = pCar->positionXYZ;
        pCarPhysicsObject->rotation = pCar->rotationEulerXYZ;
        pCarPhysicsObject->velocity = glm::vec3(0.0f);
        pCarPhysicsObject->pAssociatedDrawingMeshInstance = pCar;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pCarPhysicsObject);
    }

    //Enemy Car
    {
        sMesh* pCarEnemy = new sMesh();
        pCarEnemy->modelFileName = "assets/models/bugatti.ply";
        pCarEnemy->positionXYZ = glm::vec3(-49.0f, 2.0f, -3.3f);
        pCarEnemy->rotationEulerXYZ = glm::vec3(-90.0f, 0.0f, 0.0f);
        pCarEnemy->uniformScale = 0.44f;
        pCarEnemy->objectColourRGBA = glm::vec4(0.6f, 0.9f, 0.6f, 1.0f);
        pCarEnemy->bOverrideObjectColour = true;
        pCarEnemy->uniqueFriendlyName = "Car_Enemy";
        pCarEnemy->bIsVisible = true;
        pCarEnemy->textures[0] = "Puzzle_parts.bmp";
        pCarEnemy->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pCarEnemy);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pCarEnemyPhysicsObject = new  cPhysics::sPhysInfo();
        pCarEnemyPhysicsObject->bDoesntMove = false;
        pCarEnemyPhysicsObject->position = pCarEnemy->positionXYZ;
        pCarEnemyPhysicsObject->rotation = pCarEnemy->rotationEulerXYZ;
        pCarEnemyPhysicsObject->velocity = glm::vec3(0.0f);
        pCarEnemyPhysicsObject->pAssociatedDrawingMeshInstance = pCarEnemy;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pCarEnemyPhysicsObject);
    }

    {
        sModelDrawInfo sphereMesh;
        //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_xyz.ply",
        //::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_xyz_N.ply",
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_xyz_N_uv.ply",

            sphereMesh, program, false);
        std::cout << sphereMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    //{
    //    sModelDrawInfo newViperModelInfo;
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Viper_MkVII_xyz_n_uv.ply",
    //        newViperModelInfo, program);
    //    std::cout << newViperModelInfo.numberOfVertices << " vertices loaded" << std::endl;
    //}

    // this is the object that the Lua script, etc. is going to handle
    {
        sMesh* pNewViper = new sMesh();
        pNewViper->modelFileName = "assets/models/Viper_MkVII_xyz_n_uv.ply";
        pNewViper->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pNewViper->rotationEulerXYZ = glm::vec3(0.0f);
        pNewViper->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pNewViper->bOverrideObjectColour = true;
        pNewViper->uniqueFriendlyName = "New_Viper_Player";
        pNewViper->bIsVisible = true;
        pNewViper->textures[0] = "dirty-metal-texture_1048-4784.bmp";
        pNewViper->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pNewViper);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pViperPhysObject = new  cPhysics::sPhysInfo();
        pViperPhysObject->bDoesntMove = false;
        pViperPhysObject->position = pNewViper->positionXYZ;
        pViperPhysObject->rotation = pNewViper->rotationEulerXYZ;
        pViperPhysObject->velocity = glm::vec3(0.0f);
        pViperPhysObject->pAssociatedDrawingMeshInstance = pNewViper;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pViperPhysObject);
    }

    {
        sMesh* pNewViper1 = new sMesh();
        pNewViper1->modelFileName = "assets/models/Viper_MkVII_xyz_n_uv.ply";
        pNewViper1->positionXYZ = glm::vec3(0.0f, 50.0f, 0.0f);
        pNewViper1->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pNewViper1->bOverrideObjectColour = true;
        pNewViper1->uniqueFriendlyName = "New_Viper_Player_1";
        pNewViper1->bIsVisible = true;
        pNewViper1->textures[0] = "dua-lipa-promo.bmp";
        pNewViper1->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pNewViper1);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pViperPhysObject1 = new  cPhysics::sPhysInfo();
        pViperPhysObject1->bDoesntMove = false;
        pViperPhysObject1->position = pNewViper1->positionXYZ;
        pViperPhysObject1->rotation = pNewViper1->rotationEulerXYZ;
        pViperPhysObject1->velocity = glm::vec3(0.0f);
        pViperPhysObject1->pAssociatedDrawingMeshInstance = pNewViper1;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pViperPhysObject1);
    }

   {
       sMesh* pSkySphere = new sMesh();
       pSkySphere->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
       pSkySphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
       pSkySphere->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
//       pSkySphere->bIsWireframe = true;
       pSkySphere->bOverrideObjectColour = true;
       pSkySphere->uniformScale = 25.0f;
       pSkySphere->uniqueFriendlyName = "SkySphere";
       pSkySphere->textures[0] = "bad_bunny_1920x1080.bmp";
       pSkySphere->blendRatio[0] = 1.0f;
       pSkySphere->bIsVisible = false;
       ::g_vecMeshesToDraw.push_back(pSkySphere);
   }
    return;
}


// Add object to scene through Lua
// AddMeshToScene('plyname.ply', 'friendlyName', x, y, z);
int g_Lua_AddMeshToScene(lua_State* L)
{


    sMesh* pNewMesh = new sMesh();
    pNewMesh->modelFileName = lua_tostring(L, 1);       // 'plyname.ply'
    pNewMesh->uniqueFriendlyName = lua_tostring(L, 2);  // Friendly name
    pNewMesh->positionXYZ.x = (float)lua_tonumber(L, 3);
    pNewMesh->positionXYZ.y = (float)lua_tonumber(L, 4);
    pNewMesh->positionXYZ.z = (float)lua_tonumber(L, 5);
    pNewMesh->textures[0] = lua_tostring(L, 6);
    pNewMesh->blendRatio[0] = (float)lua_tonumber(L, 7);
    //
    pNewMesh->bIsVisible = true;
    ::g_vecMeshesToDraw.push_back(pNewMesh);

    return 0;
}

int g_Lua_AddParallelCommand(lua_State* L)
{
    int argCount = lua_gettop(L); // Get the number of arguments on the stack
    if (argCount < 6) {
        std::cerr << "Error: Expected 6 arguments, got " << argCount << "." << std::endl;
        return 0;
    }
    std::string commandType = lua_tostring(L, 1);
    std::vector<std::string> vecCommandDetails;
    vecCommandDetails.push_back(commandType);
    
    if (commandType == "SET_CAMERA")
    {

        std::string objectFriendlyName = lua_tostring(L, 2);      // 'New_Viper_Player'
        float x = (float)lua_tonumber(L, 3);                   // -50.0
        float y = (float)lua_tonumber(L, 4);                   // 15.0
        float z = (float)lua_tonumber(L, 5);                   // 30.0
        float timeSeconds = (float)lua_tonumber(L, 6);


        vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
        vecCommandDetails.push_back(::g_floatToString(x));
        vecCommandDetails.push_back(::g_floatToString(y));
        vecCommandDetails.push_back(::g_floatToString(z));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pSetCamera = ::g_pCommandFactory->pCreateCommandObject(
            "Set Camera", vecCommandDetails);

        ::g_pCommandDirector->addParallel(pSetCamera);
    }
    else if (commandType == "MOVE_TO")
    {

        std::string objectFriendlyName = lua_tostring(L, 2);      // 'New_Viper_Player'
        float x = (float)lua_tonumber(L, 3);                   // -50.0
        float y = (float)lua_tonumber(L, 4);                   // 15.0
        float z = (float)lua_tonumber(L, 5);                   // 30.0
        float timeSeconds = (float)lua_tonumber(L, 6);


        vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
        vecCommandDetails.push_back(::g_floatToString(x));
        vecCommandDetails.push_back(::g_floatToString(y));
        vecCommandDetails.push_back(::g_floatToString(z));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pMoveViper = ::g_pCommandFactory->pCreateCommandObject(
            "Move Relative ConstVelocity+Time", vecCommandDetails);

        ::g_pCommandDirector->addParallel(pMoveViper);
    }
    else if (commandType == "ORIENT_TO")
    {

        std::string objectFriendlyName = lua_tostring(L, 2);      // 'New_Viper_Player'
        float x = (float)lua_tonumber(L, 3);                   // -50.0
        float y = (float)lua_tonumber(L, 4);                   // 15.0
        float z = (float)lua_tonumber(L, 5);                   // 30.0
        float timeSeconds = (float)lua_tonumber(L, 6);


        vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
        vecCommandDetails.push_back(::g_floatToString(x));
        vecCommandDetails.push_back(::g_floatToString(y));
        vecCommandDetails.push_back(::g_floatToString(z));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pOrientViper = ::g_pCommandFactory->pCreateCommandObject(
            "Orient Relative Time", vecCommandDetails);
        ::g_pCommandDirector->addParallel(pOrientViper);
    }
    else if (commandType == "FOLLOW_CURVE")
    {
        std::string objectFriendlyName = lua_tostring(L, 2);

        float startPosX = (float)lua_tonumber(L, 3);
        float startPosY = (float)lua_tonumber(L, 4);
        float startPosZ = (float)lua_tonumber(L, 5);

        float control1X = (float)lua_tonumber(L, 6);
        float control1Y = (float)lua_tonumber(L, 7);
        float control1Z = (float)lua_tonumber(L, 11);

        float control2X = (float)lua_tonumber(L, 9);
        float control2Y = (float)lua_tonumber(L, 10);
        float control2Z = (float)lua_tonumber(L, 11);

        float endPosX = (float)lua_tonumber(L, 12);
        float endPosY = (float)lua_tonumber(L, 13);
        float endPosZ = (float)lua_tonumber(L, 14);

        float timeSeconds = (float)lua_tonumber(L, 15);

        vecCommandDetails.push_back(objectFriendlyName);
        vecCommandDetails.push_back(::g_floatToString(startPosX));
        vecCommandDetails.push_back(::g_floatToString(startPosY));
        vecCommandDetails.push_back(::g_floatToString(startPosZ));

        vecCommandDetails.push_back(::g_floatToString(control1X));
        vecCommandDetails.push_back(::g_floatToString(control1Y));
        vecCommandDetails.push_back(::g_floatToString(control1Z));

        vecCommandDetails.push_back(::g_floatToString(control2X));
        vecCommandDetails.push_back(::g_floatToString(control2Y));
        vecCommandDetails.push_back(::g_floatToString(control2Z));

        vecCommandDetails.push_back(::g_floatToString(endPosX));
        vecCommandDetails.push_back(::g_floatToString(endPosY));
        vecCommandDetails.push_back(::g_floatToString(endPosZ));
        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pFollowCurveViper = ::g_pCommandFactory->pCreateCommandObject(
            "Follow Curve", vecCommandDetails);
        ::g_pCommandDirector->addParallel(pFollowCurveViper);
    }
    else if (commandType == "FOLLOW_OBJECT")
    {

        std::string objectFriendlyNameFollower = lua_tostring(L, 2);
        std::string objectFriendlyNameToFollow = lua_tostring(L, 3);

        float offsetX = (float)lua_tonumber(L, 4);
        float offsetY = (float)lua_tonumber(L, 5);
        float offsetZ = (float)lua_tonumber(L, 6);

        float timeSeconds = (float)lua_tonumber(L, 7);

        vecCommandDetails.push_back(objectFriendlyNameFollower);
        vecCommandDetails.push_back(objectFriendlyNameToFollow);
        vecCommandDetails.push_back(::g_floatToString(offsetX));
        vecCommandDetails.push_back(::g_floatToString(offsetY));
        vecCommandDetails.push_back(::g_floatToString(offsetZ));

        vecCommandDetails.push_back(::g_floatToString(timeSeconds));

        iCommand* pFollowObjectViper = ::g_pCommandFactory->pCreateCommandObject(
            "Follow Object", vecCommandDetails);
        ::g_pCommandDirector->addParallel(pFollowObjectViper);
    }


    // We'll return some value to indicate if the command worked or not
    // Here, we'll push "true" if it worked
    lua_pushboolean(L, true);
    // return 1 because we pushed 1 thing onto the stack
    return 1;
   
}


//glm::vec3 camOverlayPosition = glm::vec3(scene.pModels[7].pMeshTransform.x, scene.pModels[7].pMeshTransform.y, scene.pModels[7].pMeshTransform.z);
//glm::vec3 direction = glm::normalize(flyCam.camLocation - camOverlayPosition);
//
//glm::quat rotation = glm::quatLookAt(direction, flyCam.camUpVector);
//glm::vec3 eulerAngles = glm::eulerAngles(rotation);
//
//glm::quat adjustedRotation = rotation * glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, glm::radians(90.0f))); // Example: Rotate 90 degrees around Y
//glm::vec3 adjustedEulerAngles = glm::eulerAngles(adjustedRotation);
//
//// Apply adjusted angles
//scene.pModels[7].pMeshTransform.xRotation = glm::degrees(adjustedEulerAngles.x);
//scene.pModels[7].pMeshTransform.yRotation = glm::degrees(adjustedEulerAngles.y);
//scene.pModels[7].pMeshTransform.zRotation = glm::degrees(adjustedEulerAngles.z);
























//using namespace std;

void ConsoleStuff(void)
{
    // "o" for output
//    std::ofstream myFile("someData.txt");
    // Write something
    //myFile << "Hello" << std::endl;
    //myFile << "there";
    //myFile.close();

    // Now read this file
//    std::ifstream myFile2("someData.txt");
//    std::string someString;
//    myFile2 >> someString;
//    std::cout << someString << std::endl;
//
    //std::string aword;
    //while (aword != "END_OF_FILE")
    //{
    //    myFile2 >> aword;
    //    std::cout << aword << std::endl;
    //};

    //std::string aword;
    //while (myFile2 >> aword)
    //{
    //    std::cout << aword << std::endl;
    //};

    std::ifstream myFile2("assets/models/bun_zipper_res3.ply");
    if (myFile2.is_open())
    {

        std::string aword;
        while (myFile2 >> aword)
        {
            std::cout << aword << std::endl;
        };
    }
    else
    {
        std::cout << "Can't find file" << std::endl;
    }


    // iostream
    std::cout << "Type a number:" << std::endl;

    int x = 0;
    std::cin >> x;

    std::cout << "You typed: " << x << std::endl;

    std::cout << "Type your name:" << std::endl;
    std::string name;
    std::cin >> name;

    std::cout << "Hello " << name << std::endl;
    return;
}


//int& getNumber(void)
//{
//    int p = 0;
//    return p;
//}

//cTankFactory* pTankFactory = NULL;
cTankBuilder* pTheTankBuilder = NULL;

// This is here for speed 
void SetUpTankGame(void)
{
 
    ::g_pTankArena = new cArena();

    if (!pTheTankBuilder)
    {
        pTheTankBuilder = new cTankBuilder();
    }



    

    std::vector<std::string> vecTankTpyes;
//    pTankFactory->GetTankTypes(vecTankTpyes);
//    cTankFactory::get_pTankFactory()->GetTankTypes(vecTankTpyes);
    pTheTankBuilder->GetTankTypes(vecTankTpyes);
    std::cout << "The tank factory can create "
        << vecTankTpyes.size() << " types of tanks:" << std::endl;
    for (std::string tankTypeString : vecTankTpyes)
    {
        std::cout << tankTypeString << std::endl;
    }
    std::cout << std::endl;

    // Create 1 super tank
//    iTank* pTheTank = cTankFactory::get_pTankFactory()->CreateATank("Super Tank");
    iTank* pTheTank = pTheTankBuilder->CreateATank("Super Tank!");
    if (pTheTank)
    {
        ::g_vecTheTanks.push_back(pTheTank);
    }

    // Create 10 tanks
    for (unsigned int count = 0; count != 50; count++)
    {
//        iTank* pTheTank = cTankFactory::get_pTankFactory()->CreateATank("Regular Tank");
        iTank* pTheTank = pTheTankBuilder->CreateATank("Regular Tank with Shield");
        if (pTheTank)
        {
            ::g_vecTheTanks.push_back(pTheTank);
        }
    }
    
    // Also a hover tank
//    iTank* pHoverTank = cTankFactory::get_pTankFactory()->CreateATank("Hover Tank");
    iTank* pHoverTank = pTheTankBuilder->CreateATank("Hover Tank");
    if (pHoverTank)
    {
        ::g_vecTheTanks.push_back(pHoverTank);
    }



    const float WORLD_SIZE(100.0f);

    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        glm::vec3 tankLocXYZ;
        tankLocXYZ.x = getRandomFloat(-WORLD_SIZE, WORLD_SIZE);
        tankLocXYZ.y = -5.0f;
        tankLocXYZ.z = getRandomFloat(-WORLD_SIZE, WORLD_SIZE);

        pCurrentTank->setLocation(tankLocXYZ);
    }

    // Tell the tanks about the mediator
    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        pCurrentTank->setMediator(::g_pTankArena);
    }


    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        ::g_pTankArena->AddTank(pCurrentTank);
    }

    return;
}


void TankStepFrame(double timeStep)
{



    return;
}

// x = 5, y = 15    --> 0, 1
// x = 40.0, y = 80.0   --> [4][8]
// (40.0, 80.0) --> box size = 100
//   [0][0]   
void calcBoxXYFromCoord(float x, float y, int &xIndex, int &yIndex, float boxSize)
{
    xIndex = (int)(x / boxSize);
    yIndex = (int)(y / boxSize);
    return;
}


void AABBOctTree(void)
{
    struct sSquare
    {
        //       vector< cTriangles* > vecTriangleInThisSquare
        glm::vec2 minXY;
        glm::vec2 maxXY;
        float width;
        unsigned int indexColRow;
    };

    sSquare grid[10][10];
    float sqaureWidth = 10;

    for (unsigned int x = 0; x < 10; x++)
    {
        for (unsigned int y = 0; y < 10; y++)
        {
            grid[x][y].width = sqaureWidth;
            grid[x][y].minXY.x = sqaureWidth * x;
            grid[x][y].minXY.y = sqaureWidth * y;

            grid[x][y].maxXY.x = sqaureWidth * x + sqaureWidth;
            grid[x][y].maxXY.y = sqaureWidth * y + sqaureWidth;
        }
    }

    int xIndex, yIndex;
    calcBoxXYFromCoord(5.0f, 15.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;


    calcBoxXYFromCoord(40.0f, 80.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;




    return;
}