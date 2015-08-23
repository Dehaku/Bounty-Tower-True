#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#ifndef GALAXY_LINUX
#include <windows.h>
#endif

#include "Items.h"
#include "Faction.h"
#include "Tiles.h"
#include "FactionJobSecurity.h"
#include "Galaxy.h"
#include "BountyTower.h"
#include "menus.h"
#include "astar.h"
#include "Textures.h"
#include "Sounds.h"
#include "math.h"
#include "util.h"
#include "globalvars.h"
#include "Networking.h"
#include "Bullet.h"
#include "Camera.h"

#include <SFML/Audio.hpp>

#define USE_PATHER

#ifdef USE_PATHER

#include "micropather.h"
using namespace micropather;
#endif

using std::abs;

template <typename T> T &listAt(std::list<T> &list, size_t index)
{
    auto it = list.begin();
    std::advance(it, index);
    return *it;
}

template <typename T> void AnyDeletes(std::vector<T> &list)
{ // Oh my goodness, I freakkin love templates, I'll need to redesign a few things to incorporate this functionality.

    //auto it = list.begin();
    //std::advance(it, index);
    /*
    for (auto &listings : list)
    {
        if(listings.toDelete)
        {
            std::cout << "To be deleted! \n";
        }
    }
    */
    bool done = false;
    while (done == false)
    {
        bool yet = false;
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if (it->toDelete)
            {
                list.erase(it);
                yet = true;
                break;
            }
        }
        if (yet == false)
        {
            done = true;
        }
    }

    //return;
}

template <typename T> void AnyDeletes(std::list<T> &list)
{ // Oh my goodness, I freakkin love templates, I'll need to redesign a few things to incorporate this functionality.

    //auto it = list.begin();
    //std::advance(it, index);
    /*
    for (auto &listings : list)
    {
        if(listings.toDelete)
        {
            std::cout << "To be deleted! \n";
        }
    }
    */
    bool done = false;
    while (done == false)
    {
        bool yet = false;
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            //std::cout << it->name << ",'s toDelete: " << it->toDelete << std::endl;
            if (it->toDelete)
            {
                list.erase(it);
                yet = true;
                break;
            }
        }
        if (yet == false)
        {
            done = true;
        }
    }

    //return;
}


const int worldSizeX = 32;
const int worldSizeY = 32;
const int worldSizeZ = 3;

class cTile
{
public:
    Vec3 position;
    int type;
    bool teleporter;
    Vec3 telePos;
    void setPos(int x, int y, int z)
    {
        position = Vec3(x, y, z);
    }
    Vec3 getPos()
    {
        return position;
    }
    cTile()
    {
        type = 0;
        teleporter = false;
        telePos = Vec3(-1, -1, -1);
    }
};

cTile grid[worldSizeX][worldSizeY][worldSizeZ];

class worldclass
#ifdef USE_PATHER
    : public Graph
#endif
{
private:
    worldclass(const worldclass &);
    void operator=(const worldclass &);

public:
    void NodeToXYZ(void *node, int *x, int *y, int *z)
    {
        cTile *Nodeling = static_cast<cTile *>(node);

        *x = Nodeling->position.x;
        *y = Nodeling->position.y;
        *z = Nodeling->position.z;
    }

    void *XYZToNode(int x, int y, int z)
    {
        return (void *)&(grid[x][y][z]);
    }

    int Passable(int nx, int ny, int nz)
    {
        if (nx >= 0 && nx < worldSizeX && ny >= 0 && ny < worldSizeY &&
            nz >= 0 && nz < worldSizeZ)
        {
            if (grid[nx][ny][nz].type == 0)
                return 1;
            if (grid[nx][ny][nz].type == 2)
                return 2;
            if (grid[nx][ny][nz].type == 3)
                return 3;
            if (grid[nx][ny][nz].type == 10)
                return 1;
        }
        return 0;
    }

    void toggleTeleporter()
    {
        if (grid[5][1][0].type == 10)
        {
            grid[5][1][0].type = 0;
            grid[5][1][0].teleporter = false;
            grid[5][1][0].telePos = Vec3(-1, -1, -1);

            grid[20][30][2].type = 0;
            grid[20][30][2].teleporter = false;
            grid[20][30][2].telePos = Vec3(-1, -1, -1);
        }
        else
        {
            grid[5][1][0].type = 10;
            grid[5][1][0].teleporter = true;
            grid[5][1][0].telePos = Vec3(20, 30, 2);
            grid[20][30][2].type = 10;
            grid[20][30][2].teleporter = true;
            grid[20][30][2].telePos = Vec3(5, 1, 0);
        }
    }

#ifdef USE_PATHER

    MPVector<void *> microPath;
    MicroPather *pather;
    std::vector<cTile *> storedPath;

    worldclass() : pather(0)
    {
        pather = new MicroPather(
            this, 20); // Use a very small memory block to stress the pather
    }

    void drawPath()
    {
        unsigned int k;
        unsigned int pathSize = microPath.size();
        Vec3 oldPos;
        for (k = 0; k < pathSize; ++k)
        {
            Vec3 pathPos;
            NodeToXYZ(microPath[k], &pathPos.x, &pathPos.y, &pathPos.z);
            sf::Color pathColor(0, 0, 0);
            if (pathPos.z == 0)
                pathColor.r = 255;
            if (pathPos.z == 1)
                pathColor.g = 255;
            if (pathPos.z == 2)
                pathColor.b = 255;

            if (k != 0)
                effects.createLine((oldPos.x + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (oldPos.y + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (pathPos.x + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (pathPos.y + 1) * GRID_SIZE - GRID_SIZE/2, 5, pathColor);

            oldPos = pathPos;
        }
    }

    void drawStoredPath()
    {
        Vec3 oldPos;
        bool firstRun = true;

        for (auto &i : storedPath)
        {
            Vec3 pathPos;
            pathPos = Vec3(i->getPos());
            sf::Color pathColor(0, 0, 0);
            if (pathPos.z == 0)
                pathColor.r = 255;
            if (pathPos.z == 1)
                pathColor.g = 255;
            if (pathPos.z == 2)
                pathColor.b = 255;

            if (!firstRun)
                effects.createLine((oldPos.x + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (oldPos.y + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (pathPos.x + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (pathPos.y + 1) * GRID_SIZE - GRID_SIZE/2, 5, pathColor);

            oldPos = pathPos;
            firstRun = false;
        }
        storedPath.clear();
    }

    void storePath(void *node)
    {
        cTile *Nodeling = static_cast<cTile *>(node);
        storedPath.push_back(Nodeling);
    }

    int makePath(Vec3 Ori, Vec3 Tar)
    {
        int result = 0;
        if (Passable(Tar.x, Tar.y, Tar.z) == 1)
        {
#ifdef USE_PATHER
            float totalCost;

            result = pather->Solve(XYZToNode(Ori.x, Ori.y, Ori.z),
                                   XYZToNode(Tar.x, Tar.y, Tar.z), &microPath,
                                   &totalCost);

            if (result == MicroPather::SOLVED)
            {
                unsigned int pathSize = microPath.size();
                for (int i = 0; i != pathSize; i++)
                {
                    storePath(microPath[i]);
                }
            }

//printf( "Pather returned %d\n", result );

#endif
        }
        return result;
    }

    virtual ~worldclass()
    {
        delete pather;
    }

    virtual float LeastCostEstimate(void *nodeStart, void *nodeEnd)
    {
        int xStart, yStart, zStart, xEnd, yEnd, zEnd;
        NodeToXYZ(nodeStart, &xStart, &yStart, &zStart);
        NodeToXYZ(nodeEnd, &xEnd, &yEnd, &zEnd);

        double d = sqrt(pow(xEnd - xStart, 2) + pow(yEnd - yStart, 2) +
                        pow(zEnd - zStart, 2));
        /* Compute the minimum path cost using distance measurement. It is possible
		   to compute the exact minimum path using the fact that you can move only
		   on a straight line or on a diagonal, and this will yield a better result.
		*/
        return d;
    }

    virtual void AdjacentCost(void *node,
                              micropather::MPVector<StateCost> *neighbors)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        //const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
        //const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
        //const int dx[26] = { -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1};
        //const int dy[26] = { -1, -1, -1, 0, 0, 0, 1, 1, 1, -1, -1, -1, 0, 0, 1, 1, 1, -1, -1, -1, 0, 0, 0, 1, 1, 1};
        //const int dz[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};

        const int dx[10] = {0, -1, 0, 1, -1, 1, -1, 0, 1, 0};
        const int dy[10] = {0, -1, -1, -1, 0, 0, 1, 1, 1, 0};
        const int dz[10] = {-1, 0, 0, 0, 0, 0, 0, 0, 0, 1};

        //const float cost[8] = { 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f };
        float One = 1;
        float Two = 1.41f;
        float Three = 1.44f;
        const float cost[10] = {One, Two, One, Two, One,
                                One, Two, One, Two, One};

        cTile *Nodeling = static_cast<cTile *>(node);

        if (Nodeling->teleporter)
        {
            Vec3 N(Nodeling->telePos);
            StateCost nodeCost = {XYZToNode(N.x, N.y, N.z), 3};
            neighbors->push_back(nodeCost);
        }

        for (int i = 0; i < 10; ++i)
        {

            int nx = x + dx[i];
            int ny = y + dy[i];
            int nz = z + dz[i];

            //void* targetTile = XYZToNode(nx,ny,nz);

            int pass = Passable(nx, ny, nz);
            if (pass > 0)
            {
                //std::cout << dz[i];
                if (pass == 1 && dz[i] == 0)
                {
                    // Normal floor
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if (pass == 3 && dz[i] == -1 && dx[i] == 0 && dy[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if (pass == 2 && dz[i] == 1 && dx[i] == 0 && dy[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else
                {
                    // Normal floor
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), FLT_MAX};
                    neighbors->push_back(nodeCost);
                }
            }
        }
    }

    virtual void
    AdjacentCostPureFlight(void *node,
                           micropather::MPVector<StateCost> *neighbors)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        //const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
        //const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
        const int dx[26] = {-1, 0,  1, -1, 0,  1, -1, 0,  1, -1, 0,  1, -1,
                            1,  -1, 0, 1,  -1, 0, 1,  -1, 0, 1,  -1, 0, 1};
        const int dy[26] = {-1, -1, -1, 0, 0,  0,  1,  1, 1, -1, -1, -1, 0,
                            0,  1,  1,  1, -1, -1, -1, 0, 0, 0,  1,  1,  1};
        const int dz[26] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0,
                            0,  0,  0,  0,  1,  1,  1,  1,  1,  1, 1, 1, 1};
        //const float cost[8] = { 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f };
        float One = 1;
        float Two = 1.41f;
        float Three = 1.44f;
        const float cost[26] = {Three, Two,   Three, Two,   One,  Two,   Three,
                                Two,   Three, Two,   One,   Two,  One,   One,
                                Two,   One,   Two,   Three, Two,  Three, Two,
                                One,   Two,   Three, Two,   Three};

        for (int i = 0; i < 26; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            int nz = z + dz[i];

            int pass = Passable(nx, ny, nz);
            if (pass > 0)
            {
                if (pass == 1)
                {
                    // Normal floor
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else
                {
                    // Normal floor
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), FLT_MAX};
                    neighbors->push_back(nodeCost);
                }
            }
        }
    }

    virtual void PrintStateInfo(void *node)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        printf("(%d,%d,%d)", x, y, &z);
    }

#endif
};
worldclass world;

class PathingController
#ifdef USE_PATHER
    : public Graph
#endif
{
private:
    PathingController(const PathingController &);
    void operator=(const PathingController &);

public:
    void NodeToXYZ(void *node, int *x, int *y, int *z)
    {
        Tile *Nodeling = static_cast<Tile *>(node);

        *x = Nodeling->pos.x;
        *y = Nodeling->pos.y;
        *z = Nodeling->pos.z;
    }

    void *XYZToNode(int x, int y, int z)
    {
        return (void *)&(tiles[x][y][z]);
    }

    int Passable(int nx, int ny, int nz)
    {
        if (nx >= 0 && nx < GRIDS && ny >= 0 && ny < GRIDS &&
            nz >= 0 && nz < CHUNK_SIZE)
        {

            if (tiles[nx][ny][nz].goesDown && tiles[nx][ny][nz].goesUp)
                return 4;
            if (tiles[nx][ny][nz].goesDown)
                return 3; // return 2;
            if (tiles[nx][ny][nz].goesUp)
                return 2; // return 3;
            if (tiles[nx][ny][nz].walkable)
                return 1;
        }
        return 0;
    }

#ifdef USE_PATHER

    MPVector<void *> microPath;
    MicroPather *pather;
    std::vector<Tile *> storedPath;
    std::vector<Tile *> storedRPath;

    PathingController() : pather(0)
    {
        pather = new MicroPather(
            this, 20); // Use a very small memory block to stress the pather
    }

    void drawPath()
    {
        unsigned int k;
        unsigned int pathSize = microPath.size();
        Vec3 oldPos;
        for (k = 0; k < pathSize; ++k)
        {
            Vec3 pathPos;
            NodeToXYZ(microPath[k], &pathPos.x, &pathPos.y, &pathPos.z);
            sf::Color pathColor(0, 0, 0);
            if (pathPos.z == 0)
                pathColor.r = 255;
            if (pathPos.z == 1)
                pathColor.g = 255;
            if (pathPos.z == 2)
                pathColor.b = 255;

            if (k != 0)
                effects.createLine((oldPos.x + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (oldPos.y + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (pathPos.x + 1) * GRID_SIZE - GRID_SIZE/2,
                                   (pathPos.y + 1) * GRID_SIZE - GRID_SIZE/2, 5, pathColor);

            oldPos = pathPos;
        }
    }

    void drawStoredPath()
    {
        Vec3 oldPos;
        bool firstRun = true;

        for (auto &i : storedPath)
        {
            Vec3 pathPos;
            pathPos = Vec3(i->getPos());
            sf::Color pathColor(255, 255, 255, 100);

            if (!firstRun)
                effects.createLine((oldPos.x + 1) * GRID_SIZE - (GRID_SIZE/2),
                                   (oldPos.y + 1) * GRID_SIZE - (GRID_SIZE/2),
                                   (pathPos.x + 1) * GRID_SIZE - (GRID_SIZE/2),
                                   (pathPos.y + 1) * GRID_SIZE - (GRID_SIZE/2), 5, pathColor);

            oldPos = pathPos;
            firstRun = false;
        }

    }

    void drawStoredPathRainbow()
    {
        Vec3 oldPos;
        bool firstRun = true;

        for (auto &i : storedRPath)
        {
            Vec3 pathPos;
            pathPos = Vec3(i->getPos());
            sf::Color pathColor(randz(2,253), randz(2,253), randz(2,253));


            if (!firstRun)
                effects.createLine((oldPos.x + 1) * 20 - 10,
                                   (oldPos.y + 1) * 20 - 10,
                                   (pathPos.x + 1) * 20 - 10,
                                   (pathPos.y + 1) * 20 - 10, 5, pathColor);

            oldPos = pathPos;
            firstRun = false;
        }

    }

    void storePath(void *node)
    {
        Tile *Nodeling = static_cast<Tile *>(node);
        storedPath.push_back(Nodeling);
    }
    void storeRPath(void *node)
    {
        Tile *Nodeling = static_cast<Tile *>(node);
        storedRPath.push_back(Nodeling);
    }

    int makePath(Vec3 Ori, Vec3 Tar)
    {
        int result = 0;
        if (Passable(Tar.x, Tar.y, Tar.z) == 1)
        {
#ifdef USE_PATHER
            float totalCost;

            result = pather->Solve(XYZToNode(Ori.x, Ori.y, Ori.z),
                                   XYZToNode(Tar.x, Tar.y, Tar.z), &microPath,
                                   &totalCost);

            if (result == MicroPather::SOLVED)
            {
                unsigned int pathSize = microPath.size();
                for (int i = 0; i != pathSize; i++)
                {
                    storePath(microPath[i]);
                }
            }
            unsigned int pathSize = microPath.size();
            for (int i = 0; i != pathSize; i++)
            {
                storeRPath(microPath[i]);
            }



#endif
        }
        return result;
    }

    virtual ~PathingController()
    {
        delete pather;
    }

    virtual float LeastCostEstimate(void *nodeStart, void *nodeEnd)
    {
        int xStart, yStart, zStart, xEnd, yEnd, zEnd;
        NodeToXYZ(nodeStart, &xStart, &yStart, &zStart);
        NodeToXYZ(nodeEnd, &xEnd, &yEnd, &zEnd);

        double d = sqrt(pow(xEnd - xStart, 2) + pow(yEnd - yStart, 2) +
                        pow(zEnd - zStart, 2));
        /* Compute the minimum path cost using distance measurement. It is possible
		   to compute the exact minimum path using the fact that you can move only
		   on a straight line or on a diagonal, and this will yield a better result.
		*/
        return d;
    }

    virtual void AdjacentCost(void *node,
                              micropather::MPVector<StateCost> *neighbors)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        //const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
        //const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
        //const int dx[26] = { -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1};
        //const int dy[26] = { -1, -1, -1, 0, 0, 0, 1, 1, 1, -1, -1, -1, 0, 0, 1, 1, 1, -1, -1, -1, 0, 0, 0, 1, 1, 1};
        //const int dz[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};

        const int dx[10] = {0, -1, 0, 1, -1, 1, -1, 0, 1, 0};
        const int dy[10] = {0, -1, -1, -1, 0, 0, 1, 1, 1, 0};
        const int dz[10] = {-1, 0, 0, 0, 0, 0, 0, 0, 0, 1};

        //const float cost[8] = { 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f };
        float One = 1;
        float Two = 1.41f;
        float Three = 1.44f;
        const float cost[10] = {One, Two, One, Two, One,
                                One, Two, One, Two, One};

        Tile *Nodeling = static_cast<Tile *>(node);

        if (Nodeling->teleporter)
        {
            Vec3 N(Nodeling->telePos);
            StateCost nodeCost = {XYZToNode(N.x, N.y, N.z), 3};
            neighbors->push_back(nodeCost);
        }

        for (int i = 0; i < 10; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            int nz = z + dz[i];

            int pass = Passable(nx, ny, nz);
            if (pass > 0)
            {
                if (pass == 4)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if ( (pass == 1 || pass == 2 || pass == 3) && dz[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if (pass == 3 && dz[i] == -1 && dx[i] == 0 && dy[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if (pass == 2 && dz[i] == 1 && dx[i] == 0 && dy[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), FLT_MAX};
                    neighbors->push_back(nodeCost);
                }
            }
        }
    }

    virtual void
    AdjacentCostPureFlight(void *node,
                           micropather::MPVector<StateCost> *neighbors)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        //const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
        //const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
        const int dx[26] = {-1, 0,  1, -1, 0,  1, -1, 0,  1, -1, 0,  1, -1,
                            1,  -1, 0, 1,  -1, 0, 1,  -1, 0, 1,  -1, 0, 1};
        const int dy[26] = {-1, -1, -1, 0, 0,  0,  1,  1, 1, -1, -1, -1, 0,
                            0,  1,  1,  1, -1, -1, -1, 0, 0, 0,  1,  1,  1};
        const int dz[26] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0,
                            0,  0,  0,  0,  1,  1,  1,  1,  1,  1, 1, 1, 1};
        //const float cost[8] = { 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f };
        float One = 1;
        float Two = 1.41f;
        float Three = 1.44f;
        const float cost[26] = {Three, Two,   Three, Two,   One,  Two,   Three,
                                Two,   Three, Two,   One,   Two,  One,   One,
                                Two,   One,   Two,   Three, Two,  Three, Two,
                                One,   Two,   Three, Two,   Three};

        for (int i = 0; i < 26; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            int nz = z + dz[i];

            int pass = Passable(nx, ny, nz);
            if (pass > 0)
            {
                if (pass == 1)
                {
                    // Normal floor
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else
                {
                    // Normal floor
                    StateCost nodeCost = {XYZToNode(nx, ny, nz), FLT_MAX};
                    neighbors->push_back(nodeCost);
                }
            }
        }
    }

    virtual void PrintStateInfo(void *node)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        printf("(%d,%d,%d)", x, y, &z);
    }

#endif
};
PathingController pathCon;


void drawStoredPath(std::vector<Tile *> storedPath)
    {
        Vec3 oldPos;
        bool firstRun = true;

        for (auto &i : storedPath)
        {
            Vec3 pathPos;
            pathPos = Vec3(i->getPos());
            sf::Color pathColor(255, 255, 255, 100);

            if (!firstRun)
                effects.createLine((oldPos.x + 1) * GRID_SIZE - (GRID_SIZE/2),
                                   (oldPos.y + 1) * GRID_SIZE - (GRID_SIZE/2),
                                   (pathPos.x + 1) * GRID_SIZE - (GRID_SIZE/2),
                                   (pathPos.y + 1) * GRID_SIZE - (GRID_SIZE/2), 5, pathColor);

            oldPos = pathPos;
            firstRun = false;
        }
    }


//-- Prototypes
//

void drawStuffs();
int getItemVectorId(int id);

std::string getClipboardText()
{
#ifdef GALAXY_LINUX
    return "Not implemented.";
#else
    // Try opening the clipboard
    if (!OpenClipboard(NULL))
        con("ClipError1");

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL)
        con("ClipError1");

    // Lock the handle to get the actual text pointer
    char *pszText = static_cast<char *>(GlobalLock(hData));
    if (pszText == NULL)
        con("ClipError1");

    // Save text in a string class instance
    std::string text(pszText);
    con("Your Clipboard Contains: ", false);
    con(text);

    // Release the lock
    GlobalUnlock(hData);

    // Release the clipboard
    CloseClipboard();

    return text;
#endif
}

// Create the main rendering window
sf::RenderWindow window;



class Blob
{
public:
    int sizeX;
    int sizeY;
    int sizeZ;

    int regionX;
    int regionY;


    std::vector<std::vector<std::vector<Tile>>> tiles;

    void resizeGrid(int x, int y, int z)
    {
        tiles.resize(x);

        for(int i = 0; i != x; i++)
        {
            tiles.at(i).resize(y);
        }
        for(int i = 0; i != x; i++)
        {
            for(int t = 0; t != y; t++)
                tiles.at(i).at(t).resize(z);
        }
        sizeX = x;
        sizeY = y;
        sizeZ = z;
    }

    Blob()
    {
        sizeX = 0;
        sizeY = 0;
        sizeZ = 0;
        regionX = 0;
        regionY = 0;
    }
};


class UniverseTiles : public Graph
{
private:
    UniverseTiles(const UniverseTiles &);
    void operator=(const UniverseTiles &);
public:
    std::list<Blob> blobs;


    void NodeToXYZ(void *node, int *x, int *y, int *z)
    {
        Tile *Nodeling = static_cast<Tile *>(node);

        *x = Nodeling->pos.x;
        *y = Nodeling->pos.y;
        *z = Nodeling->pos.z;
    }

    void *XYZToNode(int x, int y, int z, std::vector<std::vector<std::vector<Tile>>> &Tiles)
    {
        return (void *)&(Tiles[x][y][z]);
    }

    int Passable(int nx, int ny, int nz)
    {
        if (nx >= 0 && nx < GRIDS && ny >= 0 && ny < GRIDS &&
            nz >= 0 && nz < CHUNK_SIZE)
        {

            if (tiles[nx][ny][nz].goesDown && tiles[nx][ny][nz].goesUp)
                return 4;
            if (tiles[nx][ny][nz].goesDown)
                return 3; // return 2;
            if (tiles[nx][ny][nz].goesUp)
                return 2; // return 3;
            if (tiles[nx][ny][nz].walkable)
                return 1;
        }
        return 0;
    }


    MPVector<void *> microPath;
    MicroPather *pather;
    std::vector<Tile *> storedPath;

    UniverseTiles() : pather(0)
    {
        pather = new MicroPather(
            this, 20); // Use a very small memory block to stress the pather
    }

    void drawPath()
    {
        unsigned int k;
        unsigned int pathSize = microPath.size();
        Vec3 oldPos;
        for (k = 0; k < pathSize; ++k)
        {
            Vec3 pathPos;
            NodeToXYZ(microPath[k], &pathPos.x, &pathPos.y, &pathPos.z);
            sf::Color pathColor(0, 0, 0);
            if (pathPos.z == 0)
                pathColor.r = 255;
            if (pathPos.z == 1)
                pathColor.g = 255;
            if (pathPos.z == 2)
                pathColor.b = 255;

            if (k != 0)
                effects.createLine((oldPos.x + 1) * 20 - 10,
                                   (oldPos.y + 1) * 20 - 10,
                                   (pathPos.x + 1) * 20 - 10,
                                   (pathPos.y + 1) * 20 - 10, 5, pathColor);

            oldPos = pathPos;
        }
    }

    void drawStoredPath()
    {
        Vec3 oldPos;
        bool firstRun = true;

        for (auto &i : storedPath)
        {
            Vec3 pathPos;
            pathPos = Vec3(i->getPos());
            sf::Color pathColor(255, 255, 255, 100);

            if (!firstRun)
                effects.createLine((oldPos.x + 1) * 20 - 10,
                                   (oldPos.y + 1) * 20 - 10,
                                   (pathPos.x + 1) * 20 - 10,
                                   (pathPos.y + 1) * 20 - 10, 5, pathColor);

            oldPos = pathPos;
            firstRun = false;
        }

    }

    void storePath(void *node)
    {
        Tile *Nodeling = static_cast<Tile *>(node);
        storedPath.push_back(Nodeling);
    }

    int makePath(Vec3 Ori, Vec3 Tar)
    {
        int result = 0;
        if (Passable(Tar.x, Tar.y, Tar.z) == 1)
        {

            float totalCost;

            result = pather->Solve(XYZToNode(Ori.x, Ori.y, Ori.z, tiles),
                                   XYZToNode(Tar.x, Tar.y, Tar.z, tiles), &microPath,
                                   &totalCost);

            if (result == MicroPather::SOLVED)
            {
                unsigned int pathSize = microPath.size();
                for (int i = 0; i != pathSize; i++)
                {
                    storePath(microPath[i]);
                }
            }
        }
        return result;
    }

    void makeTest()
    {
        Blob blobo;
        blobo.resizeGrid(3,3,3);
        for(int x = 0; x != blobo.sizeX; x++)
            for(int y = 0; y != blobo.sizeY; y++)
        {
            blobo.tiles[x][y][0].dirt();
            blobo.tiles[x][y][1].stoneWall();
            blobo.tiles[x][y][2].dirt();
        }
        blobo.regionX = 0;
        blobo.regionY = 0;
        blobs.push_back(blobo);
        blobo.regionX = 2;
        //blobo.regionY = 7;
        blobs.push_back(blobo);
        blobo.regionX = 1;
        blobo.regionY = 0;
        for(int x = 0; x != blobo.sizeX; x++)
            for(int y = 0; y != blobo.sizeY; y++)
        {
            blobo.tiles[x][y][1].dirt();
        }
        blobo.tiles[1][1][0].stoneWall();
        blobs.push_back(blobo);
    }

    void drawTiles()
    {
        int TiSi = GRID_SIZE; // TileSize
        int ReSi = 3*TiSi; // RegionSize, AKA Region Spacing
        for(auto &i : blobs)
        {
            for(int x = 0; x != i.sizeX; x++)
                for(int y = 0; y != i.sizeY; y++)
                    for(int z = 0; z != i.sizeZ; z++)
            {
                int StartX = 200+(i.regionX*ReSi) + x*TiSi;
                int StartY = 200+(i.regionY*ReSi) + y*TiSi;
                int EndX = 200+(i.regionX*ReSi) + (x+1)*TiSi;
                int EndY = 200+(i.regionY*ReSi) + (y+1)*TiSi;
                if(z == 0 && !i.tiles[x][y][z].walkable)
                    effects.createSquare(StartX+4,StartY+4,EndX-4,EndY-4,sf::Color::Transparent,1,sf::Color::Red);
                if(z == 1 && !i.tiles[x][y][z].walkable)
                    effects.createSquare(StartX+2,StartY+2,EndX-2,EndY-2,sf::Color::Transparent,1,sf::Color::Green);
                if(z == 2 && !i.tiles[x][y][z].walkable)
                    effects.createSquare(StartX,StartY,EndX,EndY,sf::Color::Transparent,1,sf::Color::Blue);
            }
        }
    }

    virtual ~UniverseTiles()
    {
        delete pather;
    }

    virtual float LeastCostEstimate(void *nodeStart, void *nodeEnd)
    {
        int xStart, yStart, zStart, xEnd, yEnd, zEnd;
        NodeToXYZ(nodeStart, &xStart, &yStart, &zStart);
        NodeToXYZ(nodeEnd, &xEnd, &yEnd, &zEnd);

        double d = sqrt(pow(xEnd - xStart, 2) + pow(yEnd - yStart, 2) +
                        pow(zEnd - zStart, 2));
        /* Compute the minimum path cost using distance measurement. It is possible
		   to compute the exact minimum path using the fact that you can move only
		   on a straight line or on a diagonal, and this will yield a better result.
		*/
        return d;
    }

    virtual void AdjacentCost(void *node,
                              micropather::MPVector<StateCost> *neighbors)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        //const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
        //const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
        //const int dx[26] = { -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1};
        //const int dy[26] = { -1, -1, -1, 0, 0, 0, 1, 1, 1, -1, -1, -1, 0, 0, 1, 1, 1, -1, -1, -1, 0, 0, 0, 1, 1, 1};
        //const int dz[26] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};

        const int dx[10] = {0, -1, 0, 1, -1, 1, -1, 0, 1, 0};
        const int dy[10] = {0, -1, -1, -1, 0, 0, 1, 1, 1, 0};
        const int dz[10] = {-1, 0, 0, 0, 0, 0, 0, 0, 0, 1};

        //const float cost[8] = { 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f, 1.0f, 1.41f };
        float One = 1;
        float Two = 1.41f;
        float Three = 1.44f;
        const float cost[10] = {One, Two, One, Two, One,
                                One, Two, One, Two, One};

        Tile *Nodeling = static_cast<Tile *>(node);

        if (Nodeling->teleporter)
        {
            Vec3 N(Nodeling->telePos);
            StateCost nodeCost = {XYZToNode(N.x, N.y, N.z, tiles), 3};
            neighbors->push_back(nodeCost);
        }

        for (int i = 0; i < 10; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            int nz = z + dz[i];

            int pass = Passable(nx, ny, nz);
            if (pass > 0)
            {
                if (pass == 4)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz, tiles), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if ( (pass == 1 || pass == 2 || pass == 3) && dz[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz, tiles), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if (pass == 3 && dz[i] == -1 && dx[i] == 0 && dy[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz, tiles), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else if (pass == 2 && dz[i] == 1 && dx[i] == 0 && dy[i] == 0)
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz, tiles), cost[i]};
                    neighbors->push_back(nodeCost);
                }
                else
                {
                    StateCost nodeCost = {XYZToNode(nx, ny, nz, tiles), FLT_MAX};
                    neighbors->push_back(nodeCost);
                }
            }
        }
    }

    virtual void PrintStateInfo(void *node)
    {
        int x, y, z;
        NodeToXYZ(node, &x, &y, &z);
        printf("(%d,%d,%d)", x, y, &z);
    }

};
UniverseTiles UnyTiles;




void unpointItems(std::list<Item> &items)
{

    //for(int i = 0; i != Items.size(); i++)
    for (auto i = items.begin(); i != items.begin(); i++)
    {
        if ((*i).toDelete)
        {
            /*


            std::cout << "JobList size: " << uniFact[0].jobList.size()
                      << std::endl;

            for (size_t t = 0; t != uniFact[0].jobList.size(); t++)
            {
                std::cout << "Job Point: " << &uniFact[0].jobList[t].pItem
                          << "/" << uniFact[0].jobList[t].pItem
                          << ", Item Point: " << &(*i) << "/" << &(*i)
                          << std::endl;
                fSleep(1);

                if (uniFact[0].jobList[t].pItem != nullptr &&
                    (&(*i)) != nullptr)
                {

                    std::cout << "ID! ";
                    try
                    {
                        std::cout << uniFact[0].jobList[t].pItem->id;
                    }
                    catch (std::exception &e)
                    {
                        std::cout << "Something went wrong in in here\n";
                    }
                    std::cout << uniFact[0].jobList[t].pItem->id;
                    std::cout << " ; ";
                    int id1 = uniFact[0].jobList[t].pItem->id;
                    std::cout << "ID 2! ";
                    std::cout << (*i).id;
                    std::cout << " ; ";
                    int id2 = (*i).id;
                    std::cout << "ID's: " << id1 << ":" << id2 << std::endl;

                    if (id1 ==
                        id2) //if(UniFact[0].JobList[t].pItem != NULL && &(*i) != NULL && UniFact[0].JobList[t].pItem == &(*i)) //if(ID1 == ID2)
                    {
                        std::cout << "Match! \n";
                        uniFact[0].jobList[t].pItem = nullptr;
                        //fSleep(1);
                    }
                }
            }
            */
        }
    }
}

bool removeItem(int id)
{
    int tempInt = 0;
    std::list<Item>::iterator location;
    for (auto it = worlditems.begin(); it != worlditems.end(); ++it)
    {
        if (it->id == id)
        {
            location = it;
            tempInt = 1;
        }
    }
    if (tempInt == 1)
    {
        worlditems.erase(location);
        return true;
    }
    else if (tempInt == 0)
    {
        return false;
    }
    throw std::runtime_error("Couldn't return anything sensible");
}

void updateItem()
{
    sf::Lock lock(mutex::itemList);
    if (gvars::debug)
    {
        std::cout << "Pre Item \n";
    }

    for (auto &item : worlditems)
    {
        try
        {
            if (gvars::debug)
            {
                std::cout << item.name << "'s turn! ";
            }
            if (item.produces == true)
            {
                if (gvars::debug)
                {
                    std::cout << item.name << " can produce. ";
                }
                item.prodratetimer++;
                if (item.prodratetimer >= item.prodrate && network::connectedServer == "")
                {
                    item.prodratetimer = 0;
                    std::string s;
                    s = item.produce;
                    auto a = new char[s.size() + 1];
                    a[s.size()] = 0;
                    memcpy(a, s.c_str(), s.size());
                    bool findEmpty = false;
                    int x;
                    int y;
                    int z;
                    while (findEmpty == false)
                    {
                        x = randz(item.xpos - 10, item.xpos + 10);
                        y = randz(item.ypos - 10, item.ypos + 10);
                        z = item.zpos;
                        if (tiles[abs_to_index(x / GRID_SIZE)][abs_to_index(
                                y / GRID_SIZE)][abs_to_index(
                                z / GRID_SIZE)].walkable == true)
                        {
                            findEmpty = true;
                        }
                    }
                    if (gvars::debug)
                    {
                        std::cout << "Producing: " << s << " ";
                    }
                    spawnItem(s, x, y);
                    if (gvars::debug)
                    {
                        std::cout << item.name << " has produced: " << s
                                  << " ";
                    }
                }
            }
            if (gvars::debug)
            {
                std::cout << "Done with: " << item.name << " \n";
            }

            if(item.name == "Rune")
            {
                if(randz(1,30) == 30)
                {
                    Orb orb;
                    orb.pos = sf::Vector2f(item.xpos,item.ypos);
                    orb.orbDistance = randz(30,50);
                    orb.orbSpeed = randz(1,5);
                    orb.type = randz(0,3);
                    orb.lifeTime = randz(30,600);
                    Orbs.push_back(orb);
                }
            }

            if (gvars::debug)
            {
                std::cout << "Acting on Missile \n";
            }
            if (item.targetPos != sf::Vector2f(-1, -1))
            {
                item.xpos = (item.xpos + item.targetPos.x) / 2;
                item.ypos = (item.ypos + item.targetPos.y) / 2;
                if (item.age >= 10)
                {
                    item.toDelete = true;
                    boom(item.xpos, item.ypos, randz(item.mindam, item.maxdam),
                         item.range);

                    sf::Packet pack;
                    pack << ident.tilesUpdate;

                    for (int itLength = 0; itLength != 5; itLength++)
                    {



                        for (int rot = 1; rot != 361; rot++)
                        {
                            //int Rot = GX;
                            int itemX = item.xpos / 20, itemY = item.ypos / 20;
                            int XPos = ((itemX) +
                                        cosf(rot * PI / 180) * itLength);
                            int YPos = ((itemY) +
                                        sinf(rot * PI / 180) * itLength);
                            tiles[XPos][YPos][30].stone();

                            pack << gvars::currentregionx << gvars::currentregiony;
                            pack << XPos << YPos << 30;
                            pack << tiles[XPos][YPos][30].id;

                        }
                    }
                    if(network::connectedServer != "")
                    {
                        cliSocket.send(pack);
                    }

                    con("Boom!");
                }
            }
            item.age = item.age + 1;
            if (gvars::debug)
            {
                std::cout << "Post Item \n";
            }
        }
        catch (std::exception &e)
        {
            std::cout << "Problem with an Item in Update Item \n";
        }
    }
}

std::vector<int> nngTrace(int xa, int ya, int xb, int yb, int id,
                          std::vector<int> /*exceptions*/)
{ //.at(0) = Item/NPC(23/69) .at(1) = id
    int dx = xb - xa, dy = yb - ya, steps;
    float xIncrement, yIncrement, x = xa, y = ya;
    if (abs(dx) > abs(dy))
        steps = abs(dx);
    else
        steps = abs(dy);
    xIncrement = dx / (float)steps;
    yIncrement = dy / (float)steps;
    std::vector<int> vectorID;
    for (int k = 0; k < steps; k++)
    {
        x += xIncrement;
        y += yIncrement;
        if (inputState.key[Key::G])
        {
            effects.createCircle(x, y, 1, sf::Color::Blue);
        }
        int count = 0;

        { // Merely doing this so I can reuse the same code, but for items, Hehe.
            std::vector<Npc>::iterator me;
            sf::Lock lock(mutex::npcList);
            //for (me = npclist.begin(); me != npclist.end(); ++me)
            for (auto &me : npclist)
            {
                if (math::closeish(x, y, me.xpos, me.ypos) <= me.reach &&
                    me.id != id)
                {

                    std::vector<int>::iterator vec;
                    try
                    {
                        bool exists = false;
                        for (vec = vectorID.begin(); vec != vectorID.end();
                             ++vec)
                        {
                            if (*vec == me.id)
                            {
                                exists = true;
                            }
                        }
                        if (exists == false)
                        {
                            vectorID.push_back(69);
                            vectorID.push_back(getNpcVectorId(me.id));
                        }
                    }
                    catch (std::exception &e)
                    {
                        std::cout << "VectError\n";
                    }

                } //23 = Item, 69 = NPC
                count++;
            }
        }
        std::list<Item>::iterator me;

        for (me = worlditems.begin(); me != worlditems.end(); ++me)
        {
            if (math::closeish(x, y, me->xpos, me->ypos) <= 10 && me->id != id)
            {
                //Making sure not to constantly add the same
                //try{if( VectorID.at(Count-1) != Me->id){ VectorID.push_back(23); VectorID.push_back( GetItemVectorId(Me->id)); }} catch (std::exception& e){}

            } //23 = Item, 69 = NPC
            count++;
        }

        if (inputState.key[Key::Period])
        {
            effects.createCircle(x, y, 1, sf::Color::White);
        }
    }
    if (vectorID.size() == 0)
    {
        vectorID.push_back(-1);
    }
    return vectorID;
}

bool gridposTrace(int xa, int ya, int xb, int yb, int id, sf::Vector2f target)
{ // Looking in a straight line for a specific spot, Walls block vision.

    int dx = xb - xa, dy = yb - ya, steps;
    float xIncrement, yIncrement, x = xa, y = ya;
    if (abs(dx) > abs(dy))
        steps = abs(dx);
    else
        steps = abs(dy);
    xIncrement = dx / (float)steps;
    yIncrement = dy / (float)steps;

    for (int k = 0; k < steps; k++)
    {
        x += xIncrement;
        y += yIncrement;
        if (tiles[abs_to_index(x / GRID_SIZE)][abs_to_index(y / GRID_SIZE)][30]
                .id == 1010)
        {
            if (inputState.key[Key::Period] && id == gvars::myTargetid)
            {
                effects.createLine(x, y, xa, ya, 1, sf::Color::Blue);
            }
            //std::cout << "Shoulda Broke. " << std::endl;
            break;

        } // Stops the trace if it hits a wall.
        //std::cout << "Stuffs " << std::endl;
        sf::Vector2f pos(abs(x / GRID_SIZE), abs(y / GRID_SIZE));
        sf::Vector2f tar(abs(target.x / GRID_SIZE), abs(target.y / GRID_SIZE));
        if (pos == tar)
        {
            return true;
        } // Returns true and stops searching.
        if (inputState.key[Key::Period] && id == gvars::myTargetid)
        {
            effects.createLine(x, y, xa, ya, 1, sf::Color::Blue);
        }
    }
    return false; // Returns false if the target was never found.
}

std::vector<int> npcTrace(int xa, int ya, int xb, int yb, int id,
                          std::vector<int> /*exceptions = std::vector<int>()*/)
{

    int dx = xb - xa, dy = yb - ya, steps;
    float xIncrement, yIncrement, x = xa, y = ya;
    if (abs(dx) > abs(dy))
        steps = abs(dx);
    else
        steps = abs(dy);
    xIncrement = dx / (float)steps;
    yIncrement = dy / (float)steps;
    std::vector<int> vectorID;

    for (int k = 0; k < steps; k++)
    {
        x += xIncrement;
        y += yIncrement;
        bool kill = false;
        std::vector<Npc>::iterator me;
        int count = 0;
        sf::Lock lock(mutex::npcList);
        //for (me = npclist.begin(); me != npclist.end(); ++me)
        for (auto &me : npclist)
        {
            if (math::closeish(x, y, me.xpos, me.ypos) <= me.reach &&
                me.id != id)
            {
                vectorID.push_back(count);
                kill = true;
            }
            count++;
        }
        if (kill)
            return vectorID;

        if (inputState.key[Key::H])
        {
            effects.createCircle(x, y, 1, sf::Color::White);
        }
    }
    if (vectorID.size() == 0)
    {
        vectorID.push_back(-1);
    }

    throw std::runtime_error("npcTrace: couldn't return a value");
}

int getItemVectorId(int id)
{
    int index = 0;
    for (auto const &item : worlditems)
    {
        if (item.id == id)
        {
            return index;
        }
        index++;
    }
    return -1;
}

int getNpcVectorId(int id)
{
    sf::Lock lock(mutex::npcList);
    int index = 0;
    for (auto const &npc : npclist)
    {
        if (npc.id == id)
        {
            return index;
        }
        index++;
    }
    return -1;
}

struct ItemFindResult
{
    bool found;
    int xpos;
    int ypos;
    int id;
    int vectorPosition;
};

ItemFindResult findClosestItem(int orix, int oriy, std::string tarItem,
                               int /*Gxpos*/ = 0, int /*Gypos*/ = 0,
                               int /*Rxpos*/ = 0, int /*Rypos*/ = 0)
{
    // This will be difficult, go through all the items, and find the closest one.
    int closx =
        -1000000; // Make sure the default starting number is far beyond being the closest one.
    int closy = -1000000;
    int closid = 0;
    int closVect = 0;
    int vectPos = 0;
    bool first = true;
    for (auto const &item : worlditems)
    {
        if (first == true)
        {
            if (item.name == tarItem)
            {
                closx = item.xpos;
                closy = item.ypos;
                closid = item.id;
                closVect = vectPos;
                first = false;
            }
        }
        else
        {
            if (item.name == tarItem)
            {
                int one = math::closeish(orix, oriy, item.xpos, item.ypos);
                int two = math::closeish(orix, oriy, closx, closy);
                if (one < two)
                {
                    closx = item.xpos;
                    closy = item.ypos;
                    closid = item.id;
                    closVect = vectPos;
                }
            }
        }
        vectPos++;
    }
    if (first != true)
    {
        return {true, closx, closy, closid, closVect};
    }
    return {false, 0, 0, 0, 0};
}

std::set<int> npcList(int exceptions = -1)
{
    if (gvars::debug)
    {
        std::cout << "Pre npcList \n";
    }
    std::set<int> returns;

    sf::Lock lock(mutex::npcList);
    for (auto const &npc : npclist)
    {
        if (gvars::debug)
        {
            std::cout << "For NpcList \n";
        }
        if (npc.id != exceptions)
        {
            if (gvars::debug)
            {
                std::cout << "Post exception NpcList \n";
            }
            returns.insert(getNpcVectorId(npc.id));
            if (gvars::debug)
            {
                std::cout << "Post Returns NpcList \n";
            }
        }
    }
    if (gvars::debug)
    {
        std::cout << "Post For NpcList \n";
    }
    if (!returns.empty())
    {
        return returns;
    }
    throw std::runtime_error("NpcList: Couldn't return anything!");
}

struct entityvectorpointercontainer
{
    std::set<Item*> items;
    std::set<Npc*> npcs;
    std::set<Tile*> tiles;
};

entityvectorpointercontainer entityTrace(Vec3 Ori, Vec3 Tar) /* TODO: Improve this to use Z axis as well, or find new formula that can. THERE ARE NONE, GRAAAAH*/
{ // Looking in a straight line for a specific spot, Walls block vision.

    entityvectorpointercontainer EVPC;

    float dx = Tar.x - Ori.x, dy = Tar.y - Ori.y, steps;
    float xIncrement, yIncrement, x = Ori.x, y = Ori.y;
    if (abs_to_index(dx) > abs_to_index(dy))
        steps = abs_to_index(dx);
    else
        steps = abs_to_index(dy);
    xIncrement = dx / (float)steps;
    yIncrement = dy / (float)steps;

    sf::Lock lock(mutex::npcList);
    for (int k = 0; k < steps; k++)
    {
        x += xIncrement;
        y += yIncrement;
        if (tiles[abs_to_index(x / GRID_SIZE)][abs_to_index(y / GRID_SIZE)][abs_to_index(Ori.z/20)]
                .transparent == false)
        {

            if (inputState.key[Key::Quote])
            {
                effects.createLine(x, y, Ori.x, Ori.y, 1, sf::Color::Cyan);
            }
            break;

        } // Stops the trace if it hits a wall.

        for (auto &items : worlditems)
        {
            if (items.xpos == x && items.ypos == y && items.zpos == Ori.z)
                EVPC.items.insert(&items);
        }
        for (auto &npcs : npclist)
        {
            //if(npcs.xpos == x && npcs.ypos == y && npcs.zpos == Ori.z)
            if (math::closeish(npcs.xpos,npcs.ypos,x,y) <= npcs.size && npcs.zpos == Ori.z)
                EVPC.npcs.insert(&npcs);
        }
        EVPC.tiles.insert(&tiles[abs_to_index(x / GRID_SIZE)][abs_to_index(y / GRID_SIZE)][abs_to_index(Ori.z / GRID_SIZE)]);



        if (inputState.key[Key::Quote])
        {
            effects.createLine(x, y, Ori.x, Ori.y, 1, sf::Color::Blue);
        }
        sf::Vector2f pos(abs_to_index(x / GRID_SIZE),
                         abs_to_index(y / GRID_SIZE));
        sf::Vector2f tar(abs_to_index(Tar.x / GRID_SIZE),
                         abs_to_index(Tar.y / GRID_SIZE));
    }
    return EVPC;
}



void critterEquip(Npc &npc, std::list<Npc> &container)
{
    /*
    itemPtrVector iPV;
    npcPtrVector nPV;

    for (auto &part : npc.graspers)
    {
        if(part.graspItem != nullptr)
            iPV.ptrs.push_back(part.graspItem);
        if(part.graspNpc != nullptr)
            nPV.ptrs.push_back(part.graspNpc);
    }
    if(iPV.size() < 2)
    {
        for (auto &part : npc.graspers)
        {
            if(part.graspItem == nullptr)
            {

            }
        }
    }
    */


    for (int i = 0; i != 20; i++)
    {
        if(npc.invSlots[i] == nullptr)
        {
            for (auto &item : npc.inventory)
            {
                //std::cout << item.name << ", " << item.slotted << ": ";
                if(item.slotted == false)
                {
                    npc.invSlots[i] = &item;
                    npc.invSlots[i]->slotted = true;
                    break;
                    //std::cout << npc.invSlots[i]->name << npc.invSlots[i]->slotted;
                }
                //std::cout << std::endl;
            }
        }

    }
    if(inputState.key[Key::I].time == 1)
    {
        std::cout << npc.name << "'s inventory; \n";
        for (int i = 0; i != 20; i++)
        {
            //std::cout << i << ", " << (npc.invSlots[i] != nullptr) << std::endl;
            if(npc.invSlots[i] != nullptr)
            {
                //std::cout << "...que? \n";
                std::cout << i << ": " << npc.invSlots[i]->name << std::endl;
            }

        }
    }


    if(npc.graspItemLeft == nullptr && npc.graspNpcLeft == nullptr)
    {
        Item * weapon = getItemType(npc.inventory,2);

        if(weapon == nullptr)
            weapon = getItemType(npc.inventory,1);

        if(weapon == nullptr)
            return;

        if(weapon->size >= 2)
        {
            if(npc.graspItemRight == nullptr && npc.graspNpcRight == nullptr)
            {
                npc.graspItemLeft = weapon;
                npc.graspItemRight = weapon;
            }
        }
        else if(weapon->size <= 1)
        {
            if(npc.graspItemRight == nullptr)
                npc.graspItemLeft = weapon;
            else if(npc.graspItemRight != nullptr && npc.graspItemRight->id != weapon->id)
                npc.graspItemLeft = weapon;

        }
    }
    if(npc.graspItemRight == nullptr && npc.graspNpcRight == nullptr)
    {
        Item * weapon = nullptr;
        for (auto &item : npc.inventory)
        {
            if( (item.type == 1 || item.type == 2) && item.size <= 1 )
            {
                if(npc.graspItemLeft == nullptr || npc.graspItemLeft->id != item.id)
                    npc.graspItemRight = &item;
            }
        }
    }



    /*
    std::cout << npc.name << npc.id << "\n";
    if(npc.graspItemLeft != nullptr)
        std::cout << "<Left Hand: " << npc.graspItemLeft->name << npc.graspItemLeft->id << std::endl;
    if(npc.graspItemRight != nullptr)
        std::cout << ">Right Hand: " << npc.graspItemRight->name << npc.graspItemRight->id  << std::endl;
    */


}

void critterBrain(Npc &npc, std::list<Npc> &container)
{

    if(!npc.alive)
        return;

    npc.container = &container;

    if(tiles[abs_to_index(npc.xpos/GRID_SIZE)][abs_to_index(npc.ypos/GRID_SIZE)][abs_to_index(npc.zpos/GRID_SIZE)].walkable == false)
    {
        //sf::Vector2f position(abs_to_index(npc.xpos/GRID_SIZE)*GRID_SIZE+10.5,abs_to_index(npc.ypos/GRID_SIZE)*GRID_SIZE+10.5);
        sf::Vector2f position;
        position = gridEject(sf::Vector2f(npc.xpos,npc.ypos));

        sf::Vector2f npcPos(npc.xpos,npc.ypos);

        sf::Vector2f Alter = position - npcPos;
        //npc.momentum += Alter;
        npc.xpos += Alter.x;
        npc.ypos += Alter.y;
    }


    int alph = 255;
    //npc.img.setColor(sf::Color(255, 255, 255, alph));
    npc.img.setScale(gvars::scalex, gvars::scaley);
    npc.img.setOrigin(npc.img.getTextureRect().width / 2,
    npc.img.getTextureRect().height / 2);
    if (npc.race == "Human")
    {
        npc.img.setScale(0.5, 0.5);
    }


    if(npc.name == "BTHalfCelestial")
    {
        if(randz(1,1000) == 1000)
        {
            soundmanager.playSound("AngryWallabee.ogg");
        }
    }


    npc.img.setRotation(npc.angle);

    int critterZ = npc.zpos/20;
    textList.createText(npc.xpos,npc.ypos,10,sf::Color::White,"ZPos:","",npc.zpos," /","",critterZ);
    runCritterBody(npc);
    debug("Ending Part Loop");
    debug("debug 1", false);

    /*Simulating Hunger/Thirst, Needs to be nerfed/formulated to conditions, I.E. Attributes/Parts/Weather*/
    if( (gvars::framesPassed % 10) == 0)
    {/* Every ten frames, consume nutrients  */
        npc.bloodwork("Nutrients", -1);
        npc.bloodwork("Hydration", -1);
    }


    /* *BodyPart Loop* */
    /* Critter Vision   */
    const sf::Vector2f npcPos(npc.xpos, npc.ypos);
    //sf::Vector2f targetPos(gvars::mousePos);

    /* Running through the critters pointers to see which one is valid, Then setting it to aim at it. */
    sf::Vector2f * targetPos = &npc.desiredViewAngle;
    if (npc.targetInfo.item != nullptr)
        *targetPos = sf::Vector2f(npc.targetInfo.item->xpos,npc.targetInfo.item->ypos);
    else if (npc.targetInfo.npc != nullptr)
        *targetPos = sf::Vector2f(npc.targetInfo.npc->xpos,npc.targetInfo.npc->ypos);
    else if (npc.targetInfo.tile != nullptr)
    {
        Vec3 tilePos(npc.targetInfo.tile->getPos());
        *targetPos = sf::Vector2f(tilePos.x,tilePos.y);
    }
    else if (randz(1,100) == 100)
        /* No target? Occasionally choose a nearby direction to look at. */
        *targetPos = sf::Vector2f(npc.xpos+randz(-15,15),npc.ypos+randz(-15,15));
    debug("debug 2", false);
    /*  Do the angle math to figure out which direction we need to turn */
    const float npcAngle = math::constrainAngle(npc.angle);
    const float targetAngle =
        math::constrainAngle(math::angleBetweenVectors(npcPos, *targetPos) - 90);
    /*  Turn towards it's target!    */
    if (math::angleDiff(npcAngle, targetAngle) > npc.turnSpeed)
        npc.angle += npc.turnSpeed;
    else if (math::angleDiff(npcAngle, targetAngle) < -npc.turnSpeed)
        npc.angle -= npc.turnSpeed;
    else
        npc.angle = targetAngle;

    const int endAngle = -(npc.angle - (npc.viewangle / 2));
    const int startAngle = -(npc.angle - (-npc.viewangle / 2));
    debug("debug 3", false);
    /*Drawing Vision*/
    sf::ConvexShape shape;
    int pointCounter = 1;
    shape.setPointCount(pointCounter);
    shape.setPoint(0, sf::Vector2f(0, 0));

    for (int rot = startAngle; rot != endAngle; rot++)
    {
        float xPos = npc.xpos + sinf(rot * PI / 180) * npc.viewrange;
        float yPos = npc.ypos + cosf(rot * PI / 180) * npc.viewrange;

        entityvectorpointercontainer EVPC;

        //EVPC = entityTrace(Vec3(npcPos.x,npcPos.y,npc.zpos), Vec3(xPos, yPos, npc.zpos));

        //std::cout << npc.name << "'s EVPC: " << EVPC.items.size() << "/" << EVPC.npcs.size() << "/" << EVPC.tiles.size() << std::endl;

        for (auto &i : EVPC.items)
        {
            effects.createCircle(i->xpos,i->ypos,5,sf::Color::Black);
        }
        for (auto &i : EVPC.tiles)
        {
            Vec3 Pos = i->getPos();
            effects.createCircle(Pos.x*20,Pos.y*20,5,sf::Color::Black);
        }


        if (rot == startAngle)
        {
            pointCounter++;
            shape.setPointCount(pointCounter);
            shape.setPoint(pointCounter - 1,
                           sf::Vector2f(xPos - npc.xpos, yPos - npc.ypos));
        }

        int CenterPoint = (startAngle + endAngle) / 2;
        if (rot == CenterPoint)
        {
            pointCounter++;
            shape.setPointCount(pointCounter);
            shape.setPoint(pointCounter - 1,
                           sf::Vector2f(xPos - npc.xpos, yPos - npc.ypos));
            bool failedAlign = true;
            if (npc.angle == targetAngle)
            {
                if (gridTrace(npcPos, *targetPos))
                {
                    /*effects.createLine(npc.xpos, npc.ypos, xPos, yPos, 1,
                                       sf::Color::Cyan);  */
                    failedAlign = false;
                }
            }
          /*  if (failedAlign)
                effects.createLine(npc.xpos, npc.ypos, xPos, yPos, 1,
                                   sf::Color::Red);  */
        }
        if ((rot % 10) == 0 && rot != 0)
        {
            pointCounter++;
            shape.setPointCount(pointCounter);
            shape.setPoint(pointCounter - 1,
                           sf::Vector2f(xPos - npc.xpos, yPos - npc.ypos));
        }
        if (rot == endAngle - 1)
        {
            pointCounter++;
            shape.setPointCount(pointCounter);
            shape.setPoint(pointCounter - 1,
                           sf::Vector2f(xPos - npc.xpos, yPos - npc.ypos));
        }
    }

    debug("debug 4", false);
    if(myTargetPtr != nullptr && myTargetPtr->id == npc.id)
    {
        sf::Color filling(sf::Color::Yellow);
        filling.a = (50);
        shape.setFillColor(filling);
        shape.setOutlineColor(sf::Color::Yellow);
        shape.setOutlineThickness(1);
        shape.setPosition(npcPos);
        effects.polygons.push_back(shape);
    }
    debug("debug 5", false);
    /* Critter Prioritization */

    std::vector<Npc*> enemyPtrs;
    Npc * closEnmy = nullptr;
    if(bountytower::towerlingassault)
    {
        for (auto &enemys : container)
        {
            if( enemys.factionPtr != nullptr && npc.factionPtr != nullptr && enemys.faction != npc.faction)
            {
                for (auto &i : npc.factionPtr->factRelations)
                {
                    if(enemys.faction == i.faction && i.appeal < 1000 && enemys.alive)
                    {
                        //std::cout << "ZE ENEMY HAS BEEN SPOTTED AT " << enemys.xpos << "/" << enemys.ypos << std::endl;
                        enemyPtrs.push_back(&enemys);
                    }
                }
            }
        }
        for (auto &enemy : enemyPtrs)
        {
            //effects.createLine(npc.xpos,npc.ypos,enemy->xpos,enemy->ypos,2,sf::Color::Yellow);
            if(closEnmy == nullptr)
                closEnmy = enemy;
            else if(math::closeish(npc.xpos,npc.ypos,enemy->xpos,enemy->ypos) <
                    math::closeish(npc.xpos,npc.ypos,closEnmy->xpos,closEnmy->ypos)
                    )
            {
                closEnmy = enemy;
            }

        }

        if(closEnmy != nullptr && inputState.key[Key::LAlt] && true == false)
        {
            effects.createLine(npc.xpos,npc.ypos,closEnmy->xpos,closEnmy->ypos,4,sf::Color::Red);
            //hasPath = true;
            //endPos = Vec3(closEnmy->xpos/GRID_SIZE,closEnmy->ypos/GRID_SIZE,closEnmy->zpos/GRID_SIZE);
        }

    }

    critterEquip(npc,container);

    // Method Two, Struct Desires
    struct Desire
    {
        std::string type;
        float potency;
    };
    std::vector<Desire> desires;

    debug("Declaring and added Desires");
    // Declaring and adding Desires
    Desire newDesire;
    { //Sustainence
        newDesire.type = "Sustainence";
        newDesire.potency = 0;
    }
    desires.push_back(newDesire);
    { //Apathy
        newDesire.type = "Apathy";
        newDesire.potency = 100;
    }
    desires.push_back(newDesire);
    { //SelfDefense
        newDesire.type = "SelfDefense";
        newDesire.potency = 0;
    }
    desires.push_back(newDesire);
    { //Social
        newDesire.type = "Social";
        newDesire.potency = 0;
    }
    desires.push_back(newDesire);
    { //Work
        newDesire.type = "Work";
        newDesire.potency = 0;

        if(npc.factionPtr != nullptr && npc.factionPtr->jobList.size() != 0)
        {
            for (auto &jobs : npc.factionPtr->jobList)
            {
                if(jobs.pWorker == nullptr)
                {
                    newDesire.potency = 50+500;
                    break;
                }
            }
            if(npc.jobPtr != nullptr)
                newDesire.potency += 750;
        }

    }
    desires.push_back(newDesire);
    { //Assault
        newDesire.type = "Assault";
        newDesire.potency = 0;
        if(bountytower::towerlingassault && npc.faction == "Towerlings")
            newDesire.potency = 700;



    }
    desires.push_back(newDesire);
    debug("debug 6", false);
    /*Causation to Desires*/
    // Get Critters max nutrition, then reduce it by critters nutrients in blood
    float nutrients = npc.maxhunger - npc.bloodwork("Nutrients", 0);
    float hydration = npc.maxthirst - npc.bloodwork("Hydration", 0);

    for (auto &des : desires)
    {
        if (npc.name != "Mini Turret" && des.type == "Sustainence" && GRID_SIZE == 20)
            des.potency += hydration + nutrients;
        if (des.type == "SelfDefense")
        {
            // This line makes the game freeze
            des.potency = 10000;
        }
    }

    debug("debug 7", false);
    // Finding the highest Desire
    bool inComplete;
    Desire *highestDesire = nullptr;
    bool firstIter = true;

ReDesire:
    inComplete = false;
    debug("Finding highest desire");
    for (auto &i : desires)
    {
        if (firstIter)
        {
            highestDesire = &i;
            firstIter = false;
        }
        if (i.potency > (*highestDesire).potency)
            highestDesire = &i;
    }

    if (highestDesire == nullptr)
    {
        throw std::runtime_error("critterBrain: Something went wrong!");
    }

    debug("debug 8", false);
    // Acting on Highest Desire

    Vec3 startPos(npc.xpos/GRID_SIZE,npc.ypos/GRID_SIZE,npc.zpos/GRID_SIZE);
    Vec3 endPos;

    bool hasPath = false;

    debug("Acting on highest Desire:" + (*highestDesire).type);
    if ((*highestDesire).type == "Apathy")
    {// TODO: Add blublublub

    }
    if ((*highestDesire).type == "SelfDefense")
    {
        (*highestDesire).potency = 0;
        inComplete = true;
    }
    if ((*highestDesire).type == "Sustainence")
    {
        bool inInv = false;
        for (auto &inv : npc.inventory)
            if (inv.massFlesh > 0)
            {
                //npc.Target.item = &Inv;
                inInv = true;
                break;
            }
        if (inInv == false)
        {
            for (auto &lclItms : worlditems)
            {
                if (npc.targetInfo.item == nullptr && lclItms.massFlesh > 0 &&
                    lclItms.user == nullptr)
                {
                    npc.targetInfo.item =
                        &lclItms; // Is this creating a copy? The Behavior in testing still makes multiple critters target the same item.
                    lclItms.user = &npc;
                }
                else if (lclItms.massFlesh > 0 && lclItms.user == nullptr)
                {

                    float CurrentItem = math::closeish(
                        npc.xpos, npc.ypos, (*npc.targetInfo.item).xpos,
                        (*npc.targetInfo.item).ypos);
                    float NewItem = math::closeish(npc.xpos, npc.ypos,
                                                   lclItms.xpos, lclItms.ypos);
                    if (NewItem < CurrentItem)
                    {
                        (*npc.targetInfo.item).user = nullptr;
                        npc.targetInfo.item = &lclItms;
                        (*npc.targetInfo.item).user = &npc;
                    }
                }
            }
            if (npc.targetInfo.item != nullptr)
            {
                Vec3 ItemPos((*npc.targetInfo.item).xpos,(*npc.targetInfo.item).ypos,(*npc.targetInfo.item).zpos);
                Vec3 myPos(npc.xpos,npc.ypos,npc.zpos);
                endPos = Vec3(npc.targetInfo.item->xpos/GRID_SIZE, npc.targetInfo.item->ypos/GRID_SIZE, npc.targetInfo.item->zpos/GRID_SIZE);
                hasPath = true;

                //if (math::closeish(npc.xpos, npc.ypos, ItemPos.x, ItemPos.y) <= npc.size * 3)
                if(math::distance(myPos,ItemPos) <= npc.size*3 && myPos.z/GRID_SIZE == ItemPos.z/GRID_SIZE)
                {
                    Item *tar = npc.targetInfo.item;
                    npc.targetInfo.item = nullptr;
                    (*tar).user = nullptr;
                    npc.inventory.push_back((*tar));
                    (*tar).toDelete = true;
                }
            }
        }

        if (npc.targetInfo.item != nullptr && inInv == false)
        {
        }
    }
    if ((*highestDesire).type == "Work")
    {
        if(npc.jobPtr == nullptr)
        {
            for(auto &jobs : npc.factionPtr->jobList)
            {
                if(jobs.pWorker != nullptr)
                {
                    continue;
                }
                else if(jobs.toDelete)
                {
                    continue;
                }
                std::cout << npc.name << ", workin " << jobs.name << "/" << jobs.type << std::endl;
                jobs.pWorker = &npc;
                npc.jobPtr = &jobs;
                break;
            }
        }
        else
        {
            debug("I have a job, Now if only my programmer did.");
            Vec3 wPos(npc.jobPtr->workPos);
            Vec3 myPos(npc.xpos,npc.ypos,npc.zpos);
            if(npc.jobPtr != nullptr && npc.jobPtr->type == "Build")
            {
                debug("I have build job.");
                std::string desiredItem = "Wood";
                debug("Checking for material");
                Item * material = npc.hasItem(desiredItem);

                debug("Material checked");
                if(material == nullptr)
                {
                    for (auto &item : worlditems)
                    {
                        if(item.name == desiredItem)
                        {
                            if(npc.targetInfo.item == nullptr && item.user == nullptr)
                            {
                                npc.targetInfo.item = &item;
                                item.user = &npc;
                            }

                            else if(npc.targetInfo.item != nullptr)
                            {
                                sf::Vector2f myItemPos(npc.targetInfo.item->xpos,npc.targetInfo.item->ypos);
                                if(math::closeish(npc.xpos,npc.ypos,item.xpos,item.ypos) <= math::closeish(npc.xpos,npc.ypos,myItemPos.x,myItemPos.y)
                                   && item.user == nullptr)
                                {
                                    npc.targetInfo.item->user = nullptr;
                                    npc.targetInfo.item = &item;
                                    item.user = &npc;
                                }

                            }
                        }
                    }
                    Item * item = npc.targetInfo.item;
                    if(item != nullptr)
                    {
                        endPos = Vec3(npc.targetInfo.item->xpos/GRID_SIZE, npc.targetInfo.item->ypos/GRID_SIZE, npc.targetInfo.item->zpos/GRID_SIZE);
                        hasPath = true;
                        if(math::closeish(npc.xpos,npc.ypos,item->xpos,item->ypos) <= npc.size*2)
                        {
                            item->user = nullptr;
                            npc.inventory.push_back(*item);
                            item->toDelete = true;
                            npc.targetInfo.item = nullptr;
                        }
                    }

                }
                debug("Checking if any desiredItem exists");
                if(material == nullptr && npc.targetInfo.item == nullptr)
                {
                    debug("It doesn't");
                    inComplete = true;
                    npc.jobPtr->errorReason = "No " + desiredItem + " located. \n";
                    (*highestDesire).potency = 0;
                }
                else
                    npc.jobPtr->errorReason = "";

                if(material != nullptr)
                {
                    endPos = Vec3(npc.jobPtr->workPos.x/GRID_SIZE,npc.jobPtr->workPos.y/GRID_SIZE, npc.jobPtr->workPos.z/GRID_SIZE);
                    hasPath = true;
                }

                if(math::distance(myPos,wPos) <= npc.size*3 && myPos.z/GRID_SIZE == wPos.z/GRID_SIZE && material != nullptr)
                {
                    debug("Close to workPos and has material.");
                    endPos = Vec3(myPos.x/GRID_SIZE,myPos.y/GRID_SIZE,myPos.z/GRID_SIZE);
                    hasPath = false;

                    npc.jobPtr->completionProgress += npc.skills.intelligence / 2;

                    for (float rot = 1; rot < 361 * (percentIs( npc.jobPtr->completionTimer,npc.jobPtr->completionProgress) / 100); rot++)
                    {

                        float xPos = wPos.x + sin(rot * PI / 180) * 10;
                        float yPos = wPos.y + cos(rot * PI / 180) * 10;

                        effects.createLine( wPos.x, wPos.y, xPos, yPos, 1, sf::Color(150, 150, 150, 150));
                    }

                    if (npc.jobPtr->completionProgress >= npc.jobPtr->completionTimer)
                    {
                        tiles[abs_to_index(wPos.x / GRID_SIZE)][abs_to_index(wPos.y / GRID_SIZE)][abs_to_index(wPos.z / GRID_SIZE)].wall();
                        material->toDelete = true;
                        npc.jobPtr->toDelete = true;
                        npc.jobPtr->pWorker->hasJob = false;/* Oops... this amuses me. */
                        npc.jobPtr = nullptr;

                        if(network::connectedServer != "")
                        {
                            sf::Packet pack;
                            pack << ident.tilesUpdate;
                            pack << gvars::currentregionx << gvars::currentregiony;
                            pack << std::abs(wPos.x / GRID_SIZE) << std::abs(wPos.y / GRID_SIZE) << std::abs(wPos.z / GRID_SIZE);
                            pack << tiles[abs_to_index(wPos.x / GRID_SIZE)][abs_to_index(wPos.y / GRID_SIZE)][abs_to_index(wPos.z / GRID_SIZE)].id;
                            cliSocket.send(pack);
                        }

                    }

                }
            }
            else if(npc.jobPtr != nullptr && npc.jobPtr->type == "Chop" && npc.jobPtr->pItem != nullptr)
            {
                Item * itemPtr = npc.jobPtr->pItem;
                endPos = Vec3(abs_to_index(itemPtr->xpos/GRID_SIZE),abs_to_index(itemPtr->ypos/GRID_SIZE),abs_to_index(itemPtr->zpos/GRID_SIZE));
                hasPath = true;

                if(math::closeish(npc.xpos,npc.ypos,endPos.x*GRID_SIZE,endPos.y*GRID_SIZE) <= npc.size*3)
                {

                    npc.jobPtr->completionProgress += npc.skills.intelligence / 2;

                    for (float rot = 1; rot < 361 * (percentIs( npc.jobPtr->completionTimer,npc.jobPtr->completionProgress) / 100); rot++)
                    {

                        float xPos = itemPtr->xpos + sin(rot * PI / 180) * 10;
                        float yPos = itemPtr->ypos + cos(rot * PI / 180) * 10;

                        effects.createLine( itemPtr->xpos, itemPtr->ypos, xPos, yPos, 1, sf::Color(150, 150, 150, 150));
                    }

                    if (npc.jobPtr->completionProgress >= npc.jobPtr->completionTimer)
                    {

                        itemPtr->toDelete = true;
                        for(int Woody = 0; Woody != 5; Woody++)
                            spawnItem("Wood",itemPtr->xpos+randz(-3,3),itemPtr->ypos+randz(-3,3),itemPtr->zpos);
                        npc.jobPtr->toDelete = true;
                        npc.jobPtr = nullptr;
                    }
                }

            }
            else if(npc.jobPtr != nullptr && npc.jobPtr->type == "PickUp" && npc.jobPtr->pItem != nullptr)
            {
                Item * itemPtr = npc.jobPtr->pItem;
                endPos = Vec3(abs_to_index(itemPtr->xpos/GRID_SIZE),abs_to_index(itemPtr->ypos/GRID_SIZE),abs_to_index(itemPtr->zpos/GRID_SIZE));
                hasPath = true;

                //if(math::closeish(npc.xpos,npc.ypos,endPos.x*20,endPos.y*20) <= npc.size*3)
                if(math::distance(myPos,wPos) <= npc.size*3 && myPos.z/GRID_SIZE == wPos.z/GRID_SIZE)
                {
                    itemPtr->user = nullptr;
                    npc.inventory.push_back(*itemPtr);
                    itemPtr->toDelete = true;
                    npc.jobPtr->toDelete = true;
                    npc.jobPtr = nullptr;
                }
            }
            else if(npc.jobPtr != nullptr && npc.jobPtr->type == "Dig")
            {
                endPos = Vec3(npc.jobPtr->workPos.x/GRID_SIZE,npc.jobPtr->workPos.y/GRID_SIZE,npc.jobPtr->workPos.z/GRID_SIZE);
                hasPath = true;

                if(math::distance(myPos,wPos) <= npc.size*3 && myPos.z/GRID_SIZE == wPos.z/GRID_SIZE)
                {
                    debug("Close to workPos");
                    endPos = Vec3(myPos.x/GRID_SIZE,myPos.y/GRID_SIZE,myPos.z/GRID_SIZE);
                    hasPath = false;

                    npc.jobPtr->completionProgress += npc.skills.intelligence / 2;
                    debug("post job completetion progress");

                    for (float rot = 1; rot < 361 * (percentIs( npc.jobPtr->completionTimer,npc.jobPtr->completionProgress) / 100); rot++)
                    {

                        float xPos = wPos.x + sin(rot * PI / 180) * 10;
                        float yPos = wPos.y + cos(rot * PI / 180) * 10;

                        effects.createLine( wPos.x, wPos.y, xPos, yPos, 1, sf::Color(150, 150, 150, 150));
                    }
                    debug("post line drawing");

                    if (npc.jobPtr->completionProgress >= npc.jobPtr->completionTimer)
                    {
                        debug("Job complete!");

                        tiles[abs_to_index(wPos.x / GRID_SIZE)][abs_to_index(wPos.y / GRID_SIZE)][abs_to_index(wPos.z / GRID_SIZE)].stone();
                        for(int Stoney = 0; Stoney != 5; Stoney++)
                            spawnItem("Rock",wPos.x+randz(-3,3),wPos.y+randz(-3,3),wPos.z);

                        npc.jobPtr->toDelete = true;
                        npc.jobPtr = nullptr;
                    }
                }
            }
            else if(npc.jobPtr != nullptr && npc.jobPtr->type == "FlipSwitch")
            {
                endPos = Vec3(npc.jobPtr->workPos.x/GRID_SIZE,npc.jobPtr->workPos.y/GRID_SIZE,npc.jobPtr->workPos.z/GRID_SIZE);
                hasPath = true;

                if(math::distance(myPos,wPos) <= npc.size*3 && myPos.z/GRID_SIZE == wPos.z/GRID_SIZE)
                {
                    debug("Close to workPos");
                    endPos = Vec3(myPos.x/GRID_SIZE,myPos.y/GRID_SIZE,myPos.z/GRID_SIZE);
                    hasPath = false;

                    npc.jobPtr->completionProgress += npc.skills.intelligence / 2;
                    debug("post job completetion progress");
                    int percentage = percentIs( npc.jobPtr->completionTimer,npc.jobPtr->completionProgress);
                    textList.createText(wPos.x-10,wPos.y-20,15,sf::Color::Yellow,"%" + std::to_string(percentage));

                    if (npc.jobPtr->completionProgress >= npc.jobPtr->completionTimer)
                    {
                        debug("Job complete!");

                        tiles[abs_to_index(wPos.x / GRID_SIZE)][abs_to_index(wPos.y / GRID_SIZE)][abs_to_index(wPos.z / GRID_SIZE)].state = "On";

                        npc.jobPtr->toDelete = true;
                        npc.jobPtr = nullptr;
                    }
                }
            }
            else if(npc.jobPtr != nullptr && npc.jobPtr->type == "Move")
            {
                endPos = Vec3(wPos.x/GRID_SIZE,wPos.y/GRID_SIZE,wPos.z/GRID_SIZE);
                hasPath = true;

                if(math::distance(myPos,wPos) <= npc.size*2 && myPos.z/GRID_SIZE == wPos.z/GRID_SIZE
                   && npc.storedPath.empty())
                {

                    npc.dirMove(sf::Vector2f(wPos.x,wPos.y));
                }

                if(math::distance(myPos,wPos) <= npc.moverate && myPos.z/GRID_SIZE == wPos.z/GRID_SIZE)
                {
                    hasPath = false;

                    npc.xpos = wPos.x;
                    npc.ypos = wPos.y;
                    npc.zpos = wPos.z;

                    npc.jobPtr->toDelete = true;
                    npc.jobPtr = nullptr;
                }
            }

        }
    }
    if ((*highestDesire).type == "Assault")
    {
        if(closEnmy != nullptr)
        {
            hasPath = true;
            endPos = Vec3(closEnmy->xpos/GRID_SIZE,closEnmy->ypos/GRID_SIZE,closEnmy->zpos/GRID_SIZE);
        }
        Item * rangewep = npc.getItemType(2);
        Item * meleewep = npc.getItemType(1);
        if(inputState.key[Key::LAlt] && true == false)
        {
            //rangewep->getRange();
            if(rangewep != nullptr)
                effects.createCircle(npc.xpos,npc.ypos,rangewep->getRange(),sf::Color(255,0,0,50),2,sf::Color::Red);
            if(meleewep != nullptr)
                effects.createCircle(npc.xpos,npc.ypos,meleewep->getRange(),sf::Color(0,0,255,50),2,sf::Color::Blue);
        }
        bool withinRange = false;
        if(rangewep != nullptr)
        {
            withinRange = (math::closeish(npc.xpos,npc.ypos,closEnmy->xpos,closEnmy->ypos) <= rangewep->getRange());
            if(getItemType(rangewep->internalitems,3) == nullptr)
                withinRange = false;
            if(withinRange)
            {
                rangewep->user = &npc;

                if(rangewep->trigger())
                {
                    std::string Status = rangewep->activate(Vec3f(closEnmy->xpos,closEnmy->ypos,closEnmy->zpos));
                    AnyDeletes(rangewep->internalitems);
                    if(Status != "Success")
                        chatBox.addChat(npc.name + ", cannot fire " + rangewep->name + " due to :" + Status, sf::Color::Yellow);
                }
            }
        }

        if(meleewep != nullptr)
        {
            if(!withinRange)
                withinRange = (math::closeish(npc.xpos,npc.ypos,closEnmy->xpos,closEnmy->ypos) <= meleewep->range);
            if(withinRange)
            {
                meleewep->user = &npc;
                if(meleewep->trigger())
                {
                    std::string Status = meleewep->activate(Vec3f(closEnmy->xpos,closEnmy->ypos,closEnmy->zpos));
                    AnyDeletes(meleewep->internalitems);
                    if(Status != "Success")
                        chatBox.addChat(npc.name + ", cannot strike with " + meleewep->name + " due to :" + Status, sf::Color::Yellow);
                }
            }
        }
        if(withinRange)
        {
            hasPath = false;
        }
            //std::cout << rangewep->name << ",'s range: " << rangewep->range << std::endl;
    }

    debug("Checking inComplete:" + std::to_string(inComplete));
    // Incase the highest desire isn't completable, Go through again for the next highest desire.

    if (inComplete)
        goto ReDesire;


    textList.createText(npc.xpos - 30, npc.ypos - 15, 10, sf::Color::Red,
                        (*highestDesire).type, ":", (*highestDesire).potency);


    /* End of Critter Prioritization */
    npc.momMove();
    debug(npc.name + ", mhmm.", false);
    debug("debug 9", false);
    debug("(" + std::to_string(npc.xpos) + "/" + std::to_string(npc.ypos) + "/" + std::to_string(npc.zpos) + ")");
    bool npcWalkable = tiles[abs_to_index(npc.xpos/GRID_SIZE)][abs_to_index(npc.ypos/GRID_SIZE)][abs_to_index(npc.zpos/GRID_SIZE)].walkable;
    debug("pro debug 1", false);

    if(hasPath && (gvars::framesPassed % 5) == 0 && npcWalkable)
    //if(hasPath && npcWalkable)
    {
        debug("hasPath");
        bool prevWalkable = tiles[endPos.x][endPos.y][endPos.z].walkable;
        tiles[endPos.x][endPos.y][endPos.z].walkable = true;
        debug("hasPath");
        int result = pathCon.makePath(startPos, endPos);
        //std::cout << "path result: " << result << std::endl;
        debug("hasPath");
        tiles[endPos.x][endPos.y][endPos.z].walkable = prevWalkable;
        npc.storedPath.clear();
        for(auto &i : pathCon.storedPath)
            npc.storedPath.push_back(i);
        pathCon.storedPath.clear();
    }
    if(hasPath == false)
        npc.storedPath.clear();
    debug("post hasPath");

    if(!npc.storedPath.empty())
    {
        if(inputState.key[Key::LAlt])
            drawStoredPath(npc.storedPath);
        Vec3 Pos(npc.storedPath[1]->getPos());

        double pathTime = (((npc.storedPath.size()*GRID_SIZE)*1.2)/npc.moverate)/30;

        std::ostringstream out;
        out << std::setprecision(2) << pathTime;

        std::string pathy = "PathTime: ";
        pathy.append(out.str()  );

        Vec3 endPathPos(npc.storedPath[npc.storedPath.size()-1]->getPos());
        textList.createText((endPathPos.x)*GRID_SIZE-GRID_SIZE,(endPathPos.y)*GRID_SIZE-GRID_SIZE,10,sf::Color(255,255,255), pathy );

        npc.dirMove(sf::Vector2f(Pos.x*GRID_SIZE+(GRID_SIZE/2),Pos.y*GRID_SIZE+(GRID_SIZE/2)));

        if(Pos.z != npc.zpos)
            npc.zpos = Pos.z*GRID_SIZE;

        if(npc.storedPath.size() >= 2)
        {
            if(npc.storedPath[0]->teleporter && npc.storedPath[1]->teleporter)
            {
                npc.xpos = npc.storedPath[0]->telePos.x*GRID_SIZE;
                npc.ypos = npc.storedPath[0]->telePos.y*GRID_SIZE;
                npc.zpos = npc.storedPath[0]->telePos.z*GRID_SIZE;
            }
        }
        Vec3 myPos(npc.xpos,npc.ypos,npc.zpos);
        Vec3 posExtended(Pos.x*GRID_SIZE+(GRID_SIZE/2),Pos.y*GRID_SIZE+(GRID_SIZE/2),Pos.z*GRID_SIZE+(GRID_SIZE/2));

        if(math::distance(myPos,posExtended) <= npc.moverate)
            npc.storedPath.erase(npc.storedPath.begin() );
    }
    else if(npc.targetInfo.item != nullptr && pathCon.storedPath.size() == 1 || npc.targetInfo.item != nullptr && pathCon.storedPath.size() == 2)
    {
        debug("dir Moving");
        npc.dirMove(sf::Vector2f((*npc.targetInfo.item).xpos,
                                         (*npc.targetInfo.item).ypos));
    }

    debug("Removing stuffs");
    if(npc.factionPtr != nullptr)
        removeJobs(npc.factionPtr->jobList);
    removeItems(npc.inventory);
    debug("endCritterbrain2");
}

void critterBrain(std::list<Npc> &npcs)
{
    /*
    //Vec3 startPos(35,35,30);
    //Vec3 endPos(46, 46, 30);

    Vec3 startPos(48,44,29);
    Vec3 endPos(44, 44, 29);

    if(inputState.key[Key::K].time > 1)
        endPos = Vec3(50,50,30);
    if(inputState.key[Key::L].time > 1)
        endPos = Vec3(50,50,29);
    //Vec3 endPos(gvars::mousePos.x/20, gvars::mousePos.y/20, gvars::currentz);

    int result = pathCon.makePath(startPos, endPos);
    pathCon.drawStoredPathRainbow();
    pathCon.storedPath.clear();
    pathCon.storedRPath.clear();
    std::cout << result << ", Is the test. \n";

    */


    {
        sf::Lock lock(mutex::npcList);

    for (auto &npc : npcs)
    {
        critterBrain(npc, npcs);
    }

    }

}

void drawNPCs()
{
    for (auto &npc : npclist)
    {
        if (npc.hasSpawned == true)
        {
            int CritterZ = npc.zpos/GRID_SIZE;
            if(aabb(npc.xpos,npc.ypos,gvars::topLeft.x,gvars::topRight.x,gvars::topLeft.y,gvars::bottomRight.y) && CritterZ == gvars::currentz)
            {

                if (npc.name == "Azabul")
            {
                sf::Vector2f ta1 = npc.tentArm1;
                sf::Vector2f ta2 = npc.tentArm2;
                sf::Vector2f te1 = npc.tentEnd1;
                sf::Vector2f te2 = npc.tentEnd2;

                effects.createLine(npc.xpos, npc.ypos, ta1.x + npc.xpos,
                                   ta1.y + npc.ypos, 2, sf::Color(0, 200, 200),
                                   1, sf::Color(0, 255, 255));
                effects.createLine(npc.xpos, npc.ypos, ta2.x + npc.xpos,
                                   ta2.y + npc.ypos, 2, sf::Color(0, 200, 200),
                                   1, sf::Color(0, 255, 255));
                effects.createLine(
                    ta1.x + npc.xpos, ta1.y + npc.ypos,
                    te1.x + ta1.x + npc.xpos, te1.y + ta1.y + npc.ypos, 2,
                    sf::Color(0, 200, 200), 1, sf::Color(0, 255, 255));
                effects.createLine(
                    ta2.x + npc.xpos, ta2.y + npc.ypos,
                    te2.x + ta2.x + npc.xpos, te2.y + ta2.y + npc.ypos, 2,
                    sf::Color(0, 200, 200), 1, sf::Color(0, 255, 255));
                effects.drawEffects();
            }



            npc.drawImg();
            effects.createCircle(npc.xpos, npc.ypos, npc.size,
                                 sf::Color(50, 50, 50, 50));
            }
        }
    }
    debug("Done drawing NPCs");
}

void drawItems()
{
    //App.setActive(true);

    for (auto &worlditem : worlditems)
    {
        int itemZ = worlditem.zpos/20;
        if(aabb(worlditem.xpos,worlditem.ypos,gvars::topLeft.x,gvars::topRight.x,gvars::topLeft.y,gvars::bottomRight.y) && itemZ == gvars::currentz)
        {
            worlditem.img.setColor(sf::Color(255, 255, 255, 255));
            worlditem.img.setScale(gvars::scalex, gvars::scaley);
            worlditem.drawImg();
        }
    }
    debug("Done Drawing Items");
}

void displayChat(sf::Vector2f position)
{
    if (gCtrl.phase != "MainMenu")
        effects.createSquare(
            position.x - 10, position.y + 10, position.x + 500,
            (position.y - ((chatBox.chatStorage.size() + 1) * 10)),
            sf::Color(0, 0, 0, 100), 2, sf::Color::Cyan);

    for (size_t i = 0; i != chatBox.chatStorage.size(); i++)
    {
        textList.createText(
            position.x,
            ((position.y - ((chatBox.chatStorage.size()) * 10))-10) + (i * 10), 10,
            chatBox.chatStorage[i].color, chatBox.chatStorage[i].line);
    }
    if(network::chatting)
    {
        if(gvars::secondSwitch)
            textList.createText(gvars::bottomLeft.x+10,gvars::bottomLeft.y-15,10,sf::Color::White,"Chat: " + cliCon.chatString);
        else
            textList.createText(gvars::bottomLeft.x+10,gvars::bottomLeft.y-15,10,sf::Color::White,"Chat: " + cliCon.chatString + "|");
    }

}

void displayMouseItem()
{
    if(mouseItem != nullptr)
    {
        sf::Sprite SP;
        sf::Vector2u TexySize = mouseItem->img.getTexture()->getSize();
        SP.setTexture(*mouseItem->img.getTexture());
        //sf::Vector2f rPos(gvars::topLeft.x + vPos.x, gvars::topLeft.y + vPos.y);
        SP.setPosition(gvars::mousePos);
        SP.setOrigin(TexySize.x/2,TexySize.y/2);
        window.draw(SP);
    }
}

void hoverItemHUD()
{
    window.setView(window.getDefaultView());
    sf::Vector2i pixelPosi = sf::Mouse::getPosition(window);
    sf::Vector2f pixelPos(pixelPosi.x,pixelPosi.y);
    //std::cout << "pixelPos: " << pixelPos.x << ":" << pixelPos.y << std::endl;

    sf::Sprite SP;
    //sf::Vector2u TexySize = myTargetPtr->invSlots[i]->img.getTexture()->getSize();
    SP.setTexture(texturemanager.getTexture("Food.png"));

    SP.setPosition(pixelPos);
    //SP.setOrigin(TexySize.x/2,TexySize.y/2);
    window.draw(SP);

    if(myTargetPtr != nullptr && bountytower::bountytower)
        for (int i = 0; i != 20; i++)
    {

        sf::Vector2f vPos(gvars::slotPos[i]);
        //sf::Vector2f rPos(gvars::topLeft.x + gvars::slotPos[i].x, gvars::topLeft.y + gvars::slotPos[i].y);
        if(math::closeish(pixelPos,vPos) <= 20)
        {
            //std::cout << "Success! on: " << i << std::endl;
            if(myTargetPtr->invSlots[i] != nullptr)
            {

                //window.setView(gvars::view1);
                textList.createText(gvars::mousePos.x,gvars::mousePos.y-15,15,sf::Color::Cyan,myTargetPtr->invSlots[i]->name);
                //window.setView(window.getDefaultView());
                if(inputState.lmb)
                {
                    //std::cout << "Clicked on " << myTargetPtr->invSlots[i]->name << std::endl;
                    gvars::hovering = true;
                    mouseItem = myTargetPtr->invSlots[i];
                }
            }
        }
    }
    window.setView(gvars::view1);
}

void drawStuffs()
{
    //textList.createText(15,15,10,sf::Color::White,"Server Port: " + std::to_string(network::mainPort));
    //textList.createText(15,30,10,sf::Color::White,"Internal Port: " + std::to_string(network::mainPort+23));

    //sf::Vector2f correction = gridEject(gvars::mousePos);
    //effects.createCircle(correction.x,correction.y,5,sf::Color::Red);

    for(int i = 0; i != peers.connected.size(); i++)
    {
        std::string Text = std::to_string(peers.connected[i].ping) + "Peer: " + peers.connected[i].name;
        textList.createText(gvars::topRight.x-150,gvars::topRight.y+(i*10)+10,10,sf::Color::Yellow, Text);
    }


    drawNewTiles();
    {
        sf::Lock lock(mutex::itemList);
        drawItems();
    }

    //processLiquid();
    /*
    for(int x = 0; x != GRIDS; x++)
        for(int y = 0; y != GRIDS; y++)
    {
        if(!tiles[x][y][30].liquids.empty())
        {
            int liqAmt = tiles[x][y][30].liquids[0].amount;
            std::string liqName = tiles[x][y][30].liquids[0].name;
            sf::Color liqClr = tiles[x][y][30].liquids[0].color;
            liqClr.a = 100;

            if(math::closeish(x*20,y*20,gvars::mousePos.x,gvars::mousePos.y) <= 100)
            {
                effects.createSquare(x*20,y*20,(x+1)*20,(y+1)*20,liqClr);
                liqClr.a = 255;
                textList.createText(x*20+10,y*20+10,10,liqClr,std::to_string(liqAmt));
            }
        }
    }
    */

    /*
    if(inputState.key[Key::L].time == 1)
        for(int x = 0; x != GRIDS; x++)
            for(int y = 0; y != GRIDS; y++)
    {
        if(tiles[x][y][30].liquids.empty())
        {
            Liquid liq;
            liq.amount = randz(1,10);
            liq.name = "Water";
            liq.color = sf::Color::Blue;
            tiles[x][y][30].liquids.push_back(liq);
        }
    }
    */

    {
        sf::Lock lock(mutex::npcList);
        drawSelectedCritterHUD();
        drawNPCs();
    }

    //hoverItemHUD();


    drawJobList(window.getView().getCenter().x - 500,
                window.getView().getCenter().y);
    debug("Drew Joblist");

    displayChat(sf::Vector2f(gvars::bottomLeft.x + 5, gvars::bottomLeft.y - 5));
    debug("Drew Chat");

    //purtyOrbitals();

    if(inputState.key[Key::F].time == 1)
    {
        Orb orb;
        orb.pos = gvars::mousePos;
        orb.orbDistance = 50;
        orb.orbSpeed = 3;
        orb.type = 3;
        orb.lifeTime = 510;
        Orbs.push_back(orb);
    }

    {
        for(auto &i : Orbs)
        {
            i.updateOrb();
            i.drawOrb(Orbs.size());
        }
        bool finished = false;
        while(finished == false)
        {
            bool Done = true;
            for(std::vector<Orb>::iterator i = Orbs.begin(); i != Orbs.end(); i++)
            {
                finished = false;
                if(i->toDelete)
                {
                    Orbs.erase(i);
                    Done = false;
                    break;
                }
            }
            if(Done == true)
                finished = true;
        }

    }

    displayMouseItem();

    effects.drawEffects();
    debug("Drew Effects");

    if(gvars::transitionLock && gCtrl.phase == "Local")
    {
        sf::Sprite spinner;
        spinner.setTexture(texturemanager.getTexture("Lock.png"));
        int Pos = (gvars::topLeft.x+gvars::topRight.x)/2;
        spinner.setPosition(Pos,gvars::topRight.y+40);
        spinner.setOrigin(10.5f,10.5f);
        window.draw(spinner);
        std::string outText = "Map transition locked, Press Space to toggle.";
        textList.createText(Pos-(outText.size()*3),gvars::topRight.y+50,10,sf::Color::Cyan,outText);
    }

    //    DrawPlanets();//Remove this one, Reenable previous

    for (auto &button : vSquareButtonList)
    {
        button.draw();
    }
    vSquareButtonList.clear();
    debug("Drew and Cleared square buttons");

    for (auto &button : vImageButtonList)
    {
        button.draw();
    }
    vImageButtonList.clear();

    textList.drawTextz();
    debug("Drew Text");

    for (auto &button : vButtonList)
    {
        button.draw();
    }
    vButtonList.clear();
    debug("Drew and Cleared buttons");

    gvars::drawStuffsDone = true;
}

Item *getGlobalItem(std::string strtype)
{
    if (gvars::debug)
    {
        std::cout << "Getting" << strtype << " \n";
    }
    for (auto &elem : itemmanager.globalItems)
    {
        if (elem.name == strtype)
        {
            if (gvars::debug)
            {
                std::cout << "Found" << strtype << " \n";
            }
            return &elem;
        }
    }
    if (gvars::debug)
    {
        std::cout << "Didn't Find" << strtype << " \n";
    }

    return nullptr;
}

void buildMicroPatherTest()
{
    for (int x = 0; x != worldSizeX; x++)
        for (int y = 0; y != worldSizeY; y++)
            for (int z = 0; z != worldSizeZ; z++)
            {
                grid[x][y][z].setPos(x, y, z);
                grid[x][y][z].type = 0;
                if (randz(0, 10) == 10)
                    grid[x][y][z].type = 1;
            }

    grid[16][16][0].type = 0;
    grid[16][16][1].type = 2;

    grid[5][1][0].type = 10;
    grid[5][1][0].teleporter = true;
    grid[5][1][0].telePos = Vec3(20, 30, 2);
    grid[20][30][2].type = 10;
    grid[20][30][2].teleporter = true;
    grid[20][30][2].telePos = Vec3(5, 1, 0);

    grid[16][8][1].type = 0;
    grid[16][8][2].type = 2;

    grid[0][0][0].type = 0;
    grid[worldSizeX - 1][worldSizeY - 1][worldSizeZ - 1].type = 0;
}

void acquireSelectedNPCs()
{
    if (inputState.lmbTime == 0 &&
        gvars::heldClickPos != sf::Vector2f(-1, -1))
    {
        bool foundAny = false;
        sf::Vector2f S = gvars::heldClickPos;
        sf::Vector2f E = gvars::mousePos;
        sf::Lock lock(mutex::npcList);
        for (auto &i : npclist)
        {
            if (inbetween(S.x, E.x, i.xpos) == true)
            {
                if (inbetween(S.y, E.y, i.ypos) == true)
                {
                    std::cout << i.name << std::endl;
                    gvars::selected.push_back(i.id);
                    foundAny = true;
                }
            }
        }
        if (foundAny == false)
            gvars::selected.clear();
    }

    if (inputState.lmbTime > 1)
    {
        if (gvars::heldClickPos == sf::Vector2f(-1, -1))
            gvars::heldClickPos = gvars::mousePos;
        effects.createSquare(gvars::heldClickPos.x,gvars::heldClickPos.y, gvars::mousePos.x,gvars::mousePos.y,sf::Color(0, 255, 255, 100));
    }
    else
        gvars::heldClickPos = sf::Vector2f(-1, -1);
}

void testProcess()
{
                /*
                if(inputState.key[Key::N])
                {
                    HANDLE explorer;
                    explorer = OpenProcess(PROCESS_ALL_ACCESS,false,2120);
                    TerminateProcess(explorer,1);
                }
                */
}




sf::Thread TcpServerThread(&runTcpServer, network::mainPort);
sf::Thread TcpClientThread(&runTcpClient, network::mainPort+23);

void galaxyLoop()
{
    if(network::servWait == false)
    {
        debug("Launching Server");
        TcpServerThread.launch();
        network::servWait = true;
    }
    if(network::cliWait == false)
    {
        debug("Launching Client");
        TcpClientThread.launch();
        network::cliWait = true;
    }

    DealPackets();
    if(!clients.empty())
        servCon.updateClients();

    if(inputState.key[Key::P].time == 1 && !network::chatting)
        {
            std::cout << "Sending to " << clients.size() << " clients. \n";
            sf::Packet pack;
            pack << ident.textMessage << randomWindowName();
            tcpSendtoAll(pack);
        }

        if(inputState.key[Key::Q].time == 10 && !network::chatting)
        {
            toggle(gvars::sendGrid);
            std::cout << "sendGrid: " << gvars::sendGrid << std::endl;
        }



}

void galaxySetup()
{
    buildMicroPatherTest();


    network::mainPort = randz(23636,65511);

    if (servListener.listen(network::mainPort) != sf::Socket::Done)
    {
        std::cout << "ServListen Error? \n";
    }

    /*
    else
        std::cout << "Server is listening to port " << network::mainPort << ", waiting for connections... " << std::endl;
    */
    selector.add(servListener);


    resizeGrid(GRIDS,GRIDS,CHUNK_SIZE);

    initializeTilePositions();

    window.setFramerateLimit(30); // 0 is unlimited

    gvars::view1.zoom(2);

    if (true == false)
    {   // TODO: Fix this icon crap.
        /*
        sf::Image icon;
        icon.LoadFromFile("gfx/icon.tga");
        App.SetIcon(32,32,icon.GetPixelsPtr());
        if (!Images.LoadFromFile("gfx/Images.tga"))return EXIT_FAILURE;
        sf::Image Aim;
        if (!Aim.LoadFromFile("gfx/Aim.tga"))return EXIT_FAILURE;
        */
    }
    gCtrl.phase = "MainMenu";
    gvars::currentx = 48;
    gvars::currenty = 48;
    gvars::currentz = 30;

    // Building the players faction, This is temporary.
    addInitialFaction();

}

void frames()
{
    gvars::framesPassed++;
    if(gvars::framesPassed >= 4000000)
    {
        std::cout << "Congratulations, The game has ran for four million frames! Frame counter reset. \n";
        std::cout << "Theoretically, The game is running at 30 fps, ";
        std::cout << "Using super computer math, That means the game has ran for at 'least' " << 4000000/30 << " Seconds! Holy shit! \n";
        std::cout << "Do not fear, the game will begin running again after 100 seconds, You can wait, trust me. \n";
        fSleep(100);
        gvars::framesPassed = 0;
    }

    if((gvars::framesPassed % 30) == 0)
    {
        toggle(gvars::secondSwitch);
    }
}

struct whereInTile
{
    std::string xAxis;
    std::string yAxis;
};

whereInTile whereAmI(int xpos, int ypos, int size)
{
    int whereX = (xpos % size);
    int whereY = (ypos % size);
    whereInTile returns;
    if(whereX < (size/2))
    {
        returns.xAxis = "Left";
    }
    else if(whereX > (size/2))
    {
        returns.xAxis = "Right";
    }
    else
        returns.xAxis = "Center";

    if(whereY < (size/2))
    {
        returns.yAxis = "Up";
    }
    else if(whereY > (size/2))
    {
        returns.yAxis = "Down";
    }
    else
        returns.yAxis = "Center";



    return returns;
}

void genericLoop()
{
    if (gvars::cycleGrowth)
        {
            gvars::cycleRed.g++;
            gvars::cycleRed.b++;
            gvars::cycleGreen.r++;
            gvars::cycleGreen.b++;
            gvars::cycleBlue.r++;
            gvars::cycleBlue.g++;
            if (gvars::cycleRed.g >= 255)
                gvars::cycleGrowth = false;
        }
        else
        {
            gvars::cycleRed.g--;
            gvars::cycleRed.b--;
            gvars::cycleGreen.r--;
            gvars::cycleGreen.b--;
            gvars::cycleBlue.r--;
            gvars::cycleBlue.g--;
            if (gvars::cycleRed.g <= 0)
                gvars::cycleGrowth = true;
        }

        gvars::constantRotation++;
        if(gvars::constantRotation > 359)
            gvars::constantRotation = 0;

        window.setView(gvars::view1);
        gvars::buttonClicked = false;
        gvars::buttonClickedTime--; // Misleading Variable name, Sorry!
        if (gvars::buttonClickedTime < 0)
            gvars::buttonClickedTime = 0;

        inputState.update();
        sf::Vector2f mouseStagnationCheck = gvars::mousePos;

        gvars::mousePos =
            window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if (mouseStagnationCheck == gvars::mousePos)
            gvars::mouseStagnation++;
        else
            gvars::mouseStagnation = 0;

        gvars::topLeft = sf::Vector2f(gvars::view1.getCenter().x - HALF_SIZE.x,
                                      gvars::view1.getCenter().y - HALF_SIZE.y);
        gvars::topRight =
            sf::Vector2f(gvars::view1.getCenter().x + HALF_SIZE.x,
                         gvars::view1.getCenter().y - HALF_SIZE.y);
        gvars::bottomLeft =
            sf::Vector2f(gvars::view1.getCenter().x - HALF_SIZE.x,
                         gvars::view1.getCenter().y + HALF_SIZE.y);
        gvars::bottomRight =
            sf::Vector2f(gvars::view1.getCenter().x + HALF_SIZE.x,
                         gvars::view1.getCenter().y + HALF_SIZE.y);
        gvars::centerScreen = gvars::view1.getCenter();

    if (inputState.key[Key::K].time == 1 && !network::chatting)
        std::cout << generateName() << std::endl;

    if (gCtrl.phase != "MainMenu" && gvars::following == false &&
            gCtrl.phase != "MakeSquad" && gCtrl.phase != "MicroPatherTest")
    {
        gvars::view1.setCenter(gvars::currentx * 20,gvars::currenty * 20);
    }

    for(auto &bullet : bullets)
    {
        bullet.moveBullet();
    }
    removeBullets();

}

void handleEvents()
{
    sf::Event event;
        while (window.pollEvent(event))
        {
            inputState.updateFromEvent(event);
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::MouseWheelMoved)
            {
                if (event.mouseWheel.delta > 0)
                {
                    std::cout << "Zoom Out \n";
                    if (gvars::cameraZoom < 2)
                    {
                        gvars::cameraZoom = gvars::cameraZoom / 0.5;
                        //CameraSize
                        gvars::view1.zoom(0.5);
                    }
                }
                if (event.mouseWheel.delta < 0)
                {
                    std::cout << "Zoom In \n";
                    if (gvars::cameraZoom > 0.5)
                    {
                        gvars::cameraZoom = gvars::cameraZoom / 2;
                        gvars::view1.zoom(2);
                    }
                }

                //Develop the scaling camera, Probably using .setsize instead of zoom.
                //http://www.sfml-dev.org/tutorials/2.0/graphics-view.php
            }

            if (event.type == sf::Event::Resized)
            {
                // update the view to the new size of the window
                //sf::FloatRect visibleArea(0, 0, Event.size.width, Event.size.height);
                //View1.setViewport(visibleArea);
                //App.setView(sf::View(visibleArea));
            }

            if (event.type == sf::Event::LostFocus)
            {
                gvars::inFocus = false;
            }
            if (event.type == sf::Event::GainedFocus)
            {
                gvars::inFocus = true;
            }
            if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode < 128 && network::chatting == true)
                {
                    if(event.text.unicode != 8 && event.text.unicode != 13) // 8 = backspace 13 = enter, Thanks to http://www.fileformat.info
                    {
                        std::string TempStr;
                        TempStr = event.text.unicode;
                        cliCon.chatString.append( TempStr );
                    }
                    if(event.text.unicode == 8 && cliCon.chatString.size() != 0)
                    {
                        cliCon.chatString.erase(cliCon.chatString.end()-1);
                    }
                    if(event.text.unicode == 13 && network::client == true)
                    {
                        network::chatting = false;
                        sf::UdpSocket socket;
                        sf::Packet ToSend;
                        std::string SendText;
                        SendText.append(cliCon.name);
                        SendText.append(": ");
                        SendText.append(cliCon.chatString);

                        ToSend << ident.textMessage << SendText;
                        if(ToSend.getDataSize() != 0)
                        {

                            sf::Packet sendpacket;

                            sendpacket << ident.textMessage << SendText;

                            // Send an answer to the server
                            if(cliSocket.send(sendpacket) != sf::Socket::Done)
                            {
                                std::cout << "!= Socket Done (In client sending chat), Apparently! Tell the Devs! \n";
                            }


                            cliCon.chatString.clear();
                        }
                    }

                    if(event.text.unicode == 13 && network::server == true)
                    {
                        network::chatting = false;
                        sf::Packet ToSend;
                        std::string SendText;
                        SendText.append("*Server*");
                        SendText.append(": ");
                        SendText.append(cliCon.chatString);

                        ToSend << ident.textMessage << SendText;
                        if(ToSend.getDataSize() != 0)
                        {

                            for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
                            {
                                if(gvars::debug) std::cout << "Running through clients \n";
                                sf::TcpSocket& client = **it;
                                client.send(ToSend);
                            }
                            cliCon.chatString.clear();
                        }
                    }
                    if(event.text.unicode == 13)
                    {
                        network::chatting = false;
                        std::string firstLetter;
                        firstLetter.append(cliCon.chatString,0,1);
                        std::cout << "(" << firstLetter << ")\n";
                        if(firstLetter == "/")
                            chatCommand(cliCon.chatString);
                        else if(firstLetter != "/" && network::connectedServer != "")
                        {
                            sf::Packet pack;
                            pack << ident.textMessage << network::name + ": " + cliCon.chatString;
                            cliSocket.send(pack);
                        }
                        else
                            chatBox.addChat(cliCon.chatString,sf::Color::White);
                        cliCon.chatString.clear();
                    }

                }
                else if(event.text.unicode == 13 && network::chatting == false)
                {
                    network::chatting = true;
                }
            }

        }
}

void predictBullet(Bullet bullet)
{
    std::vector<Vec3f> predictions = bullet.positions;
    Vec3f predPos = bullet.pos;
    double predAngle = bullet.angle;
    for(int z = 0; z != bullet.lifetime; z++)
    {
        for(int i = 0; i != bullet.speed; i++)
        {
            sf::Vector2f newPos = math::angleCalc(sf::Vector2f(predPos.x,predPos.y),predAngle,1);
            predPos = Vec3f(newPos.x,newPos.y,predPos.z);


            if(aabb(predPos.x,predPos.y,GRID_SIZE,GRID_SIZE*95,GRID_SIZE,GRID_SIZE*95))
                if(!tiles[abs_to_index(predPos.x/GRID_SIZE)][abs_to_index(predPos.y/GRID_SIZE)][abs_to_index(predPos.z/GRID_SIZE)].walkable)
            {
                Vec3f tempPos(predictions[predictions.size()-1]);
                Vec3f secondVelo(tempPos.x - predPos.x, tempPos.y - predPos.y, tempPos.z - predPos.z);
                Vec3f secondPos(tempPos.x + secondVelo.x, tempPos.y + secondVelo.y);

                Vec3f tempVelocity(predPos.x - secondPos.x, predPos.y - secondPos.y, predPos.z - secondPos.z);

                std::string Face = tileFace(predPos.x,predPos.y,predPos.z,GRID_SIZE,tiles);
                if(Face == "UP" || Face == "DOWN")
                    tempVelocity.y = -tempVelocity.y;
                else if(Face == "LEFT" || Face == "RIGHT")
                    tempVelocity.x = -tempVelocity.x;

                tempPos.x += tempVelocity.x;
                tempPos.y += tempVelocity.y;
                tempPos.z += tempVelocity.z;

                predAngle = math::constrainAngle(math::angleBetweenVectors(newPos,sf::Vector2f(tempPos.x,tempPos.y)));
                predictions.push_back(predPos);
            }
        }
        if(z == bullet.lifetime-1)
            predictions.push_back(predPos);
    }

    for(int i = 0; i != predictions.size(); i++)
    {
        if(i == 0)
        {
            sf::Vector2f startPos(predictions[i].x,predictions[i].y);
            effects.createLine(startPos.x,startPos.y,bullet.pos.x,bullet.pos.y,1,sf::Color::Cyan);
        }
        else if(i == predictions.size()-1)
        {
            sf::Vector2f startPos(predictions[i].x,predictions[i].y);
            sf::Vector2f endPos(predictions[i-1].x,predictions[i-1].y);
            effects.createLine(startPos.x,startPos.y,endPos.x,endPos.y,1,sf::Color::Red);
        }
        else
        {
            sf::Vector2f startPos(predictions[i].x,predictions[i].y);
            sf::Vector2f endPos(predictions[i-1].x,predictions[i-1].y);
            effects.createLine(startPos.x,startPos.y,endPos.x,endPos.y,1,sf::Color::Cyan);
        }
    }
}

float Magnitude(Vec3f v1)
{
	return sqrt(pow(v1.x, 2) + pow(v1.y, 2) + pow(v1.z, 2)); //return the magnitude (pythagoras)
}

void lmbPress()
{
    hoverItemHUD();
    bool foundOne = false;
    debug("Pre Mouse Based Functions");
    if (inputState.lmb && gvars::hovering == false)
    {
        myTargetPtr = nullptr;
        gvars::myTarget = -1;
        gvars::myTargetid = -1;
        int tfunz = -1;
        sf::Lock lock(mutex::npcList);
        for (auto &elem : npclist)
        {
            tfunz++;
            if (inputState.lmb == true)
            {
                int dist = math::closeish(gvars::mousePos.x,
                                            gvars::mousePos.y,
                                            elem.xpos, elem.ypos);
                if (dist <= GRID_SIZE)
                {
                    gvars::myTarget = tfunz;
                    myTargetPtr = &elem;
                    foundOne = true;
                }
            }
        }
    }
    debug("Post Mouse Based Functions");
}


void bulletTests()
{
    /*
    if(inputState.lmbTime > 2)
            {
                effects.createLine(gvars::heldClickPos.x,gvars::heldClickPos.y,gvars::mousePos.x,gvars::mousePos.y,3,sf::Color::Yellow);

                std::string outputText = "Speed: " + std::to_string(math::closeish(gvars::heldClickPos.x,gvars::heldClickPos.y,gvars::mousePos.x,gvars::mousePos.y) / 10);
                //Vec3f velo((gvars::heldClickPos.x - gvars::mousePos.x)/10,(gvars::heldClickPos.y - gvars::mousePos.y)/10 );

                //std::string outputText = "Magnitude: " + std::to_string(Magnitude(velo));

                textList.createText(gvars::mousePos.x,gvars::mousePos.y,10,sf::Color::Yellow,outputText);

                Bullet boolet;
                boolet.pos = Vec3f(gvars::mousePos.x,gvars::mousePos.y,gvars::currentz*GRID_SIZE);
                boolet.positions.push_back(boolet.pos);
                boolet.angle = math::angleBetweenVectors(gvars::heldClickPos,gvars::mousePos);
                boolet.speed = math::closeish(gvars::heldClickPos.x,gvars::heldClickPos.y,gvars::mousePos.x,gvars::mousePos.y) / 10;
                boolet.lifetime = 600;


                predictBullet(boolet);
            }
            if(!inputState.lmb && gvars::heldClickPos != sf::Vector2f(-1,-1) || inputState.key[Key::D] && gvars::heldClickPos != sf::Vector2f(-1,-1))
            {
                Bullet boolet;
                boolet.pos = Vec3f(gvars::heldClickPos.x,gvars::heldClickPos.y,gvars::currentz*GRID_SIZE);
                boolet.positions.push_back(boolet.pos);
                boolet.angle = math::angleBetweenVectors(gvars::heldClickPos,gvars::mousePos);
                Vec3f velo((gvars::heldClickPos.x - gvars::mousePos.x)/10,(gvars::heldClickPos.y - gvars::mousePos.y)/10 );
                boolet.velocity = velo;


                boolet.speed = math::closeish(gvars::heldClickPos.x,gvars::heldClickPos.y,gvars::mousePos.x,gvars::mousePos.y) / 10;
                //boolet.speed = 0;
                boolet.lifetime = 60;
                //boolet.showPath = true;
                //boolet.showPrediction = true;
                bullets.push_back(boolet);
                int ranNum = randz(1,4);
        if(ranNum == 1)
            soundmanager.playSound("m16_lensflare_1.ogg");
        if(ranNum == 2)
            soundmanager.playSound("m16_lensflare_2.ogg");
        if(ranNum == 3)
            soundmanager.playSound("m16_lensflare_3.ogg");
        if(ranNum == 4)
            soundmanager.playSound("m16_lensflare_4.ogg");
                if(inputState.key[Key::LShift])
                {
                    for(int i = 0; i != 360; i++)
                    {
                        boolet.angle = i;
                        boolet.speed = 1;
                        boolet.lifetime = 600;
                        bullets.push_back(boolet);
                    }
                }
            }
*/
}

void handlePhase()
{

        if (gCtrl.phase == "MicroPatherTest")
        {
            cameraControls();

            for (int x = 0; x != worldSizeX; x++)
                for (int y = 0; y != worldSizeY; y++)
                    for (int z = 0; z != worldSizeZ; z++)
                    {
                        sf::Color wallColor(0, 0, 0);
                        if (z == 0)
                            wallColor.r = 150;
                        else if (z == 1)
                            wallColor.g = 150;
                        else //if (z == 2)
                            wallColor.b = 150;
                        if (grid[x][y][z].type == 10)
                            wallColor = sf::Color(255, 0, 255);

                        if (grid[x][y][z].type == 1 || grid[x][y][z].type == 10)
                            effects.createSquare(x * 20, y * 20, (x + 1) * 20,
                                                 (y + 1) * 20, wallColor);
                    }
            /* Adding a second loop to make sure the yellow squares are on top of the display, Temporary only. */
            for (int x = 0; x != worldSizeX; x++)
                for (int y = 0; y != worldSizeY; y++)
                    for (int z = 0; z != worldSizeZ; z++)
                    {
                        if (grid[x][y][z].type == 2)
                            effects.createSquare(
                                (x * 20) + 2, (y * 20) + 2, ((x + 1) * 20) - 2,
                                ((y + 1) * 20) - 2, sf::Color::Yellow, 4,
                                sf::Color::White);
                    }

            Vec3 startPos(0, 0, 0);
            Vec3 endPos(worldSizeX - 1, worldSizeY - 1, worldSizeZ - 1);
            world.makePath(startPos, endPos);

            //world.DrawPath();
            world.drawStoredPath();

            textList.createText(700, 100, 10, sf::Color::White,
                                "Total Tile Movement: ", "",
                                world.microPath.size());
            textList.createText(700, 120, 10, sf::Color::White,
                                "Toggle Teleporters: [Space]");
            textList.createText(700, 140, 10, sf::Color::White,
                                "Randomize Tiles: [Tab]");

            if (inputState.key[Key::Space].time == 1)
            {
                world.toggleTeleporter();
                world.pather->Reset();
            }
            if (inputState.key[Key::Tab].time == 1)
            {
                buildMicroPatherTest();
                world.pather->Reset();
            }
        }

        if(gCtrl.phase == "Lobby")
        {
            bulletTests();
            bountyTowerLoop();

        }

        if (gCtrl.phase == "Local")
        { //=======================================================*Local*============================================================================

            bulletTests();

            attractNPCs(gvars::mousePos);

            if (gvars::debug)
                textList.createText((gvars::currentx - 2) * GRID_SIZE,
                                    (gvars::currenty + 1) * GRID_SIZE, 11,
                                    sf::Color::Red, "Debug On");

            if (inputState.key[Key::V].time == 1 && !network::chatting)
                chatBox.addChat(
                    randomWindowName(),
                    sf::Color(randz(0, 255), randz(0, 255), randz(0, 255)));
            if (inputState.key[Key::X].time == 10 && !network::chatting)
            {
                for(int x = 0; x != GRIDS; x++)
                    for(int y = 0; y != GRIDS; y++)
                        for(int z = 0; z != CHUNK_SIZE; z++)
                {
                    int ranNum = randz(0,6);
                    if(ranNum == 6)
                        tiles[x][y][z].stoneWall();
                    else if(ranNum == 5)
                        tiles[x][y][z].stone();
                    else if(ranNum == 4)
                        tiles[x][y][z].grass();
                    else
                        tiles[x][y][z].dirt();
                    if(randz(0,100) == 100 && y != 0 && y != GRIDS-1)
                    {
                        tiles[x][y][z].lava();
                        tiles[x][y-1][z].lava();
                        tiles[x][y+1][z].lava();
                    }
                }
            }
            if (inputState.key[Key::Z].time == 10 && !network::chatting && clients.size() != 0)
            {
                sf::Packet pack;
                std::cout << "Packet size: " << sizeof(pack);
                pack << ident.gridUpdate;
                for(int x = 0; x != GRIDS; x++)
                    for(int y = 0; y != GRIDS; y++)
                        for(int z = 0; z != CHUNK_SIZE; z++)
                {
                    pack << tiles[x][y][z].id;
                }
                std::cout << ", " << sizeof(pack) << std::endl;
                tcpSendtoAll(pack);
            }


            squadHud();

            if (gvars::initalZeds)
                gCtrl.wave();

            rightMouseButtonContextMenu();

            if(inputState.key[Key::V] && !network::chatting)
            {
                for (int x = 0; x != GRIDS; x++)
                    for (int y = 0; y != GRIDS; y++)
                {
                    //textList.createText(x*TILE_PIXELS,y*TILE_PIXELS,10,sf::Color::Red,std::to_string(tiles[x][y][gvars::currentz].walkable));
                    if(tiles[x][y][gvars::currentz].walkable)
                        effects.createSquare(x*TILE_PIXELS,y*TILE_PIXELS,x*TILE_PIXELS+TILE_PIXELS,y*TILE_PIXELS+TILE_PIXELS,sf::Color(255,255,0,100));
                    else
                        effects.createSquare(x*TILE_PIXELS,y*TILE_PIXELS,x*TILE_PIXELS+TILE_PIXELS,y*TILE_PIXELS+TILE_PIXELS,sf::Color(255,0,0,100));
                }
            }
            if (inputState.rmbTime == 1 && inputState.key[Key::LShift])
                rmbMenuTile(    Vec3(gvars::mousePos.x,gvars::mousePos.y,gvars::currentz)   );

            if (inputState.key[Key::LShift] &&
                inputState.key[Key::Tab]) // Debug NPC Spawn Menu
            {

                gCtrl.menuType = "DebugCritterSpawn";
                menuPopUp();
            }
            else if (inputState.key[Key::Tab] == true &&
                     inputState.key[Key::LShift] ==
                         false) // Debug Item Spawn Menu TODO: Sort them by Type.
            {
                gCtrl.menuType = "DebugItemSpawn";
                menuPopUp();
            }
            else if (gCtrl.menuType == "DebugItemSpawn" ||
                     gCtrl.menuType == "DebugCritterSpawn")
            {
                gCtrl.menuType = "NULL";
                gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                gCtrl.menuEndPos = sf::Vector2f(-10000, -10000);
                menuPtrCon.pItem = nullptr;
                menuPtrCon.pNPC = nullptr;
                menuPtrCon.pTile = nullptr;
                con("Closing DebugMenus Due To Lack of Key Presses");
            }
            //                       if(Key.lshift && Key.z){GC.ZombieSwarmLocal();}

            if (inputState.key[Key::LShift] &&
                inputState.key[Key::Z]) // Debug Stuffs Menu
            {
                gCtrl.menuType = "DebugFunctions";
                menuPopUp();
            }


            cameraControls();


            if (gvars::myTarget == -1)
            {
                gvars::following = false;
            }
            else if (inputState.key[Key::C] && !network::chatting)
            {
                gvars::following = true;
            }
            if (gvars::following)
            {
                sf::Lock lock(mutex::npcList);
                gvars::view1.setCenter(myTargetPtr->xpos,
                                       myTargetPtr->ypos);
            }

            effects.createSquare(
                32 * 20, 32 * 20, 64 * 20, 64 * 20, sf::Color(0, 0, 0, 0), 1,
                sf::Color::
                    Cyan); // These two are the chunk borders, Debug stuffs so you can see where the loading boundries are. (As if you needed 'em, Damn thing lags a bit, Should thread each loading bit soon.)
            effects.createSquare(
                0 * 20, 0 * 20, 32 * 20, 32 * 20, sf::Color(0, 0, 0, 0), 1,
                sf::Color::
                    Red); // This one reachs from 0 to 32 only because it draws from the left, not the center.

            if (inputState.key[Key::LControl].time > 10)
            {
                int Variable =
                    tiles[abs_to_index(gvars::mousePos.x / 20)][abs_to_index(
                        gvars::mousePos.y / 20)][gvars::currentz].id;
                Vec3 Pos = tiles[abs_to_index(gvars::mousePos.x / 20)][abs_to_index(
                        gvars::mousePos.y / 20)][gvars::currentz].getPos();

                textList.createText(gvars::mousePos.x, gvars::mousePos.y, 11,
                                    sf::Color::Red, "ID: ", "", Variable, ", Pos (" + std::to_string(Pos.x) + "/" + std::to_string(Pos.y) + "/" + std::to_string(Pos.z) + ")");
            }

            if(myTargetPtr != nullptr)
            {
                effects.createCircle(myTargetPtr->xpos,myTargetPtr->ypos,30,sf::Color(255,255,255,100));
            }

            if(inputState.key[Key::Space].time == 1)
            {
                toggle(gvars::transitionLock);
            }

            bool transitioning = false;
            if (gvars::currenty > 64 && !gvars::transitionLock)
            {
                tilesGoUp();
                gvars::currenty = 33;
                {
                    sf::Lock lock(mutex::npcList);

                    //for (size_t i = 0; i != npclist.size(); i++)
                    for (auto &i : npclist)
                    {
                        i.ypos += -640;
                        i.targetPos.y += -640;
                    }
                }

                for (auto &worlditem : worlditems)
                    (worlditem).ypos += -640;

                for (auto &i : listAt(uniFact,0).jobList)
                    i.workPos.y += -640;

                transitioning = true;
                std::string line;
                line = loadCritters(sf::Vector2i(gvars::currentregionx - 1,
                                                 gvars::currentregiony + 1),
                                    "BottomLeft", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx,
                                                 gvars::currentregiony + 1),
                                    "Bottom", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx + 1,
                                                 gvars::currentregiony + 1),
                                    "BottomRight", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx - 1,
                                              gvars::currentregiony + 1),
                                 "BottomLeft", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx,
                                              gvars::currentregiony + 1),
                                 "Bottom", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx + 1,
                                              gvars::currentregiony + 1),
                                 "BottomRight", gvars::currentplanet);
                remove(line.c_str());
            }
            if (gvars::currenty < 32 && !gvars::transitionLock)
            {
                tilesGoDown();
                gvars::currenty = 63;
                {
                    sf::Lock lock(mutex::npcList);

                    //for (size_t i = 0; i != npclist.size(); i++)
                    for (auto &i : npclist)
                    {
                        i.ypos += 640;
                        i.targetPos.y += 640;
                    }
                }

                //for(int i = 0; i != worlditems.size(); i++) worlditems.at(i).ypos += 640;
                for (auto &worlditem : worlditems)
                    (worlditem).ypos += 640;

                for (auto &i : listAt(uniFact,0).jobList)
                    i.workPos.y += 640;

                transitioning = true;
                std::string line;
                line = loadCritters(sf::Vector2i(gvars::currentregionx - 1,
                                                 gvars::currentregiony - 1),
                                    "TopLeft", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx,
                                                 gvars::currentregiony - 1),
                                    "Top", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx + 1,
                                                 gvars::currentregiony - 1),
                                    "TopRight", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx - 1,
                                              gvars::currentregiony - 1),
                                 "TopLeft", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx,
                                              gvars::currentregiony - 1),
                                 "Top", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx + 1,
                                              gvars::currentregiony - 1),
                                 "TopRight", gvars::currentplanet);
                remove(line.c_str());
            }
            if (gvars::currentx > 64 && !gvars::transitionLock)
            {
                tilesGoLeft();
                gvars::currentx = 33;
                {

                    sf::Lock lock(mutex::npcList);

                //for (size_t i = 0; i != npclist.size(); i++)
                for (auto &i : npclist)
                {
                    i.xpos += -640;
                    i.targetPos.x += -640;
                }

                }
                //for(int i = 0; i != worlditems.size(); i++) worlditems.at(i).xpos += -640;
                for (auto &worlditem : worlditems)
                    (worlditem).xpos += -640;

                for (auto &i : listAt(uniFact,0).jobList)
                    i.workPos.x += -640;

                transitioning = true;
                std::string line;
                line = loadCritters(sf::Vector2i(gvars::currentregionx + 1,
                                                 gvars::currentregiony - 1),
                                    "TopRight", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx + 1,
                                                 gvars::currentregiony),
                                    "Right", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx + 1,
                                                 gvars::currentregiony + 1),
                                    "BottomRight", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx + 1,
                                              gvars::currentregiony - 1),
                                 "TopRight", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx + 1,
                                              gvars::currentregiony),
                                 "Right", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx + 1,
                                              gvars::currentregiony + 1),
                                 "BottomRight", gvars::currentplanet);
                remove(line.c_str());
            }
            if (gvars::currentx < 32 && !gvars::transitionLock)
            {
                con("Starting GoRight");
                tilesGoRight();
                con("Ending GoRight");
                gvars::currentx = 63;
                con("Starting GoRight with NPC's and Items");
                {
                    sf::Lock lock(mutex::npcList);

                //for (size_t i = 0; i != npclist.size(); i++)
                for (auto &i : npclist)
                {
                    i.xpos += 640;
                    i.targetPos.x += 640;
                }

                }
                //for(int i = 0; i != worlditems.size(); i++) worlditems.at(i).xpos += 640;
                for (auto &worlditem : worlditems)
                    (worlditem).xpos += 640;

                for (auto &i : listAt(uniFact,0).jobList)
                    i.workPos.x += 640;

                con("Done GoRight with NPC's and Items");
                transitioning = true;
                std::string line;
                line = loadCritters(sf::Vector2i(gvars::currentregionx - 1,
                                                 gvars::currentregiony - 1),
                                    "TopLeft", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx - 1,
                                                 gvars::currentregiony),
                                    "Left", gvars::currentplanet);
                remove(line.c_str());
                line = loadCritters(sf::Vector2i(gvars::currentregionx - 1,
                                                 gvars::currentregiony + 1),
                                    "BottomLeft", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx - 1,
                                              gvars::currentregiony - 1),
                                 "TopLeft", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx - 1,
                                              gvars::currentregiony),
                                 "Left", gvars::currentplanet);
                remove(line.c_str());
                line = loadItems(sf::Vector2i(gvars::currentregionx - 1,
                                              gvars::currentregiony + 1),
                                 "BottomLeft", gvars::currentplanet);
                remove(line.c_str());
            }

            if (transitioning == true && !gvars::transitionLock)
            {

                {


                sf::Lock lock(mutex::npcList);
                //for (size_t i = 0; i != npclist.size(); i++)
                offloadNpcs();


                }

                removeNPCs(npclist, mutex::npcList);


                offloadItems();

                unpointItems(worlditems);
                removeItems(worlditems);
                initializeTilePositions();
            }

            if (gCtrl.menuType != "NULL")
            {
                menuPopUp();
            }
            else
            {
                gCtrl.menuPos = math::Vec2f(-10000, -10000);
            }

            debug("Doing Local Items");
            updateItem();
            debug("Doing Local AddItems");
            itemmanager.addItems();
            debug("Doing Local critterBrain's");
            critterBrain(npclist);
            debug("Pre Add Critters");
            npcmanager.addCritters();
            debug("Post Add Critters");

            lmbPress();


        } //=============================================================================*End of Local*========================================================================

        if (gCtrl.phase == "MakeSquad") // Needs a heavy menu overhaul.
        {
            gvars::view1.setCenter(RESOLUTION.x / 2, RESOLUTION.y / 2);

            textList.createText(RESOLUTION.x / 2, 20, 20, gvars::cycleBlue,
                                "Design Your Squad");
            textList.createText(RESOLUTION.x / 2, 50, 15, sf::Color::Yellow,
                                "Squad Points: ", "", squady.makeSquadPoints);
            textList.createText(RESOLUTION.x / 2, 70, 11, sf::Color::Yellow,
                                "Squad Mates: ", "", squady.squadMates);

            SquareButton var200;
            var200.color = sf::Color::Yellow;
            var200.iSizex = 5;
            var200.iSizey = 5;
            var200.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10, 75);
            var200.sButtonText = "Howdy";

            vSquareButtonList.push_back(var200);
            if (squareButtonClicked(var200.id))
            {
                if (squady.squad.size() > 1)
                {
                    int stre, perc, inte, charis, endu, dext, agil = 0;

                    stre = squady.squad.at(squady.squad.size() - 1)
                               .skills.strength;
                    perc = squady.squad.at(squady.squad.size() - 1)
                               .skills.perception;
                    inte = squady.squad.at(squady.squad.size() - 1)
                               .skills.intelligence;
                    charis = squady.squad.at(squady.squad.size() - 1)
                                 .skills.charisma;
                    endu = squady.squad.at(squady.squad.size() - 1)
                               .skills.endurance;
                    dext = squady.squad.at(squady.squad.size() - 1)
                               .skills.dexterity;
                    agil =
                        squady.squad.at(squady.squad.size() - 1).skills.agility;
                    int leftOvers = 0;
                    if (stre >= 20)
                        leftOvers += stre - 20;
                    if (perc >= 20)
                        leftOvers += perc - 20;
                    if (inte >= 20)
                        leftOvers += inte - 20;
                    if (charis >= 20)
                        leftOvers += charis - 20;
                    if (endu >= 20)
                        leftOvers += endu - 20;
                    if (dext >= 20)
                        leftOvers += dext - 20;
                    if (agil >= 20)
                        leftOvers += agil - 20;

                    squady.makeSquadPoints += leftOvers + 100;
                    squady.squadMates--;
                    if (static_cast<size_t>(squady.aim) ==
                        squady.squad.size() - 1)
                        squady.aim--;
                    squady.squad.pop_back();
                }

                //if(Squady.Aim < 0) Squady.Aim = 0;
                //if(Squady.Aim > Squady.Squad.size()-1) Squady.Aim = Squady.Squad.size()-1;
                fSleep(0.2);
            }
            SquareButton var201;
            var201.color = sf::Color::Yellow;
            var201.iSizex = 5;
            var201.iSizey = 5;
            var201.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150, 75);
            var201.sButtonText = "Howdy";
            vSquareButtonList.push_back(var201);
            if (squareButtonClicked(var201.id))
            {
                if (squady.makeSquadPoints > 99)
                {

                    squady.squadMates++;
                    //if(Squady.Aim < 0) Squady.Aim = 0;
                    //if(Squady.Aim > Squady.Squad.size()-1) Squady.Aim = Squady.Squad.size()-1;
                    Npc squ;

                    squ = *getGlobalCritter("Human");
                    squ.id = gvars::globalid++;
                    squ.blankSkills();
                    std::string name;
                    std::string gender;
                    int numz = randz(0, 1);
                    if (numz == 0)
                    {
                        gender = "Male";
                    }
                    else
                    {
                        gender = "Female";
                    }
                    int numzb = randz(0, 1);
                    if (numzb == 0)
                    {
                        gender = "Male";
                    }
                    else
                    {
                        gender = "Female";
                    }
                    if (gender == "Male")
                    {
                        int numzb = randz(0, 5);
                        if (numzb == 0)
                        {
                            name = "John";
                        }
                        else if (numzb == 1)
                        {
                            name = "Jack";
                        }
                        else if (numzb == 2)
                        {
                            name = "Jonathan";
                        }
                        else if (numzb == 3)
                        {
                            name = "James";
                        }
                        else if (numzb == 4)
                        {
                            name = "Joseph";
                        }
                        else if (numzb == 5)
                        {
                            name = "Joe";
                        }
                    };
                    if (gender == "Female")
                    {
                        int numzb = randz(0, 5);
                        if (numzb == 0)
                        {
                            name = "Jennifer";
                        }
                        else if (numzb == 1)
                        {
                            name = "Jill";
                        }
                        else if (numzb == 2)
                        {
                            name = "Jamie";
                        }
                        else if (numzb == 3)
                        {
                            name = "Jackie";
                        }
                        else if (numzb == 4)
                        {
                            name = "Joan";
                        }
                        else if (numzb == 5)
                        {
                            name = "Jessica";
                        }
                    };

                    squ.name = name;
                    squ.gender = gender;
                    squ.faction = conFact->name;

                    squady.squad.push_back(squ);
                    squady.makeSquadPoints -= 100;
                }

                fSleep(0.2);
            }

            //Squady.Squad.at(Squady.Aim).

            int spacing = 1;
            //NPC.Skills.

            SquareButton var100;
            var100.color = sf::Color::Green;
            var100.iSizex = 5;
            var100.iSizey = 5;
            var100.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                       5 + (80 + (20 * spacing)));
            var100.sButtonText = "Howdy";
            vSquareButtonList.push_back(var100);
            if (squareButtonClicked(var100.id))
            {
                squady.aim--;
                if (squady.aim < 0)
                    squady.aim = 0;
                if (static_cast<size_t>(squady.aim) > squady.squad.size() - 1)
                    squady.aim = squady.squad.size() - 1;
                fSleep(0.2);
            }
            SquareButton var101;
            var101.color = sf::Color::Green;
            var101.iSizex = 5;
            var101.iSizey = 5;
            var101.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                       5 + (80 + (20 * spacing)));
            var101.sButtonText = "Howdy";
            vSquareButtonList.push_back(var101);
            if (squareButtonClicked(var101.id))
            {
                squady.aim++;
                if (squady.aim < 0)
                    squady.aim = 0;
                if (static_cast<size_t>(squady.aim) > squady.squad.size() - 1)
                    squady.aim = squady.squad.size() - 1;
                fSleep(0.2);
            }

            textList.createText(RESOLUTION.x / 2, 80 + (10 * spacing++), 11,
                                sf::Color::Green, "Human: ",
                                squady.squad.at(squady.aim).name);

            textList.createText(RESOLUTION.x / 2, 80 + (20 * spacing), 11,
                                sf::Color::White, "Strength: ", "",
                                squady.squad.at(squady.aim).skills.strength);

            SquareButton var;
            var.color = sf::Color::Red;
            var.iSizex = 5;
            var.iSizey = 5;
            var.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                    5 + (80 + (20 * spacing)));
            var.sButtonText = "Howdy";
            vSquareButtonList.push_back(var);
            if (squareButtonClicked(var.id))
            {
                if (squady.makeSquadPoints < 1000)
                {
                    squady.makeSquadPoints += 1;
                    squady.squad.at(squady.aim).skills.strength -= 1;
                }
            }
            SquareButton var2;
            var2.color = sf::Color::Red;
            var2.iSizex = 5;
            var2.iSizey = 5;
            var2.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                     5 + (80 + (20 * spacing++)));
            var2.sButtonText = "Howdy";
            vSquareButtonList.push_back(var2);
            if (squareButtonClicked(var2.id))
            {
                if (squady.makeSquadPoints > 0)
                {
                    squady.makeSquadPoints -= 1;
                    squady.squad.at(squady.aim).skills.strength += 1;
                }
            }

            textList.createText(RESOLUTION.x / 2, 80 + (20 * spacing), 11,
                                sf::Color::White, "Perception: ", "",
                                squady.squad.at(squady.aim).skills.perception);

            SquareButton var3;
            var3.color = sf::Color::Red;
            var3.iSizex = 5;
            var3.iSizey = 5;
            var3.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                     5 + (80 + (20 * spacing)));
            var3.sButtonText = "Howdy";
            vSquareButtonList.push_back(var3);
            if (squareButtonClicked(var3.id))
            {
                if (squady.makeSquadPoints < 1000)
                {
                    squady.makeSquadPoints += 1;
                    squady.squad.at(squady.aim).skills.perception -= 1;
                }
            }
            SquareButton var4;
            var4.color = sf::Color::Red;
            var4.iSizex = 5;
            var4.iSizey = 5;
            var4.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                     5 + (80 + (20 * spacing++)));
            var4.sButtonText = "Howdy";
            vSquareButtonList.push_back(var4);
            if (squareButtonClicked(var4.id))
            {
                if (squady.makeSquadPoints > 0)
                {
                    squady.makeSquadPoints -= 1;
                    squady.squad.at(squady.aim).skills.perception += 1;
                }
            }

            textList.createText(
                RESOLUTION.x / 2, 80 + (20 * spacing), 11, sf::Color::White,
                "Intelligence: ", "",
                squady.squad.at(squady.aim).skills.intelligence);

            SquareButton var5;
            var5.color = sf::Color::Red;
            var5.iSizex = 5;
            var5.iSizey = 5;
            var5.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                     5 + (80 + (20 * spacing)));
            var5.sButtonText = "Howdy";
            vSquareButtonList.push_back(var5);
            if (squareButtonClicked(var5.id))
            {
                if (squady.makeSquadPoints < 1000)
                {
                    squady.makeSquadPoints += 1;
                    squady.squad.at(squady.aim).skills.intelligence -= 1;
                }
            }
            SquareButton var6;
            var6.color = sf::Color::Red;
            var6.iSizex = 5;
            var6.iSizey = 5;
            var6.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                     5 + (80 + (20 * spacing++)));
            var6.sButtonText = "Howdy";
            vSquareButtonList.push_back(var6);
            if (squareButtonClicked(var6.id))
            {
                if (squady.makeSquadPoints > 0)
                {
                    squady.makeSquadPoints -= 1;
                    squady.squad.at(squady.aim).skills.intelligence += 1;
                }
            }

            textList.createText(RESOLUTION.x / 2, 80 + (20 * spacing), 11,
                                sf::Color::White, "Charisma: ", "",
                                squady.squad.at(squady.aim).skills.charisma);

            SquareButton var7;
            var7.color = sf::Color::Red;
            var7.iSizex = 5;
            var7.iSizey = 5;
            var7.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                     5 + (80 + (20 * spacing)));
            var7.sButtonText = "Howdy";
            vSquareButtonList.push_back(var7);
            if (squareButtonClicked(var7.id))
            {
                if (squady.makeSquadPoints < 1000)
                {
                    squady.makeSquadPoints += 1;
                    squady.squad.at(squady.aim).skills.charisma -= 1;
                }
            }
            SquareButton var8;
            var8.color = sf::Color::Red;
            var8.iSizex = 5;
            var8.iSizey = 5;
            var8.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                     5 + (80 + (20 * spacing++)));
            var8.sButtonText = "Howdy";
            vSquareButtonList.push_back(var8);
            if (squareButtonClicked(var8.id))
            {
                if (squady.makeSquadPoints > 0)
                {
                    squady.makeSquadPoints -= 1;
                    squady.squad.at(squady.aim).skills.charisma += 1;
                }
            }

            textList.createText(RESOLUTION.x / 2, 80 + (20 * spacing), 11,
                                sf::Color::White, "Endurance: ", "",
                                squady.squad.at(squady.aim).skills.endurance);

            SquareButton var9;
            var9.color = sf::Color::Red;
            var9.iSizex = 5;
            var9.iSizey = 5;
            var9.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                     5 + (80 + (20 * spacing)));
            var9.sButtonText = "Howdy";
            vSquareButtonList.push_back(var9);
            if (squareButtonClicked(var9.id))
            {
                if (squady.makeSquadPoints < 1000)
                {
                    squady.makeSquadPoints += 1;
                    squady.squad.at(squady.aim).skills.endurance -= 1;
                }
            }
            SquareButton var10;
            var10.color = sf::Color::Red;
            var10.iSizex = 5;
            var10.iSizey = 5;
            var10.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                      5 + (80 + (20 * spacing++)));
            var10.sButtonText = "Howdy";
            vSquareButtonList.push_back(var10);
            if (squareButtonClicked(var10.id))
            {
                if (squady.makeSquadPoints > 0)
                {
                    squady.makeSquadPoints -= 1;
                    squady.squad.at(squady.aim).skills.endurance += 1;
                }
            }

            textList.createText(RESOLUTION.x / 2, 80 + (20 * spacing), 11,
                                sf::Color::White, "Dexterity: ", "",
                                squady.squad.at(squady.aim).skills.dexterity);

            SquareButton var11;
            var11.color = sf::Color::Red;
            var11.iSizex = 5;
            var11.iSizey = 5;
            var11.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                      5 + (80 + (20 * spacing)));
            var11.sButtonText = "Howdy";
            vSquareButtonList.push_back(var11);
            if (squareButtonClicked(var11.id))
            {
                if (squady.makeSquadPoints < 1000)
                {
                    squady.makeSquadPoints += 1;
                    squady.squad.at(squady.aim).skills.dexterity -= 1;
                }
            }
            SquareButton var12;
            var12.color = sf::Color::Red;
            var12.iSizex = 5;
            var12.iSizey = 5;
            var12.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                      5 + (80 + (20 * spacing++)));
            var12.sButtonText = "Howdy";
            vSquareButtonList.push_back(var12);
            if (squareButtonClicked(var12.id))
            {
                if (squady.makeSquadPoints > 0)
                {
                    squady.makeSquadPoints -= 1;
                    squady.squad.at(squady.aim).skills.dexterity += 1;
                }
            }

            textList.createText(RESOLUTION.x / 2, 80 + (20 * spacing), 11,
                                sf::Color::White, "Agility: ", "",
                                squady.squad.at(squady.aim).skills.agility);

            SquareButton var13;
            var13.color = sf::Color::Red;
            var13.iSizex = 5;
            var13.iSizey = 5;
            var13.vPos = sf::Vector2f((RESOLUTION.x / 2) - 10,
                                      5 + (80 + (20 * spacing)));
            var13.sButtonText = "Howdy";
            vSquareButtonList.push_back(var13);
            if (squareButtonClicked(var13.id))
            {
                if (squady.makeSquadPoints < 1000)
                {
                    squady.makeSquadPoints += 1;
                    squady.squad.at(squady.aim).skills.agility -= 1;
                }
            }
            SquareButton var14;
            var14.color = sf::Color::Red;
            var14.iSizex = 5;
            var14.iSizey = 5;
            var14.vPos = sf::Vector2f((RESOLUTION.x / 2) + 150,
                                      5 + (80 + (20 * spacing++)));
            var14.sButtonText = "Howdy";
            vSquareButtonList.push_back(var14);
            if (squareButtonClicked(var14.id))
            {
                if (squady.makeSquadPoints > 0)
                {
                    squady.makeSquadPoints -= 1;
                    squady.squad.at(squady.aim).skills.agility += 1;
                }
            }

            //cText.CreateText(RESOLUTION.x/2,80+(20*Spacing),11,sf::Color::White," ");
            SquareButton var50;
            var50.color = sf::Color::Yellow;
            var50.iSizex = 5;
            var50.iSizey = 5;
            var50.vPos = sf::Vector2f((RESOLUTION.x / 2) + 180,
                                      5 + (80 + ((20 * spacing++) / 2)));

            var50.sForwardText = "Done";
            var50.textColor = sf::Color::White;
            var50.textSize = 11;

            vSquareButtonList.push_back(var50);
            if (squareButtonClicked(var50.id))
            {

                gCtrl.phase = "World";
                gCtrl.buildWorldTest();

                initializeFactions(13);
            }

            //DrawStuffs();
        }

        if (gCtrl.phase == "System")
        {
            class Planet
            {
            public:
                sf::Vector2f position;
                sf::Image sprite;
                float density;
                float volume;

                Planet()
                {
                    density = 100;
                    volume = 20000;
                }
            };

            //std::vector<Planet> Planetsz;
            /*
            for(int i = 0; i != Planets.size(); i++)
            {
                for(int t = 0; t != Planets.size(); t++)
                {
                    float MassDifference = Planets[i].Density/Planets[i].Volume - Planets[t].Density/Planets[t].Volume;
                    std::cout << MassDifference
                }
            }
            */
        }

        if (gCtrl.phase == "World")
        {
            cameraControls();

            gCtrl.worldLoop();

            effects.createSquare(gvars::topLeft.x, gvars::topLeft.y,
                                 gvars::topLeft.x + 300, gvars::topLeft.y + 150,
                                 sf::Color(0, 0, 0, 100));

            int id;
            int infected;
            if (gvars::mousePos.x >= 2000 || gvars::mousePos.y >= 2000 ||
                gvars::mousePos.x < 0 || gvars::mousePos.y < 0)
            {
                id = -1;
                infected = -1;
            }
            else
            {
                id = worldMap[abs_to_index(gvars::mousePos.x / 20)]
                             [abs_to_index(gvars::mousePos.y / 20)].id;
                infected = worldMap[abs_to_index(
                    gvars::mousePos.x / 20)][abs_to_index(gvars::mousePos.y /
                                                          20)].infected;
            }
            debug("Pre-World HUD");
            int hudz = 0;

            textList.createText(gvars::topLeft.x + 2,
                                gvars::topLeft.y + (hudz++) * 11, 22,
                                sf::Color::Yellow, "World Population: ", "",
                                factionPopulation());
            hudz++;
            hudz++;
            textList.createText(gvars::topLeft.x + 2,
                                gvars::topLeft.y + (hudz++) * 11, 11,
                                sf::Color::White, "CurrentTileID: ", "", id);
            textList.createText(
                gvars::topLeft.x + 2, gvars::topLeft.y + (hudz++) * 11, 11,
                sf::Color::White, "CurrentTileInfected: ", "", infected);
            textList.createText(
                gvars::topLeft.x + 2, gvars::topLeft.y + (hudz++) * 11, 11,
                sf::Color::White, "FactionOwned: ",
                worldMap[math::clamp(abs(gvars::mousePos.x / 20), 0, 99)]
                        [math::clamp(abs(gvars::mousePos.y / 20), 0, 99)]
                            .owner);
            textList.createText(
                gvars::topLeft.x + 2, gvars::topLeft.y + (hudz++) * 11, 11,
                sf::Color::White, "FactionMembers: ", "",
                factionMembers(worldMap[math::clamp(
                    abs(gvars::mousePos.x / 20), 0,
                    99)][math::clamp(abs(gvars::mousePos.y / 20), 0, 99)]
                                   .owner));
            textList.createText(
                gvars::topLeft.x + 2, gvars::topLeft.y + (hudz++) * 11, 11,
                sf::Color::White, "FactionAggression: ", "",
                factionAggression(worldMap[math::clamp(
                    abs(gvars::mousePos.x / 20), 0,
                    99)][math::clamp(abs(gvars::mousePos.y / 20), 0, 99)]
                                      .owner));
            textList.createText(
                gvars::topLeft.x + 2, gvars::topLeft.y + (hudz++) * 11, 11,
                sf::Color::White, "FactionTerritories: ", "",
                factionTerritories(worldMap[math::clamp(
                    abs(gvars::mousePos.x / 20), 0,
                    99)][math::clamp(abs(gvars::mousePos.y / 20), 0, 99)]
                                       .owner));
            textList.createText(
                gvars::topLeft.x + 2, gvars::topLeft.y + (hudz++) * 11, 11,
                sf::Color::White, "FactionPower: ", "",
                factionPower(worldMap[math::clamp(
                    abs(gvars::mousePos.x / 20), 0,
                    99)][math::clamp(abs(gvars::mousePos.y / 20), 0, 99)]
                                 .owner));

            textList.createText(gvars::topLeft.x + 2,
                                gvars::topLeft.y + (hudz++) * 11, 11,
                                sf::Color::White, "AimedPos(DELETEME): ", "",
                                abs(gvars::mousePos.x / 20), "/", "",
                                abs(gvars::mousePos.y / 20));

            drawWorldTiles();
            int xMouse(gvars::mousePos.x / 20);
            int yMouse(gvars::mousePos.y / 20);
            sf::Vector2f pos(xMouse, yMouse);
            effects.createSquare(pos.x * 20, pos.y * 20, (pos.x * 20) + 20,
                                 (pos.y * 20) + 20, sf::Color(0, 0, 0, 0), 2,
                                 sf::Color(0, 200, 200, 255));

            if (inputState.key[Key::C] && true == false)
            { // Not sure what this was for, Perhaps an early alpha planet builder?
                for (int x = 0; x != GRIDS; x++)
                {
                    for (int y = 0; y != GRIDS; y++)
                    {
                        if (tiles[x][y][0].id == 1001)
                        {
                            sf::Image grazz;
                            grazz.create(20, 20, sf::Color(0, 0, 0, 255));
                            for (int i = 0; i != 20; i++)
                            {
                                for (int t = 0; t != 20; t++)
                                {
                                    int pix = randz(0, 1);
                                    if (pix == 0)
                                        grazz.setPixel(
                                            i, t, sf::Color(0, 255, 0, 255));
                                    if (pix == 1)
                                        grazz.setPixel(
                                            i, t, sf::Color(0, 128, 0, 255));
                                    grazz.setPixel(i, t,
                                                   sf::Color(0, 128, 0, 255));
                                }
                            }
                            sf::Texture why;
                            why.loadFromImage(grazz);
                            //                Tiles[x][y][0].Img.setTexture(Why);
                        }
                    }
                }

                con("Starting Now");
                gCtrl.addvPlanet();
            }

            if (inputState.rmb == true)
            { // Entering Menu and such.
                debug("After RMB");

                gCtrl.menuType = "WorldRMB";
                menuPopUp();
            }
            debug("Done with world.");
            if (gCtrl.menuType != "NULL")
            {
                menuPopUp();
            }
            else
            {
                gCtrl.menuPos = math::Vec2f(-10000, -10000);
            }
        }

        if (gCtrl.phase == "MainMenu")
        { //=======================================================*Main Menu*============================================================================
            gCtrl.buildMainMenu();
            gvars::view1.setCenter(HALF_SIZE.x, HALF_SIZE.y);
            textList.createText(500, 0, 25, sf::Color::White, "Welcome!", "",
                                -6698, "", "", -6698, "", "", -6698, 1, 0);
            textList.createText(
                450, 25, 11, sf::Color::Red,
                "Take this 'game' with a grain of salt, It's not done yet");
            textList.createText(450, 45, 11, gvars::cycleGreen,
                                "Design Your Squad");

            textList.createText(450, 60, 11, sf::Color::Red,
                                "Press 'm' to view MicroPather basic test");

            // TODO: Simply add cText.CreateText for the Button Text, Or at least make it an option, Since sButtonText is designed for text 'on' the button.
            SquareButton var;
            var.color = gvars::cycleRed;
            var.iSizex = 10;
            var.iSizey = 6;

            var.vPos = sf::Vector2f(440, 52);
            var.sButtonText = "Howdy";
            vSquareButtonList.push_back(var);

            textList.createText(450, 75, 11, sf::Color::Red,
                                "Press r to turn on the "
                                "debugger, If it slows down the "
                                "game, Minimize the console.");
            textList.createText(395, 755, 10, sf::Color::White,
                                "Pathfinding: MicroPather by Lee Thomason");
            /* textList.createText(gvars::mousePos.x,gvars::mousePos.y-10,10,sf::Color::White,"X: "+std::to_string(gvars::mousePos.x)+" Y: "+std::to_string(gvars::mousePos.y)); */

            if (inputState.key[Key::M] && !network::chatting)
            {
                gCtrl.phase = "MicroPatherTest";
                gvars::currentx = 14;
                gvars::currenty = 14;
            }

            if (squareButtonClicked(var.id))
            {
                if (gvars::debug)
                    std::cout << "Switching to MakeSquad\n";
                gCtrl.phase = "MakeSquad";

                for (int amount = 0; amount != squady.squadMates; amount++)
                {

                    Npc var;
                    var = *getGlobalCritter("Human");
                    var.id = gvars::globalid++;
                    var.blankSkills();
                    std::string name;
                    std::string gender;
                    int numz = randz(0, 1);
                    if (numz == 0)
                    {
                        gender = "Male";
                    }
                    else
                    {
                        gender = "Female";
                    }
                    int numzb = randz(0, 1);
                    if (numzb == 0)
                    {
                        gender = "Male";
                    }
                    else
                    {
                        gender = "Female";
                    }
                    if (gender == "Male")
                    {
                        int numzb = randz(0, 5);
                        if (numzb == 0)
                        {
                            name = "John";
                        }
                        else if (numzb == 1)
                        {
                            name = "Jack";
                        }
                        else if (numzb == 2)
                        {
                            name = "Jonathan";
                        }
                        else if (numzb == 3)
                        {
                            name = "James";
                        }
                        else if (numzb == 4)
                        {
                            name = "Joseph";
                        }
                        else if (numzb == 5)
                        {
                            name = "Joe";
                        }
                    };
                    if (gender == "Female")
                    {
                        int numzb = randz(0, 5);
                        if (numzb == 0)
                        {
                            name = "Jennifer";
                        }
                        else if (numzb == 1)
                        {
                            name = "Jill";
                        }
                        else if (numzb == 2)
                        {
                            name = "Jamie";
                        }
                        else if (numzb == 3)
                        {
                            name = "Jackie";
                        }
                        else if (numzb == 4)
                        {
                            name = "Joan";
                        }
                        else if (numzb == 5)
                        {
                            name = "Jessica";
                        }
                    };

                    var.name = name;
                    var.gender = gender;

                    squady.squad.push_back(var);
                }
            }
            if (inputState.key[Key::Comma] && !network::chatting)
            {
                if (gvars::debug)
                    std::cout << "Comma was pressed \n";
                gCtrl.phase = "Local";
                if (gvars::debug)
                    std::cout << "Building Local Test\n";
                gCtrl.buildLocalTest();
                if (gvars::debug)
                    std::cout << "Done Building Local Test\n";
            }
            if (inputState.key[Key::Period] && !network::chatting)
            {
                gCtrl.phase = "World";
                gCtrl.buildWorldTest();
            }
            if (inputState.key[Key::RControl] && !network::chatting)
            {
                gCtrl.phase = "Test";
            }
        } //=============================================================================*End of Main Menu*========================================================================

}

void scaleImages()
{
    if (inputState.key[Key::Numpad7] && !network::chatting)
        {
            gvars::scalex += 0.1;
            fSleep(0.1);
        }
        if (inputState.key[Key::Numpad8] && !network::chatting)
        {
            gvars::scaley += 0.1;
            fSleep(0.1);
        }
        if (inputState.key[Key::Numpad4] && !network::chatting)
        {
            gvars::scalex -= 0.1;
            fSleep(0.1);
        }
        if (inputState.key[Key::Numpad5] && !network::chatting)
        {
            gvars::scaley -= 0.1;
            fSleep(0.1);
        }

}

void cleanMenu()
{
    if (gCtrl.menuEndPos == sf::Vector2f(-10000, -10000))
        {
            gCtrl.menuPtrCon.pItem = nullptr;
            gCtrl.menuPtrCon.pNPC = nullptr;
            gCtrl.menuPtrCon.pTile = nullptr;
            gCtrl.menuPtrCon.pVecItem.clear();
            gCtrl.menuPtrCon.pVecNPC.clear();
            gCtrl.menuPtrCon.pVecTile.clear();
        }
        debug("Finished removing process");

        if (inputState.lmb && gvars::buttonClickedTime == 0 &&
            !aabb(gvars::mousePos, gCtrl.menuPos.x, gCtrl.menuEndPos.x,
                  gCtrl.menuPos.y, gCtrl.menuEndPos.y) &&
            gCtrl.menuPos != sf::Vector2f(-10000, -10000))
        {
            gCtrl.menuType = "NULL";
            gCtrl.menuPos = sf::Vector2f(-10000, -10000);
            gCtrl.menuEndPos = sf::Vector2f(-10000, -10000);
            gCtrl.menuPtrCon.pItem = nullptr;
            gCtrl.menuPtrCon.pNPC = nullptr;
            gCtrl.menuPtrCon.pTile = nullptr;
            con("Closing Menus Due To Outside Clicking");
        }
}


void newItemstuffs()
{
    std::cout << "Kaboom! \n";
    makeItems(itemlist,1);
    printItems(itemlist);
    std::cout << "itemlist maxdam: " << totalDamageofItems(itemlist) << std::endl;

    for( auto &item : itemlist)
    {
        std::cout << "====== \n";
        itemPtrVector IPV;
        IPV = makeItems(item.internalitems,3);
        for ( auto &internal : IPV.ptrs)
        {
            makeItems(internal->internalitems,3);
            std::cout << "X \n";
            printItems(internal->internalitems);
            std::cout << "X \n";
        }
        printItems(item.internalitems);
        std::cout << "====== \n";
    }
    std::cout << "Giga: " << totalDamageofItemsInternalized(itemlist) << std::endl;
}

void playThemeTrack()
{
    //gvars::soundVolume = 0;
    gvars::musicVolume = 0;
    int ranNum = randz(1,3);
    if(ranNum == 1)
        playMusic("Electro_Cabello.ogg");
    else if(ranNum == 2)
        playMusic("Neo Western.ogg");
    else if(ranNum == 3)
        playMusic("Jalandhar.ogg");
}

int main()
{
    soundmanager.init();
    initializeMusic();
    playThemeTrack();

    srand(clock());
    texturemanager.init();

    itemmanager.initializeItems();
    npcmanager.initializeCritters();


    galaxySetup();
    //bountyTowerSetup();

    window.create(sf::VideoMode(RESOLUTION.x, RESOLUTION.y, 32), randomWindowName());
    window.setVerticalSyncEnabled(true);

    textList.loadFont();

    soundmanager.playSound("Startup.wav");
    newItemstuffs();


    while (window.isOpen())
    {
        gvars::hovering = false;
        if (inputState.key[Key::R] && !network::chatting)
        {
            toggle(gvars::debug);
            fSleep(0.5);
        }

        if(inputState.key[Key::U])
        {
            soundmanager.playSound("Startup.wav");
        }

        //std::cout << "soundstuffs: " << soundmanager.playSounds.size() << std::endl;

        frames();
        scaleImages();
        handleEvents();

        genericLoop();
        galaxyLoop();

        handlePhase();

        acquireSelectedNPCs();
        selectedNPCprocess();

        debug("Pre Draw Stuffs");
        hoverItemIDdisplay();
        drawStuffs();
        window.display();
        window.clear();
        debug("Post Draw Stuffs");

        debug("Starting Removing process, NPC/Unpoint/Items/GC.Menu");

        AnyDeletes(soundmanager.playSounds);

        removeNPCs(npclist, mutex::npcList);
        unpointItems(worlditems);
        removeItems(worlditems);
        soundmanager.cleanSounds();
        cleanMenu();
    } // End of game loop
    return EXIT_SUCCESS;
}
