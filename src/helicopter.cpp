#include "helicopter.h"

#include "mygl/geometry.h"

#include <stdexcept>

Helicopter helicopterLoad(const std::string& filepath)
{
    std::vector<Model> models = modelLoad(filepath);

    if(models.size() != Helicopter::PART_COUNT)
    {
        throw std::runtime_error("[Helicopter] number of parts do not match!");
    }

    Helicopter heli;
    heli.partModel.resize(models.size());
    heli.partTransformations.resize(Helicopter::ePart::PART_COUNT, Matrix4D::identity());
    heli.position.y = 5.5f;

    /* re-assign models to match enums (just to be safe ;) (order should actually match the one in the obj file)) */
    for(const auto& obj : models)
    {
        if(obj.name == "body") heli.partModel[Helicopter::BODY] = obj;
        else if(obj.name == "rotor") heli.partModel[Helicopter::ROTOR] = obj;
        else if(obj.name == "tail_rotor") heli.partModel[Helicopter::TAIL_ROTOR] = obj;
        else if(obj.name == "slides") heli.partModel[Helicopter::SLIDES] = obj;
        else if(obj.name == "spotlight") heli.partModel[Helicopter::SPOTLIGHT] = obj;
        else if(obj.name == "strobelight") heli.partModel[Helicopter::STROBELIGHT] = obj;
        else if(obj.name == "positionlight") heli.partModel[Helicopter::POSTIONLIGTHS] = obj;
        else throw std::runtime_error("[Helicopter] unkown part name: " + obj.name);
    }

    return heli;
}

void helicopterDelete(Helicopter& heli)
{
    for(auto& model : heli.partModel)
    {
        modelDelete(model);
    }

    heli.partModel.clear();
    heli.partTransformations.clear();
}

void helicopterMove(Helicopter& heli, bool control[], float dt)
{
    /* retrieve input for controls */
    float throttle = + control[Helicopter::eControl::THROTTLE_UP] - control[Helicopter::eControl::THROTTLE_DOWN];
    float yaw = + control[Helicopter::eControl::YAW_LEFT] - control[Helicopter::eControl::YAW_RIGHT];
    float pitch = + control[Helicopter::eControl::PITCH_DOWN] - control[Helicopter::eControl::PITCH_UP];
    float tilt = + control[Helicopter::eControl::ROLL_RIGHT] - control[Helicopter::eControl::ROLL_LEFT];

    auto up = Vector3D(heli.rotation[1]);
    auto lift = normalize(up) * (throttle * heli.lift);

    /* compute change in position and angles */
    heli.position += dt * lift + dt * Vector3D(up.x, 0.0f, up.z) * heli.velocity;
    heli.angles.x += dt * pitch - dt * heli.angles.x/M_PI_4;
    heli.angles.y += dt * yaw;
    heli.angles.z += dt * tilt - dt * heli.angles.z/M_PI_4;

    /* final transformation matrices */
    heli.rotation = Matrix4D::rotationY(heli.angles.y) * Matrix4D::rotationX(heli.angles.x) * Matrix4D::rotationZ(heli.angles.z);
    heli.transformation = Matrix4D::translation(heli.position) * heli.rotation;

    /* animation of rotors */
    heli.rotorRotation += dt*8*M_PI;
    #define TRANS_INV(vec, matrix) Matrix4D::translation(vec) * (matrix) * Matrix4D::translation(-vec);
    heli.partTransformations[Helicopter::ROTOR] = TRANS_INV(Vector4D(0.0, 3.1597, -0.17219), Matrix4D::rotationY(heli.rotorRotation));
    heli.partTransformations[Helicopter::TAIL_ROTOR] = TRANS_INV(Vector4D(0.263874, 2.87729, -6.52368), Matrix4D::rotationX(heli.rotorRotation));
}
