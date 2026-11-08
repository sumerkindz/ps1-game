#include <stdint.h>
#include <psxgpu.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define OT_LENGTH 8
#define PACKET_BYTES 4096

static uint32_t ordering_table[OT_LENGTH];
static uint8_t packet_buffer[PACKET_BYTES];

int main(void) {
    DISPENV display_env;
    DRAWENV draw_env;

    ResetGraph(0);
    FntLoad(960, 0);

    SetDefDrawEnv(&draw_env, 0, 0, SCREEN_W, SCREEN_H);
    SetDefDispEnv(&display_env, 0, 0, SCREEN_W, SCREEN_H);
    setRGB0(&draw_env, 12, 18, 42);
    draw_env.isbg = 1;

    PutDrawEnv(&draw_env);
    PutDispEnv(&display_env);
    SetDispMask(1);

    int x = 20;
    int y = 80;
    int dx = 2;
    int dy = 1;

    for (;;) {
        if (x <= 0 || x >= SCREEN_W - 32) dx = -dx;
        if (y <= 0 || y >= SCREEN_H - 32) dy = -dy;
        x += dx;
        y += dy;

        ClearOTagR(ordering_table, OT_LENGTH);

        TILE *square = (TILE *)packet_buffer;
        setTile(square);
        setXY0(square, x, y);
        setWH(square, 32, 32);
        setRGB0(square, 255, 210, 40);
        addPrim(&ordering_table[1], square);

        FntSort(&ordering_table[0], packet_buffer + sizeof(TILE),
                8, 16, "MY FIRST PLAYSTATION GAME");
        FntSort(&ordering_table[0], packet_buffer + sizeof(TILE) + 1024,
                8, 32, "A yellow square is moving!");

        DrawSync(0);
        VSync(0);
        DrawOTag(&ordering_table[OT_LENGTH - 1]);
    }

    return 0;
}
