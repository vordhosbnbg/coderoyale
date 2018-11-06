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
    GIANT=2,
    QUEEN=3
};

enum class StructureType
{
    EMPTY_SITE=0,
    BARRACKS_KNIGHT=1,
    BARRACKS_ARCHER=2,
    BARRACKS_GIANT=3,
    TOWER=4,
    MINE=5
};


inline std::string structureTypeToString(StructureType sType)
{
    switch(sType)
    {
        case StructureType::EMPTY_SITE:
            return "EMPTY_SITE";
        case StructureType::BARRACKS_KNIGHT:
            return "BARRACKS-KNIGHT";
        case StructureType::BARRACKS_ARCHER:
            return "BARRACKS-ARCHER";
        case StructureType::BARRACKS_GIANT:
            return "BARRACKS-GIANT";
        case StructureType::TOWER:
            return "TOWER";
        case StructureType::MINE:
            return "MINE";
    }
    return "";
}

inline std::string unitTypeToString(UnitType uType)
{
    switch(uType)
    {
        case UnitType::KNIGHT:
            return "KNIGHT";
        case UnitType::ARCHER:
            return "ARCHER";
        case UnitType::GIANT:
            return "GIANT";
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
protected:
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
    Structure(const Position& pos, int radius, int goldAvailable, int maxMineSize, int team, StructureType sType, int siteId) :
        ObjectWithPositionAndRadius(pos, radius),
        _goldAvailable(goldAvailable),
        _maxMineSize(maxMineSize),
        _sType(sType),
        _team(team),
        _siteId(siteId) {}

    virtual ~Structure(){}

    void print(){ DBG_INFO("[S] - (" << _pos.x << "," << _pos.y << "), radius: " << _radius
                           << ", id: " << _siteId << ", team: " << _team << ", type: " << structureTypeToString(_sType)
                           << ", goldAvl: " << _goldAvailable << ", maxMineSize: " << _maxMineSize);}
    static std::shared_ptr<Structure> createStructureFromInput(const SiteInfoMap& siteInfo);

    inline int getGoldAvailable() const { return _goldAvailable; }
    inline int getMaxMineSize() const { return _maxMineSize; }
    inline StructureType getType() const { return _sType; }
    inline int getTeam() const { return _team; }
    inline int getSiteId() const { return _siteId; }
protected:
    int _goldAvailable;
    int _maxMineSize;
    StructureType _sType;
    int _team;
    int _siteId;
};

class EmptySite : public Structure
{
public:
    EmptySite(const Position& pos, int radius, int goldAvailable, int maxMineSize, int siteId) :
        Structure(pos, radius, goldAvailable, maxMineSize, -1, StructureType::EMPTY_SITE, siteId) {}
    virtual ~EmptySite(){}
};

class Tower : public Structure
{
public:
    Tower(const Position& pos, int radius, int goldAvailable, int maxMineSize, int team, int health, int attackRadius, int siteId) :
        Structure(pos, radius, goldAvailable, maxMineSize, team, StructureType::TOWER, siteId), _health(health), _attackRadius(attackRadius) {}
    virtual ~Tower(){}

    inline int getHealth() const { return _health; }

protected:
    int _health;
    int _attackRadius;
};

class Mine : public Structure
{
public:
    Mine(const Position& pos, int radius, int goldAvailable, int maxMineSize, int team, int income,  int siteId) :
        Structure(pos, radius, goldAvailable, maxMineSize, team, StructureType::MINE, siteId), _income(income) {}
    virtual ~Mine(){}

    inline int getMineSize() const { return _income; }

protected:
    int _income;
};

class Barracks : public Structure
{
public:
    Barracks(const Position& pos, int radius, int goldAvailable, int maxMineSize, int team, StructureType sType, int siteId, int turnsUntilTrain) :
        Structure(pos, radius, goldAvailable, maxMineSize, team, sType, siteId),
        _turnsUntilTrain(turnsUntilTrain) {}
    virtual ~Barracks(){}

    inline int getTurnsUntilTrain() const { return _turnsUntilTrain; }
private:
    int _turnsUntilTrain;
};

class BarracksArchers : public Barracks
{
public:
    BarracksArchers(const Position& pos, int radius, int goldAvailable, int maxMineSize, int team, int siteId, int turnsUntilTrain) :
        Barracks(pos, radius, goldAvailable, maxMineSize, team, StructureType::BARRACKS_ARCHER, siteId, turnsUntilTrain) {}
    virtual ~BarracksArchers(){}
};

class BarracksKnights : public Barracks
{
public:
    BarracksKnights(const Position& pos, int radius, int goldAvailable, int maxMineSize, int team, int siteId, int turnsUntilTrain) :
        Barracks(pos, radius, goldAvailable, maxMineSize, team, StructureType::BARRACKS_KNIGHT, siteId, turnsUntilTrain) {}
    virtual ~BarracksKnights(){}
};

class BarracksGiants : public Barracks
{
public:
    BarracksGiants(const Position& pos, int radius, int goldAvailable, int maxMineSize, int team, int siteId, int turnsUntilTrain) :
        Barracks(pos, radius, goldAvailable, maxMineSize, team, StructureType::BARRACKS_GIANT, siteId, turnsUntilTrain) {}
    virtual ~BarracksGiants(){}
};



std::shared_ptr<Structure> Structure::createStructureFromInput(const SiteInfoMap& siteInfo)
{
    std::shared_ptr<Structure> retVal;
    int siteId;
    int goldAvailable; // used in future leagues
    int maxMineSize; // used in future leagues
    int structureType; // -1 = No structure, 1 = Tower, 2 = Barracks
    int owner; // -1 = No structure, 0 = Friendly, 1 = Enemy
    int param1; // in case of barracks - turns until train, in case of tower - hp, in case of mine - income rate
    int param2; // in case of barracks - barrack type - 0 for KNIGHT, 1 for ARCHER, in case of tower - attack radius

    std::cin >> siteId >> goldAvailable >> maxMineSize >> structureType >> owner >> param1 >> param2; std::cin.ignore();
    DBG_INPUT(siteId << " " << goldAvailable << " " << maxMineSize << " " << structureType << " " << owner << " " << param1 << " " << param2);

    SiteInfoMap::const_iterator it = siteInfo.find(siteId);

    if(it != siteInfo.end())
    {
        const Position& pos = it->second.pos;
        int radius = it->second.radius;
        switch (structureType)
        {
            case -1:
                retVal = std::make_shared<EmptySite>(pos, radius, goldAvailable, maxMineSize, siteId);
                break;
            case 0:
                retVal = std::make_shared<Mine>(pos, radius, goldAvailable, maxMineSize, owner, param1, siteId);
                break;
            case 1:
                retVal = std::make_shared<Tower>(pos, radius, goldAvailable, maxMineSize, owner, param1, param2, siteId);
                break;
            case 2:
            {
                switch (param2)
                {
                    case 0:
                        retVal = std::make_shared<BarracksKnights>(pos, radius, goldAvailable, maxMineSize, owner, siteId, param1);
                        break;
                    case 1:
                        retVal = std::make_shared<BarracksArchers>(pos, radius, goldAvailable, maxMineSize, owner, siteId, param1);
                        break;
                    case 2:
                        retVal = std::make_shared<BarracksGiants>(pos, radius, goldAvailable, maxMineSize, owner, siteId, param1);
                        break;
                }
            }
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

class Giant : public Unit
{
public:
    Giant(const Position& pos, int team, int health) : Unit(pos, 0, team, UnitType::GIANT, health) {}
    virtual ~Giant(){}
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
    DBG_INPUT(x << " " << y << " " << owner << " " << unitTypeInt << " " << health);
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
        case 2:
            retVal = std::make_shared<Giant>(pos, owner, health);
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
        knights.reserve(40);
        archers.reserve(40);
        giants.reserve(40);
        barracksArchers.reserve(40);
        barracksKnights.reserve(40);
        barracksGiants.reserve(40);
        towers.reserve(40);
        mines.reserve(40);
    }
    void reset()
    {
        queen.reset();
        knights.clear();
        archers.clear();
        giants.clear();
        barracksArchers.clear();
        barracksKnights.clear();
        barracksGiants.clear();
        towers.clear();
        mines.clear();
    }
    std::shared_ptr<Queen> queen;
    std::vector<std::shared_ptr<Knight>> knights;
    std::vector<std::shared_ptr<Archer>> archers;
    std::vector<std::shared_ptr<Giant>> giants;
    std::vector<std::shared_ptr<BarracksKnights>> barracksKnights;
    std::vector<std::shared_ptr<BarracksArchers>> barracksArchers;
    std::vector<std::shared_ptr<BarracksGiants>> barracksGiants;
    std::vector<std::shared_ptr<Tower>> towers;
    std::vector<std::shared_ptr<Mine>> mines;
};

class GameContext
{
public:
    GameContext() : _gold(0), _touchedSite(-1), _currentTurn(0), _queenOrdered(false), _saveGold(0) { _emptySites.reserve(30);}

    inline void readInit()
    {
        int numSites;
        std::cin >> numSites; std::cin.ignore();
        DBG_INPUT(numSites);
        for(auto cntSite = 0 ; cntSite < numSites ; ++cntSite)
        {
            int siteId;
            StructureInitialInfo sInfo;
            std::cin >> siteId >> sInfo.pos.x >> sInfo.pos.y >> sInfo.radius; std::cin.ignore();
            DBG_INPUT(siteId << " " << sInfo.pos.x << " " << sInfo.pos.y << " " << sInfo.radius);
            _sInfo[siteId] = sInfo;
        }
    }
    inline int getNumSites() { return _sInfo.size(); }
    inline int getAvailableGold() { return _gold - _saveGold; }
    inline void readTurnInput()
    {
        DBG_INFO("[INPUT] Starting input parsing.");
        DBG_INFO("[INPUT] Reset team state.");
        _friendlyTeam.reset();
        _enemyTeam.reset();
        _emptySites.clear();
        DBG_INFO("Waaaat?");

        std::cin >> _gold >> _touchedSite; std::cin.ignore();
        DBG_INPUT(_gold << " " << _touchedSite);
        DBG_INFO("[STRAT] Gold: " << _gold << " touching site: " << _touchedSite);
        int numSites = getNumSites();
        DBG_INFO("[INPUT] Creating site objects");
        for (auto cntSite = 0; cntSite < numSites; ++cntSite)
        {
            // create derived class object
            std::shared_ptr<Structure> newStructure = Structure::createStructureFromInput(_sInfo);
            if(newStructure)
            {
                //newStructure->print();

                // put it in apropriate place
                TeamState& targetTeamState = newStructure->getTeam() == 0 ? _friendlyTeam : _enemyTeam;

                const StructureType sType = newStructure->getType();
                switch (sType)
                {
                    case StructureType::EMPTY_SITE:
                        _emptySites.emplace_back(std::static_pointer_cast<EmptySite>(newStructure));
                        break;
                    case StructureType::BARRACKS_KNIGHT:
                        targetTeamState.barracksKnights.emplace_back(std::static_pointer_cast<BarracksKnights>(newStructure));
                        break;
                    case StructureType::BARRACKS_ARCHER:
                        targetTeamState.barracksArchers.emplace_back(std::static_pointer_cast<BarracksArchers>(newStructure));
                        break;
                    case StructureType::BARRACKS_GIANT:
                        targetTeamState.barracksGiants.emplace_back(std::static_pointer_cast<BarracksGiants>(newStructure));
                        break;
                    case StructureType::TOWER:
                        targetTeamState.towers.emplace_back(std::static_pointer_cast<Tower>(newStructure));
                        break;
                    case StructureType::MINE:
                        targetTeamState.mines.emplace_back(std::static_pointer_cast<Mine>(newStructure));
                        break;
                }
            }
            else
            {
                DBG_INFO("[ERROR] Can't create structure object.");
            }
        }
        DBG_INFO("[INPUT] Finished creating site objects.");

        int numUnits;

        std::cin >> numUnits; std::cin.ignore();
        DBG_INPUT(numUnits);

        DBG_INFO("[INPUT] Start creating unit objects.");
        for(auto cntUnit = 0; cntUnit < numUnits; ++cntUnit)
        {
            std::shared_ptr<Unit> newUnit = Unit::createUnitFromInput();

            TeamState& targetTeamState = newUnit->getTeam() == 0 ? _friendlyTeam : _enemyTeam;
            switch (newUnit->getType())
            {
                case UnitType::KNIGHT:
                    targetTeamState.knights.emplace_back(std::static_pointer_cast<Knight>(newUnit));
                    break;
                case UnitType::ARCHER:
                    targetTeamState.archers.emplace_back(std::static_pointer_cast<Archer>(newUnit));
                    break;
                case UnitType::GIANT:
                    targetTeamState.giants.emplace_back(std::static_pointer_cast<Giant>(newUnit));
                    break;
                case UnitType::QUEEN:
                    targetTeamState.queen = std::static_pointer_cast<Queen>(newUnit);
                    break;
            }
        }
        DBG_INFO("[INPUT] Finished creating unit objects.");
    }

    inline void queenWAIT() { std::cout << "WAIT" << std::endl; _queenOrdered = true; }
    inline void queenMOVE(const Position& pos) { std::cout << "MOVE " << pos.x << " " << pos.y << std::endl; _queenOrdered = true; }
    inline void queenBUILD(int siteId, StructureType sType)
    {
        std::cout << "BUILD " << siteId << " " << structureTypeToString(sType) << std::endl;
        _queenOrdered = true;
    }



    inline void takeAction()
    {
        constexpr int avgGoldPerBarracks = 60;
        constexpr int nbEnemyTowersTriggerGiant = 5;
        constexpr int nbFriendlyTowersMax = 4;
        constexpr int nbArchersMax = 2;
        constexpr int nbMinesMin = 3;
        constexpr int minAvgArcherHp = 30;
        constexpr int priceOfArchers = 100;
        constexpr int priceOfKnights = 80;
        constexpr int priceOfGiant = 140;

        _queenOrdered = false;
        _saveGold = 0;
        if(!_emptySites.empty())
        {
            DBG_INFO("[STRAT] Empty sites exist.");

            // sort the empty places by distance from the queen
            DBG_INFO("[STRAT] Sorting empty sites by distance to our queen...");
            std::sort(_emptySites.begin(), _emptySites.end(),
                      [&](const std::shared_ptr<EmptySite>& a,
                      const std::shared_ptr<EmptySite>& b) -> bool
            {
                return _friendlyTeam.queen->distanceTo(*a) < _friendlyTeam.queen->distanceTo(*b);
            });

            bool minesCanBeUpgraded = false;
            for(const std::shared_ptr<Mine>& minePtr : _friendlyTeam.mines)
            {
                if(minePtr->getMineSize() < minePtr->getMaxMineSize())
                {
                    minesCanBeUpgraded = true;
                    break;
                }
            }

            if(_friendlyTeam.mines.size() < nbMinesMin)
            {
                DBG_INFO("[STRAT] Need more mines - lets expand");
                for(const std::shared_ptr<EmptySite>& site : _emptySites)
                {
                    site->print();
                    if(site->getGoldAvailable() != 0)
                    {
                        queenBUILD(_emptySites.front()->getSiteId(), StructureType::MINE);
                        break;
                    }
                }
            }
            else if(_friendlyTeam.towers.size() < nbFriendlyTowersMax)
            {
                queenBUILD(_emptySites.front()->getSiteId(), StructureType::TOWER);
            }
            else if(minesCanBeUpgraded)
            {
                // sort the empty places by distance from the queen
                DBG_INFO("[STRAT] Sorting mines by distance to our queen...");
                std::sort(_friendlyTeam.mines.begin(), _friendlyTeam.mines.end(),
                          [&](const std::shared_ptr<Mine>& a,
                          const std::shared_ptr<Mine>& b) -> bool
                {
                    return _friendlyTeam.queen->distanceTo(*a) < _friendlyTeam.queen->distanceTo(*b);
                });

                for(const std::shared_ptr<Mine>& minePtr : _friendlyTeam.mines)
                {
                    if(minePtr->getMineSize() < minePtr->getMaxMineSize())
                    {
                        DBG_INFO("Mine with id (" << minePtr->getSiteId() << ") is level "
                                 << minePtr->getMineSize() << " of " << minePtr->getMaxMineSize()
                                 << ", attempting to upgrade.");
                        queenBUILD(minePtr->getSiteId(), StructureType::MINE);
                        break;
                    }
                }
            }
            else if(_friendlyTeam.barracksArchers.size() + _friendlyTeam.barracksKnights.size() + _friendlyTeam.barracksGiants.size() < (_gold / avgGoldPerBarracks))
            {
                StructureType newBarracksType = StructureType::BARRACKS_KNIGHT;
                if(_friendlyTeam.barracksArchers.empty())
                {
                    DBG_INFO("[STRAT] No archer barracks - let's build some.");
                    newBarracksType = StructureType::BARRACKS_ARCHER;
                }
                else if(_friendlyTeam.barracksGiants.empty() && _enemyTeam.towers.size() > nbEnemyTowersTriggerGiant)
                {
                    DBG_INFO("[STRAT] Enemy team has more than " << nbEnemyTowersTriggerGiant << " towers - lets create some giant barracks.");
                    newBarracksType = StructureType::BARRACKS_GIANT;
                }
                else
                {
                    DBG_INFO("[STRAT] We have enough money so let's make some knights barracks.");
                    newBarracksType = StructureType::BARRACKS_KNIGHT;
                }

                queenBUILD(_emptySites.front()->getSiteId(), newBarracksType);
            }
            else
            {
                DBG_INFO("[STRAT] We have enough barracks let's avoid those enemy knights");

                queenMOVE(_friendlyTeam.barracksArchers[0]->getPosition());
            }


        }
        else
        {
            DBG_INFO("[STRAT] We have enough barracks let's avoid those enemy knights");

            queenMOVE(_friendlyTeam.barracksArchers[0]->getPosition());
        }

        if(!_queenOrdered)
        {
            queenWAIT();
        }

        DBG_INFO("[STRAT] Evaluating training opportunities - current gold: " << _gold);
        {
            std::vector<int> barracksToTrain;
            barracksToTrain.reserve(_friendlyTeam.barracksArchers.size()+_friendlyTeam.barracksKnights.size()+_friendlyTeam.barracksGiants.size());

            if(_gold > priceOfKnights)
            {
                DBG_INFO("[STRAT] We have at least 80 gold - we can train units");

                int averageHealthArchers = 0;
                for(const std::shared_ptr<Archer>& archerPtr : _friendlyTeam.archers)
                {
                    averageHealthArchers+= archerPtr->getHealth();
                }
                if(_friendlyTeam.archers.size() > 0)
                {
                    averageHealthArchers /= _friendlyTeam.archers.size();
                    DBG_INFO("[STRAT] Average health of archers - " << averageHealthArchers);
                }
                else
                {
                    averageHealthArchers = 100;
                }
                bool archersExpiringSoon = !_friendlyTeam.archers.empty() && averageHealthArchers < minAvgArcherHp;
                bool needArchers = _friendlyTeam.archers.size() < nbArchersMax || archersExpiringSoon;
                bool needGiants = _friendlyTeam.giants.empty() && _enemyTeam.towers.size() > nbEnemyTowersTriggerGiant;

                DBG_INFO("[STRAT] Need archers - " << needArchers << ", need giants - " << needGiants << ", archersExpiringSoon - " << archersExpiringSoon);
                if(needArchers && getAvailableGold() < priceOfArchers)
                {
                    DBG_INFO("[STRAT] We need to build archers soon - let's save some gold.");
                    _saveGold += priceOfArchers;
                }
                if(needGiants && getAvailableGold() < priceOfGiant)
                {
                    _saveGold += priceOfGiant;
                }

                if(needArchers)
                {
                    DBG_INFO("[STRAT] We need archers");
                    if(!_friendlyTeam.barracksArchers.empty())
                    {
                        DBG_INFO("[STRAT] We have archer barracks");
                        if(_gold > priceOfArchers)
                        {
                            DBG_INFO("[STRAT] We have enough money for archers");
                            if(_friendlyTeam.barracksArchers[0]->getTurnsUntilTrain() > 0)
                            {
                                DBG_INFO("[STRAT] We have to wait to train archers for " << _friendlyTeam.barracksArchers[0]->getTurnsUntilTrain() << " more turns.");
                            }
                            else
                            {
                                barracksToTrain.emplace_back(_friendlyTeam.barracksArchers[0]->getSiteId());
                                _gold -= priceOfArchers;
                            }
                        }
                    }

                }
                if(needGiants)
                {
                    DBG_INFO("[STRAT] We need giants");
                    if(!_friendlyTeam.barracksGiants.empty())
                    {
                        DBG_INFO("[STRAT] We have giants barracks");
                        if(_gold < priceOfGiant)
                        {
                            DBG_INFO("[STRAT] We have enough money for giants");
                            if(_friendlyTeam.barracksGiants[0]->getTurnsUntilTrain() > 0)
                            {
                                DBG_INFO("[STRAT] We have to wait to train giants for " << _friendlyTeam.barracksArchers[0]->getTurnsUntilTrain() << " more turns.");
                            }
                            else
                            {
                                barracksToTrain.emplace_back(_friendlyTeam.barracksGiants[0]->getSiteId());
                                _gold -= priceOfGiant;
                            }
                        }
                    }

                }

                for(std::shared_ptr<Barracks> currentBarracks : _friendlyTeam.barracksKnights)
                {
                    if(_gold < _saveGold)
                    {
                        DBG_INFO("[STRAT] We need money for archers/giants - pause training of knights and try to save.");
                        break;
                    }
                    if(_gold < priceOfKnights)
                    {
                        DBG_INFO("[STRAT] No more money for training knights.");
                        break;
                    }
                    if(currentBarracks->getTurnsUntilTrain() > 0)
                    {
                        DBG_INFO("[STRAT] Barracks(" << currentBarracks->getSiteId() << ") has " << currentBarracks->getTurnsUntilTrain() << " more turns until training is available.");
                        continue;
                    }

                    barracksToTrain.emplace_back(currentBarracks->getSiteId());
                    _gold -= priceOfKnights;
                    DBG_INFO("[STRAT] We have money for kingts - lets do this shit!");
                }

            }

            std::cout << "TRAIN";
            if(!barracksToTrain.empty())
            {
                for(int id : barracksToTrain)
                {
                    std::cout << " " << id;
                }
            }
            std::cout << std::endl;

        }
    }

    inline void processOneTurn()
    {
        DBG_INFO("Starting turn " << _currentTurn);
        auto startTurn = std::chrono::high_resolution_clock::now();
        readTurnInput();
        using namespace std::chrono_literals;
        auto endReadInput = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> inputTime = std::chrono::duration_cast<std::chrono::microseconds>(endReadInput-startTurn);
        DBG_INFO("[TIME] Input : " << std::fixed << inputTime.count());
        takeAction();
        auto endTurn = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> actionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTurn-endReadInput);
        DBG_INFO("[TIME] Action : " << std::fixed << actionTime.count());
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
    bool _queenOrdered;
    int _saveGold;
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
