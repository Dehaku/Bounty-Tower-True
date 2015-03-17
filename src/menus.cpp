#include "menus.h"

#include "Effects.h"
#include "Text.h"
#include "InputState.h"
#include "Galaxy.h"
#include "FactionJobSecurity.h"
#include "Faction.h"
#include "math.h"
#include "util.h"
#include "globalvars.h"

#include <iostream>

std::vector<Button> vButtonList;
std::vector<SquareButton> vSquareButtonList;
MenuPointerContainer menuPtrCon;

using std::abs;

template <typename T> T &listAt(std::list<T> &list, size_t index)
{
    auto it = list.begin();
    std::advance(it, index);
    return *it;
}

void Button::draw()
{
    if (beenPressed == true)
    {
        //Effectz.CreateCircle(vPos.x,vPos.y,iSize,sf::Color(Color.r/2,Color.g/2,Color.b/2),2,White);
        effects.createSquare(vPos.x - iSize, vPos.y - (iSize / 1.5),
                             vPos.x + iSize, vPos.y + (iSize / 1.5),
                             sf::Color(color.r / 2, color.g / 2, color.b / 2),
                             2, sf::Color::White);
    }
    else if (math::closeish(gvars::mousePos.x, gvars::mousePos.y, vPos.x,
                            vPos.y) < iSize)
    {
        //Effectz.CreateCircle(vPos.x,vPos.y,iSize,Color,2,White);
        effects.createSquare(vPos.x - iSize, vPos.y - (iSize / 1.5),
                             vPos.x + iSize, vPos.y + (iSize / 1.5), color, 2,
                             sf::Color::White);
    }
    else
    {
        //Effectz.CreateCircle(vPos.x,vPos.y,iSize,Color,1,Black);
        effects.createSquare(vPos.x - iSize, vPos.y - (iSize / 1.5),
                             vPos.x + iSize, vPos.y + (iSize / 1.5), color, 2,
                             sf::Color::Black);
    }
    textList.createText(vPos.x + 10, vPos.y - (textSize / 2), textSize,
                        textColor, sForwardText);
    effects.drawEffects();
}

Button::Button() : iSize{}, textSize{}
{
    beenPressed = false;
    id = gvars::glbbtn++;
}

void SquareButton::draw()
{
    if (beenPressed == true)
    {
        //Effectz.CreateCircle(vPos.x,vPos.y,iSize,sf::Color(Color.r/2,Color.g/2,Color.b/2),2,White);
        effects.createSquare(vPos.x - iSizex, vPos.y - iSizey, vPos.x + iSizex,
                             vPos.y + iSizey,
                             sf::Color(color.r / 2, color.g / 2, color.b / 2),
                             2, sf::Color::White);
    }
    else if (aabb(gvars::mousePos, vPos.x - iSizex, vPos.x + iSizex,
                  vPos.y - iSizey, vPos.y + iSizey))
    {
        //Effectz.CreateCircle(vPos.x,vPos.y,iSize,Color,2,White);
        effects.createSquare(vPos.x - iSizex, vPos.y - iSizey, vPos.x + iSizex,
                             vPos.y + iSizey, color, 2, sf::Color::White);
        if (gvars::mouseStagnation > 10 && sButtonText.length() != 0)
        {
            effects.createSquare(gvars::mousePos.x + 10, gvars::mousePos.y - 6,
                                 gvars::mousePos.x + (sButtonText.length() * 7),
                                 gvars::mousePos.y + 6, sf::Color::Black, 1,
                                 sf::Color(175, 175, 0));
            textList.createText(gvars::mousePos.x + 12, gvars::mousePos.y - 6,
                                11, textColor, sButtonText);
        }
    }
    else
    {
        //Effectz.CreateCircle(vPos.x,vPos.y,iSize,Color,1,Black);
        effects.createSquare(vPos.x - iSizex, vPos.y - iSizey, vPos.x + iSizex,
                             vPos.y + iSizey, color, 2, sf::Color::Black);
    }
    textList.createText(vPos.x + 10, vPos.y - (textSize / 2), textSize,
                        textColor, sForwardText);
    effects.drawEffects();
}

SquareButton::SquareButton() : iSizex{}, iSizey{}, textSize{}
{
    beenPressed = false;
    textColor = sf::Color(175, 175, 0);
    id = gvars::glbbtn++;
}

void Buttons::pressed()
{
    for (auto const &button : vButtonList)
    {
        if (math::closeish(gvars::mousePos.x, gvars::mousePos.y, button.vPos.x,
                           button.vPos.y) < button.iSize)
        {
            std::cout << "Button Click!\n";
        }
    }
}

void SquareButtons::pressed()
{
    for (auto const &button : vSquareButtonList)
    {
        if (aabb(gvars::mousePos, button.vPos.x - button.iSizex,
                 button.vPos.x + button.iSizex, button.vPos.y - button.iSizey,
                 button.vPos.y + button.iSizey))
        {
            std::cout << "Button Click!\n";
            // Is this never called? Is this class useless?
        }
    }
}

int createButton(sf::Vector2f vPos, int iSize, sf::Color color,
                 std::string text)
{
    Button var;
    var.vPos = vPos;
    var.iSize = iSize;
    var.color = color;
    var.sButtonText = text;
    vButtonList.push_back(var);
    return var.id;
}

int createSquareButton(sf::Vector2f vPos, int iSizex, int iSizey,
                       sf::Color color, std::string text)
{
    SquareButton var;
    var.vPos = vPos;
    var.iSizex = iSizex;
    var.iSizey = iSizey;
    var.color = color;
    var.sButtonText = text;
    vSquareButtonList.push_back(var);
    return var.id;
}

bool buttonClicked(int id)
{
    for (auto &button : vButtonList)
    {
        if (button.id == id)
        {
            if (math::closeish(gvars::mousePos.x, gvars::mousePos.y,
                               button.vPos.x, button.vPos.y) < button.iSize &&
                (inputState.lmbTime == 1 || inputState.lmbTime > 20))
            {
                button.beenPressed = true;
                std::cout << "Pressed! \n";
                gvars::buttonClicked = true;
                return true;
            }
        }
    }
    return false;
}

bool squareButtonClicked(int id)
{
    for (auto &button : vSquareButtonList)
    {
        if (button.id == id)
        {
            if (aabb(gvars::mousePos, button.vPos.x - button.iSizex,
                     button.vPos.x + button.iSizex,
                     button.vPos.y - button.iSizey,
                     button.vPos.y + button.iSizey) &&
                (inputState.lmbTime == 1 || inputState.lmbTime > 20))
            {
                button.beenPressed = true;
                std::cout << "Pressed! \n";
                gvars::buttonClicked = true;
                gvars::buttonClickedTime = 3;
                return true;
            }
        }
    }
    return false;
}

MenuPointerContainer::MenuPointerContainer()
{
    pItem = nullptr;
    pNPC = nullptr;
    pTile = nullptr;
}

// TODO: Add functionality to allow you to press 1-9
// to activate the menu buttons.
void menuPopUp()
{

    if (gCtrl.menuPos.x == -10000)
    {
        gCtrl.menuPos = gvars::mousePos;
    }

    sf::Vector2f tiled(abs(gCtrl.menuPos.x / 20) * 20,
                       abs(gCtrl.menuPos.y / 20) * 20);

    effects.createSquare(tiled.x, tiled.y, tiled.x + 20, tiled.y + 20,
                         sf::Color::Transparent, 1, sf::Color::Cyan);

    if (gCtrl.menuType == "Orbital Drop")
    {

        int options = 8;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 150,
                                        (gCtrl.menuPos.y + (options * 13)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::Black, 2, sf::Color::Cyan);
        int iY = 0;
        int brd = 140;                      // ButtonRightDisplacement.
        int bs = 7;                         // ButtonSize;
        int bsy = 5;                        // ButtonSize;
        int mbd = 8;                        // MoveButtonDown
        sf::Color butCol = sf::Color::Cyan; // ButtonColor.

        for (int i = 0; i != options; i++)
        {

            if (i == 0)
            {
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Orbital Drop - Missle");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Strikes the location!");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num1].time == 1)
                {
                    effects.createSquare(gCtrl.menuPos.x - 10,
                                         gCtrl.menuPos.y - 2,
                                         gCtrl.menuPos.x + 10,
                                         gCtrl.menuPos.y + 2, sf::Color::Black);
                    effects.createSquare(
                        gCtrl.menuPos.x - 2, gCtrl.menuPos.y - 10,
                        gCtrl.menuPos.x + 2, gCtrl.menuPos.y + 10,
                        sf::Color::Black);
                    Item var;
                    var = *getGlobalItem("Missile");
                    var.targetPos = sf::Vector2f(gCtrl.menuPos);
                    var.xpos = gCtrl.menuPos.x - 200;
                    var.ypos = gCtrl.menuPos.y - 200;
                    var.zpos = 100;
                    worlditems.push_back(var);

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }
        }
    }

    if (gCtrl.menuType == "BuildStructure")
    {

        int options = 8;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 150,
                                        (gCtrl.menuPos.y + (options * 13)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::Black, 2, sf::Color::Cyan);
        int iY = 0;
        int brd = 140;                      // ButtonRightDisplacement.
        int bs = 7;                         // ButtonSize;
        int bsy = 5;                        // ButtonSize;
        int mbd = 8;                        // MoveButtonDown
        sf::Color butCol = sf::Color::Cyan; // ButtonColor.

        for (int i = 0; i != options; i++)
        {

            if (i == 0)
            {
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Build - Wall");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Makes a Wall!");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num1].time == 1)
                {
                    rmbMenuTile(    Vec3(gCtrl.menuPos.x,gCtrl.menuPos.y,gvars::currentz*20)   );
                }
            }

            if (i == 1)
            {
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Dig - Natural Wall");
                int Butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Digs out a natural wall.");
                if (squareButtonClicked(Butt) ||
                    inputState.key[Key::Num1].time == 1)
                {
                    digWall(Vec3(gCtrl.menuPos.x,gCtrl.menuPos.y,gvars::currentz*20));
                }
            }

            iY++;
        }
    }

    if (gCtrl.menuType == "BlankRMB")
    {

        int options = 8;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 150,
                                        (gCtrl.menuPos.y + (options * 13)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::Black, 2, sf::Color::Cyan);
        int iY = 0;
        int brd = 140;                      // ButtonRightDisplacement.
        int bs = 7;                         // ButtonSize;
        int bsy = 5;                        // ButtonSize;
        int mbd = 8;                        // MoveButtonDown
        sf::Color butCol = sf::Color::Cyan; // ButtonColor.

        for (int i = 0; i != options; i++)
        {

            if (i == 0)
            {

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Build");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol,
                    "This opens the menu to build various structures!");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num1].time == 1)
                {
                    gCtrl.menuType = "BuildStructure";
                    //fSleep(0.2);
                    return;
                }
            }
            if (i == 1)
            {

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Orbital Drop");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Causes an orbital ship to release "
                                     "specific cargo at target location.");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num2].time == 1)
                {

                    gCtrl.menuType = "Orbital Drop";
                    //fSleep(0.2);
                    return;
                }
            }
            if (i == 2 && myTargetPtr != nullptr)
            {

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Critter - Move");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Makes currently selected fella move to the menu position.");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num3].time == 1)
                {

                    Job job;
                    job.name = "MovetoSelectedLocation";
                    job.type = "Move";
                    job.workPos.x = gCtrl.menuPos.x;
                    job.workPos.y = gCtrl.menuPos.y;
                    job.workPos.z = gvars::currentz*GRID_SIZE;

                    if(myTargetPtr->jobPtr != nullptr)
                    {
                        myTargetPtr->jobPtr->pWorker = nullptr;
                        myTargetPtr->jobPtr = nullptr;
                    }

                    job.pWorker = myTargetPtr;
                    int jobListSize = myTargetPtr->factionPtr->jobList.size();

                    myTargetPtr->factionPtr->jobList.push_back(job);
                    myTargetPtr->jobPtr = &listAt(myTargetPtr->factionPtr->jobList,jobListSize);

                    return;
                }
            }



            if (i == -5)
            {

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Dig");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Dig out the current wall tile");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num2].time == 1)
                {

                    gCtrl.menuType = "Dig";
                    //fSleep(0.2);
                    return;
                }
            }

            iY++;
        }
    }

    if (gCtrl.menuType == "CritterRMB")
    {
        gCtrl.menuPos = sf::Vector2f(-10000, -10000);
        gCtrl.menuEndPos = sf::Vector2f(-10000, -10000);
        gCtrl.menuType = "NULL";
    }

    if (gCtrl.menuType == "ItemContext")
    {

        int options = 8;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 150,
                                        (gCtrl.menuPos.y + (options * 13)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::Black, 2, sf::Color::Cyan);
        int iY = 0;
        int brd = 140;                      // ButtonRightDisplacement.
        int bs = 7;                         // ButtonSize;
        int bsy = 5;                        // ButtonSize;
        int mbd = 8;                        // MoveButtonDown
        sf::Color butCol = sf::Color::Cyan; // ButtonColor.

        for (int i = 0; i != options; i++)
        {

            if (i == 0)
            {
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                if (gCtrl.menuPtrCon.pItem->pickupable)
                    textList.createText(
                        gCtrl.menuPos.x + 2, gCtrl.menuPos.y + (iY * 13), 12,
                        sf::Color::White,
                        gCtrl.menuPtrCon.pItem->name + " - PickUp");
                else
                    textList.createText(
                        gCtrl.menuPos.x + 2, gCtrl.menuPos.y + (iY * 13), 12,
                        sf::Color::White,
                        gCtrl.menuPtrCon.pItem->name +
                            " - xPickUpx, Cannot be picked up.");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Makes someone pickup this item.");
                if ((squareButtonClicked(butt) &&
                     gCtrl.menuPtrCon.pItem->pickupable) ||
                    (inputState.key[Key::Num1].time == 1 &&
                     gCtrl.menuPtrCon.pItem->pickupable))
                {

                    for (auto &i : uniFact)
                    {
                        if (i.playerControlled)
                        {
                            Job job;
                            job.pItem = gCtrl.menuPtrCon.pItem;

                            job.name = "PickUpItem";
                            job.type = "PickUp";

                            i.jobList.push_back(job);
                        }
                    }
                }
            }

            if (i == 1)
            {
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                if (!gCtrl.menuPtrCon.pItem->pickupable)
                    textList.createText(
                        gCtrl.menuPos.x + 2, gCtrl.menuPos.y + (iY * 13), 12,
                        sf::Color::White,
                        gCtrl.menuPtrCon.pItem->name + " - ChopDown");
                else
                    textList.createText(
                        gCtrl.menuPos.x + 2, gCtrl.menuPos.y + (iY * 13), 12,
                        sf::Color::White,
                        gCtrl.menuPtrCon.pItem->name +
                            " - xChopDownx, Cannot be chopped up.");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol,
                    "Makes someone seek and destroy this item.");
                if ((squareButtonClicked(butt) &&
                     !gCtrl.menuPtrCon.pItem->pickupable) ||
                    (inputState.key[Key::Num1].time == 1 &&
                     !gCtrl.menuPtrCon.pItem->pickupable))
                {

                    for (auto &i : uniFact)
                    {
                        if (i.playerControlled)
                        {
                            Job job;
                            job.pItem = gCtrl.menuPtrCon.pItem;

                            job.name = "ChopDownTree";
                            job.type = "Chop";

                            i.jobList.push_back(job);
                        }
                    }
                }
            }

            iY++;
        }
    }

    if (gCtrl.menuType == "ItemRMB")
    {
        int options = 8;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 150,
                                        (gCtrl.menuPos.y + (options * 13)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::Black, 2, sf::Color::Cyan);
        int iY = 0;
        int brd = 140;                      // ButtonRightDisplacement.
        int bs = 7;                         // ButtonSize;
        int bsy = 5;                        // ButtonSize;
        int mbd = 8;                        // MoveButtonDown
        sf::Color butCol = sf::Color::Cyan; // ButtonColor.

        for (size_t i = 0; i != gCtrl.menuPtrCon.pVecItem.size(); i++)
        {

            effects.createLine(
                gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                sf::Color::Cyan);
            textList.createText(
                gCtrl.menuPos.x + 2, gCtrl.menuPos.y + (iY * 13), 12,
                sf::Color::White, gCtrl.menuPtrCon.pVecItem[i]->name);
            int butt = createSquareButton(
                math::Vec2f(gCtrl.menuPos.x + brd,
                            (gCtrl.menuPos.y + (iY * 13)) + mbd),
                bs, bsy, butCol, "Strikes the location!");
            if (squareButtonClicked(butt))
            {

                gCtrl.menuType = "ItemContext";
                gCtrl.menuPtrCon.pItem = gCtrl.menuPtrCon.pVecItem[i];
                gCtrl.menuPtrCon.pVecItem.clear();
                return;
            }

            if (/*i == -1*/ false) // `i` can't be -1
            {
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(
                    gCtrl.menuPos.x + 2, gCtrl.menuPos.y + (iY * 13), 12,
                    sf::Color::White, gCtrl.menuPtrCon.pVecItem[0]->name);
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Strikes the location!");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num1].time == 1)
                {

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }

            iY++;
        }
    }

    if (gCtrl.menuType == "WorldRMB")
    {
        int options = 1;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 150,
                                        (gCtrl.menuPos.y + (options * 13)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::Black, 2, sf::Color::Cyan);
        int iY = 0;
        int brd = 140;                      // ButtonRightDisplacement.
        int bs = 7;                         // ButtonSize;
        int bsy = 5;                        // ButtonSize;
        int mbd = 8;                        // MoveButtonDown
        sf::Color butCol = sf::Color::Cyan; // ButtonColor.

        for (int i = 0; i != options; i++)
        {

            if (i == 0)
            {

                if (worldMap[abs_to_index(gCtrl.menuPos.x / 20)]
                            [abs_to_index(gCtrl.menuPos.y / 20)].id == 0)
                    effects.createLine(
                        gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                        gCtrl.menuPos.x + 90,
                        (gCtrl.menuPos.y + (iY * 13)) + 13, 1, sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Enter city");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Welcome to the jungle baby.");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num1].time == 1)
                {
                    /*
                    generateChunk("Building", 500,
                                  sf::Vector2i(abs(gCtrl.menuPos.x / 20),
                                               abs(gCtrl.menuPos.y / 20)));
                    */
                    gvars::currentregionx = abs(gCtrl.menuPos.x / 20);
                    gvars::currentregiony = abs(gCtrl.menuPos.y / 20);
                    gCtrl.phase = "Local";


                    buildLocalfromWorld(sf::Vector2i(
                        abs(gCtrl.menuPos.x / 20), abs(gCtrl.menuPos.y / 20)));
                    initializeTilePositions();

                    tiles[32+11][32+15][30].teleportPad(Vec3(32+21,32+15,30));
                    tiles[32+21][32+15][30].teleportPad(Vec3(32+11,32+15,30));

                    buildStartingCritters(worldMap[abs_to_index(
                        gCtrl.menuPos.x / 20)][abs_to_index(gCtrl.menuPos.y /
                                                            20)].infected);
                    spawnItem("Sword", 990, 1020);

                    spawnItem("Gun", 1010, 1020);

                    spawnItem("Wood", (32+12)*20+3,(32+12)*20+3);
                    spawnItem("Wood", (32+12)*20-3,(32+12)*20+3);
                    spawnItem("Wood", (32+12)*20+3,(32+12)*20-3);
                    spawnItem("Wood", (32+12)*20-3,(32+12)*20-3);
                    spawnItem("Wood", (32+12)*20,(32+12)*20);

                    con("Hunting for Trees 4");
                    for (int i = 0; i != 8; i++)
                    {
                        int Tx = randz(50, 1950);
                        int Ty = randz(50, 1950);
                        con(Tx, false);
                        con(":", false);
                        con(Ty, false);
                        if (tiles[abs_to_index(Tx / 20)][abs_to_index(
                                Ty / 20)][30].id == 1001 ||
                            tiles[abs_to_index(Tx / 20)][abs_to_index(
                                Ty / 20)][30].id == 1003)
                        {
                            con(", Is Plantable.");
                            spawnItem("Fruit Tree", Tx, Ty);
                        }
                    }

                    spawnItem("Broken Vending Machine", 1030, 1030);

                    spawnCritter("Mini Turret", 1000, 1070, 30*20);

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
                iY++;
            }
        }
    }

    if (gCtrl.menuType == "DebugFunctions")
    {

        int options = 10;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 150,
                                        (gCtrl.menuPos.y + (options * 13)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::Black, 2, sf::Color::Cyan);
        int iY = 0;
        int brd = 140;                      // ButtonRightDisplacement.
        int bs = 7;                         // ButtonSize;
        int bsy = 5;                        // ButtonSize;
        int mbd = 8;                        // MoveButtonDown
        sf::Color butCol = sf::Color::Cyan; // ButtonColor.

        for (int i = 0; i != options; i++)
        {

            if (i == 0)
            {
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Close Menu");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Close Menu");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num1].time == 1)
                {
                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }
            if (i == 1)
            { // Missile Strike
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Missile Strike");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol, "Missile Strike");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num2].time == 1)
                {

                    effects.createSquare(gCtrl.menuPos.x - 10,
                                         gCtrl.menuPos.y - 2,
                                         gCtrl.menuPos.x + 10,
                                         gCtrl.menuPos.y + 2, sf::Color::Black);
                    effects.createSquare(
                        gCtrl.menuPos.x - 2, gCtrl.menuPos.y - 10,
                        gCtrl.menuPos.x + 2, gCtrl.menuPos.y + 10,
                        sf::Color::Black);
                    Item var;
                    var = *getGlobalItem("Missile");
                    var.targetPos = sf::Vector2f(gCtrl.menuPos);
                    var.xpos = gCtrl.menuPos.x - 200;
                    var.ypos = gCtrl.menuPos.y - 200;
                    var.zpos = 100;
                    worlditems.push_back(var);

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }

            if (i == 2)
            { // Shift wall/stone

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Shift Wall/Stone");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol);
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num3].time == 1)
                {

                    if (tiles[abs_to_index(gCtrl.menuPos.x / GRID_SIZE)]
                             [abs_to_index(gCtrl.menuPos.y / GRID_SIZE)][30]
                                 .id != 1010)
                    {
                        tiles[abs_to_index(gCtrl.menuPos.x / GRID_SIZE)]
                             [abs_to_index(gCtrl.menuPos.y / GRID_SIZE)][30]
                                 .wall();
                        fSleep(0.2);
                    }
                    else
                    {
                        tiles[abs_to_index(gCtrl.menuPos.x / GRID_SIZE)]
                             [abs_to_index(gCtrl.menuPos.y / GRID_SIZE)][30]
                                 .stone();
                        fSleep(0.2);
                    }

                    con("TODO: Make this a Toggle Mode");

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }

            if (i == 3)
            { // Spawn zombie horde

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x + 2,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Spawn Zombie Horde");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol);
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num4].time == 1)
                {

                    gCtrl.zombieSwarmLocal();

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }

            if (i == 4)
            { // Print NPC console info
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(
                    gCtrl.menuPos.x, gCtrl.menuPos.y + (iY * 13), 12,
                    sf::Color::White, "Print NPC's ConsoleInfo");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol);
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num5].time == 1)
                {

                    std::list<Npc>::iterator zit;
                    sf::Lock lock(mutex::npcList);
                    for (zit = npclist.begin(); zit != npclist.end(); ++zit)
                    {
                        if (zit->alive == true)
                        {
                            zit->printConsoleInfo();
                        }
                    }

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }

            if (i == 5)
            { // Print bloodcontents
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(
                    gCtrl.menuPos.x, gCtrl.menuPos.y + (iY * 13), 12,
                    sf::Color::White, "Print NPC's Bloodcontents");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol);
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num5].time == 1)
                {
                    sf::Lock lock(mutex::npcList);
                    for (auto &elem : npclist)
                    {
                        elem.printBloodContent();
                    }

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }

            if (i == 6)
            { // Give everyone zombification
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(
                    gCtrl.menuPos.x, gCtrl.menuPos.y + (iY * 13), 12,
                    sf::Color::White, "Give Everyone Zombification");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol,
                    "You horrible monster, Why would you do this?!");
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num6].time == 1)
                {
                    sf::Lock lock(mutex::npcList);
                    for (auto &elem : npclist)
                    {
                        elem.bloodwork("Zombification", 100);
                    }

                    gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                    gCtrl.menuType = "NULL";

                    fSleep(0.2);
                    break;
                }
            }

            if (i == 7)
            { // Delete all critters and items
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Delete all critters and items");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol);
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num7].time == 1)
                {
                    sf::Lock lock(mutex::npcList);
                    sf::Lock itemlock(mutex::itemList);
                    for (auto &elem : worlditems)
                    {
                        elem.toDelete = true;
                    }
                    for (auto &elem : npclist)
                    {
                        elem.toDelete = true;
                    }

                }
            }

            if (i == 8)
            { // Starve all critters

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Starve all critters");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol);
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num8].time == 1)
                {
                    sf::Lock lock(mutex::npcList);
                    for (auto &elem : npclist)
                    {
                        elem.hunger = 50;
                    }
                }
            }
            if (i == 9)
            { // Arena

                //Effectz.CreateLine(GC.MenuPos.x,(GC.MenuPos.y+(iY*13))+8,GC.MenuPos.x+90,(GC.MenuPos.y+(iY*13))+8,3,Black,1,Yellow);
                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 13)) + 13,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 13)) + 13, 1,
                    sf::Color::Cyan);
                textList.createText(gCtrl.menuPos.x,
                                    gCtrl.menuPos.y + (iY * 13), 12,
                                    sf::Color::White, "Giant Enclosed Arena");
                int butt = createSquareButton(
                    math::Vec2f(gCtrl.menuPos.x + brd,
                                (gCtrl.menuPos.y + (iY * 13)) + mbd),
                    bs, bsy, butCol);
                if (squareButtonClicked(butt) ||
                    inputState.key[Key::Num8].time == 1)
                {
                    for (int ItLength = 0; ItLength != 16; ItLength++)
                    {
                        if (ItLength != 15)
                        {
                            for (int Rot = 1; Rot != 361; Rot++)
                            {
                                int XPos = abs(gCtrl.menuPos.x / 20) +
                                           sin(Rot * PI / 180) * ItLength;
                                int YPos = abs(gCtrl.menuPos.y / 20) +
                                           cos(Rot * PI / 180) * ItLength;
                                tiles[XPos][YPos][30].stone();
                            }
                        }
                        else
                        {
                            for (int Rot = 1; Rot != 361; Rot++)
                            {
                                int XPos = abs(gCtrl.menuPos.x / 20) +
                                           sin(Rot * PI / 180) * ItLength;
                                int YPos = abs(gCtrl.menuPos.y / 20) +
                                           cos(Rot * PI / 180) * ItLength;
                                tiles[XPos][YPos][30].wall();
                            }
                        }
                    }
                }
            }

            iY++;
        }
    }

    if (gCtrl.menuType == "DebugItemSpawn")
    {
        gCtrl.menuEndPos = sf::Vector2f(
            gCtrl.menuPos.x + 100,
            (gCtrl.menuPos.y + (itemmanager.globalItems.size() * 10)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::White);
        int iY = 0;
        for (auto &elem : itemmanager.globalItems)
        {
            effects.createLine(
                gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 11)) + 8,
                gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 11)) + 8, 3,
                sf::Color::Black, 1, sf::Color::Yellow);
            textList.createText(gCtrl.menuPos.x, gCtrl.menuPos.y + (iY * 11),
                                11, sf::Color::Cyan, elem.name);
            int butt =
                createButton(math::Vec2f(gCtrl.menuPos.x + 90,
                                         (gCtrl.menuPos.y + (iY * 11)) + 5),
                             5, sf::Color::Red);
            if (buttonClicked(butt))
            {
                spawnItem(elem.name, gCtrl.menuPos.x, gCtrl.menuPos.y, gvars::currentz*20 );
                fSleep(0.2);
            }
            iY++;
        }
    }

    if (gCtrl.menuType == "DebugCritterSpawn")
    {
        gCtrl.menuEndPos = sf::Vector2f(
            gCtrl.menuPos.x + 100,
            (gCtrl.menuPos.y + (npcmanager.globalCritter.size() * 10)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::White);
        int iY = 0;
        for (auto &elem : npcmanager.globalCritter)
        {
            effects.createLine(
                gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 11)) + 8,
                gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 11)) + 8, 3,
                sf::Color::Black, 1, sf::Color::Yellow);
            textList.createText(gCtrl.menuPos.x, gCtrl.menuPos.y + (iY * 11),
                                11, sf::Color::Cyan, elem.name);
            int butt =
                createButton(math::Vec2f(gCtrl.menuPos.x + 90,
                                         (gCtrl.menuPos.y + (iY * 11)) + 5),
                             5, sf::Color::Red);
            if (buttonClicked(butt))
            {
                spawnCritter(elem.name, gCtrl.menuPos.x, gCtrl.menuPos.y, gvars::currentz*20);
                fSleep(0.2);
                break;
            }
            iY++;
        }
    }

    if (gCtrl.menuType == "CritterContext" && true == false)
    {
        int options = 2;
        gCtrl.menuEndPos = sf::Vector2f(gCtrl.menuPos.x + 100,
                                        (gCtrl.menuPos.y + (options * 10)) + 5);
        effects.createSquare(gCtrl.menuPos.x, gCtrl.menuPos.y,
                             gCtrl.menuEndPos.x, gCtrl.menuEndPos.y,
                             sf::Color::White);
        int iY = 0;
        for (int i = 0; i != options; i++)
        {

            if (i == 0)
            {

                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 11)) + 8,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 11)) + 8, 3,
                    sf::Color::Black, 1, sf::Color::Yellow);
                textList.createText(gCtrl.menuPos.x,
                                    gCtrl.menuPos.y + (iY * 11), 11,
                                    sf::Color::Cyan, "Close Menu");
                int butt =
                    createButton(math::Vec2f(gCtrl.menuPos.x + 90,
                                             (gCtrl.menuPos.y + (iY * 11)) + 5),
                                 5, sf::Color::Red);
                if (buttonClicked(butt))
                {

                    std::list<Item>::iterator item;
                    for (item = worlditems.begin(); item != worlditems.end();
                         item++)
                    {
                        gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                        gCtrl.menuType = "NULL";
                    }

                    fSleep(0.2);
                    break;
                }
            }
            if (i == 1)
            {

                effects.createLine(
                    gCtrl.menuPos.x, (gCtrl.menuPos.y + (iY * 11)) + 8,
                    gCtrl.menuPos.x + 90, (gCtrl.menuPos.y + (iY * 11)) + 8, 3,
                    sf::Color::Black, 1, sf::Color::Yellow);
                textList.createText(gCtrl.menuPos.x,
                                    gCtrl.menuPos.y + (iY * 11), 11,
                                    sf::Color::Cyan, "PickUp");
                int butt =
                    createButton(math::Vec2f(gCtrl.menuPos.x + 90,
                                             (gCtrl.menuPos.y + (iY * 11)) + 5),
                                 5, sf::Color::Red);
                if (buttonClicked(butt))
                {

                    std::list<Item>::iterator item;
                    for (item = worlditems.begin(); item != worlditems.end();
                         item++)
                    {
                        if (math::closeish(gCtrl.menuPos.x, gCtrl.menuPos.y,
                                           item->xpos, item->ypos) <= 10)
                        {
                            if (item->pickupable == true)
                            {
                                sf::Lock lock(mutex::npcList);
                                //for (auto &i : npclist)
                                //listAt(npclist, gvars::myTargetid)

                                //listAt(npclist, gvars::myTargetid)
                                myTargetPtr->action = "Pickup";
                                myTargetPtr->target = item->name;
                                myTargetPtr->targetId = item->id;
                                myTargetPtr->targetPos =
                                    sf::Vector2f(item->xpos, item->ypos);
                                gCtrl.menuPos = sf::Vector2f(-10000, -10000);
                                gCtrl.menuType = "NULL";
                            }
                        }
                    }

                    fSleep(0.2);
                    break;
                }
            }

            iY++;
        }

        if (gvars::myTarget == -1 && gCtrl.menuType == "CritterContext")
        {
            gCtrl.menuPos = sf::Vector2f(-10000, -10000);
            gCtrl.menuType = "NULL";
            con("Closing Menu due to No Target && CritterContext");
        }
    }
}

void rightMouseButtonContextMenu()
{
    if (inputState.rmbTime ==
        1) // This is the RightMouseButton's context menu for Tiles/Critters/Items, This will probably be rather robust
    {

        {
            sf::Lock lock(mutex::npcList);
            //for (size_t i = 0; i != npclist.size(); i++)
            for (auto &i : npclist)
        {
            if (math::closeish(gvars::mousePos.x, gvars::mousePos.y,
                               i.xpos,
                               i.ypos) <= i.size)
            {
                gCtrl.menuType = "CritterRMB";
                menuPopUp();
                return;
            }
        }
        }


        for (auto &worlditem : worlditems)
        {
            if (math::closeish(gvars::mousePos.x, gvars::mousePos.y,
                               (worlditem).xpos, (worlditem).ypos) <= 10)
            {
                gCtrl.menuType = "ItemRMB";
                gCtrl.menuPtrCon.pVecItem.push_back(&(worlditem));
            }
        }
        if (gCtrl.menuPtrCon.pVecItem.size() != 0)
        {
            menuPopUp();
            return;
        }

        gCtrl.menuType = "BlankRMB";
        menuPopUp();
    }
}
