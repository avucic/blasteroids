#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "player.h"
#include "asteroid.h"
#include "bullet.h"
#include "collision.h"
#include "global.h"


//globals=========
const int WIDTH           = 800;
const int HEIGHT          = 400;
const int FRAME_MARGIN    = 30;
const int FPS             = 60;
const int ASTEROIDS_COUNT = 5;
const int BULLETS_COUNT   = 5;
enum KEYS {UP, DOWN, LEFT, RIGHT, SPACE};

void error(char *msg);
void initAsteroids(Asteroid asteroids[], int size);
void controllAsteroids(Asteroid asteroids[], int size);
void drawAsteroids(Asteroid asteroids[], int size);
void initBullets(Bullet bullets[], int size);
void fireBullet(Bullet bullets[], int size, Player *player);
void drawBullets(Bullet bullets[], int size);
void controllBullets(Bullet bullets[], int size);
void startAsteroids(Asteroid asteroids[], int size);
int  isCollision(Motion *m1, Motion *m2);
void collideBulletsAndAsteroids(Bullet bullets[], int bSize, Asteroid asteroids[], int aSize);
void collideAsteroidsAndPlayer(Asteroid asteroids[], Player player, int size);

int main(int argc, char **argv)
{
    ALLEGRO_TRANSFORM t;
    ALLEGRO_DISPLAY *display         = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer             = NULL;
    ALLEGRO_FONT *font18             = NULL;

    Asteroid asteroids[ASTEROIDS_COUNT];
    Bullet bullets[BULLETS_COUNT];
    Player *player = NULL;

    bool done      = false;
    bool redraw    = true;
    bool keys[5]   = {false, false, false, false, false};
    int i          = 0;

    if (!al_init())
        error("Error with game initialization!");

    al_init_primitives_addon();
    al_install_keyboard();
    al_init_font_addon();
    al_init_ttf_addon();

    display = al_create_display(WIDTH, HEIGHT);
    if (!display)
        return -1;

    event_queue = al_create_event_queue();
    timer       = al_create_timer(1.0 / FPS);

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    srand(time(NULL));

    //init game objects============================================
    //
    player = Player_create(WIDTH / 2 - 12, HEIGHT  - 20);
    if (!player)
        error("Error with game initialization!");

    initAsteroids(asteroids, ASTEROIDS_COUNT);
    initBullets(bullets, BULLETS_COUNT);

    al_start_timer(timer);

    while (!done)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            done = true;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            redraw = true;
            if (keys[RIGHT] && player->motion.x < WIDTH )   Player_move(player, EAST);
            if (keys[LEFT]  && player->motion.x > 0)        Player_move(player, WEST);
            if (keys[UP]    && player->motion.y > 10)       Player_move(player, NORTH);
            if (keys[DOWN]  && player->motion.y < HEIGHT )  Player_move(player, SOUTH);

            controllAsteroids(asteroids, ASTEROIDS_COUNT);
            controllBullets(bullets, BULLETS_COUNT);
            collideBulletsAndAsteroids(bullets, BULLETS_COUNT, asteroids, ASTEROIDS_COUNT);
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (ev.keyboard.keycode)
            {
            case ALLEGRO_KEY_ESCAPE:
                done = true;
                break;
            case ALLEGRO_KEY_UP:
                keys[UP] = true;
                break;
            case ALLEGRO_KEY_DOWN:
                keys[DOWN] = true;
                break;
            case ALLEGRO_KEY_RIGHT:
                keys[RIGHT] = true;
                break;
            case ALLEGRO_KEY_LEFT:
                keys[LEFT] = true;
                break;
            case ALLEGRO_KEY_SPACE:
                keys[SPACE] = true;
                fireBullet(bullets, BULLETS_COUNT, player);
                break;
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP)
        {
            switch (ev.keyboard.keycode)
            {
            case ALLEGRO_KEY_UP:
                keys[UP] = false;
                break;
            case ALLEGRO_KEY_DOWN:
                keys[DOWN] = false;
                break;
            case ALLEGRO_KEY_RIGHT:
                keys[RIGHT] = false;
                break;
            case ALLEGRO_KEY_LEFT:
                keys[LEFT] = false;
                break;
            case ALLEGRO_KEY_SPACE:
                keys[SPACE] = false;
                break;
            }
        }

        // Draw stuff to screen
        if (redraw && al_is_event_queue_empty(event_queue))
        {
            redraw = false;
            drawAsteroids(asteroids, ASTEROIDS_COUNT);
            Player_draw(player);
            drawBullets(bullets, BULLETS_COUNT);

            al_flip_display();
            al_clear_to_color(al_map_rgb(0, 0, 0));
        }
    }
    return 0;
}

void error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void initBullets(Bullet bullets[], int size)
{
    for (int i = 0; i < size; i++)
        Bullet_init(&bullets[i]);
}

void initAsteroids(Asteroid asteroids[], int size)
{
    for (int i = 0; i < size; i++)
    {
        asteroids[i].ID = i;
        Asteroid_init(&asteroids[i], rand() % WIDTH, -(rand() % 120));
    }
}

void controllAsteroids(Asteroid asteroids[], int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (asteroids[i].live)
        {

            asteroids[i].motion.y += asteroids[i].motion.speed;
        }
        else
        {
            asteroids[i].live     = true;
            asteroids[i].motion.x = 30 + rand() % (WIDTH - 60);
            asteroids[i].motion.y = - 100;
        }
        if (asteroids[i].motion.y > HEIGHT + 100)
            asteroids[i].live     = false;
    }
}


// void controllAsteroids(Asteroid asteroids[], int size)
// {
//     for(int i = 0; i < size; i++)
//     {
//         if(asteroids[i].live)
//         {
//             asteroids[i].motion.y += asteroids[i].motion.speed;
//             if(asteroids[i].motion.y > WIDTH)
//                 asteroids[i].live = false;
//         }
//     }
// }

void drawAsteroids(Asteroid asteroids[], int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (asteroids[i].live)
            Asteroid_draw(&asteroids[i]);
    }
}
void drawBullets(Bullet bullets[], int size)
{
    for ( int i = 0; i < size; i++)
        if (bullets[i].live)
            Bullet_draw(&bullets[i]);
}

void controllBullets(Bullet bullets[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if (bullets[i].live)
        {
            bullets[i].motion.y -= bullets[i].motion.speed;
            if (bullets[i].motion.y < 0)
                bullets[i].live = false;
        }
    }
}


int isCollision(Motion *m1, Motion *m2)
{
    if (m1->x - m1->bx < m2->x + m2->bx &&
            m1->x + m1->bx > m2->x - m2->bx &&
            m1->y - m1->by < m2->y + m2->by &&
            m1->y + m1->by > m2->y - m2->by)
    {
        return 1;
    }
    // else if (m1->x < 0)
    // {
    //     return 1;
    // }
    return 0;
}

void collideBulletsAndAsteroids(Bullet bullets[], int bSize, Asteroid asteroids[], int aSize)
{
    for (int i = 0; i < bSize; i++)
    {
        if (bullets[i].live)
        {
            for (int j = 0; j < aSize; j++)
            {
                if (asteroids[j].live)
                {
                    if (isCollision(&bullets[i].motion, &asteroids[j].motion))
                    {
                        bullets[i].live   = false;
                        asteroids[j].live = false;
                    }
                }
            }
        }
    }
}


// int collideAsteroidsAndPlayer(Asteroid asteroids[],Player player int size)
// {

// }


void fireBullet(Bullet bullets[], int size, Player *player)
{
    for ( int i = 0; i < size; i++)
    {
        if (!bullets[i].live)
        {
            bullets[i].motion.x = player->motion.x ;
            bullets[i].motion.y = player->motion.y - 17;
            bullets[i].live = true;
            break;
        }
    }
}



