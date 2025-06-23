#include <stdio.h>
#include <math.h>
#include <nds.h>
#include <filesystem.h>
#include <nf_lib.h>

#define TOP_SCREEN 0
#define BOTTOM_SCREEN 1

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 192
#define TOUCH_SENSITIVITY 2.75
#define MAX_TOUCH_DISTANCE 64

#define PI 3.14159265358979323846
#define DEBUG true

struct Vector2 {
    int x;
    int y;
};

struct Sprite2D {
    int screen;
    int id;
    struct Vector2 pos; // relative to parent
};

struct CrossHair{
    struct Sprite2D sprite;
    struct Vector2 pos;
};

struct Bow {
    struct Vector2 pos; // position of the bow
    struct Vector2 anchor_point;
    struct Vector2 relative_distance;
    float angle; // in radians
    struct Sprite2D left;
    struct Sprite2D right;
    int width; // width of the bow
};

struct Arrow {
    struct Vector2 pos_def; // default position of the arrow SPRITE
    struct Vector2 pos; // position of the arrow
    struct Sprite2D sprite;
    int width; // width of the arrow
    float angle; // angle of the arrow in radians
    bool active; // whether the arrow is active or not
    int speed; // speed of the arrow
};

void draw(struct CrossHair *crosshair)
{
    // Draw the crosshair sprite at the new position.
    NF_MoveSprite(
    crosshair->sprite.screen, 
    crosshair->sprite.id, 
    crosshair->pos.x + crosshair->sprite.pos.x, 
    crosshair->pos.y + crosshair->sprite.pos.y);
}

float rad2base512(float radians)
{
    return radians * (256.0f / PI);
}

int sign(int x) {
    return (x >> 31) | (!!x);
}

void SpriteRotScale(int screen, u8 id, s32 angle, u32 sx, u32 sy)
{
    // Angle limits
    if (angle < -512)
        angle += 512;
    if (angle > 512)
        angle -= 512;

    angle = -angle << 6; // Switch from base 512 to base 32768

    // Update rotation and scale in OAM
    if (screen == 0)
        oamRotateScale(&oamMain, id, angle, sx, sy);
    else
        oamRotateScale(&oamSub, id, angle, sx, sy);
}

bool readInput(uint16_t *keys_held, touchPosition *touch_pos)
{
    // Read key input.
    scanKeys();
    *keys_held = keysHeld();

    // If pen down, update touch input.
    if (*keys_held & KEY_TOUCH)
        touchRead(touch_pos);

    // Handle key presses.
    if ((*keys_held & (KEY_START | KEY_SELECT)) == (KEY_START | KEY_SELECT))
        return true;

    return false;
}

void shoot()
{
    // Placeholder for shooting logic.
}

int main(int argc, char **argv)
{
    uint16_t keys_held;
    touchPosition touch_pos;

    NF_Set2D(TOP_SCREEN, 0);
    NF_Set2D(BOTTOM_SCREEN, 0);
    
    consoleDemoInit();
    printf("\n NitroFS init. Please wait.\n\n");
    printf(" Iniciando NitroFS,\n por favor, espere.\n\n");
    swiWaitForVBlank();

    // Set the Root Folder
    nitroFSInit(NULL);
    NF_SetRootFolder("NITROFS");
    
    // Initialize 2D engine in both screens and use mode 0
    NF_Set2D(TOP_SCREEN, 0);
    NF_Set2D(BOTTOM_SCREEN, 1);

    // Initialize the Tiled Backgrounds System on the Top Screen
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(TOP_SCREEN);
    NF_InitTiledBgSys(BOTTOM_SCREEN);

    NF_InitTextSys(BOTTOM_SCREEN);

    // Initialize sprite system
    NF_InitSpriteBuffers();     // Initialize storage buffers
    NF_InitSpriteSys(TOP_SCREEN);
    NF_InitSpriteSys(BOTTOM_SCREEN);

    // Load the Tiled Background
    NF_LoadTiledBg("backgrounds/bg", "bg", 256, 256);
    NF_CreateTiledBg(TOP_SCREEN, 0, "bg");
    NF_CreateTiledBg(BOTTOM_SCREEN, 0, "bg");

    // Load font
    if (DEBUG){
        NF_LoadTextFont16("fonts/font16", "down", 256, 256, 0);
        NF_CreateTextLayer16(1, 0, 0, "down");
        NF_WriteText16(BOTTOM_SCREEN, 0, 1, 1, "DEBUG LINE"); // Text with default color
        NF_UpdateTextLayers();
    }
    
    // Load sprite files from NitroFS
    NF_LoadSpriteGfx("sprites/crosshair", 0, 32, 32);
    NF_LoadSpritePal("sprites/crosshair", 0);
 
    NF_LoadSpriteGfx("sprites/bow", 1, 64, 64);
    NF_LoadSpritePal("sprites/bow", 1);

    NF_LoadSpriteGfx("sprites/string", 2, 64, 64);
    NF_LoadSpritePal("sprites/string", 2);

    NF_LoadSpriteGfx("sprites/arrow", 3, 64, 64);
    NF_LoadSpritePal("sprites/arrow", 3);

    // Transfer the required sprites to VRAM
    NF_VramSpriteGfx(TOP_SCREEN, 0, 0, true);
    NF_VramSpritePal(TOP_SCREEN, 0, 0);

    NF_VramSpriteGfx(BOTTOM_SCREEN, 1, 0, true);
    NF_VramSpritePal(BOTTOM_SCREEN, 1, 0);

    NF_VramSpriteGfx(BOTTOM_SCREEN, 2, 1, true);
    NF_VramSpritePal(BOTTOM_SCREEN, 2, 1);

    NF_VramSpriteGfx(BOTTOM_SCREEN, 3, 2, true);
    NF_VramSpritePal(BOTTOM_SCREEN, 3, 2);

    NF_VramSpriteGfx(TOP_SCREEN, 3, 2, true);
    NF_VramSpritePal(TOP_SCREEN, 3, 2);

    // create cross hair
    struct CrossHair crosshair = {
        .pos = {SCREEN_WIDTH/2, SCREEN_HEIGHT},
        .sprite = {
            .screen = TOP_SCREEN,
            .id = 0,
            .pos = {-16, -16}
        }
    };

    NF_CreateSprite(
        crosshair.sprite.screen,
        crosshair.sprite.id,
        0, 0,
        crosshair.pos.x + crosshair.sprite.pos.x,
        crosshair.pos.y + crosshair.sprite.pos.y
    );

    // create bow
    struct Bow bow = {
        .pos = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2.5},
        .anchor_point = {0, 0},
        .relative_distance = {0, 0},
        .angle = 0.0f,
        .left = {
            .screen = BOTTOM_SCREEN,
            .id = 1,
            .pos = {-64, -32}
        },
        .right = {
            .screen = BOTTOM_SCREEN,
            .id = 2,
            .pos = {0, -32}
        },
        .width = 64
    };

    // left half
    NF_CreateSprite(
        bow.left.screen,
        bow.left.id,
        0, 0,
        bow.pos.x + bow.left.pos.x,
        bow.pos.y + bow.left.pos.y
    );

    NF_EnableSpriteRotScale(BOTTOM_SCREEN, bow.left.id, 1, false);
    
    // right half
    NF_CreateSprite(
        bow.right.screen,
        bow.right.id,
        0, 0,
        bow.pos.x + bow.right.pos.x,
        bow.pos.y + bow.right.pos.y
    );

    NF_EnableSpriteRotScale(BOTTOM_SCREEN, bow.right.id, 2, false);
    oamRotateScale(&oamSub, bow.right.id, 0, -256, 256);

    // string left
    NF_CreateSprite(
        bow.left.screen,
        3,
        1, 1,
        bow.pos.x + bow.left.pos.x,
        bow.pos.y + bow.left.pos.y
    );
    NF_EnableSpriteRotScale(BOTTOM_SCREEN, 3, 3, false);

    // string right
    NF_CreateSprite(
        bow.right.screen,
        4,
        1, 1,
        bow.pos.x + bow.right.pos.x,
        bow.pos.y + bow.right.pos.y
    );
    NF_EnableSpriteRotScale(BOTTOM_SCREEN, 4, 4, false);
    oamRotateScale(&oamSub, 4, 0, -256, 256);

    // arrow
    struct Arrow arrow = {
        .pos_def = {-32, -32}, // default position of the arrow sprite
        .pos = {bow.pos.x, bow.pos.y},
        .sprite = {
            .screen = BOTTOM_SCREEN,
            .id = 5,
            .pos = {-32, -32} // relative to bow position
        },
        .width = 64,
        .angle = 0.,
        .active = false,
        .speed = 2
    };

    // bottom screen sprite
    NF_CreateSprite(
        arrow.sprite.screen,
        arrow.sprite.id,
        2, 1,
        arrow.pos.x + arrow.sprite.pos.x,
        arrow.pos.y + arrow.sprite.pos.y
    );

    NF_EnableSpriteRotScale(arrow.sprite.screen, arrow.sprite.id, 5, false);

    // top screen sprite
    NF_CreateSprite(
        TOP_SCREEN,
        arrow.sprite.id,
        2, 0,
        arrow.pos.x + arrow.sprite.pos.x,
        arrow.pos.y + arrow.sprite.pos.y
    );
    NF_EnableSpriteRotScale(TOP_SCREEN, arrow.sprite.id, 5, false);
    NF_ShowSprite(TOP_SCREEN, arrow.sprite.id, false);

    struct Vector2 new_pos = {0, 0};

    while (1)
    {
        swiWaitForVBlank();
        if (readInput(&keys_held, &touch_pos)) { break; }
        
        if (keysDown() & KEY_TOUCH){
            bow.anchor_point.x = touch_pos.px;
            bow.anchor_point.y = touch_pos.py;
        }
        
        if (keys_held & KEY_TOUCH) {
            // bow.relative_distance.x = bow.anchor_point.x - touch_pos.px;
            // if (abs(bow.relative_distance.x) > MAX_TOUCH_DISTANCE)
            //     bow.relative_distance.x = sign(bow.relative_distance.x) * MAX_TOUCH_DISTANCE;

            // bow.relative_distance.y = fmax(fmin(0, bow.anchor_point.y - touch_pos.py), -MAX_TOUCH_DISTANCE);

            // crosshair.pos.x = SCREEN_WIDTH/2 + TOUCH_SENSITIVITY * bow.relative_distance.x;
            // crosshair.pos.y = SCREEN_HEIGHT + TOUCH_SENSITIVITY * bow.relative_distance.y;

            // bow.relative_distance.y = fmax(fmin(0, bow.anchor_point.y - touch_pos.py), -MAX_TOUCH_DISTANCE);

            new_pos.x = SCREEN_WIDTH/2 + TOUCH_SENSITIVITY * (bow.anchor_point.x - touch_pos.px);
            new_pos.y = SCREEN_HEIGHT + TOUCH_SENSITIVITY * (bow.anchor_point.y - touch_pos.py);
            
            if ((new_pos.x > crosshair.sprite.pos.x) && (new_pos.x < SCREEN_WIDTH - crosshair.sprite.pos.x))
            {
                if ((new_pos.y > crosshair.sprite.pos.y) && (new_pos.y < SCREEN_HEIGHT - crosshair.sprite.pos.y))
                {
                    crosshair.pos.x = new_pos.x;
                    crosshair.pos.y = new_pos.y;
                    bow.relative_distance.x = (bow.anchor_point.x - touch_pos.px);
                    bow.relative_distance.y = (bow.anchor_point.y - touch_pos.py);     
                } 
                else 
                {
                    crosshair.pos.x = new_pos.x;
                    bow.relative_distance.x = (bow.anchor_point.x - touch_pos.px);
                }
            } 
            else if ((new_pos.y > crosshair.sprite.pos.y) && (new_pos.y < SCREEN_HEIGHT - crosshair.sprite.pos.y))
            {
                crosshair.pos.y = new_pos.y;
                bow.relative_distance.y = (bow.anchor_point.y - touch_pos.py);            
            }         

            draw(&crosshair);

            // rotate bow
            bow.angle = atan2(bow.relative_distance.y, bow.relative_distance.x) + PI / 2;

            // bows
            SpriteRotScale(BOTTOM_SCREEN, bow.left.id, floor(rad2base512(bow.angle)), 256, 256);
            NF_MoveSprite(BOTTOM_SCREEN, bow.left.id, 
                bow.pos.x + bow.left.pos.x + bow.width/2 * (1 - cos(bow.angle)),
                bow.pos.y + bow.left.pos.y - bow.width/2 * sin(bow.angle));

            SpriteRotScale(BOTTOM_SCREEN, bow.right.id, floor(rad2base512(bow.angle)), -256, 256);
            NF_MoveSprite(BOTTOM_SCREEN, bow.right.id, 
                bow.pos.x + bow.right.pos.x - bow.width/2 * (1 - cos(bow.angle)), 
                bow.pos.y + bow.right.pos.y + bow.width/2 * sin(bow.angle));

            // string left
            SpriteRotScale(BOTTOM_SCREEN, 3, 
                floor(rad2base512(bow.angle - atan2(0.2 * bow.relative_distance.y, 63))), 256, 256);

            NF_MoveSprite(BOTTOM_SCREEN, 3, 
                bow.pos.x + bow.left.pos.x + bow.width/2 * (1 - cos(bow.angle)),
                bow.pos.y + bow.left.pos.y - bow.width/2 * sin(bow.angle));
            
            // string right
            SpriteRotScale(BOTTOM_SCREEN, 4,
                floor(rad2base512(bow.angle + atan2(0.2 * bow.relative_distance.y, 63))), -256, 256);

            NF_MoveSprite(BOTTOM_SCREEN, 4, 
                bow.pos.x + bow.right.pos.x - bow.width/2 * (1 - cos(bow.angle)),
                bow.pos.y + bow.right.pos.y + bow.width/2 * sin(bow.angle));

            // arrow
            if (!arrow.active){
                SpriteRotScale(BOTTOM_SCREEN, arrow.sprite.id, floor(rad2base512(bow.angle)), 256, 256);
                arrow.sprite.pos.y = arrow.pos_def.y - 0.2 * bow.relative_distance.y;
                NF_MoveSprite(BOTTOM_SCREEN, arrow.sprite.id,
                    arrow.pos.x + arrow.sprite.pos.x - (arrow.width/2 + arrow.sprite.pos.y) * sin(bow.angle), 
                    arrow.pos.y + arrow.sprite.pos.y - (arrow.width/2 + arrow.sprite.pos.y) * (1 - cos(bow.angle)));
            }
        }

        else if ((keysUp() & KEY_TOUCH) && (!arrow.active)){
            arrow.active = true;
            arrow.angle = bow.angle;
            arrow.pos.x -= (arrow.width/2 + arrow.sprite.pos.y) * sin(bow.angle);
            arrow.pos.y -= (arrow.width/2 + arrow.sprite.pos.y) * (1 - cos(bow.angle));
            shoot();
        }

        if (arrow.active){
            if ((arrow.pos.y - (arrow.width + arrow.sprite.pos.y) * sin(arrow.angle) < 0) && (arrow.sprite.screen == BOTTOM_SCREEN)){
                NF_ShowSprite(arrow.sprite.screen, arrow.sprite.id, false);
                
                arrow.sprite.screen = TOP_SCREEN;
                arrow.pos.x = crosshair.pos.x + floor(crosshair.pos.y * tan(arrow.angle));
                arrow.pos.y = SCREEN_HEIGHT;
                SpriteRotScale(arrow.sprite.screen, arrow.sprite.id, floor(rad2base512(arrow.angle)), 256, 256);
                NF_ShowSprite(arrow.sprite.screen, arrow.sprite.id, true);
            }
    
            if (arrow.sprite.screen == TOP_SCREEN){
                if (pow(arrow.pos.x - crosshair.pos.x, 2) + pow(arrow.pos.y - crosshair.pos.y, 2) < 100){
                    NF_ShowSprite(arrow.sprite.screen, arrow.sprite.id, false);
                    arrow.sprite.screen = BOTTOM_SCREEN;
                    NF_ShowSprite(arrow.sprite.screen, arrow.sprite.id, true);
                    arrow.pos.x = bow.pos.x;
                    arrow.pos.y = bow.pos.y;

                    arrow.active = false;
                }
            }

            arrow.pos.x += sin(arrow.angle) * arrow.speed;
            arrow.pos.y -= cos(arrow.angle) * arrow.speed;
            NF_MoveSprite(arrow.sprite.screen, arrow.sprite.id,
                    arrow.pos.x + arrow.sprite.pos.x,
                    arrow.pos.y + arrow.sprite.pos.y);
        }

        if (DEBUG) {
            char buffer[32];

            NF_ClearTextLayer16(BOTTOM_SCREEN, 0);
            snprintf(buffer, sizeof(buffer), "Angle: %f, %d, %d", bow.angle * 180/PI, crosshair.pos.x, crosshair.pos.y); 
            NF_WriteText16(BOTTOM_SCREEN, 0, 1, 1, buffer);            
            NF_UpdateTextLayers();
        }

        // Update OAM array
        NF_SpriteOamSet(TOP_SCREEN);
        NF_SpriteOamSet(BOTTOM_SCREEN);

        // Wait for the screen refresh
        swiWaitForVBlank();

        // Update OAM
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }

    return 0;
}