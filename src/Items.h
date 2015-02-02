#ifndef __ITEMS_H_INCLUDED__
#define __ITEMS_H_INCLUDED__

#include <list>

#include <SFML/Graphics.hpp>

class NPC;
class Item
{
public:
    bool toDelete;
    bool pickupable;
    sf::Sprite img;
    int id;
    int cbaseid;
    std::string name;
    std::string inventor;
    std::string contains;
    // This is to only be filled with a part name if it is put inside
    // a creature, otherwise, it is to be left blank. This item is
    // meant to be put inside a critters inventory,
    // All inventory checks are to ignore instances where this has
    // more than .size() = 0, Opposite is true for items contained in critter.
    std::string insidePart;
    int hasInternalUse;

    NPC *user;

    int range;
    int age;
    int weight;
    int value;
    sf::Vector2f targetPos;
    float xpos;
    float ypos;
    float zpos;
    int rxpos;
    int rypos;
    int gxpos;
    int gypos;
    int imgstrx;
    int imgstry;
    int imgendx;
    int imgendy;
    bool isWeapon;
    // 0 = none, 1 = melee weapon, 2 = ranged weapon,
    // 3 = thrown weapon, 4 = food, 5 = water, 6 = cell, 7 = Ore,
    // 8 = Digging Tool, 9 = Cutting Tool, 10 = Fishing Tool,
    // 11 = Liquid Container, 12 = Plant, 13 = ?
    int type;
    // 0 = none, 1 = hand gun, 2 = rifle, 3 = slash, 4 = blunt, 5 = pierce
    std::string usedskill;
    float amount;
    // 0 None 1 Chem 2 Solar 3 Nuclear 4 Soul 5 Magic
    int celltype;
    int cellcharge;
    int cellcost;
    int tillrot;

    int hungervalue;
    int massMetal;
    int massFlesh;
    int massVeggy;
    int massGlass;
    int massPlastic;
    int massWater;
    int massOil;

    int thirstvalue;
    int mindam;
    int maxdam;
    int ammotype;
    int maxclip;
    int currentclip;
    bool hasCell;
    bool useCell;
    bool isMagic;
    bool stacks;
    bool rotten;
    bool isLiquidContainer;
    bool isObjectContainer;
    bool pushable;
    bool blocksmovement;
    bool collectssolar;
    bool collectsheat;
    bool makeslight;
    bool needspower;
    bool produces;
    int prodrate;
    int prodratetimer;
    std::string produce;
    std::string matreqprod;
    bool needmatprod;
    bool deconstrutionauto;
    int timerdecon;
    bool buried;
    bool hidden;
    bool ProdOn;
    bool IsOn;
    bool Sleepable;
    int liquidContainMax;
    int objectContainMax;
    int containAmount;
    void chargecell(int amount);
    void soulcell(int soultype);
    bool getid(int idz);
    bool boolname(char *cakezebra);
    void printInfo();
    void drawImg();
    void spawn(int gposx, int gposy, int rposx, int rposy, int posx, int posy);
    void printConsoleInfo();
    Item();
};

Item *GetGlobalItem(std::string strtype);

extern std::list<Item> worlditems;

class cItemManager
{
public:
    std::vector<Item> GlobalItem;
    std::vector<Item> AddedItems;

    void AddItems();

    void InitializeItems();
};

extern cItemManager itemmanager;

void RemoveItems(std::list<Item> &Items);

void zSaveItem(int planet, sf::Vector2i Region, Item &object);

void SaveItem(int planet, sf::Vector2i Region, Item &Critter);

std::string LoadItems(sf::Vector2i WorldPos, std::string Direction,
                      int planet = 500);

void SpawnItem(std::string Object, int xpos, int ypos);

void RefreshImages();

Item *GetItemPtrfromVector(std::list<Item> &Vector, std::string Name);

Item *GetItemPtrfromVector2(std::list<Item> &Vector, std::string Name);

Item *GetItemPtrfromVectorVarSearch(std::list<Item> &Vector,
                                    std::string VarSearch, float AtLeast = 1);

Item *FindClosestItemPtr(int Orix, int Oriy, std::string TarItem, int Gxpos = 0,
                         int Gypos = 0, int Rxpos = 0, int Rypos = 0);

#endif // ITEMS_H_INCLUDED
