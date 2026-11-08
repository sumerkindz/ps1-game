#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxpad.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define OT_LENGTH 8
#define PACKET_BYTES 8192
#define BOARD_W 10
#define BOARD_H 16
#define CELL 12
#define BOARD_X 56
#define BOARD_Y 32

static uint32_t ordering_table[OT_LENGTH];
static uint8_t packet_buffer[PACKET_BYTES];
static uint8_t pad_buffer[2][34];
static uint8_t board[BOARD_H][BOARD_W];

static const uint8_t piece_shapes[7][4] = {
    {0x0, 0xF, 0x0, 0x0},
    {0x0, 0x6, 0x6, 0x0},
    {0x0, 0x2, 0x7, 0x0},
    {0x0, 0x6, 0x3, 0x0},
    {0x0, 0x3, 0x6, 0x0},
    {0x0, 0x1, 0x7, 0x0},
    {0x0, 0x4, 0x7, 0x0}
};

static const uint8_t colors[8][3] = {
    {0, 0, 0},
    {55, 215, 245},
    {250, 220, 65},
    {185, 90, 245},
    {80, 225, 125},
    {245, 80, 95},
    {75, 120, 250},
    {250, 155, 60}
};

typedef struct {
    uint8_t cells[4][4];
    int x;
    int y;
    uint8_t color;
} Piece;

static Piece current_piece;
static uint8_t next_piece;
static uint32_t random_state = 0x51A7u;
static uint32_t score;
static uint16_t lines_cleared;
static uint16_t previous_buttons;
static uint16_t left_frames;
static uint16_t right_frames;
static uint16_t frame_count;
static int game_over;

static uint32_t next_random(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

static uint8_t random_piece(void) {
    return (uint8_t)(next_random() % 7);
}

static void load_shape(Piece *piece, uint8_t type) {
    int x;
    int y;

    memset(piece->cells, 0, sizeof(piece->cells));
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            piece->cells[y][x] = (piece_shapes[type][y] >> x) & 1;
        }
    }
    piece->color = (uint8_t)(type + 1);
}

static int collides(const Piece *piece, int x_offset, int y_offset) {
    int x;
    int y;

    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            int board_x;
            int board_y;

            if (!piece->cells[y][x]) {
                continue;
            }

            board_x = piece->x + x + x_offset;
            board_y = piece->y + y + y_offset;
            if (board_x < 0 || board_x >= BOARD_W || board_y >= BOARD_H) {
                return 1;
            }
            if (board_y >= 0 && board[board_y][board_x]) {
                return 1;
            }
        }
    }
    return 0;
}

static void spawn_piece(void) {
    load_shape(&current_piece, next_piece);
    current_piece.x = 3;
    current_piece.y = 0;
    next_piece = random_piece();
    if (collides(&current_piece, 0, 0)) {
        game_over = 1;
    }
}

static void reset_game(void) {
    memset(board, 0, sizeof(board));
    score = 0;
    lines_cleared = 0;
    game_over = 0;
    next_piece = random_piece();
    spawn_piece();
}

static void rotate_piece(void) {
    Piece rotated = current_piece;
    int x;
    int y;

    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            rotated.cells[y][x] = current_piece.cells[3 - x][y];
        }
    }

    if (!collides(&rotated, 0, 0)) {
        current_piece = rotated;
    } else if (!collides(&rotated, -1, 0)) {
        rotated.x--;
        current_piece = rotated;
    } else if (!collides(&rotated, 1, 0)) {
        rotated.x++;
        current_piece = rotated;
    }
}

static void lock_piece(void) {
    int x;
    int y;

    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            int board_x = current_piece.x + x;
            int board_y = current_piece.y + y;

            if (current_piece.cells[y][x]) {
                if (board_y < 0) {
                    game_over = 1;
                    return;
                }
                board[board_y][board_x] = current_piece.color;
            }
        }
    }

    {
        int row = BOARD_H - 1;
        int cleared = 0;

        while (row >= 0) {
            int full = 1;
            for (x = 0; x < BOARD_W; x++) {
                if (!board[row][x]) {
                    full = 0;
                    break;
                }
            }

            if (full) {
                int above;
                for (above = row; above > 0; above--) {
                    memcpy(board[above], board[above - 1], BOARD_W);
                }
                memset(board[0], 0, BOARD_W);
                cleared++;
            } else {
                row--;
            }
        }

        if (cleared) {
            static const uint16_t points[] = {0, 100, 300, 500, 800};
            lines_cleared = (uint16_t)(lines_cleared + cleared);
            score += points[cleared] * (uint32_t)(lines_cleared / 10 + 1);
        }
    }

    spawn_piece();
}

static void add_tile(uint8_t **packet, int x, int y, int w, int h,
                     uint8_t r, uint8_t g, uint8_t b, int priority) {
    TILE *tile = (TILE *)*packet;
    setTile(tile);
    setXY0(tile, x, y);
    setWH(tile, w, h);
    setRGB0(tile, r, g, b);
    addPrim(&ordering_table[priority], tile);
    *packet += sizeof(TILE);
}

static void draw_block(uint8_t **packet, int x, int y, uint8_t color,
                       int ghost) {
    const uint8_t *rgb = colors[color];
    uint8_t r = ghost ? (uint8_t)(rgb[0] / 3) : rgb[0];
    uint8_t g = ghost ? (uint8_t)(rgb[1] / 3) : rgb[1];
    uint8_t b = ghost ? (uint8_t)(rgb[2] / 3) : rgb[2];

    add_tile(packet, x + 1, y + 1, CELL - 2, CELL - 2, r, g, b, 2);
}

static void draw_game(void) {
    char score_text[24];
    char lines_text[24];
    uint8_t *packet = packet_buffer;
    Piece ghost = current_piece;
    uint8_t *text_packet;
    int x;
    int y;

    ClearOTagR(ordering_table, OT_LENGTH);
    add_tile(&packet, 0, 0, SCREEN_W, SCREEN_H, 8, 12, 31, 5);
    add_tile(&packet, BOARD_X - 3, BOARD_Y - 3,
             BOARD_W * CELL + 6, BOARD_H * CELL + 6, 55, 68, 105, 4);
    add_tile(&packet, BOARD_X, BOARD_Y,
             BOARD_W * CELL, BOARD_H * CELL, 12, 19, 43, 3);

    for (y = 0; y < BOARD_H; y++) {
        for (x = 0; x < BOARD_W; x++) {
            if (board[y][x]) {
                draw_block(&packet, BOARD_X + x * CELL,
                           BOARD_Y + y * CELL, board[y][x], 0);
            }
        }
    }

    if (!game_over) {
        while (!collides(&ghost, 0, 1)) {
            ghost.y++;
        }
        for (y = 0; y < 4; y++) {
            for (x = 0; x < 4; x++) {
                if (ghost.cells[y][x]) {
                    draw_block(&packet,
                               BOARD_X + (ghost.x + x) * CELL,
                               BOARD_Y + (ghost.y + y) * CELL,
                               ghost.color, 1);
                }
            }
        }

        for (y = 0; y < 4; y++) {
            for (x = 0; x < 4; x++) {
                if (current_piece.cells[y][x]) {
                    draw_block(&packet,
                               BOARD_X + (current_piece.x + x) * CELL,
                               BOARD_Y + (current_piece.y + y) * CELL,
                               current_piece.color, 0);
                }
            }
        }
    }

    {
        Piece preview;
        load_shape(&preview, next_piece);
        for (y = 0; y < 4; y++) {
            for (x = 0; x < 4; x++) {
                if (preview.cells[y][x]) {
                    draw_block(&packet, 198 + x * 10, 136 + y * 10,
                               preview.color, 0);
                }
            }
        }
    }

    text_packet = (uint8_t *)FntSort(&ordering_table[0], packet,
                                    194, 24, "BLOCK DROP");
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 48, "SCORE");
    snprintf(score_text, sizeof(score_text), "%lu", (unsigned long)score);
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 60, score_text);
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 82, "LINES");
    snprintf(lines_text, sizeof(lines_text), "%u", lines_cleared);
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 94, lines_text);
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 120, "NEXT");
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 184, "LEFT/RIGHT MOVE");
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 198, "X/O ROTATE");
    text_packet = (uint8_t *)FntSort(&ordering_table[0], text_packet,
                                    194, 212, "UP DROP");
    if (game_over) {
        FntSort(&ordering_table[0], text_packet,
                72, 112, "GAME OVER - PRESS START");
    }

    DrawSync(0);
    VSync(0);
    DrawOTag(&ordering_table[OT_LENGTH - 1]);
}

int main(void) {
    DISPENV display_env;
    DRAWENV draw_env;
    PADTYPE *pad;

    ResetGraph(0);
    FntLoad(960, 0);
    memset(pad_buffer, 0xFF, sizeof(pad_buffer));
    InitPAD(pad_buffer[0], sizeof(pad_buffer[0]),
            pad_buffer[1], sizeof(pad_buffer[1]));
    StartPAD();

    SetDefDrawEnv(&draw_env, 0, 0, SCREEN_W, SCREEN_H);
    SetDefDispEnv(&display_env, 0, 0, SCREEN_W, SCREEN_H);
    setRGB0(&draw_env, 8, 12, 31);
    draw_env.isbg = 1;
    PutDrawEnv(&draw_env);
    PutDispEnv(&display_env);
    SetDispMask(1);

    reset_game();

    for (;;) {
        uint16_t buttons = 0;
        uint16_t pressed;
        int gravity_frames = 30 - (lines_cleared / 10) * 2;
        if (gravity_frames < 5) {
            gravity_frames = 5;
        }

        pad = (PADTYPE *)pad_buffer[0];
        if (pad->stat == 0) {
            buttons = (uint16_t)~pad->btn;
        }
        pressed = (uint16_t)(buttons & (uint16_t)~previous_buttons);

        if (game_over) {
            if (pressed & PAD_START) {
                reset_game();
            }
        } else {
            if (buttons & PAD_LEFT) {
                if (!(previous_buttons & PAD_LEFT)) {
                    if (!collides(&current_piece, -1, 0)) {
                        current_piece.x--;
                    }
                    left_frames = 0;
                } else if (++left_frames >= 12 && (left_frames - 12) % 4 == 0) {
                    if (!collides(&current_piece, -1, 0)) {
                        current_piece.x--;
                    }
                }
            } else {
                left_frames = 0;
            }

            if (buttons & PAD_RIGHT) {
                if (!(previous_buttons & PAD_RIGHT)) {
                    if (!collides(&current_piece, 1, 0)) {
                        current_piece.x++;
                    }
                    right_frames = 0;
                } else if (++right_frames >= 12 && (right_frames - 12) % 4 == 0) {
                    if (!collides(&current_piece, 1, 0)) {
                        current_piece.x++;
                    }
                }
            } else {
                right_frames = 0;
            }

            if (pressed & (PAD_CROSS | PAD_CIRCLE)) {
                rotate_piece();
            }

            if (pressed & PAD_UP) {
                while (!collides(&current_piece, 0, 1)) {
                    current_piece.y++;
                    score += 2;
                }
                lock_piece();
            } else if ((buttons & PAD_DOWN) && frame_count % 3 == 0) {
                if (!collides(&current_piece, 0, 1)) {
                    current_piece.y++;
                    score++;
                } else {
                    lock_piece();
                }
            } else if (frame_count % (uint16_t)gravity_frames == 0) {
                if (!collides(&current_piece, 0, 1)) {
                    current_piece.y++;
                } else {
                    lock_piece();
                }
            }
        }

        previous_buttons = buttons;
        frame_count++;
        draw_game();
    }

    StopPAD();
    return 0;
}
