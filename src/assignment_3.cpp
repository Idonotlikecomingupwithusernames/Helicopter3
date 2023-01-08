#include <cstdlib>
#include <iostream>

#include "mygl/shader.h"
#include "mygl/model.h"
#include "mygl/camera.h"

#include "helicopter.h"

struct SpotLight
{
    Vector3D position;
    Vector3D direction;
    Vector4D color;

    float constant;
    float linear;
    float quadratic; 
    float phi;
};

struct
{
    Camera camera;
    bool cameraFollowHeli;
    float zoomSpeedMultiplier;

    Helicopter heli;
    Model modelGround;

    ShaderProgram shaderColor;

    SpotLight whiteLights[3];
    SpotLight redLight;

    bool whiteLightsAreOn = true;
    bool redLightIsOn = true;
    bool isDay = false;
} sScene;

struct
{
    bool mouseButtonPressed = false;
    Vector2D mousePressStart;

    bool keyPressed[Helicopter::eControl::CONTROL_COUNT] = {false, false, false, false, false, false, false, false};
} sInput;


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    /* input for camera control */
    if(key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        sScene.cameraFollowHeli = false;
        sScene.camera.lookAt = {0.0f, 0.0f, 0.0f};
        cameraUpdateOrbit(sScene.camera, {0.0f, 0.0f}, 0.0f);
    }
    if(key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        sScene.cameraFollowHeli = false;
    }
    if(key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        sScene.cameraFollowHeli = true;
    }

    /* input for helicopter control */
    if(key == GLFW_KEY_W)
    {
        sInput.keyPressed[Helicopter::eControl::PITCH_DOWN] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_S)
    {
        sInput.keyPressed[Helicopter::eControl::PITCH_UP] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if(key == GLFW_KEY_A)
    {
        sInput.keyPressed[Helicopter::eControl::ROLL_LEFT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_D)
    {
        sInput.keyPressed[Helicopter::eControl::ROLL_RIGHT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if(key == GLFW_KEY_Q)
    {
        sInput.keyPressed[Helicopter::eControl::YAW_LEFT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_E)
    {
        sInput.keyPressed[Helicopter::eControl::YAW_RIGHT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if(key == GLFW_KEY_LEFT_SHIFT)
    {
        sInput.keyPressed[Helicopter::eControl::THROTTLE_UP] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_SPACE)
    {
        sInput.keyPressed[Helicopter::eControl::THROTTLE_DOWN] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    /* close window on escape */
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    /* make screenshot and save in work directory */
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        screenshotToPNG("screenshot.png");
    }

    /* Switch between day/night */
    if(key == GLFW_KEY_M && action == GLFW_PRESS){
        sScene.isDay = !sScene.isDay;
    }

    /* Switch white lights on/off */
    if(key == GLFW_KEY_N && action == GLFW_PRESS){
        sScene.whiteLightsAreOn = !sScene.whiteLightsAreOn;
    }
}

void mousePosCallback(GLFWwindow* window, double x, double y)
{
    if(sInput.mouseButtonPressed)
    {
        Vector2D diff = sInput.mousePressStart - Vector2D(x, y);
        cameraUpdateOrbit(sScene.camera, diff, 0.0f);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        sInput.mouseButtonPressed = (action == GLFW_PRESS);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraUpdateOrbit(sScene.camera, {0, 0}, sScene.zoomSpeedMultiplier * yoffset);
}


void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    sScene.camera.width = width;
    sScene.camera.height = height;
}

void sceneInit(float width, float height)
{
    sScene.camera = cameraCreate(width, height, to_radians(45.0), 0.01, 500.0, {10.0, 10.0, 10.0}, {0.0, 0.0, 0.0});
    sScene.cameraFollowHeli = true;
    sScene.zoomSpeedMultiplier = 0.05f;

    sScene.heli = helicopterLoad("assets/helicopter/helicopter.obj");
    sScene.modelGround = modelLoad("assets/ground/ground.obj").front();

    sScene.shaderColor = shaderLoad("shader/default.vert", "shader/color.frag");

    /* Setup lights very inefficiently */
    sScene.whiteLights[0].position = Vector3D(-1.0, 5.0, -2.0);
    sScene.whiteLights[1].position = Vector3D(1.0, 5.0, -2.0);
    sScene.whiteLights[0].direction = Vector4D(1.0, 0.0, 0.0, 1.0);
    sScene.whiteLights[1].direction = Vector4D(-1.0, 0.0, 0.0, 1.0);

    for(int i = 0; i < 2; i++){
        sScene.whiteLights[i].color = Vector4D(1.0, 1.0, 1.0, 1.0);
        sScene.whiteLights[i].constant = 0.4;
        sScene.whiteLights[i].linear = 0.09;
        sScene.whiteLights[i].quadratic = 0.032;
        sScene.whiteLights[i].phi = to_radians(30);
    }

    sScene.whiteLights[2] = {
        Vector3D(0.0, 5.0, 0.0),
        Vector3D(0.0, 0.0, 1.0),
        Vector4D(1.0, 1.0, 1.0, 1.0),
        0.6,
        0.09,
        0.032,
        to_radians(50.0)
    };

    sScene.redLight = {
        Vector3D(0.0, 2.0, -5.0),
        Vector3D(0.0, 1.0, 0.0),
        Vector4D(1.0, 0.2, 0.2, 1.0),
        0.6,
        0.09,
        0.032,
        to_radians(-60.0)
    };
}

void sceneUpdate(float dt)
{
    helicopterMove(sScene.heli, sInput.keyPressed, dt);

    if (sScene.cameraFollowHeli)
        cameraFollow(sScene.camera, sScene.heli.position);

    /* Toggle strobe every 2 seconds (takes a few seconds to work properly) */
    sScene.redLightIsOn = !int(fmod(int(glfwGetTime()), 2)) ? !sScene.redLightIsOn : sScene.redLightIsOn;
}

void sceneDraw()
{
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*------------ render scene -------------*/
    {
        /* setup camera and model matrices */
        Matrix4D proj = cameraProjection(sScene.camera);
        Matrix4D view = cameraView(sScene.camera);

        glUseProgram(sScene.shaderColor.id);
        shaderUniform(sScene.shaderColor, "uProj",  proj);
        shaderUniform(sScene.shaderColor, "uView",  view);
        shaderUniform(sScene.shaderColor, "uModel",  sScene.heli.transformation);

        /* Pass camera position to calculate viewDir */
        shaderUniform(sScene.shaderColor, "uCamPos",  sScene.camera.position);

        /* Pass ambient factor */
        shaderUniform(sScene.shaderColor, "uAmbient",  float(0.1));

        /* Pass directional light */
        shaderUniform(sScene.shaderColor, "uDirLight.direction",  Vector3D(0.0, -1.0, -1.0));

        if (sScene.isDay){
            shaderUniform(sScene.shaderColor, "uDirLight.color",  Vector4D(0.4, 0.4, 0.3, 1.0));
        } else {
            shaderUniform(sScene.shaderColor, "uDirLight.color",  Vector4D(0.05, 0.05, 0.07, 1.0));
        }

        /* Toggle white lights */
        if (sScene.whiteLightsAreOn){
            shaderUniform(sScene.shaderColor, "uSpotLight[0].color", sScene.whiteLights[0].color);
            shaderUniform(sScene.shaderColor, "uSpotLight[1].color", sScene.whiteLights[1].color);
            shaderUniform(sScene.shaderColor, "uSpotLight[2].color", sScene.whiteLights[2].color);
        } else {
            shaderUniform(sScene.shaderColor, "uSpotLight[0].color",  Vector4D(0.0, 0.0, 0.0, 1.0));
            shaderUniform(sScene.shaderColor, "uSpotLight[1].color",  Vector4D(0.0, 0.0, 0.0, 1.0));
            shaderUniform(sScene.shaderColor, "uSpotLight[2].color",  Vector4D(0.0, 0.0, 0.0, 1.0));
        }

        /* Toggle strobe */
        if (sScene.redLightIsOn){
            shaderUniform(sScene.shaderColor, "uSpotLight[3].color", sScene.redLight.color);
        } else {
            shaderUniform(sScene.shaderColor, "uSpotLight[3].color",  Vector4D(0.0, 0.0, 0.0, 1.0));
        }

        /* Pass light values very inefficiently */
        shaderUniform(sScene.shaderColor, "uSpotLight[0].position", sScene.whiteLights[0].position);
        shaderUniform(sScene.shaderColor, "uSpotLight[0].direction", sScene.whiteLights[0].direction);
        shaderUniform(sScene.shaderColor, "uSpotLight[0].constant", sScene.whiteLights[0].constant);
        shaderUniform(sScene.shaderColor, "uSpotLight[0].linear", sScene.whiteLights[0].linear);
        shaderUniform(sScene.shaderColor, "uSpotLight[0].quadratic", sScene.whiteLights[0].quadratic);
        shaderUniform(sScene.shaderColor, "uSpotLight[0].phi", sScene.whiteLights[0].phi);

        shaderUniform(sScene.shaderColor, "uSpotLight[1].position", sScene.whiteLights[1].position);
        shaderUniform(sScene.shaderColor, "uSpotLight[1].direction", sScene.whiteLights[1].direction);
        shaderUniform(sScene.shaderColor, "uSpotLight[1].constant", sScene.whiteLights[1].constant);
        shaderUniform(sScene.shaderColor, "uSpotLight[1].linear", sScene.whiteLights[1].linear);
        shaderUniform(sScene.shaderColor, "uSpotLight[1].quadratic", sScene.whiteLights[1].quadratic);
        shaderUniform(sScene.shaderColor, "uSpotLight[1].phi", sScene.whiteLights[1].phi);

        shaderUniform(sScene.shaderColor, "uSpotLight[2].position", sScene.whiteLights[2].position);
        shaderUniform(sScene.shaderColor, "uSpotLight[2].direction", sScene.whiteLights[2].direction);
        shaderUniform(sScene.shaderColor, "uSpotLight[2].constant", sScene.whiteLights[2].constant);
        shaderUniform(sScene.shaderColor, "uSpotLight[2].linear", sScene.whiteLights[2].linear);
        shaderUniform(sScene.shaderColor, "uSpotLight[2].quadratic", sScene.whiteLights[2].quadratic);
        shaderUniform(sScene.shaderColor, "uSpotLight[2].phi", sScene.whiteLights[2].phi);

        shaderUniform(sScene.shaderColor, "uSpotLight[3].position", sScene.redLight.position);
        shaderUniform(sScene.shaderColor, "uSpotLight[3].direction", sScene.redLight.direction);
        shaderUniform(sScene.shaderColor, "uSpotLight[3].constant", sScene.redLight.constant);
        shaderUniform(sScene.shaderColor, "uSpotLight[3].linear", sScene.redLight.linear);
        shaderUniform(sScene.shaderColor, "uSpotLight[3].quadratic", sScene.redLight.quadratic);
        shaderUniform(sScene.shaderColor, "uSpotLight[3].phi", sScene.redLight.phi);

        /* render heli */
        for(unsigned int i = 0; i < sScene.heli.partModel.size(); i++)
        {
            auto& model = sScene.heli.partModel[i];
            auto& transform = sScene.heli.partTransformations[i];
            glBindVertexArray(model.mesh.vao);

            shaderUniform(sScene.shaderColor, "uModel", sScene.heli.transformation * transform);

            for(auto& material : model.material)
            {
                /* set material properties */
                shaderUniform(sScene.shaderColor, "uMaterial.diffuse", material.diffuse);
                /*
                * This is the correct implementation, which unfortunetaly leads to whitewashing everything
                shaderUniform(sScene.shaderColor, "uMaterial.ambient", Vector4D(material.ambient, 1.0));
                */
                shaderUniform(sScene.shaderColor, "uMaterial.diffuse", Vector4D(material.diffuse, 1.0));
                shaderUniform(sScene.shaderColor, "uMaterial.specular", Vector4D(material.specular, 1.0));
                shaderUniform(sScene.shaderColor, "uMaterial.shininess", material.shininess);

                glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT, (const void*) (material.indexOffset*sizeof(unsigned int)) );
            }
        }

        /* render ground */
        shaderUniform(sScene.shaderColor, "uModel", Matrix4D::identity());
        glBindVertexArray(sScene.modelGround.mesh.vao);

        for(auto& material : sScene.modelGround.material)
        {
            /* set material properties */
            shaderUniform(sScene.shaderColor, "uMaterial.diffuse", material.diffuse);
            /*
            * This is the correct implementation, which unfortunetaly leads to whitewashing everything
            shaderUniform(sScene.shaderColor, "uMaterial.ambient", Vector4D(material.ambient, 1.0));
            */
            shaderUniform(sScene.shaderColor, "uMaterial.diffuse", Vector4D(material.diffuse, 1.0));
            shaderUniform(sScene.shaderColor, "uMaterial.specular", Vector4D(material.specular, 1.0));
            shaderUniform(sScene.shaderColor, "uMaterial.shininess", material.shininess);

            glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT, (const void*) (material.indexOffset*sizeof(unsigned int)) );
        }

    }

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);

}

int main(int argc, char** argv)
{
    /*---------- init window ------------*/
    int width = 1280;
    int height = 720;
    GLFWwindow* window = windowCreate("Assignment 3 - Texturing", width, height);
    if(!window) { return EXIT_FAILURE; }

    /* set window callbacks */
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);

    /*---------- init opengl stuff ------------*/
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    /* setup scene */
    sceneInit(width, height);

    /*-------------- main loop ----------------*/
    double timeStamp = glfwGetTime();
    double timeStampNew = 0.0;
    while(!glfwWindowShouldClose(window))
    {
        /* poll and process input and window events */
        glfwPollEvents();

        /* update scene */
        timeStampNew = glfwGetTime();
        sceneUpdate(timeStampNew - timeStamp);
        timeStamp = timeStampNew;

        /* draw all objects in the scene */
        sceneDraw();
        
        /* swap front and back buffer */
        glfwSwapBuffers(window);
    }

    /*-------- cleanup --------*/
    helicopterDelete(sScene.heli);
    shaderDelete(sScene.shaderColor);
    windowDelete(window);

    return EXIT_SUCCESS;
}
