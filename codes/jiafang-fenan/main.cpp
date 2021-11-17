#include <cstdio>
#include <vector>
#include <cmath>
#include <unistd.h>

using namespace std;

vector<int> arr;
vector<int> arr2;

const int n = 3;
const int w = 48;
const int nw = w * w * w;

inline bool bad(int i)
{
    for (int s = 1; s < nw; s *= w) {
        int j = (i / s) % w;
        if (j <= 0 || j >= w - 1)
            return true;
    }
    return false;
}

inline void kuo(int i, int z, int k)
{
    arr2[i] -= k * n;
    for (int s = 1; s < nw; s *= w) {
        if (!bad(i + z * s))
            arr2[i + z * s] += k;
    }
}

void body(int odd, int i)
{
    if (bad(i)) return;
    if (arr[i] >= 2 * n) {
        // zhen kuo fu kuo
        kuo(i, 1, 1);
        kuo(i, -1, 1);
    } else if (arr[i] <= -2 * n) {
        // zhen xi fu xi
        kuo(i, 1, -1);
        kuo(i, -1, -1);
    } else if (arr[i] >= n && arr[i] < 2 * n) {
        if (odd) {
            // zhen kuo fu xi
            kuo(i, 1, 1);
            kuo(i, -1, -1);
        } else {
            // zhen xi fu kuo
            kuo(i, 1, 1);
            kuo(i, -1, -1);
        }
    } else if (arr[i] <= -n && arr[i] > -2 * n) {
        if (odd) {
            // zhen xi fu kuo
            kuo(i, 1, 1);
            kuo(i, -1, -1);
        } else {
            // zhen kuo fu xi
            kuo(i, 1, 1);
            kuo(i, -1, -1);
        }
    }
}

void comp(int odd)
{
    for (int i = 0; i < nw; i++) {
        arr2[i] = 0;
    }
    for (int i = 0; i < nw; i++) {
        body(odd, i);
    }
    for (int i = 0; i < nw; i++) {
        if (!bad(i))
            arr[i] += arr2[i];
    }
}

inline char color(int i)
{
    i = abs(i);
    if (i < n) return ' ';
    if (i < 2*n) return '.';
    return '*';
}

void show()
{
    for (int i = 0; i < w; i++)
        printf("==");
    putchar('\n');
    for (int i = 0; i < w * w; i++) {
        //printf("%d ", arr[i * w + w / 2]);
        printf("%c ", color(arr[i * w + w / 2]));
        if (i % w == w - 1) putchar('\n');
    }
}

const float clrtab[5][3] = {
    {0, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {1, 1, 0},
    {1, 0, 0},
};

inline int colorindex(int x)
{
    if (x <= -2 * n)
        return 1;
    if (x <= -n)
        return 2;
    if (x < n)
        return 0;
    if (x <= 2 * n)
        return 3;
    return 4;
}

void dump(int frame)
{
    char buf[256];
    sprintf(buf, "/tmp/clr%d.obj", frame);
    FILE *f_clr = fopen(buf, "w");
    for (int i = 0; i < nw; i++) {
        int js[n], t = 0;
        for (int s = 1; s < nw; s *= w) {
            js[t++] = (i / s) % w;
        }
        int ci = colorindex(arr[i]);
        float r = clrtab[ci][0], g = clrtab[ci][1], b = clrtab[ci][2];
        fprintf(f_clr, "v %f %f %f\n", r, g, b);
    }
    fclose(f_clr);
}

void dumppos() {
    FILE *f_pos = fopen("/tmp/pos0.obj", "w");
    float fac = 2.f / w;
    float off = -1.f;
    for (int i = 0; i < nw; i++) {
        int js[n], t = 0;
        for (int s = 1; s < nw; s *= w) {
            js[t++] = (i / s) % w;
        }
        float x = js[0] * fac, y = js[1] * fac, z = js[2] * fac;
        fprintf(f_pos, "v %f %f %f\n", x + off, y + off, z + off);
    }
    fclose(f_pos);
}

inline void assign(int x, int y, int z, int v) {
    arr[x + w * y + w * w * z] = v;
}

int main()
{
    arr.resize(nw);
    arr2.resize(nw);
    dumppos();
    assign(w / 2, w / 2, w / 2, 1024);
    for (int i = 0; i < 100; i++) {
        printf("frame %d\n", i);
        comp(i % 2);
        //show();
        dump(i);
        //usleep(100000);
    }
    return 0;
}
