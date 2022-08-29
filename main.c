#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "ctype.h"
#include "math.h"
#include "stdbool.h"
#include "GL/glut.h"
#include "GL/gl.h"
#include "GL/glu.h"

#define WIN_WIDTH  400
#define WIN_HEIGHT 800
#define CELL_SIZE (WIN_WIDTH/10)

struct color {
    float r;
    float g;
    float b;
    float a;
};

struct coords {
    int x;
    int y;
    bool destroyed;
};

enum block_types {
    I_BLOCK = 0,
    J_BLOCK,
    L_BLOCK,
    O_BLOCK,
    S_BLOCK,
    T_BLOCK,
    Z_BLOCK
};

struct block {
    struct color clr;
    int type;
    struct coords* bricks;
};


bool ENABLE_GRID_RENDER = false;
int FALL_SPEED = 300;
int SCORE = 0;
int LINES_SCORES[4] = { 
    40,
    100,
    300,
    1200,
};
char* SCORE_STR = NULL;

struct block* scene_blocks[50] = { NULL };
struct block* active_block = NULL;

struct color back_clr = { .r = 0.0f, .g = 0.1f, .b = 0.1f, .a = 1.0f };

void new_block(int block_type, struct color clr) {
    struct block* b = malloc(sizeof(struct block));
    b->type = block_type;
    b->clr = clr;
    b->bricks = malloc(4*sizeof(struct coords));

    struct coords init_pos = { .x = CELL_SIZE*3, .y = WIN_HEIGHT-CELL_SIZE*2};
    b->bricks[0] = init_pos;

    switch(block_type) {
        case I_BLOCK:
            b->bricks[1] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y};
            b->bricks[2] = (struct coords) { .x = init_pos.x + CELL_SIZE*2, .y = init_pos.y};
            b->bricks[3] = (struct coords) { .x = init_pos.x + CELL_SIZE*3, .y = init_pos.y};
            break;
        case J_BLOCK:
            b->bricks[1] = (struct coords) { .x = init_pos.x,               .y = init_pos.y + CELL_SIZE};
            b->bricks[2] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y};
            b->bricks[3] = (struct coords) { .x = init_pos.x + CELL_SIZE*2, .y = init_pos.y};
            break;
        case L_BLOCK:
            b->bricks[1] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y};
            b->bricks[2] = (struct coords) { .x = init_pos.x + CELL_SIZE*2, .y = init_pos.y};
            b->bricks[3] = (struct coords) { .x = init_pos.x + CELL_SIZE*2, .y = init_pos.y + CELL_SIZE};
            break;
        case O_BLOCK:
            init_pos.x += CELL_SIZE;
            b->bricks[0].x += CELL_SIZE;
            b->bricks[1] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y};
            b->bricks[2] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y + CELL_SIZE};
            b->bricks[3] = (struct coords) { .x = init_pos.x,               .y = init_pos.y + CELL_SIZE};
            break;
        case S_BLOCK:
            b->bricks[1] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y};
            b->bricks[2] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y + CELL_SIZE};
            b->bricks[3] = (struct coords) { .x = init_pos.x + CELL_SIZE*2, .y = init_pos.y + CELL_SIZE};
            break;
        case T_BLOCK:
            b->bricks[1] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y};
            b->bricks[2] = (struct coords) { .x = init_pos.x + CELL_SIZE*2, .y = init_pos.y};
            b->bricks[3] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y + CELL_SIZE};
            break;
        case Z_BLOCK:
            b->bricks[0].y += CELL_SIZE;
            b->bricks[1] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y};
            b->bricks[2] = (struct coords) { .x = init_pos.x + CELL_SIZE,   .y = init_pos.y + CELL_SIZE};
            b->bricks[3] = (struct coords) { .x = init_pos.x + CELL_SIZE*2, .y = init_pos.y};

        default:
            break;
    };

    // setting destroyed status
    for (int i = 0; i < 4; i++) {
        b->bricks[i].destroyed = false;
    }

    // add new block to scene blocks
    int i = 0;
    while (scene_blocks[i] != NULL) {
        i++;
    }
    scene_blocks[i] = b;
    active_block = b;
}

void gen_block() {
    int type = rand() % 7;
    int brightness = 5;
    float clr_r = (float) (rand() % 10)/brightness;
    float clr_g = (float) (rand() % 10)/brightness;
    float clr_b = (float) (rand() % 10)/brightness;
    struct color clr = { .r = clr_r, .g = clr_g, .b = clr_b, .a = 1.0f };
    new_block(type, clr);
}

void slice_lines() {
    if (active_block == NULL) return;
    // finding minimal y of probable sliced line
    struct coords* bricks = active_block->bricks;
    int min_y = CELL_SIZE * 10;
    for (int i = 0; i < 4; i++) {
        if (bricks[i].y < min_y) {
            min_y = bricks[i].y;
        }
    }

    int lines_bricks_count[4] = { 0, 0, 0, 0};
    struct coords* lines_bricks[4][10] = { { NULL } };
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 50; j++) {
            if (scene_blocks[j] != NULL) {
                bricks = scene_blocks[j]->bricks;
                int destroyed_count = 0;
                for (int k = 0; k < 4; k++) {
                    if (!bricks[k].destroyed) {
                        if (bricks[k].y == (min_y + CELL_SIZE*i)) {
                            lines_bricks_count[i]++;
                            lines_bricks[i][lines_bricks_count[i]-1] = &(bricks[k]);
                        }
                    }
                    else destroyed_count++;
                }
                // deleting block if all bricks were destroyed
                if (destroyed_count == 4) {
                    free(scene_blocks[j]);
                    scene_blocks[j] = NULL;
                }
            }
        }    
    }

    // destroying brick in sliced lines
    struct coords* brick;
    int sliced_lines_count = 0;
    for (int i = 0; i < 4; i++) {
        if (lines_bricks_count[i] == 10) {
            for (int j = 0; j < 10; j++) {
                brick = lines_bricks[i][j];
                brick->destroyed = true;
            }
            sliced_lines_count++;
        }
    }

    // performing bricks falling, which situated above sliced lines
    for (int i = 0; i < 50; i++) {
        if (scene_blocks[i] != NULL) {
            for (int j = 0; j < 4; j++) {
                brick = &(scene_blocks[i]->bricks[j]);
                if (!brick->destroyed && sliced_lines_count > 0) {
                    int idx = (int)((brick->y-min_y)/CELL_SIZE);
                    int sliced_below = 0;
                    for (int k = 0; k < idx; k++)
                        if (lines_bricks_count[k] == 10)
                            sliced_below++;
                    brick->y -= (CELL_SIZE * sliced_below);
                }
            }
        }
    }

    //updating score
    SCORE += LINES_SCORES[sliced_lines_count-1];
}

void init(void) {
    glClearColor(back_clr.r, back_clr.g , back_clr.b , back_clr.a);
    glColor3f(1.0f , 1.0f , 1.0f);
    glPointSize(4.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0 , WIN_WIDTH , 0.0 , WIN_HEIGHT);
}

void draw_grid() {
    if (!ENABLE_GRID_RENDER) return;
    glColor3f(0.0f, 1.0f, 0.0f);
    glPointSize(1.0);
    for (int i = 0; i < WIN_HEIGHT; i += CELL_SIZE) {
        glBegin(GL_LINES);
        glVertex2d(0, i);
        glVertex2d(WIN_WIDTH, i);
        glEnd();
    }
    for (int i = 0; i < WIN_WIDTH; i += CELL_SIZE) {
        glBegin(GL_LINES);
        glVertex2d(i, 0);
        glVertex2d(i, WIN_HEIGHT);
        glEnd();
    }
}

void draw_scene() {
    struct coords brick;
    struct color clr;
    for (int i = 0; i < 50; i++) {
        if (scene_blocks[i] != NULL) {
            clr = scene_blocks[i]->clr;
            glColor4f(clr.r, clr.g, clr.b, clr.a);
            for (int j = 0; j < 4; j++) {
                brick = scene_blocks[i]->bricks[j];
                if (!brick.destroyed) {
                    glRectd(brick.x, brick.y, brick.x+CELL_SIZE, brick.y+CELL_SIZE);
                }
            }
        }
    }
}

void draw_score() {
    if (SCORE_STR != NULL)
        free(SCORE_STR);
    char* SCORE_STR = malloc(15);
    sprintf(SCORE_STR, "Score: %d", SCORE);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(WIN_WIDTH-CELL_SIZE*4, WIN_HEIGHT-CELL_SIZE);
    for (char* c = SCORE_STR; *c != '\0'; c++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    
    draw_grid();
    draw_scene();
    draw_score();

    glFlush();
}

void finish_game() {
    for (int i = 0; i < 50; i++) {
        free(scene_blocks[i]);
        scene_blocks[i] = NULL;
    }
    active_block = NULL;
    SCORE = 0;
}

bool is_active_collided(bool exclude_sides) {
    if (active_block == NULL) return false;
    // collision with bounds
    struct coords* bricks = active_block->bricks;
    for (int i = 0; i < 4; i++) {
        if ( (!exclude_sides && (bricks[i].x < 0 || bricks[i].x >= WIN_WIDTH)) || bricks[i].y < 0) {
            return true;
        }
    }
    // collision with other blocks
    struct coords* target_bricks;
    for (int i = 0; i < 50; i++) {
        if (scene_blocks[i] != NULL && scene_blocks[i] != active_block) {
            target_bricks = scene_blocks[i]->bricks;
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    if (!target_bricks[j].destroyed && !bricks[k].destroyed) {
                        if (target_bricks[j].x == bricks[k].x && target_bricks[j].y == bricks[k].y) {
                            return true;
                        }
                    }
                }
                
            }
        }
    }
    return false;
}

void block_fall_timer(int t) {
    if (active_block != NULL) {
        for (int i = 0; i < 4; i++) {
            active_block->bricks[i].y -= CELL_SIZE;
        }
        if (is_active_collided(true)) {
            // back to previous state
            for (int i = 0; i < 4; i++) {
                if (active_block->bricks[i].y >= WIN_HEIGHT-CELL_SIZE*3) {
                    finish_game();
                    break;
                }
                active_block->bricks[i].y += CELL_SIZE;
            }
            slice_lines();
            gen_block();
        }
    }

    glutPostRedisplay();
    if (!t)
        glutTimerFunc(FALL_SPEED, block_fall_timer, 0);
}

void rotate_active(bool reverse) {
    if (active_block == NULL)          return;
    if (active_block->type == O_BLOCK) return;
    struct coords* bricks = active_block->bricks;
    int init_x = bricks[0].x;
    int init_y = bricks[0].y;
    for (int i = 0; i < 4; i++) {
        bricks[i].x -= init_x;
        bricks[i].y -= init_y;
    }
    
    int tmp;
    for (int i = 0; i < 4; i++) {
        tmp = bricks[i].x;
        if (reverse) {
           bricks[i].x = bricks[i].y;
           bricks[i].y = -tmp;
        }
        else {
            bricks[i].x = -bricks[i].y;
            bricks[i].y = tmp;
        }
    }

    for (int i = 0; i < 4; i++) {
        bricks[i].x += init_x;
        bricks[i].y += init_y;
    }

    if (is_active_collided(false)) {
        rotate_active(!reverse);
    }

}

void on_keyboard_down(unsigned char key, int x, int y) {
    if (active_block != NULL) {
        switch (tolower(key)) {
            case 'd':
                for (int i = 0; i < 4; i++) {
                    active_block->bricks[i].x += CELL_SIZE;
                }
                if (is_active_collided(false)) {
                    for (int i = 0; i < 4; i++) {
                        active_block->bricks[i].x -= CELL_SIZE;
                    }
                }
                break;
            case 'a':
                for (int i = 0; i < 4; i++) {
                    active_block->bricks[i].x -= CELL_SIZE;
                }
                if (is_active_collided(false)) {
                    for (int i = 0; i < 4; i++) {
                        active_block->bricks[i].x += CELL_SIZE;
                    }
                }
                break;
            case 's':
                block_fall_timer(1);
                break;
            case 'e':
                rotate_active(true);
                break;
            case ' ':
                rotate_active(false);
                break;
            case 'g':
                ENABLE_GRID_RENDER = !ENABLE_GRID_RENDER;
        };
        glutPostRedisplay();
    }
}


int main(int argc , char** argv) {
    srand(time(NULL));
    glutInit(&argc , argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutInitWindowPosition(700 , 100);
    glutCreateWindow("Tetris?");
    glutTimerFunc(FALL_SPEED, block_fall_timer, 0);
    glutDisplayFunc(display);
    glutKeyboardFunc(on_keyboard_down);
    init();
    
    gen_block();

    glutMainLoop();
    return 0;
}

