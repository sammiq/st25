/* seg006: the 3D space view (ships, planets, shots and explosions) */

#include "compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mem.h"
#include "bitmap.h"
#include "cache.h"
#include "gfx.h"
#include "sprite.h"
#include "fixmath.h"
#include "obj3d.h"


#define ABS(x)  ((x) > 0 ? (x) : -(x))
#define SGN(x)  ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

#define MAX_R3  48

/* One row of a filled shape. */
typedef struct Span {
    int16_t right;
    int16_t left;
} Span;

typedef struct Point {
    int16_t x;
    int16_t y;
} Point;

void MsgBox(char *title, char *msg, int16_t a, int16_t b, int16_t c, int16_t d);    /* seg008: message box */
void GameOverMenu(void);               /* seg019: game over */
void SaveWrite(void *p, int16_t n);     /* seg019: save game I/O */
void SaveRead(void *p, int16_t n);
void BridgeSfx(int16_t n);              /* seg001 */
void SetRepair(int16_t n);

/* seg023 */
void StarsReset(void);
void DrawStars(void);

/* seg027 */
int16_t Project(int16_t a, int16_t z);

/* seg018 */
int16_t DrawPlanet(Obj3D *o);
void FreePlanet(Obj3D *o);

int16_t view_cx;
int16_t view_cy;
int16_t view_w;                  /* half the view width */
int16_t view_h;
int16_t view_x0;
int16_t view_x1;
int16_t view_y0;
int16_t view_y1;
Rect clip_rect;
char *view_ptr;          /* first row of the view on the screen */
extern Bitmap *screen;
extern Bitmap *backbuf;
int32_t proj_dist;
float star_near;
Vec32 fr_left;               /* frustum edges */
Vec32 fr_right;
Vec32 fr_top;
Vec32 fr_bottom;
extern Vec32 cam_pos;
extern Matrix cam_orient;
Matrix cam_matrix;           /* cam_orient transposed */
extern Sprite view_sprite;           /* the view, as a sprite */
extern Obj3D player_ship;
extern Shot shot_obj;
extern Obj3D dummy_planet;
extern Obj3D ships[4];
extern uint32_t blink_count;
extern unsigned char byte_45B3C[];
int16_t span_fill;
extern int16_t view_tab[24][13];        /* bitmap for each yaw and pitch, 0x8000 mirrored */

Obj3D *r3_list[MAX_R3] = { 0 };
Obj3D *r3_sorted[MAX_R3] = { 0 };
char *shape_names[4] = { "ENT", "KLING", "ORION", "ROM" };
unsigned char dither[8] = { 0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0 };

void CamReset(void);
void SetView(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t fov);
void DrawStatic(void);
void ClearR3List(void);
void AddR3(Obj3D *o);
void DelR3(Obj3D *o);
void ClearView(void);
void DrawStarfield(void);
int CompareR3(const void *a, const void *b);
void RenderR3(int16_t statics);
Obj3D *ProjectR3(Obj3D *o);
int16_t LoadShotShape(Obj3D *o);
void FreeShotShape(Obj3D *o);
int16_t SelectView(Obj3D *o);
void ReleaseShipShape(Obj3D *o);
double DDiv(double a, double b);
void DrawR3Shape(Obj3D *o);
int16_t InView(Obj3D *o);
void OrthoR3(void);
void DrawBeam(Obj3D *o);
void SpanCircle(Span *s, int16_t cx, int16_t cy, int16_t r);
void SpanQuad(Span *s, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t r0, int16_t r1);
void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t c);
Point *LinePoints(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void SpanLine(Span *s, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void FillSpans(Span *s, int16_t c);
void OutlineSpans(Span *s, int16_t c);
void SaveR3(Obj3D *o);
void SaveR3Shot(Obj3D *o);
void RestoreR3(Obj3D *o);
void RestoreR3Shot(Obj3D *o);
void SaveSpace(void);
void LoadSpace(void);

void CamReset(void)
{
    MemSet(&cam_pos, sizeof(cam_pos), 0);
    MatIdentity(&cam_orient);
}

/* Set the view rectangle and the field of view. */
void SetView(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t fov)
{
    view_cx = (x0 + x1) / 2;
    view_cy = (y0 + y1) / 2;
    view_w = (x1 - x0 + 1) / 2;
    view_h = (y1 - y0 + 1) / 2;
    clip_rect.x0 = view_x0 = view_cx - view_w;
    clip_rect.x1 = view_x1 = view_cx + view_w;
    clip_rect.y0 = view_y0 = view_cy - view_h;
    clip_rect.y1 = view_y1 = view_cy + view_h;
    view_ptr = screen->data + view_y0 * 320;
    proj_dist = FixDiv(view_w, FixTan(fov));
    star_near = 50;
    fr_left.x = -view_w;
    fr_left.y = 0;
    fr_left.z = proj_dist;
    fr_right.x = view_w;
    fr_right.y = 0;
    fr_right.z = proj_dist;
    fr_top.x = 0;
    fr_top.y = view_h;
    fr_top.z = proj_dist;
    fr_bottom.x = 0;
    fr_bottom.y = -view_h;
    fr_bottom.z = proj_dist;
    StarsReset();
}

/* Fill the view with one of the static bitmaps, tiled. */
void DrawStatic(void)
{
    int16_t vw;
    int16_t vh;
    int16_t bw;
    int16_t bh;
    int16_t x;
    int16_t y;
    int16_t xx;
    int16_t yy;
    int16_t step;
    char *p;
    unsigned char *src;
    char *rowp;
    char *dst;
    Bitmap *bmp;
    MemHandle h;
    char name[10];
    char c;

    sprintf(name, "static%d", rand() % 5);
    h = CacheLoad(name, "bmp");
    bmp = LockMem(h);
    vw = clip_rect.x1 - clip_rect.x0 + 1;
    vh = clip_rect.y1 - clip_rect.y0 + 1;
    bw = bmp->w;
    bh = bmp->h;
    src = bmp->data;
    dst = screen->data + clip_rect.y0 * 320 + clip_rect.x0;
    step = bh * 320;
    for (y = 0; y != bh; ++y, dst += 320)
        for (x = 0; x != bw; ++x, ++src)
            if (*src) {
                c = *src;
                rowp = p = dst + x;
                for (yy = y; yy < vh; yy += bh) {
                    for (xx = x; xx < vw; xx += bw, p += bw)
                        *p = c;
                    p = rowp += step;
                }
            }
    UnlockMem(h);
    CacheRelease(h);
}

void ClearR3List(void)
{
    Obj3D **p;

    for (p = r3_list; p != &r3_list[MAX_R3]; p++)
        if (*p)
            (*p)->visible = 0;
    MemSet(r3_list, sizeof(r3_list), 0);
}

void AddR3(Obj3D *o)
{
    Obj3D **p;

    for (p = r3_list; p != &r3_list[MAX_R3]; p++)
        if (*p == 0) {
            *p = o;
            return;
        }
    Fatal("AddR3: out of shapes.");
}

void DelR3(Obj3D *o)
{
    Obj3D **p;

    for (p = r3_list; p != &r3_list[MAX_R3]; p++)
        if (*p == o) {
            *p = 0;
            o->visible = 0;
            return;
        }
    Fatal("DelR3: shape not found.");
}

void ClearView(void)
{
    int16_t w;
    int16_t h;
    char *p;

    w = clip_rect.x1 - clip_rect.x0 + 1;
    h = clip_rect.y1 - clip_rect.y0 + 1;
    p = view_ptr + clip_rect.x0;
    while (h--) {
        MemSet(p, w, 0);
        p += 320;
    }
}

void DrawStarfield(void)
{
    DrawStars();
}

/* qsort order: furthest first */
int CompareR3(const void *pa, const void *pb)
{
    Obj3D *const *a = pa;
    Obj3D *const *b = pb;
    int32_t d;

    d = (*a)->scale - (*b)->scale;
    if (d < 0)
        return 1;
    if (d == 0)
        return 0;
    return -1;
}

/* Draw the view: stars, then everything in r3_list from back to front. */
void RenderR3(int16_t statics)
{
    uint16_t n;
    uint16_t i;
    int16_t unused;
    Obj3D *o;
    Obj3D *p;
    int16_t dead;

    dead = 0;
    view_sprite.dirty = 1;
    MatTranspose(&cam_orient, &cam_matrix);
    ClearView();
    DrawStarfield();
    for (i = n = 0; i != MAX_R3; i++) {
        if ((o = r3_list[i]) == 0)
            continue;
        o->drawn = 0;
        switch (o->visible) {
        case R3_SHIP:
            if (o->expl) {
                o->expl_frame++;
                if (o->expl_frame > 45) {
                    if (o == &player_ship)
                        dead = 1;
                    else
                        DelR3(o);
                    o = 0;
                } else {
                    int16_t reach;
                    int16_t size;
                    Obj3D *q;

                    reach = o->reach;
                    size = o->size;
                    o->size = o->expl_frame >= 24 ? 12 : 6;
                    o->reach *= o->expl_frame >= 24 ? 2 : 1;
                    q = ProjectR3(o);
                    o->reach = reach;
                    o->size = size;
                    o = q;
                }
            } else
                o = ProjectR3(o);
            break;
        case R3_SHOT:
            o = ProjectR3(o);
            break;
        case R3_PLANET:
            o = ProjectR3(o);
            break;
        default:
            break;
        }
        if (o)
            r3_sorted[n++] = o;
    }
    if (n) {
        qsort(r3_sorted, n, sizeof(Obj3D *), CompareR3);
        for (i = 0; i != n; i++) {
            p = r3_sorted[i];
            p->drawn = 1;
            switch (p->visible) {
            case R3_SHOT:
                if (p->kind == 1)
                    DrawBeam(p);
                else
                    DrawR3Shape(p);
                break;
            case R3_SHIP:
                if (p->expl) {
                    int16_t size;
                    int16_t visible;
                    int16_t kind;

                    if (p->expl_frame < 21)
                        DrawR3Shape(p);
                    visible = p->visible;
                    kind = p->kind;
                    size = p->size;
                    p->visible = R3_EXPL;
                    p->kind = 4;
                    p->size = p->expl_frame >= 24 ? 12 : 6;
                    DrawR3Shape(p);
                    p->visible = visible;
                    p->kind = kind;
                    p->size = size;
                } else
                    DrawR3Shape(p);
                break;
            case R3_PLANET:
                DrawR3Shape(p);
                break;
            default:
                break;
            }
        }
    }
    if (dead) {
        MsgBox("", "#GENE\\SPACE000#The Enterprise has been destroyed!", 20, 20, 0xB0, 0);
        GameOverMenu();
    }
    if (statics && !player_ship.expl)
        DrawStatic();
}

/* Transform an object into camera space and project its centre.
 * Returns 0 if it is out of view. */
Obj3D *ProjectR3(Obj3D *o)
{
    Vec32 v;
    Matrix m;

    o->view_m = o->orient;
    o->view = o->pos;
    Vec32Sub(&o->view, &cam_pos);
    o->scale = (uint16_t)VecLen32(&o->view.x);
    MatTVecMul32(&o->view, &cam_matrix, &v);
    o->view = v;
    if (!InView(o))
        return 0;
    if (o->visible == R3_SHIP || o->visible == R3_PLANET || o->visible == R3_SHOT) {
        MatMul(&o->view_m, &cam_matrix, &m);
        o->view_m = m;
    }
    o->sx = Project(o->view.x, o->view.z) + view_cx;
    o->sy = view_cy - Project(o->view.y, o->view.z);
    return o;
}

/* draw: load the bitmap of a shot or explosion */
int16_t LoadShotShape(Obj3D *o)
{
    char name[16];

    switch (o->kind) {
    case 3:
        strcpy(name, "expl0X");
        name[5] = (3 & o->life) + '0';
        o->r3s = CacheLoad(name, "shp");
        o->r3s_type = 0;
        if (o->life == 5)
            BridgeSfx(0x26);
        break;
    case 1:
        DrawBeam(o);
        break;
    case 0:
        strcpy(name, "shotX");
        name[4] = (1 & o->life) + '1';
        o->r3s = CacheLoad(name, "shp");
        o->r3s_type = 0;
        break;
    case 2:
        strcpy(name, "plasm0");
        name[5] = (blink_count & 3) + '0';
        o->r3s = CacheLoad(name, "shp");
        o->r3s_type = 0;
        if ((blink_count & 3) == 0)
            o->size++;
        break;
    case 4:
        o->r3s = CacheLoad("ark7", "shp");
        o->r3s_type = 0;
        break;
    case 5:
        o->r3s = CacheLoad("derelict", "shp");
        o->r3s_type = 0;
        break;
    case 6:
        o->r3s = CacheLoad("masada", "shp");
        o->r3s_type = 0;
        break;
    case 7:
        o->r3s = CacheLoad("republic", "shp");
        o->r3s_type = 0;
        break;
    }
    return 0;
}

/* free: release a shot's bitmap */
void FreeShotShape(Obj3D *o)
{
    if (o->kind == 1) {
        if (o->r3s)
            FreeMem(o->r3s);
    } else
        CacheRelease(o->r3s);
    o->r3s = 0;
}

/* draw: pick the ship bitmap that shows the side facing the camera */
int16_t SelectView(Obj3D *o)
{
    Vec32 rel;
    Vec32 v;
    Matrix m;
    int32_t pitch;
    int32_t yaw;
    int16_t idx;
    int16_t j;
    uint16_t len;
    char name[8];

    if (o->kind == 4) {
        sprintf(name, "BigExp%02d", o->expl_frame);
        o->r3s = CacheGet(name, "shp", 1, 1, 1);
        o->r3s_type = 0;
    } else {
        if (o->scale < star_near)
            Fatal("object too close.");
        MemSet(&rel, sizeof(rel), 0);
        Vec32Sub(&rel, &o->view);
        MatTranspose(&o->view_m, &m);
        MatTVecMul32(&rel, &m, &v);
        rel = v;
        yaw = FixAtan2(rel.x, rel.z);
        if (yaw < 0)
            yaw += 0x400;
        pitch = FixACos(FixDiv(rel.y, o->scale));
        if (pitch < 0x80 || pitch > 0x180) {
            len = FixLength(rel.x, rel.z, 0);
            pitch = FixASin(FixDiv(len, o->scale));
            if (rel.y < 0)
                pitch = 0x200 - pitch;
        }
        idx = (pitch * 6 + 0x100) / 0x200;
        if (idx > 6)
            idx = 6;
        if ((yaw += 42) >= 0x400)
            yaw -= 0x400;
        j = yaw * 12 / 0x400;
        idx = view_tab[j * 2][idx * 2];
        LoadR3S(o, shape_names[o->kind], idx & 0x7FFF);
        if ((idx & 0x8000) != 0) {
            o->rot.e[0][1] = -o->rot.e[0][1];
            o->rot.e[0][2] = -o->rot.e[0][2];
            o->rot.e[1][0] = -o->rot.e[1][0];
            o->rot.e[2][0] = -o->rot.e[2][0];
            o->mirror = 1;
        } else
            o->mirror = 0;
        strcpy(o->shape, shape_names[o->kind]);
        o->view_idx = idx;
    }
    return 0;
}

/* free: release a ship's bitmap */
void ReleaseShipShape(Obj3D *o)
{
    if (o->kind == 4)
        ReleaseCache(o->r3s);
    else
        ReleaseR3S(o->r3s);
    o->r3s = 0;
}

double DDiv(double a, double b)
{
    return a / b;
}

/* Draw an object's bitmap rotated and scaled: map the bitmap's corners to
 * the screen, scan-convert the outline, then step through the bitmap for
 * each row. */
/* One row of a texture mapped shape: n pixels from the 256 wide bitmap at
 * tex, stepping (u, v) in 8.8 fixed point; pixels of colour 0 are skipped.
 * The dithered form (blur) steps the 8 pixel cycle starting at slot d so
 * that the sampling points jitter, and cancel out over the cycle. */
void DrawTexRow(unsigned char *tex, char *dst, int16_t n, uint16_t u, uint16_t v,
                       int16_t du, int16_t dv, int blur, int d)
{
    static const int16_t jitter[8] = { 0, 0xC0, 0, 0x120, 0, 0xC0, 0, 0x160 };
    unsigned char t;

    if (blur) {
        du += 0x80;
        dv += 0x80;
    }
    for (; n > 0; n--, dst++) {
        t = tex[((v >> 8) << 8) | (u >> 8)];
        if (t)
            *dst = t;
        if (blur) {
            u += du - jitter[d];
            v += dv - jitter[d];
            d = (d + 1) & 7;
        } else {
            u += du;
            v += dv;
        }
    }
}

void DrawR3Shape(Obj3D *o)
{
    double t[2];
    double m[4];
    double uv[2];
    double im[4];
    double k;
    double invk;
    double pts[4][2];
    int16_t top;
    int16_t bot;
    int16_t left[200];
    int16_t right[200];
    int16_t i;
    int16_t y;
    Bitmap *hdr;

    if (o->draw)
        o->draw(o);
    if (o->r3s == 0)
        goto release;
    hdr = (Bitmap *)((char *)LockMem(o->r3s) + o->r3s_type);
    k = (double)(o->size * proj_dist) / o->view.z;
    invk = 1.0 / k;
    switch (o->visible) {
    case R3_SHOT:
        m[0] = k;
        m[1] = 0.0;
        m[2] = 0.0;
        m[3] = k;
        im[0] = invk;
        im[1] = 0.0;
        im[2] = 0.0;
        im[3] = invk;
        break;
    case R3_PLANET:
    case R3_EXPL:
        {
            int16_t ang;
            double c;
            double s;
            Matrix mt;
            Matrix m2;

            MatTranspose(&o->rot, &mt);
            MatMul(&mt, &o->view_m, &m2);
            ang = FixAtan2(m2.e[0][1], m2.e[0][0]);
            c = FixCos(ang) / 16384.0;
            s = FixSin(ang) / 16384.0;
            m[0] = c;
            m[1] = -s;
            m[2] = s;
            m[3] = c;
            im[0] = m[3];
            im[1] = -m[1];
            im[2] = -m[2];
            im[3] = m[0];
            if (k < 0.01)
                k = 0.01;
            if (k > 10)
                k = 10;
            m[0] *= k;
            m[1] *= k;
            m[2] *= k;
            m[3] *= k;
            im[0] /= k;
            im[1] /= k;
            im[2] /= k;
            im[3] /= k;
        }
        break;
    default:
        {
            int16_t d0;
            int16_t d1;
            int16_t d2;
            int16_t ia;
            int16_t ib;
            int16_t a[2][3];
            int16_t px;
            int16_t py;

            d0 = FixMul2(o->rot.e[0][0], o->rot.e[1][1], -o->rot.e[1][0], o->rot.e[0][1]);
            d1 = FixMul2(o->rot.e[1][0], o->rot.e[2][1], -o->rot.e[2][0], o->rot.e[1][1]);
            d2 = FixMul2(o->rot.e[0][0], o->rot.e[2][1], -o->rot.e[2][0], o->rot.e[0][1]);
            d0 = ABS(d0) / 4;
            d1 = ABS(d1) / 2;
            d2 = ABS(d2) / 1;
            ia = 0;
            ib = 2;
            if (d0 > d1) {
                if (d0 > d2)
                    ib = 1;
                else
                    ;
            } else if (d1 > d2)
                ia = 1;
            a[0][0] = FixMul2(o->rot.e[ib][1], o->view_m.e[ia][0], -o->rot.e[ia][1], o->view_m.e[ib][0]);
            a[0][1] = FixMul2(o->rot.e[ib][1], o->view_m.e[ia][1], -o->rot.e[ia][1], o->view_m.e[ib][1]);
            a[0][2] = FixMul2(o->rot.e[ib][1], o->view_m.e[ia][2], -o->rot.e[ia][1], o->view_m.e[ib][2]);
            a[1][0] = FixMul2(-o->rot.e[ib][0], o->view_m.e[ia][0], o->rot.e[ia][0], o->view_m.e[ib][0]);
            a[1][1] = FixMul2(-o->rot.e[ib][0], o->view_m.e[ia][1], o->rot.e[ia][0], o->view_m.e[ib][1]);
            a[1][2] = FixMul2(-o->rot.e[ib][0], o->view_m.e[ia][2], o->rot.e[ia][0], o->view_m.e[ib][2]);
            {
                int16_t lim;

                lim = ABS(o->view.z) << 1;
                if (ABS(o->view.x) >= lim || ABS(o->view.y) >= lim)
                    goto done;
            }
            px = FixDiv(o->view.x, o->view.z);
            py = FixDiv(o->view.y, o->view.z);
            m[0] = (a[0][0] - FixMul(a[0][2], px)) / 16384.0;
            m[1] = (a[0][1] - FixMul(a[0][2], py)) / 16384.0;
            m[2] = (a[1][0] - FixMul(a[1][2], px)) / 16384.0;
            m[3] = (a[1][1] - FixMul(a[1][2], py)) / 16384.0;
            k = DDiv(k, (int16_t)FixMul2(o->rot.e[ia][0], o->rot.e[ib][1], -o->rot.e[ib][0], o->rot.e[ia][1]) / 16384.0);
            m[1] = -m[1];
            m[2] = -m[2];
            if (o->mirror) {
                m[0] = -m[0];
                m[1] = -m[1];
            }
            invk = DDiv(DDiv(1.0, k), m[0] * m[3] - m[1] * m[2]);
            im[0] = m[3];
            im[1] = -m[1];
            im[2] = -m[2];
            im[3] = m[0];
            m[0] *= k;
            m[1] *= k;
            m[2] *= k;
            m[3] *= k;
            im[0] *= invk;
            im[1] *= invk;
            im[2] *= invk;
            im[3] *= invk;
        }
    }
    o->inv[0] = im[0];
    o->inv[1] = im[1];
    o->inv[2] = im[2];
    o->inv[3] = im[3];
    t[0] = o->sx - m[0] * (hdr->x + 2) - m[2] * (hdr->y + 2);
    t[1] = o->sy - m[1] * (hdr->x + 2) - m[3] * (hdr->y + 2);
    uv[0] = hdr->x + 2 - im[0] * o->sx - im[2] * o->sy;
    uv[1] = hdr->y + 2 - im[1] * o->sx - im[3] * o->sy;
    {
        double w;

        w = hdr->w + 2;
        {
        double h;

        h = hdr->h + 2;
        pts[0][0] = 1 * m[0] + 1 * m[2] + t[0];
        pts[0][1] = 1 * m[1] + 1 * m[3] + t[1];
        pts[1][0] = 1 * m[0] + h * m[2] + t[0];
        pts[1][1] = 1 * m[1] + h * m[3] + t[1];
        pts[2][0] = w * m[0] + h * m[2] + t[0];
        pts[2][1] = w * m[1] + h * m[3] + t[1];
        pts[3][0] = w * m[0] + 1 * m[2] + t[0];
        pts[3][1] = w * m[1] + 1 * m[3] + t[1];
        }
    }
    for (i = clip_rect.y0; i <= clip_rect.y1; i++) {
        left[i] = clip_rect.x1 + 1;
        right[i] = clip_rect.x0 - 1;
    }
    top = clip_rect.y1;
    bot = clip_rect.y0;
    for (i = 0; i < 4; i++) {
        int16_t a;
        int16_t b;
        int16_t y0;
        int16_t y1;
        int32_t x;
        int32_t dx;
        double slope;

        a = i;
        b = (i + 1) & 3;
        if (pts[a][1] > pts[b][1]) {
            a = b;
            b = i;
        }
        y0 = ceil(pts[a][1]);
        y1 = floor(pts[b][1]);
        if (y0 > y1 || y0 > clip_rect.y1 || y1 < clip_rect.y0)
            continue;
        if (y0 < clip_rect.y0)
            y0 = clip_rect.y0;
        if (y1 > clip_rect.y1)
            y1 = clip_rect.y1;
        if (y0 < top)
            top = y0;
        if (y1 > bot)
            bot = y1;
        if (pts[a][1] == pts[b][1])
            slope = 0;
        else
            slope = (pts[b][0] - pts[a][0]) / (pts[b][1] - pts[a][1]);
        dx = 65536L * slope;
        x = 65536L * (pts[a][0] + (y0 - pts[a][1]) * slope);
        for (y = y0; y <= y1; y++) {
            int16_t xr;
            int16_t xl;

            xr = x >> 16;
            xl = (x + 0xFFFF) >> 16;
            if (xl < clip_rect.x0)
                xl = clip_rect.x0;
            if (xl < left[y])
                left[y] = xl;
            if (xr > clip_rect.x1)
                xr = clip_rect.x1;
            if (xr > right[y])
                right[y] = xr;
            x += dx;
        }
    }
    while (top <= bot && left[top] > right[top])
        top++;
    while (top <= bot && left[bot] > right[bot])
        bot--;
    if (top > bot)
        goto done;
    {
        char *row;
        char *dst;
        Bitmap *buf;
        char *p;
        int16_t du;
        int16_t dv;
        int16_t ddu;
        int16_t ddv;
        int16_t u0;
        int16_t v0;
        int16_t opaque;
        int16_t hash;

        if (o->visible == R3_SHIP) {
            if (byte_45B3C[0])
                opaque = (o->size * proj_dist << 1) / 5 <= o->view.z;
            else
                opaque = 1;
        } else
            opaque = (o->size * proj_dist << 1) / 3 <= o->view.z;
        if (!opaque) {
            if (o->visible == R3_SHOT)
                hash = (blink_count >> 3) ^ blink_count;
            else {
                hash = o->rot.e[0][0] + o->rot.e[0][1] + o->rot.e[0][2]
                    + o->rot.e[1][0] + o->rot.e[1][1] + o->rot.e[1][2]
                    + o->rot.e[2][0] + o->rot.e[2][1] + o->rot.e[2][2];
                hash = ((hash >> 3) + (hash >> 6)) ^ hash;
            }
        }
        du = 256 * im[0];
        dv = 256 * im[1];
        ddu = 256 * im[2];
        ddv = 256 * im[3];
        u0 = ddu * top + (int16_t)(256 * uv[0]);
        v0 = ddv * top + (int16_t)(256 * uv[1]);
        {
            int16_t w;
            int16_t h;
            int16_t i;
            char *q;

            /* copy the bitmap into a 256 wide buffer with a clear border,
             * so the drawing loops can step through it with byte maths */
            buf = (Bitmap *)backbuf->data;
            buf->w = 256;
            buf->h = 249;
            p = buf->data;
            w = hdr->w;
            h = hdr->h;
            q = p;
            if (h > 245)
                Fatal("Shape height too big in DrawR3Shape!");
            if (w > 252)
                Fatal("Shape width too big in DrawR3Shape!");
            for (i = 0; i < 2; i++)
                MemSet(q + (i << 8), w + 4, 0);
            for (i = 0; i < 2; i++)
                MemSet(q + ((h + 2 + i) << 8), w + 4, 0);
            q += 0x200;
            for (i = 0; i < h; i++) {
                q[i << 8] = q[(i << 8) + 1] = 0;
                q[(i << 8) + w + 2] = q[(i << 8) + w + 3] = 0;
            }
            if (o->visible == R3_SHIP && o->has_shields) {
                if (o->shields_on)
                    o->shield_glow = max(0, min(o->shield_glow + 1, 28));
                else
                    o->shield_glow = max(0, min(o->shield_glow - 1, 28));
                if (o->shield_glow >= 20)
                    CopyRectRemap(buf, 2, 2, hdr, 0, 0, w, h, o->shield_glow - 20);
                else
                    CopyRect(buf, 2, 2, hdr, 0, 0, w, h);
            } else
                CopyRect(buf, 2, 2, hdr, 0, 0, w, h);
        }
        row = screen->data + top * 320;
        for (y = top; y <= bot; y++) {
            int16_t x;
            int16_t n;
            int16_t u;
            int16_t v;
            int16_t d;
            int16_t dd;

            x = left[y];
            n = right[y] + 1 - x;
            u = x * du + u0;
            v = x * dv + v0;
            u0 += ddu;
            v0 += ddv;
            dst = row + x;
            row += 320;
            if (n != 0) {
                if (opaque) {
                    u += 128;
                    v += 128;
                    DrawTexRow((unsigned char *)p, dst, n, u, v, du, dv, 0, 0);
                } else {
                    d = (x + y + hash) & 7;
                    dd = dither[d];
                    u += dd;
                    v += dd;
                    /* every other pixel skips a little, in a pattern that
                     * depends on d, to blur distant shapes */
                    DrawTexRow((unsigned char *)p, dst, n, u, v, du, dv, 1, d);
                }
            }
        }
    }
done:
    UnlockMem(o->r3s);
release:
    if (o->free)
        o->free(o);
}

/* Is any of the object inside the view frustum? */
int16_t InView(Obj3D *o)
{
    Vec32 v;
    int32_t dx;
    int32_t dy;

    v = o->view;
    if (v.z < star_near)
        return 0;
    if (v.x > 0)
        dx = v.z * fr_right.x - (v.x - o->reach) * fr_right.z;
    else
        dx = fr_left.z * (v.x + o->reach) - fr_left.x * v.z;
    if (v.y > 0)
        dy = fr_top.y * v.z - fr_top.z * (v.y - o->reach);
    else
        dy = (v.y + o->reach) * fr_bottom.z - v.z * fr_bottom.y;
    return dx > 0 && dy > 0;
}

/* Straighten the orientation matrices of ships and planets. */
void OrthoR3(void)
{
    int16_t i;
    Obj3D *o;

    for (i = 0; i != MAX_R3; i++) {
        if ((o = r3_list[i]) == 0)
            continue;
        if (o->visible == R3_SHIP || o->visible == R3_PLANET)
            MatOrtho(&o->orient);
    }
}

/* draw: a beam, two circles joined along the shot's direction */
void DrawBeam(Obj3D *o)
{
    Vec32 a;
    Vec32 b;
    int32_t d[3];
    int16_t sx0;
    int16_t sy0;
    int16_t sx1;
    int16_t sy1;
    int16_t unused1;
    int16_t r0;
    int16_t r1;
    int16_t n;
    int16_t unused2;
    Span *spans;
    int16_t unused3;
    int16_t unused4;

    o->r3s = 0;
    a = b = o->view;
    d[0] = o->view_m.e[2][0] * 200L / 0x4000;
    d[1] = o->view_m.e[2][1] * 200L / 0x4000;
    d[2] = o->view_m.e[2][2] * 200L / 0x4000;
    for (n = 0; n != 3; n++)
        (&a.x)[n] -= d[n], (&b.x)[n] += d[n];
    if (b.z < star_near || a.z < star_near)
        return;
    r0 = Project(o->reach >> 3, a.z);
    r1 = Project(o->reach >> 3, b.z);
    r0 = max(1, min(r0, 99));
    r1 = max(1, min(r1, 99));
    spans = AllocMemPtr((n = clip_rect.y1 - clip_rect.y0 + 1) * sizeof(Span));
    while (n-- > 0) {
        spans[n].right = -32767;
        spans[n].left = 32767;
    }
    sx0 = Project(a.x, a.z) + view_cx;
    sy0 = view_cy - Project(a.y, a.z);
    sx1 = Project(b.x, b.z) + view_cx;
    sy1 = view_cy - Project(b.y, b.z);
    SpanCircle(spans, sx1, sy1, r1);
    SpanCircle(spans, sx0, sy0, r0);
    SpanQuad(spans, sx0, sy0, sx1, sy1, r0, r1);
    FillSpans(spans, o->color + 2);
    DrawLine(sx0, sy0, sx1, sy1, o->color);
    OutlineSpans(spans, o->color + 4);
    FreeMemPtr(spans);
}

#define SPAN_ROW(r, x0, x1) \
    row = (r); \
    if (row >= clip_rect.y0 && row <= clip_rect.y1) { \
        row -= clip_rect.y0; \
        s[row].left = min(s[row].left, x0); \
        s[row].right = max(s[row].right, x1); \
    }

/* Add a filled circle to the spans. */
void SpanCircle(Span *s, int16_t cx, int16_t cy, int16_t r)
{
    int16_t x;
    int16_t y;
    int16_t d;
    int16_t row;

    for (x = 0, y = r, d = 3 - r * 2; x < y; ++x) {
        SPAN_ROW(cy + y, cx - x, cx + x)
        SPAN_ROW(cy - y, cx - x, cx + x)
        SPAN_ROW(cy + x, cx - y, cx + y)
        SPAN_ROW(cy - x, cx - y, cx + y)
        if (d < 0)
            d = d + 4 * x + 6;
        else {
            d = d + 4 * (x - y) + 10;
            --y;
        }
    }
    if (x == y) {
        SPAN_ROW(cy + y, cx - x, cx + x)
        SPAN_ROW(cy - y, cx - x, cx + x)
        SPAN_ROW(cy + x, cx - y, cx + y)
        SPAN_ROW(cy - x, cx - y, cx + y)
    }
}

/* Add the quadrilateral joining two circles to the spans. */
void SpanQuad(Span *s, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t r0, int16_t r1)
{
    int16_t a;
    int16_t unused1;
    int16_t unused2;
    int16_t p[8];
    int16_t unused3;
    int16_t unused4;

    a = FixAtan2(y1 - y0, x1 - x0);
    ++r0;
    ++r1;
    p[0] = x0 + (int32_t)r0 * FixCos(a + 0x100) / 0x4000;
    p[1] = y0 + (int32_t)r0 * FixSin(a + 0x100) / 0x4000;
    p[2] = x1 + (int32_t)r1 * FixCos(a + 0x100) / 0x4000;
    p[3] = y1 + (int32_t)r1 * FixSin(a + 0x100) / 0x4000;
    p[4] = x1 + (int32_t)r1 * FixCos(a - 0x100) / 0x4000;
    p[5] = y1 + (int32_t)r1 * FixSin(a - 0x100) / 0x4000;
    p[6] = x0 + (int32_t)r0 * FixCos(a - 0x100) / 0x4000;
    p[7] = y0 + (int32_t)r0 * FixSin(a - 0x100) / 0x4000;
    SpanLine(s, p[4], p[5], p[6], p[7]);
    SpanLine(s, p[0], p[1], p[2], p[3]);
}

void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t c)
{
    int16_t n;
    int16_t unused;
    Point *p;
    Point *list;
    char *base;

    base = screen->data;
    if ((list = p = LinePoints(x0, y0, x1, y1)) == 0)
        return;
    for (n = p->x, ++p; n--; ++p) {
        if (p->y < clip_rect.y0 || p->y > clip_rect.y1)
            continue;
        if (p->x > clip_rect.x1 || p->x < clip_rect.x0)
            continue;
        base[p->y * 320 + p->x] = c;
    }
    FreeMemPtr(list);
}

/* The points of a line clipped to clip_rect: a count, then the points.
 * Returns 0 if the line is outside. */
Point *LinePoints(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t e;
    int16_t inc1;
    int16_t inc2;
    int16_t sx;
    int16_t sy;
    int16_t adx;
    int16_t ady;
    Point *p;
    Point *list;
    int16_t nx;
    int16_t ny;

    sx = x0 < clip_rect.x0 ? 1 : 0;
    sx |= x0 > clip_rect.x1 ? 2 : 0;
    sx |= y0 < clip_rect.y0 ? 4 : 0;
    sx |= y0 > clip_rect.y1 ? 8 : 0;
    sy = x1 < clip_rect.x0 ? 1 : 0;
    sy |= x1 > clip_rect.x1 ? 2 : 0;
    sy |= y1 < clip_rect.y0 ? 4 : 0;
    sy |= y1 > clip_rect.y1 ? 8 : 0;
    if (sx & sy)
        return 0;
    if (sx | sy) {
        e = sx ^ sy;
        adx = x1 - x0;
        ady = y1 - y0;
        if (e & 1) {
            ny = (int16_t)((int32_t)(int16_t)(clip_rect.x0 - x0) * ady / adx) + y0;
            nx = clip_rect.x0;
            if (sx & 1) {
                x0 = nx;
                y0 = ny;
            } else {
                x1 = nx;
                y1 = ny;
            }
        }
        if (e & 2) {
            ny = (int16_t)((int32_t)(int16_t)(clip_rect.x1 - x0) * ady / adx) + y0;
            nx = clip_rect.x1;
            if (sx & 2) {
                x0 = nx;
                y0 = ny;
            } else {
                x1 = nx;
                y1 = ny;
            }
        }
        if (e & 4) {
            nx = (int16_t)((int32_t)(int16_t)(clip_rect.y0 - y0) * adx / ady) + x0;
            ny = clip_rect.y0;
            if (sx & 4) {
                x0 = nx;
                y0 = ny;
            } else {
                x1 = nx;
                y1 = ny;
            }
        }
        if (e & 8) {
            nx = (int16_t)((int32_t)(int16_t)(clip_rect.y1 - y0) * adx / ady) + x0;
            ny = clip_rect.y1;
            if (sx & 8) {
                x0 = nx;
                y0 = ny;
            } else {
                x1 = nx;
                y1 = ny;
            }
        }
        if (!PointInRect(&clip_rect, nx, ny))
            return 0;
    }
    adx = ABS(x1 - x0);
    ady = ABS(y1 - y0);
    sx = SGN(x1 - x0);
    sy = SGN(y1 - y0);
    if (adx >= ady) {
        e = ady * 2 - adx;
        inc1 = ady * 2;
        inc2 = (ady - adx) * 2;
        list = p = AllocMemPtr((adx + 2) * sizeof(Point));
        p->x = adx;
        p->y = 0;
        ++p;
        p->x = x0;
        p->y = y0;
        ++p;
        while (x0 != x1) {
            x0 += sx;
            if (e < 0)
                e += inc1;
            else
                e += inc2, y0 += sy;
            p->x = x0;
            p->y = y0;
            ++p;
        }
    } else {
        e = adx * 2 - ady;
        inc1 = adx * 2;
        inc2 = (adx - ady) * 2;
        list = p = AllocMemPtr((ady + 2) * sizeof(Point));
        p->x = ady;
        p->y = 0;
        ++p;
        p->x = x0;
        p->y = y0;
        ++p;
        while (y0 != y1) {
            y0 += sy;
            if (e < 0)
                e += inc1;
            else
                e += inc2, x0 += sx;
            p->x = x0;
            p->y = y0;
            ++p;
        }
    }
    return list;
}

#define SPAN_POINT(x, y) \
    if ((y) >= clip_rect.y0 && (y) <= clip_rect.y1) { \
        row = (y) - clip_rect.y0; \
        s[row].right = max(s[row].right, x); \
        s[row].left = min(s[row].left, x); \
    }

/* Add the points of a line to the spans. */
void SpanLine(Span *s, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t e;
    int16_t inc1;
    int16_t inc2;
    int16_t sx;
    int16_t sy;
    int16_t dx;
    int16_t dy;
    int16_t row;

    dx = ABS(x1 - x0);
    dy = ABS(y1 - y0);
    sx = SGN(x1 - x0);
    sy = SGN(y1 - y0);
    if (dx >= dy) {
        e = dy * 2 - dx;
        inc1 = dy * 2;
        inc2 = (dy - dx) * 2;
        SPAN_POINT(x0, y0)
        while (x0 != x1) {
            x0 += sx;
            if (e < 0)
                e += inc1;
            else
                e += inc2, y0 += sy;
            SPAN_POINT(x0, y0)
        }
    } else {
        e = dx * 2 - dy;
        inc1 = dx * 2;
        inc2 = (dx - dy) * 2;
        SPAN_POINT(x0, y0)
        while (y0 != y1) {
            y0 += sy;
            if (e < 0)
                e += inc1;
            else
                e += inc2, x0 += sx;
            SPAN_POINT(x0, y0)
        }
    }
}

/* Fill the spans (one per row of clip_rect) with colour c. */
void FillSpans(Span *s, int16_t c)
{
    char *base;
    int16_t n;

    base = screen->data + clip_rect.y0 * 320;
    span_fill = (c & 0xFF) | (c << 8);
    for (n = clip_rect.y1 - clip_rect.y0 + 1; n != 0; n--, s++, base += 320) {
        int16_t l = s->left;
        int16_t r = s->right;

        if (r < l || l > clip_rect.x1 || r < clip_rect.x0)
            continue;
        if (l < clip_rect.x0)
            l = clip_rect.x0;
        if (r > clip_rect.x1)
            r = clip_rect.x1;
        memset(base + l, c, r - l + 1);
    }
}

/* Plot the ends of the spans in colour c. */
void OutlineSpans(Span *s, int16_t c)
{
    char *base;
    int16_t n;

    base = screen->data + clip_rect.y0 * 320;
    for (n = clip_rect.y1 - clip_rect.y0 + 1; n != 0; n--, s++, base += 320) {
        int16_t l = s->left;
        int16_t r = s->right;

        if (r < l || l > clip_rect.x1 || r < clip_rect.x0)
            continue;
        if (l < clip_rect.x0)
            l = clip_rect.x0;
        if (r > clip_rect.x1)
            r = clip_rect.x1;
        base[l] = c;
        base[r] = c;
    }
}

void SaveR3(Obj3D *o)
{
    SaveWrite(o, sizeof(Obj3D));
}

void SaveR3Shot(Obj3D *o)
{
    SaveWrite(o, sizeof(Shot));
}

void RestoreR3(Obj3D *o)
{
    SaveRead(o, sizeof(Obj3D));
    if (o->visible) {
        AddR3(o);
        switch (o->visible) {
        case R3_SHIP:
            o->draw = SelectView;
            o->free = ReleaseShipShape;
            break;
        case R3_PLANET:
            o->draw = DrawPlanet;
            o->free = FreePlanet;
            break;
        case R3_SHOT:
            Fatal("illegal shot saved in restore r3.");
            break;
        }
    }
}

void RestoreR3Shot(Obj3D *o)
{
    SaveRead(o, sizeof(Shot));
    if (o->visible) {
        AddR3(o);
        switch (o->visible) {
        case R3_PLANET:
        case R3_SHIP:
            Fatal("illegal non-shot saved in restore r3 shot.");
            break;
        case R3_SHOT:
            o->draw = LoadShotShape;
            o->free = FreeShotShape;
            break;
        }
    }
}

void SaveSpace(void)
{
    Obj3D *o;
    int16_t i;

    SaveR3(&player_ship);
    SaveR3Shot((Obj3D *)&shot_obj);
    SaveR3(&dummy_planet);
    SaveWrite(&cam_pos, sizeof(cam_pos));
    SaveWrite(&cam_orient, sizeof(cam_orient));
    for (i = 0; i != 4; i++)
        SaveR3(&ships[i]);
}

void LoadSpace(void)
{
    Obj3D *o;
    int16_t i;

    for (i = 0; i != MAX_R3; i++) {
        if ((o = r3_list[i]) == 0)
            continue;
        if (o->visible)
            DelR3(o);
    }
    RestoreR3(&player_ship);
    RestoreR3Shot((Obj3D *)&shot_obj);
    RestoreR3(&dummy_planet);
    SaveRead(&cam_pos, sizeof(cam_pos));
    SaveRead(&cam_orient, sizeof(cam_orient));
    for (i = 0; i != 4; i++) {
        RestoreR3(&ships[i]);
        ships[i].ai_busy = 0;
    }
    SetRepair(-1);
}
