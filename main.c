#include <stdio.h>
#include <stdlib.h>
#define _WIN32_WINNT 0x0501
#include <windows.h>

#define WIDTH 80
#define HEIGHT 24
#define c_sand (char)176
#define c_water (char)219
#define c_wall '#'
#define c_space ' '

typedef char TMap[HEIGHT][WIDTH];
TMap map;
POINT mousePosition;
POINT sellSize;
enum {s_sand = 0, s_water, s_wall, s_last} substance = s_sand;
char subChar[] = {c_sand, c_water, c_wall};
char *subName[] = {"sand", "water", "wall"};


POINT GetMousePosition(HWND hwnd, POINT sellSize)
{
    static POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);
    pt.x /= sellSize.x;
    pt.y /= sellSize.y;
    return pt;
}

POINT GetSellSize(HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    POINT sellSize;
    sellSize.x = (rect.right - rect.left) / WIDTH;
    sellSize.y = (rect.bottom - rect.top) / HEIGHT;
    return sellSize;
}

void ClearMap(void)
{
    memset(map, c_space, sizeof(map));
    map[HEIGHT-1][WIDTH-1] = '\0';
}

void SetCursorPosition(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void ShowMap(TMap map)
{
    SetCursorPosition(0, 0);
    printf("%s", map[0]);
}

void ShowInfo()
{
    SetCursorPosition(0, HEIGHT);
    printf("    ");
    for(int i = 0; i < s_last; i++)
        printf("%d-%s   ", i+1, subName[i]);
    printf("LMB-put %s    RMB - clear      ", subName[substance]);
}

void SelectSubstance()
{
    for(int i = 0; i < s_last; i++)
        if(GetKeyState('1' + i) < 0) substance = i;
}

void PutLine(POINT a, POINT b, char sub)
{
    float dx = (b.x - a.x) / (float)WIDTH;
    float dy = (b.y - a.y) / (float)WIDTH;
    for(int i = 0; i < WIDTH; i++)
        map[(int)(a.y + dy * i)][(int)(a.x + dx * i)] = sub;
}

void PutSubstance(POINT pt)
{
    static POINT old;
    if(GetKeyState(VK_LBUTTON) < 0)
    {
       PutLine(old, pt, subChar[substance]);
    }
    else if(GetKeyState(VK_RBUTTON) < 0)
    {
        PutLine(old, pt, c_space);
    }
    old = pt;
}

char IfPointInMap(int x, int y)
{
    return !((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT));
}

void MoveSand(int x, int y)
{
    for(int i = 0; i <= 1; i+=(i == 0 ? -1 : 2))
        if(IfPointInMap(x+i, y+i))
            if ((map[y+1][x+i] == c_space) ||
                (map[y+1][x+i] == c_water))
            {
                map[y][x] = map[y+1][x+i];
                map[y+1][x+i] = c_sand;
                break;
            }
}

TMap mapTmap;
char waterLevel;
POINT foundPoint;

void FindWaterPath(int x, int y)
{
    if(!IfPointInMap(x, y)) return;

    if((y >= waterLevel) && (y > foundPoint.y))
        if(mapTmap[y][x] == c_space)
        {
            foundPoint.x = x;
            foundPoint.y = y;
        }
    if(mapTmap[y][x] == c_water)
    {
        mapTmap[y][x] = c_wall;
        FindWaterPath(x + 1, y);
        FindWaterPath(x - 1, y);
        FindWaterPath(x, y + 1);
        FindWaterPath(x, y - 1);
    }
}

void MoveWater(int x, int y)
{
    if(!IfPointInMap(x, y+1)) return;
    if(map[y+1][x] == c_space)
    {
        map[y][x] = c_space;
        map[y+1][x] = c_water;
    }
    else if(map[y+1][x] == c_water)
    {
        waterLevel = y+1;
        foundPoint.y = -1;
        memcpy(mapTmap, map, sizeof(map));
        FindWaterPath(x, y+1);
        if(foundPoint.y >= 0)
        {
            map[foundPoint.y][foundPoint.x] = c_water;
            map[y][x] = c_space;
        }
    }
}

void MoveSubstance()
{
    for(int j = HEIGHT - 1; j >= 0; j--)
        for(int i = 0; i < WIDTH; i++)
        {
            if(map[j][i] == c_sand) MoveSand(i, j);
            if(map[j][i] == c_water) MoveWater(i, j);
        }
}

int main()
{
    HWND hwnd = GetConsoleWindow();
    sellSize = GetSellSize(hwnd);
    ClearMap();
    do
    {
        mousePosition = GetMousePosition(hwnd, sellSize);
        SelectSubstance();
        PutSubstance(mousePosition);

        MoveSubstance();

        ShowMap(map);
        ShowInfo();
        Sleep(50);
    }while(GetKeyState(VK_ESCAPE) >= 0);
    return 0;
}
