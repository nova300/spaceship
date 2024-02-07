#pragma once
#include <object.h>
#include "utils.h"
#include <raylib.h>
#include "program.h"
#include "graphics.h"
#include "engine.h"
#include <ecs/Component.h>
#include <ecs/Entity.h>
#include <ecs/EntityManager.h>


class Player : public Object
{
    public:
    Player(ecs::EntityManager& entitymanager);
    ~Player();
    void Update();
    ecs::Entity entity;
    ecs::Entity graphic;
    Vector3 cameraPos;
    Vector3 cameraTrg;
    private:
    void HandleInputs();
    Mesh mesh;
    float speed = 100;
    float turn_speed = 0.5;
    float elevation_speed = 100;
    float roll_angle = 30;
    float pitch_angle = 30;
    Vector3 cam_offset = Vector3{ 0.0f, 10.0f, 20.0f };
    Vector3 cam_look_offset = Vector3{ 0.0f, 0.0f, -10.0f };

    float current_roll_t = 0.0f;
    float curr_smooth_roll_vel = 0.0f;
    float current_turn_t = 0.0f;
    float curr_smooth_turn_vel = 0.0f;
    float current_pitch_angle = 0.0f;
    float curr_pitch_t = 0.0f;
    float curr_smooth_pitch_vel = 0.0f;
};


#define TEMPLATE_MAIN_SCENE spaceship
class TEMPLATE_MAIN_SCENE : public Program
{
public:
    void Update();
    void Render();
    void Init();
    void Destroy();
private:
    ecs::EntityManager manager;
    Camera camera;
    Player player = Player(manager);
};

Mesh GenerateSpaceShipMesh(void);
Mesh GenerateRockMesh(void);

