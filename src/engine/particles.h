#ifndef PARTICLES_H
#define PARTICLES_H

#include "types.h"

Particle_System*	new_particle_system		(void);
void			reset_particle_system		(Particle_System* ps);
void			update_particles		(Particle_System* ps, float dt);
void			draw_particles			(Particle_System* ps, Game_Assets* assets);

void			init_particle			(Particle* p, Game_Shape_Types shape);
Uint32			get_new_particle		(Particle_System* ps);
Uint32			spawn_particle			(Particle_System* ps, Game_Sprite* sprite, Game_Shape_Types shape);
void			randomize_particle		(Particle* p, RGBA_Color* colors, Uint32 color_count);

Uint32			get_new_particle_emitter	(Particle_System* ps);
Particle_Emitter*	get_particle_emitter		(Particle_System* ps, Uint32 index);
void			remove_particle_emitter		(Particle_System* ps, Uint32 index);

void			displace_particles		(Particle_System* ps, Transform2D transform, Game_Shape shape);
void			wrap_particles			(Particle_System* ps, Rectangle bounds);
void			explode_at_point		(Particle_System* ps,
							 float x, float y, 
							 RGBA_Color* colors, Uint32 num_colors, 
							 Game_Sprite* sprite, Game_Shape_Types shape);
void			explode_sprite			(Game_Assets* assets, Particle_System* ps, 
							 Game_Sprite* sprite, 
							 float x, float y, float angle,
							 int pieces);

#endif
