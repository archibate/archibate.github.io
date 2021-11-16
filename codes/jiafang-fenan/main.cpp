#include <cstdio>
#include <vector>
#include <cmath>
#include <unistd.h>

using namespace std;

vector<int> arr;
vector<int> arr2;

const int n = 3;
const int w = 20;
const int nw = w * w * w;

inline int c(int i)
{
    return i < 0 ? 0 : i > nw - 1 ? nw - 1 : i;
}

inline void kuo(int i, int z, int k)
{
    arr2[i] -= k * n;
    for (int s = 1; s < nw; s *= w) {
        arr2[i + z * s] += k;
    }
}

void body(int odd, int i)
{
    for (int s = 1; s < nw; s *= w) {
        int j = (i / s) % w;
        if (j <= 1 || j >= w - 1) return;
    }
    if (arr[i] >= 2 * n) {
        // zhen kuo fu kuo
        kuo(i, 1, 1);
        kuo(i, -1, 1);
    } else if (arr[i] <= 2 * n) {
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
    arr2 = arr;
    for (int i = 0; i < nw; i++) {
        body(odd, i);
    }
    arr.swap(arr2);
}

char color(int i)
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
        printf("%c ", color(arr[i * w + w / 2]));
        if (i % w == w - 1) putchar('\n');
    }
}

int main()
{
    arr.resize(nw);
    arr[10 + w * 10 + w * w * 10] = 1024;
    for (int i = 0; i < 32; i++) {
        comp(i % 2);
        show();
        usleep(100000);
    }
    return 0;
}
