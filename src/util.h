#ifndef __UTIL_H_INCLUDED__
#define __UTIL_H_INCLUDED__

#include <string>
#include <vector>
#include <set>

#include <SFML/System.hpp>

void fSleep(float Time);
void PrintToFile(std::string FileName, std::string Text, bool EndLine = true);
void Con(std::string Text, bool EndLine = true);
void Con(float Number, bool EndLine = true);
bool Booleanize(int Num);
bool Inbetween(float First, float Second, float Number);
int randz(int minValue, int maxValue);
bool AABB(int Pointx, int Pointy, int Left, int Right, int Up, int Down);
bool AABB(sf::Vector2f Point, int Left, int Right, int Up, int Down);
bool AABB(sf::Vector2i Point, int Left, int Right, int Up, int Down);
int CloseishS(int orix, int tarx);

/// Get the absolute of a numeric value,
/// converted to size_t, suitable for indexing
template <typename T> size_t abs_to_index(T value)
{
    return static_cast<size_t>(std::abs(value));
}

int StringFindNumber(std::string Stringy, std::string Term);
std::string StringFindString(std::string Stringy, std::string Term);
std::string StringFindStringNpos(std::string Stringy, std::string Term);
std::vector<std::string> StringFindElements(std::string Source,
                                            std::string Seperater = ":");
std::vector<float> NumericSplitter(float Received, float SplitVariable = 100);
float PercentageBuff(float Received);
std::string StringFindChaos(std::string Stringy, std::string Term,
                            std::string Ending);
std::set<std::string> StringFindSetChaos(std::string Stringy, std::string Term,
                                         std::string Ending);
float PercentIs(float Value, float Percentage);

std::string GenerateName(int MinLength = 2, int MaxLength = 10);
bool Toggle(bool &Boolean);

#endif // __UTIL_H_INCLUDED__