#ifndef GAME_ENTITIES_H
#define GAME_ENTITIES_H

#include "game_types.h"
#include "../engine/particles.h"

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

typedef enum Entity_Flags {
	ENTITY_FLAG_EXPLOSION_ENABLED  = (1<<0),
	ENTITY_FLAG_FRICTION_DISABLED  = (1<<1),
	ENTITY_FLAG_COLLISION_TRIGGER  = (1<<2), // Collision is true/false with no physics resolution
	ENTITY_FLAG_COLLISION_DISABLED = (1<<3),

} Entity_Flags;


Entity_System* create_entity_system();
void reset_entity_system(Entity_System* es);

Uint32 get_new_entity(Entity_System* es);
Entity* get_entity(Entity_System* es, Uint32 entity_id);
Uint32 spawn_entity(Entity_System* es, Particle_System* ps, Entity_Types type, Vector2 position);

void force_circle(Entity_System* es, float x, float y, float radius, float force);

#endif
