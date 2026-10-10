/* seg000: actors in the away-team rooms - animations, walking, the walk
 * graph and .ban screen animations */

#include "compat.h"
#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "bitmap.h"
#include "cache.h"
#include "gfx.h"
#include "sprite.h"
#include "resource.h"
#include "fixmath.h"
#include "actor.h"

/* Globals this file owns (declared in the headers). */
Ban bans[16];

#define ABS(x)  ((x) > 0 ? (x) : -(x))
#define SGN(x)  ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

typedef struct WalkNode {
    int16_t x;
    int16_t y;
} WalkNode;

/* NearestNode's list: node and distance. */
typedef struct NodeDist {
    int16_t idx;
    int16_t dist;
} NodeDist;

void SaveWrite(void *p, uint16_t n);    /* seg019 */
void SaveRead(void *p, uint16_t n);
int16_t WalkBlocked(int16_t x, int16_t y);        /* seg008: is (x, y) blocked */
void CheckRegions(int16_t x, int16_t y);       /* seg008 */
void PostRoomMsg(unsigned char msg, unsigned char who, unsigned char a, unsigned char b);     /* seg008: report */
int16_t DepthPri(int16_t y);               /* seg008: depth for y */
int16_t DepthScale(int16_t y);               /* seg008: scale for y */
void StandUp(Actor *a, int16_t n);    /* seg008 */

extern Bitmap *screen;
extern Bitmap *backbuf;
extern uint32_t blink_count;
extern int16_t video_mode;
/* The room's walk graph (.iw file). */
typedef struct WalkGraph {
    int16_t count;
    WalkNode nodes[32];
    unsigned char next[32][32];     /* next node from a towards b */
} WalkGraph;

WalkGraph walk;

void SaveActors(void);
void LoadActors(void);
void InitActors(void);
int16_t StartAnim(int16_t n, char *name, int16_t x, int16_t y, int16_t scale);
void LoadBan(char *name);
int16_t StartWalk(int16_t n, char *name, int16_t x, int16_t y, int16_t dx, int16_t dy);
void UpdateActors(void);
void AnimStep(int16_t n);
void WalkStep(int16_t n);
void DrawBans(void);
void UpdateBans(void);
void BanDraw(char *dst, char *src);
char *BanCopy(char *dst, char *src, char *from);
void EndOrStand(int16_t n);
void EndAnim(int16_t n);
void FreeActors(void);
void InitAnim(Actor *a, char *name, int16_t x, int16_t y, int16_t scale, int16_t add);
void release_anim(Actor *a);
void FreeAnim(Actor *a);
void FreeWalk(Actor *a);
void init_stand_anim(int16_t n);
void SetWalkFrame(Actor *a, int16_t x, int16_t y);
void SetWalk(Actor *a, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
int16_t CanWalk(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void LoadWalkGraph(char *name);
int16_t NearestNode(int16_t x, int16_t y);
int CompareDist(const void *a, const void *b);

void SaveActors(void)
{
    SaveWrite(actors, sizeof(actors));
}

void LoadActors(void)
{
    int16_t i;
    Actor *a;
    Sprite *s;

    SaveRead(actors, sizeof(actors));
    for (a = actors, i = 0; i != MAX_ACTORS; ++i, ++a) {
        if (a->state) {
            if (a->type != ANIM_WALK)
                a->anim = CacheLoad(a->name, "anm");
            s = &a->sprite;
            AddSprite(s);
            SpriteSetBitmap(s, LoadSpriteBitmap(a->bitmap, a->scale));
        }
    }
}

void InitActors(void)
{
    MemSet(actors, sizeof(actors), 0);
    MemSet(bans, sizeof(bans), 0);
    strcpy(actors[0].stand, "kstnd");
    strcpy(actors[1].stand, "sstnd");
    strcpy(actors[2].stand, "mstnd");
    strcpy(actors[3].stand, "rstnd");
}

/* Play name.anm as actor n, or the first free one from 8 for -1.
 * Returns the actor. */
int16_t StartAnim(int16_t n, char *name, int16_t x, int16_t y, int16_t scale)
{
    Actor *a;

    if (n == -1) {
        for (n = 8, a = &actors[8]; a != actors + MAX_ACTORS; ++n, ++a)
            if (a->state == 0)
                break;
        if (n == MAX_ACTORS)
            Fatal("All anims in use.");
    } else
        a = &actors[n];
    if (a->state) {
        release_anim(a);
        InitAnim(a, name, x, y, scale, 0);
    } else
        InitAnim(a, name, x, y, scale, 1);
    a->notify = 0;
    a->notify_arg = 0;
    return n;
}

void LoadBan(char *name)
{
    int16_t i;

    for (i = 0; i != 16; i++)
        if (bans[i].h == 0) {
            bans[i].h = CacheLoad(name, "ban");
            (bans + i)->pos = 0;
            break;
        }
}

/* Walk actor n from (x, y) to (dx, dy), straight or by the walk graph.
 * Returns 0 if it can't. */
int16_t StartWalk(int16_t n, char *name, int16_t x, int16_t y, int16_t dx, int16_t dy)
{
    Actor *a;
    int16_t unused[2];

    a = &actors[n];
    a->notify = 0;
    if (WalkBlocked(dx, dy))
        return 0;
    if (a->state)
        release_anim(a);
    else
        AddSprite(&a->sprite);
    a->state = 1;
    a->type = ANIM_WALK;
    a->next_time = blink_count + 1;
    strcpy(a->walk_name, name);
    a->dest_x = dx;
    a->dest_y = dy;
    a->step = 0;
    a->notify = 0;
    a->walk_node = a->dest_node = -1;
    if (CanWalk(x, y, dx, dy)) {
        SetWalk(a, x, y, dx, dy);
        SetWalkFrame(a, (a->fx + 0x8000) >> 16, (a->fy + 0x8000) >> 16);
    } else {
        a->walk_node = NearestNode(x, y);
        a->dest_node = NearestNode(dx, dy);
        if (a->walk_node == -1 || a->dest_node == -1) {
            strcat(a->walk_name, "S");
            a->dir = 'S';
            SetWalkFrame(a, x, y);
            init_stand_anim(n);
            return 0;
        }
        SetWalk(a, x, y, walk.nodes[a->walk_node].x, walk.nodes[a->walk_node].y);
        SetWalkFrame(a, (a->fx + 0x8000) >> 16, (a->fy + 0x8000) >> 16);
    }
    return 1;
}

void UpdateActors(void)
{
    int16_t i;
    Actor *a;

    for (a = actors, i = 0; i != MAX_ACTORS; ++i, ++a) {
        if (a->state) {
            switch (a->type) {
            case ANIM_STAND:
            case ANIM_PLAY:
                AnimStep(i);
                break;
            case ANIM_WALK:
                WalkStep(i);
                break;
            default:
                Fatal("invalid anim type.");
            }
        }
    }
}

/* Show the next frame of actor n's animation when it is due. */
void AnimStep(int16_t n)
{
    Actor *a;
    AnimFrame *data;
    AnimFrame *f;
    int16_t next;

    a = &actors[n];
    if (blink_count >= a->next_time) {
        data = LockMem(a->anim);
        f = data;
        next = f[a->frame].next[rand() & 3];
        if (a->frame != next && next == a->nframes - 1) {
            a->loops++;
            if (a->notify)
                PostRoomMsg(10, a->notify_arg, 0, 0);
        }
        a->frame = next;
        if (a->frame >= a->nframes) {
            UnlockMem(a->anim);
            if (a->stand[0] == 0)
                EndAnim(n);
            else
                init_stand_anim(n);
            return;
        }
        {
            char name[16];
            int16_t z;
            Sprite *s;

            s = &a->sprite;
            f += a->frame;
            MemCpy(name, f, 16);
            UnlockMem(a->anim);
            FreeSpriteBitmap(s->handle, a->scale);
            SpriteSetBitmap(s, LoadSpriteBitmap(name, a->scale));
            strncpy(a->bitmap, name, 9);
            a->bitmap[9] = 0;
            data = LockMem(a->anim);
            f = data;
            f += a->frame;
            z = DepthPri(a->y + f->y) + f->z;
            SpriteSetPos(s, a->x + f->x, a->y + f->y, z);
            a->next_time = f->delay + blink_count;
            UnlockMem(a->anim);
        }
    }
}

/* Move a walking actor one or two steps (further away is slower), and
 * on to the next walk graph node at the end of each leg. */
void WalkStep(int16_t n)
{
    Actor *a;
    Sprite *s;
    char unused1[16];
    int32_t nx;
    int32_t ny;
    char unused2[10];

    a = &actors[n];
    if (blink_count >= a->next_time) {
        s = &a->sprite;
        if (n == 0)
            CheckRegions(actors[0].x, actors[0].y);
        if (a->steps) {
            int16_t i;
            int16_t k;

            if (DepthScale((a->fy + 0x8000) >> 16) < 160)
                k = 1;
            else
                k = 2;
            for (i = 0; i != k && a->steps; ++i) {
                a->steps--;
                nx = a->fx + a->dx;
                ny = a->fy + a->dy;
                if ((a->steps & 3) == 0) {
                    FreeSpriteBitmap(s->handle, a->scale);
                    SetWalkFrame(a, (nx + 0x8000) >> 16, (ny + 0x8000) >> 16);
                    a->step++;
                }
                a->fx = nx;
                a->fy = ny;
                a->next_time = blink_count;
            }
        } else if (a->walk_node == -1) {
            if (a->notify) {
                a->notify = 0;
                PostRoomMsg(12, a->notify_arg & 0xFF, 0, 0);
            }
            FreeSpriteBitmap(s->handle, a->scale);
            SetWalkFrame(a, (a->fx + 0x8000) >> 16, (a->fy + 0x8000) >> 16);
            init_stand_anim(n);
        } else if (a->walk_node == a->dest_node) {
            a->walk_name[strlen(a->walk_name) - 1] = 0;
            a->walk_node = a->dest_node = -1;
            SetWalk(a, a->x, a->y, a->dest_x, a->dest_y);
        } else {
            a->walk_node = walk.next[a->walk_node][a->dest_node];
            a->walk_name[strlen(a->walk_name) - 1] = 0;
            SetWalk(a, a->x, a->y, walk.nodes[a->walk_node].x, walk.nodes[a->walk_node].y);
        }
    }
}

/* Draw the next frame of each .ban into the screen and back buffer. */
void DrawBans(void)
{
    int16_t n;
    Ban *b;
    char *base;
    char *p;

    b = bans;
    if ((blink_count & 3) != 0)
        return;
    for (n = 16; n != 0; --n, ++b) {
        if (b->h) {
            base = LockMem(b->h);
            p = base + b->pos;
            if (*(int16_t *)p == -1)
                p = base;
            if (((int16_t *)p)[1] != 0) {
                BanDraw(screen->data, p);
                BanDraw(backbuf->data, p);
            }
            b->pos = p - base;
            UnlockMem(b->h);
        }
    }
}

/* Update the screen where each .ban changed the back buffer. */
void UpdateBans(void)
{
    int16_t n;
    Ban *b;
    Rect r;
    char *base;
    char *p;

    b = bans;
    if ((blink_count & 3) != 0)
        return;
    for (n = 16; n != 0; --n, ++b) {
        if (b->h) {
            base = LockMem(b->h);
            p = base + b->pos;
            if (*(int16_t *)p == -1)
                p = base, b->pos = 0;
            if (((int16_t *)p)[1] != 0) {
                r.x0 = ((int16_t *)p)[2];
                r.y0 = ((int16_t *)p)[3];
                r.x1 = ((int16_t *)p)[4];
                r.y1 = ((int16_t *)p)[5];
                UnlockMem(b->h);
                RedrawSpritesIn(&r);
                base = LockMem(b->h);
                p = base + b->pos;
                p = BanCopy((char *)plat_vram(), p, backbuf->data);
            } else
                p += 4;
            b->pos = p - base;
            UnlockMem(b->h);
        }
    }
}

/* Draw a .ban frame: a start offset and run count, an 8 byte rectangle,
 * then runs (n+1 literal bytes, 0 transparent; -n: n+1 copies; 80h: skip).
 * Offsets are 16 bit sums, like the original's. */
void BanDraw(char *dst, char *src)
{
    unsigned char *p = (unsigned char *)src;
    uint16_t off;
    int16_t runs;
    unsigned n, i;

    off = *(uint16_t *)p;
    runs = ((int16_t *)p)[1];
    p += 12;
    while (--runs >= 0) {
        signed char c = *p++;

        if (c >= 0) {
            for (n = c + 1; n != 0; n--, off++) {
                unsigned char b = *p++;

                if (b)
                    dst[off] = b;
            }
        } else if (c == -128) {
            off += *(uint16_t *)p;
            p += 2;
        } else {
            n = (unsigned char)-c + 1;
            if (*p)
                for (i = 0; i < n; i++)
                    dst[(uint16_t)(off + i)] = *p;
            p++;
            off += n;
        }
    }
}

/* Copy the changed runs of a .ban frame from the same place in from into
 * dst; returns where the next frame starts. */
char *BanCopy(char *dst, char *src, char *from)
{
    unsigned char *p = (unsigned char *)src;
    uint16_t off;
    int16_t runs;
    unsigned n;

    off = *(uint16_t *)p;
    runs = ((int16_t *)p)[1];
    p += 12;
    while (--runs >= 0) {
        signed char c = *p++;

        if (c >= 0) {
            n = c + 1;
            p += n;
        } else if (c == -128) {
            off += *(uint16_t *)p;
            p += 2;
            continue;
        } else {
            n = (unsigned char)-c + 1;
            if (*p++ == 0) {
                off += n;
                continue;
            }
        }
        memcpy(dst + off, from + off, n);
        off += n;
    }
    return (char *)p;
}

void EndOrStand(int16_t n)
{
    Actor *a;

    a = &actors[n];
    if (a->stand[0] == 0)
        EndAnim(n);
    else
        init_stand_anim(n);
}

void EndAnim(int16_t n)
{
    Actor *a;

    a = &actors[n];
    if (a->state != 1)
        return;
    SpriteErase(&a->sprite);
    UpdateSprites();
    DelSprite(&a->sprite);
    release_anim(a);
}

void FreeActors(void)
{
    int16_t i;

    for (i = 0; i != MAX_ACTORS; i++)
        if (actors[i].state == 1)
            EndAnim(i);
    for (i = 0; i != 16; i++)
        if (bans[i].h) {
            CacheRelease(bans[i].h);
            bans[i].h = 0;
        }
}

/* Start name.anm on actor a. Standing animations come from the "j" set
 * when there is no second EMS page. */
void InitAnim(Actor *a, char *name, int16_t x, int16_t y, int16_t scale, int16_t add)
{
    char fname[16] = { 0 };
    AnimFrame *data;
    AnimFrame *f;
    Sprite *s;
    int16_t z;

    strcpy(fname, name);
    MemCpy(a->name, fname, 16);
    a->type = ANIM_PLAY;
    a->anim = CacheLoad(fname, "anm");
    a->nframes = GetFileSize(fname, "anm") / sizeof(AnimFrame);
    a->frame = 0;
    a->x = x;
    a->y = y;
    a->loops = 0;
    a->scale = scale;
    data = LockMem(a->anim);
    f = data;
    a->next_time = f->delay + blink_count;
    MemCpy(fname, f, 10);
    UnlockMem(a->anim);
    s = &a->sprite;
    if (add)
        AddSprite(s);
    SpriteSetBitmap(s, LoadSpriteBitmap(fname, scale));
    strncpy(a->bitmap, fname, 9);
    a->bitmap[9] = 0;
    a->scale = scale;
    data = LockMem(a->anim);
    f = data;
    z = DepthPri(a->y + f->y) + f->z;
    SpriteSetPos(s, a->x + f->x, a->y + f->y, z);
    UnlockMem(a->anim);
    a->state = 1;
}

void release_anim(Actor *a)
{
    switch (a->type) {
    case ANIM_STAND:
    case ANIM_PLAY:
        FreeAnim(a);
        break;
    case ANIM_WALK:
        FreeWalk(a);
        break;
    default:
        Fatal("release_anim:invalid anim type.");
    }
    a->state = 0;
}

void FreeAnim(Actor *a)
{
    FreeSpriteBitmap(a->sprite.handle, a->scale);
    CacheRelease(a->anim);
    a->anim = 0;
}

void FreeWalk(Actor *a)
{
    FreeSpriteBitmap(a->sprite.handle, a->scale);
}

/* Make actor n stand, facing the way it last walked. */
void init_stand_anim(int16_t n)
{
    Actor *a;
    char name[16];

    a = &actors[n];
    if (a->state == 0)
        Fatal("init_stand_anim: dead anim");
    StandUp(a, n);
    if (a->dir)
        sprintf(name, "%s%c", a->stand, a->dir);
    else
        sprintf(name, "%ss", a->stand);
    StartAnim(n, name, a->x, a->y, DepthScale(a->y));
    a->type = ANIM_STAND;
}

/* Show the walking actor's next frame at (x, y). */
void SetWalkFrame(Actor *a, int16_t x, int16_t y)
{
    char name[16];

    a->scale = DepthScale(y);
    sprintf(name, "%s%02d", a->walk_name, a->step & 7);
    SpriteSetBitmap(&a->sprite, LoadSpriteBitmap(name, a->scale));
    strncpy(a->bitmap, name, 9);
    a->bitmap[9] = 0;
    SpriteSetPos(&a->sprite, x, y, DepthPri(y));
    a->next_time = blink_count;
    a->x = x;
    a->y = y;
}

/* Set up a straight walk, along the longer axis one pixel per step. */
void SetWalk(Actor *a, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t dx;
    int16_t dy;

    a->fx = (int32_t)x0 << 16;
    a->fy = (int32_t)y0 << 16;
    dx = x1 - x0;
    dy = y1 - y0;
    if (ABS(dx) > ABS(dy)) {
        if (dx > 0)
            strcat(a->walk_name, "E"), a->dir = 'E';
        else
            strcat(a->walk_name, "W"), a->dir = 'W';
        if ((a->steps = ABS(dx)) == 0)
            return;
        a->dx = (int32_t)SGN(dx) << 16;
        a->dy = ((int32_t)dy << 16) / ABS(dx);
    } else {
        if (dy > 0)
            strcat(a->walk_name, "S"), a->dir = 'S';
        else
            strcat(a->walk_name, "N"), a->dir = 'N';
        if ((a->steps = ABS(dy)) == 0)
            return;
        a->dx = ((int32_t)dx << 16) / ABS(dy);
        a->dy = (int32_t)SGN(dy) << 16;
    }
}

/* Is the straight line from (x0, y0) to (x1, y1) clear? */
int16_t CanWalk(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int32_t dx;
    int32_t dy;
    int32_t x;
    int32_t y;
    int16_t n;

    dx = x1 - x0;
    dy = y1 - y0;
    if (ABS(dx) > ABS(dy)) {
        if ((n = ABS(dx)) == 0)
            return 1;
        dy = (dy << 16) / ABS(dx);
        dx = (int32_t)SGN(dx) << 16;
    } else {
        if ((n = ABS(dy)) == 0)
            return 1;
        dx = (dx << 16) / ABS(dy);
        dy = (int32_t)SGN(dy) << 16;
    }
    x = (int32_t)x0 << 16;
    y = (int32_t)y0 << 16;
    if (WalkBlocked((x + 0x8000) >> 16, (y + 0x8000) >> 16))
        return 0;
    while (n--) {
        x += dx;
        y += dy;
        if (WalkBlocked((x + 0x8000) >> 16, (y + 0x8000) >> 16))
            return 0;
    }
    return 1;
}

/* Load the room's walk graph from name.iw: node count, nodes, then for
 * each node the next node towards every other. */
#define READ(dst, n)    (MemCpy(dst, p, n), p += (n))

void LoadWalkGraph(char *name)
{
    int16_t i;
    char buf[0x482];
    char *p;

    MemSet(&walk, sizeof(walk), 0xFF);
    LoadFilePacked(name, "iw", (unsigned char *)buf);
    p = buf;
    READ(&walk.count, 2);
    READ(walk.nodes, sizeof(walk.nodes));
    for (i = 0; i != (int16_t)walk.count; i++) {
        READ(walk.next[i], walk.count);
    }
}

/* The nearest walk graph node that (x, y) can walk straight to, or -1. */
int16_t NearestNode(int16_t x, int16_t y)
{
    int16_t i;
    NodeDist *list;
    NodeDist *p;
    WalkNode *n;

    p = list = AllocMemPtr(walk.count * sizeof(NodeDist));
    n = walk.nodes;
    for (i = 0; i != walk.count; ++i, ++n, ++p) {
        p->idx = i;
        p->dist = FixLength(x - n->x, y - n->y, 0);
    }
    qsort(list, walk.count, sizeof(NodeDist), CompareDist);
    for (p = list, i = 0; i != walk.count; ++i, ++p)
        if (CanWalk(x, y, walk.nodes[p->idx].x, walk.nodes[p->idx].y)) {
            i = p->idx;
            FreeMemPtr(list);
            return i;
        }
    FreeMemPtr(list);
    return -1;
    Fatal("impossible walk");
}

int CompareDist(const void *a, const void *b)
{
    return (int16_t)(((const NodeDist *)a)->dist - ((const NodeDist *)b)->dist);
}
