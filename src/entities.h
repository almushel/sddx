#ifndef GAME_ENTITIES_H
#define GAME_ENTITIES_H

#include "defs.h"

typedef enum Entity_Types {
	ENTITY_TYPE_UNDEFINED,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_BULLET,
	ENTITY_TYPE_MISSILE,
	ENTITY_TYPE_LASER,
	ENTITY_TYPE_ENEMY_DRIFTER,
	ENTITY_TYPE_ENEMY_UFO,
	ENTITY_TYPE_ENEMY_TRACKER,
	ENTITY_TYPE_ENEMY_TURRET,
	ENTITY_TYPE_ENEMY_GRAPPLER,
	ENTITY_TYPE_ITEM_MISSILE,
	ENTITY_TYPE_ITEM_LIFEUP,
	ENTITY_TYPE_ITEM_LASER,
	ENTITY_TYPE_SPAWN_WARP,
	ENTITY_TYPE_DEMOSHIP,
	ENTITY_TYPE_COUNT
} Entity_Types;

typedef enum Entity_States {
	ENTITY_STATE_UNDEFINED,
	ENTITY_STATE_SPAWNING,
	ENTITY_STATE_ACTIVE,
	ENTITY_STATE_DESPAWNING,
	ENTITY_STATE_DYING,
	ENTITY_STATE_COUNT,
} Entity_States;

typedef enum Entity_Teams {
	ENTITY_TEAM_UNDEFINED = 0,
	ENTITY_TEAM_PLAYER,
	ENTITY_TEAM_ENEMY,
} Entity_Teams;

Uint32 spawn_entity(Game_State* game, Entity_Types type, Vector2 position);

#endif