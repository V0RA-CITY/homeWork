#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <random>

using namespace std;

//=========================================================================================
enum TCell : char
{
    Cross = 'X',
    Zero = '0',
    Empty = '_'
};

enum TProgress
{
    InProgress,
    WonAI,
    WonHuman,
    Draw
};

struct TCoord
{
    size_t x { 0 };
    size_t y { 0 };
};

struct TGame
{
    const size_t SIZE { 3 }; // 3x3 поле
    TCell** ppField{nullptr}; // наше игровое поле
    TCell AI;
    TCell human;
    TProgress progress{InProgress};
    size_t turn{0};// чей ход ? чет == человек;
};

//=========================================================================================

inline void clearSrc()
{
    system("cls");
}

int32_t __fastcall getRandomNum(int32_t min, int32_t max)
{
    const static auto seed = chrono::system_clock::now().time_since_epoch().count();
    static mt19937_64 generator(seed);
    uniform_int_distribution<int32_t> dis(min, max);
    return dis(generator);
}

//=========================================================================================

void  __fastcall initGame(TGame& g)
{
    // динамически выделяем поле
    g.ppField = new TCell * [g.SIZE];
    for (size_t i = 0; i < g.SIZE; i++)
    {
        g.ppField[i] = new TCell[g.SIZE];
    }

    for (size_t x = 0; x < g.SIZE; x++)
    {
        for (size_t y = 0; y < g.SIZE; y++)
        {
            g.ppField[x][y] = Empty;
        }
    }
    if (getRandomNum(0, 1000) > 500)
    {
        g.human = Cross;
        g.AI = Zero;
        g.turn = 0;
    }
    else
    {
        g.human = Zero;
        g.AI = Cross;
        g.turn = 1;
    }
}

void __fastcall DeinitGame(TGame& g)
{
    // Освобождаем выделенную память
    for (size_t i = 0; i < g.SIZE; ++i)
    {
        delete[] g.ppField[i];
    }
    delete[] g.ppField;
    g.ppField = nullptr;
}

void __fastcall printGame(const TGame& g)
{
    cout << "    ";
    for (size_t x = 0; x < g.SIZE; x++)
    {
        cout << x+1 << "   ";
    }
    cout << endl;

    for (size_t x = 0; x < g.SIZE; x++)
    {
        cout << x+1 << " | ";
        for (size_t y = 0; y < g.SIZE; y++)
        {
            cout << g.ppField[y][x] << " | ";
        }
        cout << endl;
    }

    cout <<  " Human: " << g.human << endl << " Computer: " << g.AI << endl;
}

TProgress __fastcall getWon(const TGame &g)
{   // Есть ли выйгрыш в строках?
    for (size_t x = 0; x < g.SIZE; x++)
    {
        if (g.ppField[x][0] == g.ppField[x][1]  && g.ppField[x][0] == g.ppField[x][2])
        {
            if (g.ppField[x][0] == g.human)
                return WonHuman;
            if (g.ppField[x][0] == g.AI)
                return WonAI;
        }
    }
    // Есть ли выйгрыш в стольбцах?
    for (size_t y = 0; y < g.SIZE; y++)
    {
        if (g.ppField[0][y] == g.ppField[1][y]  && g.ppField[0][y] == g.ppField[2][y])
        {
            if (g.ppField[0][y] == g.human)
                return WonHuman;
            if (g.ppField[0][y] == g.AI)
                return WonAI;
        }
    }

    // Есть ли выйгрышь в диагоналях?
    if (g.ppField[0][0] == g.ppField[1][1]  && g.ppField[0][0] == g.ppField[2][2])
    {
        if (g.ppField[0][0] == g.human)
            return WonHuman;
        if (g.ppField[0][0] == g.AI)
            return WonAI;
    }

    if (g.ppField[0][2] == g.ppField[1][1]  && g.ppField[1][1] == g.ppField[2][0])
    {
        if (g.ppField[1][1] == g.human)
            return WonHuman;
        if (g.ppField[1][1] == g.AI)
            return WonAI;
    }
    // Ничья?
    bool draw{true};
    for (size_t x = 0; x < g.SIZE; x++)
    {
        for (size_t y = 0; y < g.SIZE; y++)
        {
            if (g.ppField[x][y] == Empty)
            {
                draw = false;
                break;
            }
        }
        if (!draw)
            break;
    }

    if (draw)
        return Draw;

    return  InProgress;
}

TCoord __fastcall getHumanCoord(const TGame &g)
{
    TCoord c{0};
    do {
        cout << " Enter Y (1..3): ";
        cin >> c.y;
        cout << " Enter X (1..3): ";
        cin >> c.x;
        c.x--;
        c.y--;
    } while (c.x > 2 || c.y > 2 || g.ppField[c.x][c.y] != Empty);
    return c;
}

TCoord __fastcall getAiCoord(const TGame &g)
{
    // Pre win situation - поиск пред выйгрышных ситуаций для ПК
    for (size_t x = 0; x < g.SIZE; x++)
    {
        for (size_t y = 0; y < g.SIZE; y++)
        {
            if (g.ppField[x][y] == Empty)
            {
                g.ppField[x][y] = g.AI;
                if (getWon(g) == WonAI)
                {
                    g.ppField[x][y] = Empty;
                    return {x, y};
                }
                g.ppField[x][y] = Empty;
            }
        }
    }
    // Pre fail situation -
    for (size_t x = 0; x < g.SIZE; x++)
    {
        for (size_t y = 0; y < g.SIZE; y++)
        {
            if (g.ppField[x][y] == Empty)
            {
                g.ppField[x][y] = g.human;
                if (getWon(g) == WonHuman)
                {
                    g.ppField[x][y] = Empty;
                    return {x, y};
                }
                g.ppField[x][y] = Empty;
            }
        }
    }
    // Ход по приорететам + рандомный.

    // Центр
    if(g.ppField[1][1] == Empty)
    {
        return {1, 1};
    }

    // Углы
    TCoord buf[4];
    size_t num = 0;
    if (g.ppField[0][0] == Empty) // Этот угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {0,0};
        num++;
    }
    if (g.ppField[2][2] == Empty) // Этот угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {0,0};
        num++;
    }
    if (g.ppField[2][0] == Empty) // Этот угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {0,0};
        num++;
    }
    if (g.ppField[0][2] == Empty) // Этот угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {0,0};
        num++;
    }
    if (num > 0)
    {
        const size_t index = getRandomNum(0, 1000) % num;
        return buf[index]; // Возвращаем случайный угол из пустых
    }

    if (g.ppField[1][0] == Empty) // Этот не угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {1,0};
        num++;
    }
    if (g.ppField[1][2] == Empty) // Этот не угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {1,2};
        num++;
    }
    if (g.ppField[0][1] == Empty) // Этот не угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {0,1};
        num++;
    }
    if (g.ppField[2][1] == Empty) // Этот не угол пустой?
    {
        // Сохраняем его если да
        buf[num] = {2,1};
        num++;
    }
    if (num > 0)
    {
        const size_t index = getRandomNum(0, 1000) % num;
        return buf[index]; // Возвращаем случайный угол из пустых
    }
}

void congrats(const TGame &g)
{
    if (g.progress == WonHuman)
    {
        cout << "Human won :)" << endl;
    }
    else if (g.progress == WonAI)
    {
        cout << "Computer won :/" << endl;
    }
    else if (g.progress == Draw)
    {
        cout << "It's draw :(" << endl;
    }
}
//=============================================================================================
int main()
{
    TGame g;
    initGame(g);
    clearSrc();
    printGame(g);

    do{
        if (g.turn % 2 == 0)
        {
            //Ход человека
            TCoord c = getHumanCoord(g);
            g.ppField[c.x][c.y] = g.human;
        }
        else
        {
            //Ход Кмпьютера
            TCoord c = getAiCoord(g);
            g.ppField[c.x][c.y] = g.AI;
        }
        g.turn++;
        clearSrc();
        printGame(g);
        g.progress = getWon(g);
    } while (g.progress == InProgress);

    congrats(g);

    DeinitGame(g);
    return 0;
}