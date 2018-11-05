#pragma GCC optimize "O3,omit-frame-pointer,inline"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>
#include <deque>
#include <map>
#include <thread>

//#define PRINT_DEBUG_INPUT
#ifdef PRINT_DEBUG_INPUT
#define DBG_INPUT(x) {std::cerr << x << std::endl;}
#else
#define DBG_INPUT(x)
#endif

#define PRINT_DEBUG_INFO
#ifdef PRINT_DEBUG_INFO
#define DBG_INFO(x) {std::cerr << x << std::endl;}
#else
#define DBG_INFO(x)
#endif


enum class UnitType
{
    KNIGHT=0,
    ARCHER=1,
    QUEEN=2
};

enum class StructureType
{
    EMPTY_SITE=0,
    BARRACKS_KNIGHT=1,
    BARRACKS_ARCHER=2
};


inline std::string structureTypeToString(StructureType sType)
{
    switch(sType)
    {
        case StructureType::EMPTY_SITE:
            return "EMPTY_SITE";
        case StructureType::BARRACKS_KNIGHT:
            return "BARRACKS_KNIGHT";
        case StructureType::BARRACKS_ARCHER:
            return "BARRACKS_ARCHER";
    }
}

inline std::string unitTypeToString(UnitType uType)
{
    switch(uType)
    {
        case UnitType::KNIGHT:
            return "KNIGHT";
        case UnitType::ARCHER:
            return "ARCHER";
        case UnitType::QUEEN:
            return "QUEEN";
    }
}


struct Position
{
    Position(): x(0), y(0){}
    Position(int xPos, int yPos): x(xPos), y(yPos){}

    int x;
    int y;
};

class ObjectWithPositionAndRadius
{
public:
    ObjectWithPositionAndRadius(const Position& pos) : _pos(pos), _radius(0) {}
    ObjectWithPositionAndRadius(const Position& pos, int radius) : _pos(pos), _radius(radius) {}

    virtual ~ObjectWithPositionAndRadius(){}

    inline int distanceTo(const ObjectWithPositionAndRadius& other)
    {
        int retVal = static_cast<int>(std::sqrt(
                                          (other._pos.x - _pos.x)*(other._pos.x - _pos.x) +
                                          (other._pos.y - _pos.y)*(other._pos.y - _pos.y)
                                          ) - (_radius + other._radius)
                                      );
        /*DBG_INFO("[ObjectWithPositionAndRadius::distanceTo]: (" <<
                 _pos.x << "," << _pos.y << ") to (" <<
                 other._pos.x << "," << other._pos.y << ") = " << retVal);*/
        return retVal;
    }

    inline const Position& getPosition() const { return _pos; }
    inline int getRadius() const { return _radius; }
private:
    Position _pos;
    int _radius;
};


struct StructureInitialInfo
{
    Position pos;
    int radius;
};

using SiteInfoMap = std::map<int, StructureInitialInfo>;

class Structure : public ObjectWithPositionAndRadius
{
public:
    Structure(const Position& pos, int radius, int team, StructureType sType, int siteId) :
        ObjectWithPositionAndRadius(pos, radius),
        _sType(sType),
        _team(team),
        _siteId(siteId) {}

    virtual ~Structure(){}
    static std::shared_ptr<Structure> createStructureFromInput(const SiteInfoMap& siteInfo);

    inline StructureType getType() const { return _sType; }
    inline int getTeam() const { return _team; }
    inline int getSiteId() const { return _siteId; }
private:
    StructureType _sType;
    int _team;
    int _siteId;
};

class EmptySite : public Structure
{
public:
    EmptySite(const Position& pos, int radius, int siteId) :
        Structure(pos, radius, -1, StructureType::EMPTY_SITE, siteId) {}
    virtual ~EmptySite(){}
};

class Barracks : public Structure
{
public:
    Barracks(const Position& pos, int radius, int team, StructureType sType, int siteId, int turnsUntilTrain) :
        Structure(pos, radius, team, sType, siteId),
        _turnsUntilTrain(turnsUntilTrain) {}
    virtual ~Barracks(){}

    inline int getTurnsUntilTrain() const { return _turnsUntilTrain; }
private:
    int _turnsUntilTrain;
};

class BarracksArchers : public Barracks
{
public:
    BarracksArchers(const Position& pos, int radius, int team, int siteId, int turnsUntilTrain) :
        Barracks(pos, radius, team, StructureType::BARRACKS_ARCHER, siteId, turnsUntilTrain) {}
    virtual ~BarracksArchers(){}
};

class BarracksKnights : public Barracks
{
public:
    BarracksKnights(const Position& pos, int radius, int team, int siteId, int turnsUntilTrain) :
        Barracks(pos, radius, team, StructureType::BARRACKS_KNIGHT, siteId, turnsUntilTrain) {}
    virtual ~BarracksKnights(){}
};



std::shared_ptr<Structure> Structure::createStructureFromInput(const SiteInfoMap& siteInfo)
{
    std::shared_ptr<Structure> retVal;
    int siteId;
    int ignore1; // used in future leagues
    int ignore2; // used in future leagues
    int structureType; // -1 = No structure, 2 = Barracks
    int owner; // -1 = No structure, 0 = Friendly, 1 = Enemy
    int param1; // in case of barracks - turns until train
    int param2; // in case of barracks - barrack type - 0 for KNIGHT, 1 for ARCHER

    std::cin >> siteId >> ignore1 >> ignore2 >> structureType >> owner >> param1 >> param2; std::cin.ignore();

    SiteInfoMap::const_iterator it = siteInfo.find(siteId);

    if(it != siteInfo.end())
    {
        const Position& pos = it->second.pos;
        int radius = it->second.radius;
        switch (structureType)
        {
            case -1:
                retVal = std::make_shared<EmptySite>(pos, radius, siteId);
                break;
            case 2:
                if(param2 == 0)
                {
                    retVal = std::make_shared<BarracksKnights>(pos, radius, owner, siteId, param1);
                }
                else if(param2 == 1)
                {
                    retVal = std::make_shared<BarracksArchers>(pos, radius, owner, siteId, param1);
                }
                break;
        }
    }
    return retVal;
}

class Unit : public ObjectWithPositionAndRadius
{
public:
    Unit(const Position& pos, int radius, int team, UnitType uType, int health) :
        ObjectWithPositionAndRadius (pos, radius),
        _team(team),
        _unitType(uType),
        _health(health) {}

    static std::shared_ptr<Unit> createUnitFromInput();

    inline UnitType getType() const { return _unitType; }
    inline int getTeam() const { return _team; }
    inline int getHealth() const { return _health; }
private:
    int _team;
    UnitType _unitType;
    int _health;
};

class Queen : public Unit
{
public:
    Queen(const Position& pos, int team, int health) : Unit(pos, 30, team, UnitType::QUEEN, health) {}
    virtual ~Queen(){}
private:
};

class Archer : public Unit
{
public:
    Archer(const Position& pos, int team, int health) : Unit(pos, 0, team, UnitType::ARCHER, health) {}
    virtual ~Archer(){}
};

class Knight : public Unit
{
public:
    Knight(const Position& pos, int team, int health) : Unit(pos, 0, team, UnitType::KNIGHT, health) {}
    virtual ~Knight(){}
};

std::shared_ptr<Unit> Unit::createUnitFromInput()
{
    std::shared_ptr<Unit> retVal;
    int x;
    int y;
    int owner;
    int unitTypeInt; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
    int health;
    std::cin >> x >> y >> owner >> unitTypeInt >> health; std::cin.ignore();
    Position pos(x,y);
    switch (unitTypeInt)
    {
        case -1:
            retVal = std::make_shared<Queen>(pos, owner, health);
            break;
        case 0:
            retVal = std::make_shared<Knight>(pos, owner, health);
            break;
        case 1:
            retVal = std::make_shared<Archer>(pos, owner, health);
            break;
        default:
            DBG_INFO("[Unit::createUnitFromInput] Unexpected unit type: " << unitTypeInt);
            return nullptr;
    }

    return retVal;
}

template <int Width, int Height>
class Map
{
public:
    Map();
    ~Map();

private:
    char _array2d[Width*Height];
};


struct TeamState
{
    TeamState()
    {
        knights.reserve(20);
        archers.reserve(20);
        barracksArchers.reserve(10);
        barracksKnights.reserve(10);
    }
    void reset()
    {
        queen.reset();
    }
    std::shared_ptr<Queen> queen;
    std::vector<std::shared_ptr<Knight>> knights;
    std::vector<std::shared_ptr<Archer>> archers;
    std::vector<std::shared_ptr<BarracksKnights>> barracksKnights;
    std::vector<std::shared_ptr<BarracksArchers>> barracksArchers;
};

class GameContext
{
public:
    GameContext() : _gold(0), _touchedSite(-1), _currentTurn(0) {}

    inline void readInit()
    {
        int numSites;
        std::cin >> numSites; std::cin.ignore();
        for(auto cntSite = 0 ; cntSite < numSites ; ++cntSite)
        {
            int siteId;
            StructureInitialInfo sInfo;
            std::cin >> siteId >> sInfo.pos.x >> sInfo.pos.y >> sInfo.radius; std::cin.ignore();
            _sInfo[siteId] = sInfo;
        }
    }
    inline int getNumSites() { return _sInfo.size(); }

    inline void readTurnInput()
    {
        std::cin >> _gold; std::cin.ignore();
        std::cin >> _touchedSite; std::cin.ignore();

        int numSites = getNumSites();
        for (auto cntSite = 0; cntSite < numSites; ++cntSite)
        {
            // create derived class object
            std::shared_ptr<Structure> newStructure = Structure::createStructureFromInput(_sInfo);

            // put it in apropriate place
            if(newStructure->getType() == StructureType::EMPTY_SITE)
            {
                _emptySites.emplace_back(std::static_pointer_cast<EmptySite>(newStructure));
            }
            else // barracks
            {
                TeamState& targetTeamState = newStructure->getTeam() == 0 ? _friendlyTeam : _enemyTeam;

                if(newStructure->getType() == StructureType::BARRACKS_ARCHER)
                {
                    targetTeamState.barracksArchers.emplace_back(std::static_pointer_cast<BarracksArchers>(newStructure));
                }
                else if(newStructure->getType() == StructureType::BARRACKS_KNIGHT)
                {
                    targetTeamState.barracksKnights.emplace_back(std::static_pointer_cast<BarracksKnights>(newStructure));
                }
            }
        }

        int numUnits;

        std::cin >> numUnits; std::cin.ignore();

        for(auto cntUnit = 0; cntUnit < numUnits; ++cntUnit)
        {
            std::shared_ptr<Unit> newUnit = Unit::createUnitFromInput();

            TeamState& targetTeamState = newUnit->getTeam() == 0 ? _friendlyTeam : _enemyTeam;
            switch (newUnit->getType())
            {
                case UnitType::QUEEN:
                    targetTeamState.queen = std::static_pointer_cast<Queen>(newUnit);
                    break;
                case UnitType::KNIGHT:
                    targetTeamState.knights.emplace_back(std::static_pointer_cast<Knight>(newUnit));
                    break;
                case UnitType::ARCHER:
                    targetTeamState.archers.emplace_back(std::static_pointer_cast<Archer>(newUnit));
                    break;
            }
        }
    }

    inline void queenWAIT() { std::cout << "WAIT" << std::endl; }
    inline void queenMOVE(Position& pos) { std::cout << "MOVE " << pos.x << " " << pos.y << std::endl; }
    inline void queenBUILD(int siteId, bool archers = false)
    {
        std::cout << "BUILD " << siteId << " BARRACKS-" << (archers ? "ARCHER" : "KNIGHT") << std::endl;
    }



    inline void takeAction()
    {
        if(!_emptySites.empty())
        {
            DBG_INFO("[STRAT] Empty sites exist. We must expand.");
            // sort the empty places by distance from the queen
            std::sort(_emptySites.begin(), _emptySites.end(),
                      [&](const std::shared_ptr<EmptySite>& a,
                      const std::shared_ptr<EmptySite>& b) -> bool
            {
                return _friendlyTeam.queen->distanceTo(*a) > _friendlyTeam.queen->distanceTo(*b);
            });

            bool buildArchers = false;
            if(_friendlyTeam.barracksArchers.empty())
            {
                DBG_INFO("[STRAT] No archer barracks - let's build some.");
                buildArchers = true;
            }
            else
            {
                DBG_INFO("[STRAT] We have archer barracks, so let's make some knights barracks.");
            }

            queenBUILD(_emptySites.back()->getSiteId(), buildArchers);
        }

        DBG_INFO("[STRAT] Current gold: " << _gold);
        if(_gold > 80)
        {
            std::vector<int> barracksToTrain;
            DBG_INFO("[STRAT] We have at least 80 gold - we can train units");
            if(_friendlyTeam.archers.empty() && !_friendlyTeam.barracksArchers.empty() && _gold > 100)
            {
                DBG_INFO("[STRAT] We have archers barracks and don't have archers - let's build some...");
                barracksToTrain.emplace_back(_friendlyTeam.barracksArchers[0]->getSiteId());
                _gold -= 100;
            }

            for(std::shared_ptr<Barracks> currentBarracks : _friendlyTeam.barracksKnights)
            {
                if(_gold < 80)
                    break;
                barracksToTrain.emplace_back(currentBarracks->getSiteId());
                _gold -= 80;
                DBG_INFO("[STRAT] We have money for kingts - lets do this shit!");
            }

            std::cout << "TRAIN";
            if(!barracksToTrain.empty())
            {
                for(int id : barracksToTrain)
                {
                    std::cout << " " << id;
                }
                std::cout << std::endl;
            }
        }
    }

    inline void processOneTurn()
    {
        DBG_INFO("Starting turn " << _currentTurn);
        auto startTurn = std::chrono::system_clock::now();
        readTurnInput();
        using namespace std::chrono_literals;
        auto endReadInput = std::chrono::system_clock::now();
        std::chrono::duration<double> inputTime = std::chrono::duration_cast<std::chrono::milliseconds>(endReadInput-startTurn);
        DBG_INFO("[TIME] Input : " << inputTime.count());
        takeAction();
        auto endTurn = std::chrono::system_clock::now();
        std::chrono::duration<double> actionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTurn-endReadInput);
        DBG_INFO("[TIME] Action : " << actionTime.count());
        ++_currentTurn;
    }


private:
    SiteInfoMap _sInfo;
    int _gold;
    int _touchedSite;
    std::vector<std::shared_ptr<EmptySite>> _emptySites;
    TeamState _friendlyTeam;
    TeamState _enemyTeam;
    int _currentTurn;
};

int main()
{
    GameContext game;
    game.readInit();

    // game loop
    while (1)
    {
        game.processOneTurn();
    }
}
