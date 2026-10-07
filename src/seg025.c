/* seg025: space cutscenes (warp, arriving at and leaving a planet) */

#include <stdio.h>
#include "mem.h"
#include "bitmap.h"
#include "cache.h"
#include "gfx.h"
#include "event.h"
#include "sound.h"
#include "sprite.h"
#include "fixmath.h"
#include "obj3d.h"

#define VGA_MEM ((char far *)0xA0000000L)

void sub_11DCC(int n);              /* seg001 */
void sub_12FFF(void);               /* seg000 */
void sub_2F4C3(void);               /* seg020 */
void sub_2F50C(void);
void sub_2FDF2(Obj3D *o, int a, int b);
void sub_3152F(Vec32 *from, Vec32 *to, int *pitch, int *yaw);

/* seg006 */
void CamReset(void);
void AddR3(Obj3D *o);
void DelR3(Obj3D *o);
void RenderR3(int statics);

extern Bitmap far *screen;
extern Bitmap far *backbuf;
extern Vec32 cam_pos;
extern Matrix cam_orient;
extern Obj3D player_ship;
extern Obj3D dummy_planet;
extern long blink_count;

void WarpOut(void);
void OrbitArrive(void);
void OrbitDepart(void);
void StopShip(Obj3D *o);

/* The Enterprise turns away and goes to warp, seen from beside it. */
void WarpOut(void)
{
    char name[16];
    int i;
    Vec32 rel;
    Vec32 v;
    Matrix m;
    int ang;
    long dx;
    long dy;
    long dz;
    long t;
    Obj3D obj;

    MemSet(&obj, sizeof(obj), 0);
    sub_2F4C3();
    FadeOut();
    player_ship.pos.x = 0;
    player_ship.pos.y = 0;
    player_ship.pos.z = 0;
    MatIdentity(&player_ship.orient);
    MatRotYPre(0x100, &player_ship.orient);
    MemSet(screen->data, 64000, 0);
    MemSet(backbuf->data, 64000, 0);
    BlitToScreen(&screen_rect);
    cam_pos.x = FixMul(2000, player_ship.orient.e[2][0]) + player_ship.pos.x;
    cam_pos.y = FixMul(2000, player_ship.orient.e[2][1]) + player_ship.pos.y;
    cam_pos.z = FixMul(2000, player_ship.orient.e[2][2]) + player_ship.pos.z;
    cam_pos.x += FixMul(500, player_ship.orient.e[0][0]);
    cam_pos.y += FixMul(500, player_ship.orient.e[0][1]);
    cam_pos.z += FixMul(500, player_ship.orient.e[0][2]);
    cam_orient = player_ship.orient;
    rel.x = player_ship.pos.x - cam_pos.x;
    rel.y = player_ship.pos.y - cam_pos.y;
    rel.z = player_ship.pos.z - cam_pos.z;
    DigiShutdown();
    LoadSfx("WARP");
    PlayMusic(12, -1);
    for (i = 0; i != 64; i++) {
        t = GetTicks();
        MatTranspose(&cam_orient, &m);
        MatTVecMul32(&rel, &m, &v);
        ang = FixAtan2(v.x, v.z);
        MatRotYPre(ang, &cam_orient);
        if (i < 48)
            MatRotYPre(-1, &player_ship.orient);
        player_ship.pos.x += dx = FixMul(100, player_ship.orient.e[2][0]);
        player_ship.pos.y += dy = FixMul(100, player_ship.orient.e[2][1]);
        player_ship.pos.z += dz = FixMul(100, player_ship.orient.e[2][2]);
        rel.x += dx;
        rel.y += dy;
        rel.z += dz;
        if (i == 52)
            DelR3(&player_ship);
        if (i == 44)
            sub_11DCC(0x29);
        if (i == 48) {
            obj.size = 21;
            obj.reach = 225;
            obj.pos = player_ship.pos;
            obj.visible = R3_SHOT;
            obj.kind = 0;
            obj.draw = 0;
            obj.free = 0;
            AddR3(&obj);
            obj.r3s = CacheLoad("warp00", "bmp");
            obj.r3s_type = 0;
        }
        if (i > 48) {
            CacheRelease(obj.r3s);
            sprintf(name, "warp%02d", i - 48);
            obj.pos = player_ship.pos;
            obj.r3s = CacheLoad(name, "bmp");
            obj.r3s_type = 0;
        }
        FadeUp();
        sub_2F50C();
        RenderR3(0);
        SpriteErase(&mouse_sprite);
        UpdateSprites();
        sub_12FFF();
        ++blink_count;
        while ((unsigned long)t == GetTicks())
            ;
    }
    FadeOut();
    MemSet(backbuf->data, 64000, 0);
    BlitToScreen(&screen_rect);
    CamReset();
    player_ship.pos = cam_pos;
    player_ship.orient = cam_orient;
    AddR3(&player_ship);
    player_ship.visible = R3_SHIP;
    DelR3(&obj);
    CacheRelease(obj.r3s);
    FlushEvents();
    PlaySfx("bridloop");
}

/* The Enterprise slows to a stop beside the planet. */
void OrbitArrive(void)
{
    char unused[16];
    int i;
    int speed;
    int yaw;
    int pitch;
    Vec32 unused2;
    long t;
    Obj3D obj;

    sub_2F4C3();
    player_ship.pos = dummy_planet.pos;
    player_ship.pos.x += 550;
    player_ship.pos.z -= 2050;
    MatIdentity(&player_ship.orient);
    obj.pos = dummy_planet.pos;
    obj.pos.x += 900;
    MatIdentity(&obj.orient);
    cam_pos = obj.pos;
    MemSet(screen->data, 64000, 0);
    MemSet(VGA_MEM, 64000, 0);
    speed = 100;
    for (i = 0; i != 37; i++) {
        t = GetTicks();
        player_ship.pos.z += speed;
        speed -= 2;
        sub_3152F(&obj.pos, &player_ship.pos, &pitch, &yaw);
        MatRotYPre(yaw, &obj.orient);
        cam_orient = obj.orient;
        if (i < 32)
            FadeUp();
        else
            FadeDown();
        RenderR3(0);
        SpriteErase(&mouse_sprite);
        UpdateSprites();
        sub_12FFF();
        ++blink_count;
        while ((unsigned long)t == GetTicks())
            ;
    }
    FadeOut();
    MemSet(VGA_MEM, 64000, 0);
    obj.pos = dummy_planet.pos;
    obj.pos.z -= dummy_planet.reach + 250;
    MatIdentity(&obj.orient);
    MatRotYPre(0x40, &obj.orient);
    MatRotXPre(-32, &obj.orient);
    cam_orient = obj.orient;
    cam_pos = obj.pos;
    StopShip(&player_ship);
    player_ship.pos = cam_pos;
    player_ship.orient = cam_orient;
    FlushEvents();
}

/* The Enterprise turns away from the planet and speeds off. */
void OrbitDepart(void)
{
    int i;
    int speed;
    long t;
    Matrix m;
    Vec32 d;

    sub_2F4C3();
    player_ship.pos = dummy_planet.pos;
    player_ship.pos.x -= 300;
    player_ship.pos.z -= 550;
    MatIdentity(&player_ship.orient);
    MatRotYPre(0x100, &player_ship.orient);
    MatIdentity(&cam_orient);
    cam_pos = player_ship.pos;
    cam_pos.z -= 510;
    MemSet(screen->data, 64000, 0);
    MemSet(VGA_MEM, 64000, 0);
    for (speed = 25, i = 0; i != 64; ++i, speed += 4) {
        t = GetTicks();
        if (i < 32)
            MatRotYPre(8, &player_ship.orient);
        player_ship.pos.x += (long)player_ship.orient.e[2][0] * speed >> 14;
        player_ship.pos.y += (long)player_ship.orient.e[2][1] * speed >> 14;
        player_ship.pos.z += (long)player_ship.orient.e[2][2] * speed >> 14;
        d = player_ship.pos;
        Vec32Sub(&d, &cam_pos);
        if (MatFromVecs(&d.x, player_ship.orient.e[1], 0, &m))
            cam_orient = m;
        if (i < 60)
            FadeUp();
        else
            FadeDown();
        RenderR3(0);
        SpriteErase(&mouse_sprite);
        UpdateSprites();
        sub_12FFF();
        ++blink_count;
        while ((unsigned long)t == GetTicks())
            ;
    }
    FadeOut();
    MemSet(VGA_MEM, 64000, 0);
    StopShip(&player_ship);
    cam_pos = player_ship.pos;
    cam_orient = player_ship.orient;
    sub_2FDF2(&player_ship, 0, 25);
    FlushEvents();
}

void StopShip(Obj3D *o)
{
    o->move.x = o->move.y = o->move.z = o->move_speed = 0;
    o->turn.x = o->turn.y = o->turn.z = o->turn_speed = 0;
}
