#include "spaceship.h"
#include "engine.h"
#include "graphics.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

bool init = false;


void MainLoop(void);


RenderTexture2D canvas;
int canvasWidth = 320;
int canvasHeight = 200;





int main(void)
{

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "engine");

    canvas = LoadRenderTexture(canvasWidth, canvasHeight);

    //EngineInit();
    ProgramStack stack;
    stack.Push(new TEMPLATE_MAIN_SCENE());

    #if defined(PLATFORM_WEB)
    emscripten_set_main_loop(MainLoop, 0, 1);
    #else
    SetTargetFPS(60);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
       MainLoop();
    }
    #endif

    UnloadRenderTexture(canvas);
    
    CloseWindow();        // Close window and OpenGL context

    return 0;
}

void MainLoop(void)
{
    EngineTick();

    ProgramStack::UpdatePrograms();
    Program::UpdateGlobalObjects();

    
    BeginTextureMode(canvas);
        ClearBackground(BLACK);

        ProgramStack::RenderPrograms();
    EndTextureMode();
   

    BeginDrawing();
        DrawTexturePro(canvas.texture, Rectangle{0, 0, (float)canvasWidth, -(float)canvasHeight}, Rectangle{0, 0, (float)screenWidth, (float)screenHeight}, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    EndDrawing();

    ProgramStack::Clean();
}

Texture2D texture;

void TEMPLATE_MAIN_SCENE::Init()
{
    camera.position = Vector3{ 0.0f, 10.0f, 10.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    //manager.addComponent<TransformComponent>();


    auto hm = LoadImage("heightmap.png");
    auto map = GenMeshHeightmap(hm, Vector3{5000.0f, 200.0f, 5000.0f});

    auto obj = manager.createEntity();
    manager.addComponent<Position>(obj, 0, 0);
    manager.addComponent<MeshComponent>(obj, map, true);

    AddObject(&player);

}

void TEMPLATE_MAIN_SCENE::Update()
{
    //UpdateCamera(&camera, CAMERA_ORBITAL);
    camera.target = player.cameraTrg;
    camera.position = player.cameraPos;


}
void TEMPLATE_MAIN_SCENE::Render()
{
    BeginMode3D(camera);


        Render3D::Meshes(manager);
        Render3D::Sprites(manager);
        //DrawBillboard(camera, texture, Vector3Zero(), 64, WHITE);

        //DrawGrid(500, 50);

    EndMode3D();

    auto pos = manager.getComponent<Position>(player.entity);

    DrawText(TextFormat("X:%f Y:%f Z:%f W:%f ", pos.rotation.x, pos.rotation.y, pos.rotation.z, pos.rotation.w), 0, 30, 20, RED);

    DrawFPS(0,0);
}

void TEMPLATE_MAIN_SCENE::Destroy()
{

}

Mesh GenerateSpaceShipMesh(void)
{
    float dir[3] = {0, 1, 0};

    par_shapes_mesh *center = par_shapes_create_tetrahedron();
    par_shapes_rotate(center, PI / 2, dir);
    par_shapes_scale(center, 3, 1, 6 );

    par_shapes_mesh *wing1 = par_shapes_create_octahedron();
    par_shapes_rotate(wing1, PI / 2, dir);
    par_shapes_scale(wing1, 1, 0.5, 3);
    par_shapes_translate(wing1, 2.7, 0, 3);

    par_shapes_mesh *wing2 = par_shapes_create_octahedron();
    par_shapes_rotate(wing2, PI / 2, dir);
    par_shapes_scale(wing2, 1, 0.5, 3);
    par_shapes_translate(wing2, -2.7, 0, 3);

    par_shapes_merge_and_free(center, wing1);
    par_shapes_merge_and_free(center, wing2);

    par_shapes_weld(center, 0.001, NULL);
    par_shapes_compute_normals(center);

    Mesh final = MeshFromParShapeMesh(center);

    par_shapes_free_mesh(center);
    return final;
}

Mesh GenerateRockMesh(void)
{
    auto rock = par_shapes_create_rock(GetRandomValue(1, 500), 1);
    Mesh out = MeshFromParShapeMesh(rock);
    par_shapes_free_mesh(rock);
    return out;
}

static float getPitchInput()
{
    float i = 0.0f;
    if (IsKeyDown(KEY_DOWN)) i += 1;
    if (IsKeyDown(KEY_UP)) i += -1;
    return i;
}

static float getTurnInput()
{
    float i = 0.0f;
    if (IsKeyDown(KEY_LEFT)) i += -1;
    if (IsKeyDown(KEY_RIGHT)) i += 1;
    return i;
}

void Player::HandleInputs()
{
	auto turn_dir = getTurnInput();
	auto pitch_dir = getPitchInput();
	auto turn_smooth = smooth_towards(current_turn_t, turn_dir, 0.2, curr_smooth_turn_vel);
	current_turn_t = turn_smooth.x;
	curr_smooth_turn_vel = turn_smooth.y;
	auto roll_smooth = smooth_towards(current_roll_t, turn_dir, 0.4, curr_smooth_roll_vel);
	current_roll_t = roll_smooth.x;
	curr_smooth_roll_vel = roll_smooth.y;
	auto pitch_smooth = smooth_towards(curr_pitch_t, pitch_dir, 0.5, curr_smooth_pitch_vel);
	curr_pitch_t = pitch_smooth.x;
	curr_smooth_pitch_vel = pitch_smooth.y;
}


void Player::Update()
{

    HandleInputs();

    auto& pos = entityManager.getComponent<Position>(entity);
    auto velocity_local = Vector3{0.0f, curr_pitch_t * elevation_speed, -speed};
    velocity_local = Vector3RotateByQuaternion(velocity_local, pos.rotation);
    pos.position = Vector3Add(pos.position, Vector3Multiply(velocity_local, Vector3{deltaTime, deltaTime, deltaTime}));
    pos.rotation = QuaternionMultiply(pos.rotation, QuaternionFromAxisAngle(Vector3{0.0f, -1.0f, 0.0f}, turn_speed * current_turn_t * deltaTime));
    
    auto target_pitch_angle = curr_pitch_t * (pitch_angle * DEG2RAD);
    current_pitch_angle = Lerp(current_pitch_angle, target_pitch_angle, deltaTime * 8);

    auto& gpos = entityManager.getComponent<Position>(graphic);

    gpos.position = pos.position;

	gpos.rotation = pos.rotation;
    gpos.rotation = QuaternionMultiply(gpos.rotation, QuaternionFromAxisAngle(Vector3{0.0f, 0.0f, 1.0f}, -current_roll_t * (DEG2RAD * roll_angle)));
    gpos.rotation = QuaternionMultiply(gpos.rotation, QuaternionFromAxisAngle(Vector3{1.0f, 0.0f, 0.0f}, current_pitch_angle));
    //gpos.rotation = QuaternionMultiply(gpos.rotation, pos.ro)
	//player_graphic.rotate(Vector3.FORWARD, -current_roll_t * deg_to_rad(roll_angle))
	//player_graphic.rotate(Vector3.RIGHT, current_pitch_angle)

    cameraPos = Vector3Add(pos.position, Vector3RotateByQuaternion(cam_offset, pos.rotation));
    cameraTrg = Vector3Add(pos.position, Vector3RotateByQuaternion(cam_look_offset, gpos.rotation));
    
    //pos.rotation = QuaternionMultiply(pos.rotation, QuaternionFromEuler(0.5 * deltaTime, 0, 0));
}

Player::Player(ecs::EntityManager &entitymanager) : Object(entitymanager)
{
    mesh = GenerateSpaceShipMesh();
    graphic = entityManager.createEntity();
    entityManager.addComponent<Position>(graphic);
    entityManager.addComponent<MeshComponent>(graphic, mesh, true);
    entity = entityManager.createEntity();
    entityManager.addComponent<Position>(entity);
}

Player::~Player()
{
    entityManager.removeEntity(entity);
}