#include <stdio.h>
#include <math.h>
#include <nds.h>
#include <gl2d.h>

#include <nf_lib.h>
#include <filesystem.h>

#define TOUCH_BOX_RADIUS 4
#define TOP_SCREEN 0
#define BOTTOM_SCREEN 1

struct Vector2 {
    int x;
    int y;
};

struct CrossHair{
    int x;
    int y;

    int screen;
    int sprite_id;
};

void draw(struct CrossHair *crosshair)
{
    // Draw the crosshair sprite at the new position.
    NF_MoveSprite(crosshair->screen, crosshair->sprite_id, crosshair->x, crosshair->y);
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

    // Initialize display:
    consoleDemoInit();

    // Turn on MODE 0 on the Top Screen
    NF_Set2D(TOP_SCREEN, 0);
    NF_Set2D(BOTTOM_SCREEN, 0);
    
    // Set the Root Folder
    nitroFSInit(NULL);
    NF_SetRootFolder("NITROFS");
    
    // Initialize the Tiled Backgrounds System on the Top Screen
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(TOP_SCREEN);
    NF_InitTiledBgSys(1);       // Bottom screen

    // Initialize sprite system
    NF_InitSpriteBuffers();     // Initialize storage buffers
    NF_InitSpriteSys(TOP_SCREEN);        // Top screen
    NF_InitSpriteSys(BOTTOM_SCREEN);        // Bottom screen

    // Load the Tiled Background
    NF_LoadTiledBg("images/bg", "bg", 256, 256);
    NF_CreateTiledBg(TOP_SCREEN, 0, "bg");

    // Load sprite files from NitroFS
    NF_LoadSpriteGfx("images/crosshair", 0, 32, 32);
    NF_LoadSpritePal("images/crosshair", 0);

    // Transfer the required sprites to VRAM
    NF_VramSpriteGfx(TOP_SCREEN, 0, 0, true); // Ball: Keep all frames in VRAM
    NF_VramSpritePal(TOP_SCREEN, 0, 0);

    // Create crosshair sprite
    struct CrossHair crosshair;
    crosshair.x = 0;
    crosshair.y = 0;
    crosshair.screen = TOP_SCREEN;
    crosshair.sprite_id = 0;
    NF_CreateSprite(crosshair.screen, crosshair.sprite_id, 0, 0, crosshair.x, crosshair.y);

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

        crosshair.x = touch_pos.px;
        crosshair.y = touch_pos.py;
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