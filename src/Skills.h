#ifndef SKILLS_H_INCLUDED
#define SKILLS_H_INCLUDED

#include <string>
#include <vector>
#include <list>

class Skill
{
public:
    std::string name;
    std::string tree;
    std::string desc;
    int level;
    int duration;
    int cooldownint;
    int cooldown;
    int ranks;
    int ranksmax;
    int cost;
};

class Skills
{
public:
    std::list<Skill> list;
    Skills();
};

#endif // SKILLS_H_INCLUDED
