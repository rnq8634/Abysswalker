// EnemySpawner.h
#ifndef __ENEMYSPAWNER_H__
#define __ENEMYSPAWNER_H__

#include <vector>

// Forward declarations
class SceneAbyssWalker;
class Renderer;
class Player;
class EnemyBat;   
class EnemyType2; 

class EnemySpawner
{
public:
    enum class EnemySpawnType { BAT, TYPE2 };

    EnemySpawner(SceneAbyssWalker* scene, Player* player, Renderer* renderer);
    ~EnemySpawner();

    void Update(float deltaTime, const std::vector<EnemyBat*>& currentBats, const std::vector<EnemyType2*>& currentType2s);
    void Reset(); 

private:
    void AttemptSpawn(EnemySpawnType type, const std::vector<EnemyBat*>& currentBats, const std::vector<EnemyType2*>& currentType2s);

    void SpawnBat(bool spawnOnLeft);
    void SpawnType2(bool spawnOnLeft);

    SceneAbyssWalker* m_pScene;    
    Player* m_pPlayer;             
    Renderer* m_pRenderer;         

    float m_batSpawnTimer;
    const float m_batSpawnInterval = 3.5f;
    const int m_maxBats = 10;

    float m_type2SpawnTimer;
    const float m_type2SpawnInterval = 6.0f;
    const int m_maxType2 = 5;

    const int m_maxEnemies = 30; 
};

#endif // __ENEMYSPAWNER_H__