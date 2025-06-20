#include <stdio.h>
#include <math.h>
#include <nds.h>
#include <filesystem.h>
#include <nf_lib.h>

#define TOP_SCREEN 0
#define BOTTOM_SCREEN 1

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

void draw(struct CrossHair *crosshair)
{
    // Draw the crosshair sprite at the new position.
    NF_MoveSprite(
        crosshair->sprite.screen, 
        crosshair->sprite.id, 
        crosshair->pos.x + crosshair->sprite.pos.x, 
        crosshair->pos.y + crosshair->sprite.pos.y);
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

void debug(uint16_t keys_held, touchPosition touch_pos)
{
    // Print touch position information to console.
    consoleClear();

    if (keys_held & KEY_TOUCH)
        consoleSetColor(NULL, CONSOLE_LIGHT_GREEN);
    else
        consoleSetColor(NULL, CONSOLE_LIGHT_RED);

    printf("Raw coords: [%d, %d]\n", touch_pos.rawx, touch_pos.rawy);
    printf("Adjusted coords: [%d, %d]\n", touch_pos.px, touch_pos.py);
    consoleSetColor(NULL, CONSOLE_DEFAULT);
}

void shoot()
{
    // Placeholder for shooting logic.
    consoleSetColor(NULL, CONSOLE_LIGHT_BLUE);
    printf("Shoot!\n");
    consoleSetColor(NULL, CONSOLE_DEFAULT);
}

int main(int argc, char **argv)
{
    uint16_t keys_held;
    touchPosition touch_pos;

    // Turn on MODE 0 on the Top Screen
    NF_Set2D(TOP_SCREEN, 0);
    NF_Set2D(BOTTOM_SCREEN, 0);
    consoleDemoInit();

    // Set the Root Folder
    nitroFSInit(NULL);
    NF_SetRootFolder("NITROFS");
    
    // Initialize 2D engine in both screens and use mode 0
    NF_Set2D(TOP_SCREEN, 0);
    NF_Set2D(BOTTOM_SCREEN, 0);

    // Initialize the Tiled Backgrounds System on the Top Screen
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(TOP_SCREEN);
    NF_InitTiledBgSys(BOTTOM_SCREEN);

    // Initialize sprite system
    NF_InitSpriteBuffers();     // Initialize storage buffers
    NF_InitSpriteSys(TOP_SCREEN);
    NF_InitSpriteSys(BOTTOM_SCREEN);

    // Load the Tiled Background
    NF_LoadTiledBg("images/bg", "bg", 256, 256);
    NF_CreateTiledBg(TOP_SCREEN, 0, "bg");

    // Load sprite files from NitroFS
    NF_LoadSpriteGfx("images/crosshair", 0, 32, 32);
    NF_LoadSpritePal("images/crosshair", 0);
    
    NF_LoadSpriteGfx("images/crosshair1", 1, 32, 32);
    NF_LoadSpritePal("images/crosshair1", 1);

    // Transfer the required sprites to VRAM
    NF_VramSpriteGfx(TOP_SCREEN, 0, 0, true);
    NF_VramSpritePal(TOP_SCREEN, 0, 0);

    NF_VramSpriteGfx(TOP_SCREEN, 1, 1, true);
    NF_VramSpritePal(TOP_SCREEN, 1, 1);

    // create cross hair
    struct CrossHair crosshair = {
        .pos = {0, 0},
        .sprite = {
            .screen = TOP_SCREEN,
            .id = 0,
            .pos = {0, 0}
        }
    };

    NF_CreateSprite(
        crosshair.sprite.screen,
        crosshair.sprite.id,
        0, 0,
        crosshair.pos.x + crosshair.sprite.pos.x,
        crosshair.pos.y + crosshair.sprite.pos.y
    );

    while (1)
    {
        swiWaitForVBlank();
        if (readInput(&keys_held, &touch_pos)) { break; }
        debug(keys_held, touch_pos);

        struct Vector2 anchor_point;
        bool is_drawing = false;
        
        if (keysDown() & KEY_TOUCH){
            anchor_point.x = touch_pos.px;
            anchor_point.y = touch_pos.py;
            is_drawing = true;
        }
        
        else if (keysUp() & KEY_TOUCH) {
            is_drawing = false;
            shoot();
        }

        crosshair.pos.x = touch_pos.px;
        crosshair.pos.y = touch_pos.py;
        draw(&crosshair);

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