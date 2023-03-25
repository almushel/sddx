#include "assets.h"
#include "graphics.h"
#include "game_math.h"

#define HUD_TOP 536
#define HUD_SPEED 4
#define MENU_COLOR (RGB_Color){56, 56, 56}

float hudDir = 0, hudAccumulator = 0;

Game_Poly2D meterOuterPoly = {
	.vertices = {
		{ .x =  -50, .y =   10 },
		{ .x =  -44, .y =  -10 },
		{ .x =   52, .y =  -10 },
		{ .x =   46, .y =   10 }
	},
	.vert_count = 4,
};

Game_Poly2D meterInnerPoly = {
	.vertices = {
		{ .x =  -47, .y =   8 },
		{ .x =  -42, .y =  -8 },
		{ .x =   49, .y =  -8 },
		{ .x =   44, .y =   8 }
	},
	.vert_count = 4,
};

Game_Poly2D heatOuterPoly = {
	.vertices = {
		{ .x =  -44, .y =   10 },
		{ .x =  -50, .y =  -10 },
		{ .x =   46, .y =  -10 },
		{ .x =   52, .y =   10 }
	},
	.vert_count = 4,
};

Game_Poly2D heatInnerPoly = {
	.vertices = {
		{ .x =  -42, .y =   8 },
		{ .x =  -47, .y =  -8 },
		{ .x =   42, .y =  -8 },
		{ .x =   47, .y =   8 }
	},
	.vert_count = 4,
};

Game_Poly2D meterBG = {
	.vertices = {
		{ .x =  -125, .y =   22 },
		{ .x =  -113, .y =  -22 },
		{ .x =   113, .y =  -22 },
		{ .x =   125, .y =   22 }
	},
	.vert_count = 4,
};

Game_Poly2D weapon_and_score_bg = {
	.vertices = {
		{ .x =  -125, .y =   32 },
		{ .x =  -113, .y =  -32 },
		{ .x =   113, .y =  -32 },
		{ .x =   125, .y =   32 }
	},
	.vert_count = 4,
};

static RGB_Color tmColorOuter = {17, 17, 17};
static RGB_Color tmColorInner = {109, 194, 255};

static RGB_Color hmColorOuter = {128, 128, 128};
static RGB_Color hmColorInner = {255, 0, 0};

float scoreMetrics = 0;
float multiMetrics = 0;

void draw_player_lives(Game_State* game) {
		Game_Poly2D player_lives_poly = {
			.vertices = {
				{ .x =  0 , .y = -20 },
				{ .x =  13, .y =  20 },
				{ .x = -13, .y =  20 },
			},
			.vert_count = 3,
		};
		render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(player_lives_poly, (Vector2){SCREEN_WIDTH / 2 + 1, SCREEN_HEIGHT - 20}).vertices, player_lives_poly.vert_count, SD_BLUE);
//		drawPolygon(canvas.width / 2 + 1, canvas.height - 20, [{ x: 0, y: -20 }, { x: 13, y: 20 }, { x: -13, y: 20 }], '#6DC2FF', true);

		int player_width  = 0;
		int player_height = 0;		
		SDL_Texture* player_ship_texture = game_get_texture(game, "Player Ship");
		SDL_QueryTexture(player_ship_texture, 0, 0, &player_width, &player_height);
		float 	wScale = player_width / 1.7f,
				hRatio = player_height / player_width,
				hScale = wScale * hRatio;
		
//		shadowBlur = 2;
//		shadowColor = 'black';

		render_draw_texture(game->renderer, player_ship_texture, SCREEN_WIDTH / 2 - (hScale / 2) + 1, SCREEN_HEIGHT + 1, -90.0f, true);

//		font = '15px Orbitron';
//		textAlign = 'center';
//		fillStyle = 'white';
//		strokeStyle = 'black';
//		lineWidth = 2;
//		ctx.strokeText(p1.lives, canvas.width / 2 + 1, canvas.height - 8);
//		ctx.fillText(p1.lives, canvas.width / 2 + 1, canvas.height - 8);

		SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
		SDL_Rect lives_text_rect = {SCREEN_WIDTH / 2 + 1, SCREEN_HEIGHT - 8, 12, 15};
		SDL_RenderFillRect(game->renderer, &lives_text_rect);
}

void draw_thrust_meter(Game_State* game) {
	float thrust_energy = 0;
	if (game->player) thrust_energy = game->player->timer;

	tmColorInner = (RGB_Color){
		(209 - 		 thrust_energy),
		(2   * 		 thrust_energy),
		(5   + 2.5 * thrust_energy),
	};

	render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(meterOuterPoly, (Vector2){SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT - 16}).vertices, meterOuterPoly.vert_count, tmColorOuter);

	float thrustDelta = thrust_energy / 100;
	meterInnerPoly.vertices[2].x = -41 + (int)(thrustDelta * 90);
	meterInnerPoly.vertices[3].x = -41 + (int)(thrustDelta * 90) - 5;
	
	render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(meterInnerPoly, (Vector2){SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT - 16}).vertices, meterInnerPoly.vert_count, tmColorInner);
}

void draw_score(Game_State* game) {
//	updateChainTimer();

/*
		if (scoreMetrics !== null) {
			ctx.clearRect(82, canvas.height - 24, Math.round(scoreMetrics.width + 12), 18);
			ctx.save();
			ctx.globalAlpha = 0.6;
			colorRect(82, canvas.height - 24, Math.round(scoreMetrics.width + 12), 18, '#383838')
			ctx.restore();
		}
*/
//		shadowColor = 'black';
//		shadowBlur = 2;
//		font = '20px Orbitron';
//		textAlign = 'left';
//		fillStyle = 'white';
//		scoreMetrics = ctx.measureText(currentScore);

		SDL_Rect score_rect = {88, SCREEN_HEIGHT - 7.5, 16 ,20};
		SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(game->renderer, &score_rect);
/*
		if (multiMetrics !== null) {
			let rectLeft = 186 - Math.round(multiMetrics.width / 2) - 6;
			ctx.clearRect(rectLeft, canvas.height - 52, Math.round(multiMetrics.width + 12), 18);
			ctx.save();
			ctx.globalAlpha = 0.6;
			colorRect(rectLeft, canvas.height - 52, Math.round(multiMetrics.width + 12), 18, '#383838')
			ctx.restore();
		}
*/
//		shadowColor = 'black';
//		shadowBlur = 2;
//		font = '20px Orbitron';
//		fillStyle = 'white';
//		textAlign = 'center';
//		multiMetrics = ctx.measureText('x' + currentMultiplier);
		SDL_Rect multiplier_rect = {186, SCREEN_HEIGHT - 36, 16 ,20};
		SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(game->renderer, &multiplier_rect);


/*
	for (let t = 0; t < SCORE_CHAIN_TIME; t++) {
		if (currentTimeCount > t) {
			colorRect(8 + 32 * t, canvas.height - 56, 26, 26, '#6DC2FF');
		} else {
			colorRect(8 + 32 * t, canvas.height - 56, 26, 26, 'grey');
		}
	}
*/
}

void draw_active_weapon(Game_State* game) {
		//Background polygon
//		globalAlpha = 0.6;

		render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(weapon_and_score_bg, (Vector2){SCREEN_WIDTH - 100, SCREEN_HEIGHT - 30}).vertices, weapon_and_score_bg.vert_count, MENU_COLOR);
//		globalAlpha = 1;
	
		SDL_Texture* hud_weapon = game_get_texture(game, "HUD Missile"); //getWeaponHUD(p1.activeWeapon),
					 //wAmmo = p1.activeWeapon != "Machine Gun" ? p1.ammo : INFINITY_SYMBOL;
		int hw_width, hw_height;
		SDL_QueryTexture(hud_weapon, 0, 0, &hw_width, &hw_height);
//		textBaseline = 'middle'
		SDL_Rect text_rect = {SCREEN_WIDTH - 160, SCREEN_HEIGHT - hw_height + 2, 16, 14};
		SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(game->renderer, &text_rect);
//		colorAlignedText(canvas.width - 160, canvas.height - hw_height + 2, 'center', '14px Orbitron', 'white', 'Ammo');

		text_rect.y = SCREEN_HEIGHT - 16;
		SDL_RenderFillRect(game->renderer, &text_rect);
//		colorAlignedText(canvas.width - 160, canvas.height - 16, 'center', '34px Orbitron', '#6DC2FF', wAmmo);

		text_rect.x = SCREEN_WIDTH - hw_width / 2 - 6;
		SDL_RenderFillRect(game->renderer, &text_rect);
//		colorAlignedText(canvas.width - hw_width / 2 - 6, canvas.height - hw_height + 2, 'center', '14px Orbitron', 'white', p1.activeWeapon);
		render_draw_texture(game->renderer, hud_weapon, SCREEN_WIDTH - hw_width - 6, SCREEN_HEIGHT - hw_height + 6, 0, 0);
//		ctx.drawImage(wHUD, canvas.width - hw_width - 6, canvas.height - hw_height + 6);
}

void draw_weapon_heat(Game_State* game) {
	float weapon_heat = 0;
	if (game->player) weapon_heat = game->player->timer;
	
	if (weapon_heat < 100) hmColorOuter = (RGB_Color){17, 17, 17};
	else hmColorOuter = (RGB_Color){255, 165, 0};

	//109, 194, 255
	hmColorInner = (RGB_Color){
		SDL_clamp(109 + weapon_heat, 0, 255),
		SDL_clamp(194 - 2 * weapon_heat, 0, 255),
		SDL_clamp(255 - 2.5 * weapon_heat, 0, 255),
	};

	render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(heatOuterPoly, (Vector2){SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT - 16}).vertices, heatOuterPoly.vert_count, hmColorOuter);

	float heatDelta = weapon_heat / 100.0f;
	heatInnerPoly.vertices[2].x = -41 + (int)(heatDelta * 90) - 5;
	heatInnerPoly.vertices[3].x = -41 + (int)(heatDelta * 90);
	
	render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(heatInnerPoly, (Vector2){SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT - 16}).vertices, heatInnerPoly.vert_count, hmColorInner);
}

/*
void transitionHUD(dir) {
	hudDir = dir;
}

void updateHUDTransition() {
	if (hudDir === 0) {
		return;
	}
	
	let currentPos = parseInt(hud.style.top);
	
	if (hudDir > 0) {
		hudAccumulator += HUD_SPEED * deltaT;
	} else if (hudDir < 0) {
		hudAccumulator -= HUD_SPEED * deltaT;
	}

	if (Math.abs(hudAccumulator) >= HUD_SPEED) {
		let add = hudAccumulator > 0 ? HUD_SPEED : -HUD_SPEED;
		currentPos += add;
		hudAccumulator += -add;
	}

	if (currentPos > 600) {
		currentPos = 600;
		hudDir = 0;
		hudAccumulator = 0;
	} else if (currentPos < HUD_TOP) {
		currentPos = HUD_TOP;
		hudDir = 0;
		hudAccumulator = 0;
	}
	
	hud.style.top = currentPos + 'px';
}
*/

void draw_HUD(Game_State* game) {
//	updateHUDTransition();

	if (true) {//gameState == gameStarted) {
//		ctx.globalAlpha = 0.6;
		render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(weapon_and_score_bg, (Vector2){100, SCREEN_HEIGHT - 30}).vertices, weapon_and_score_bg.vert_count, MENU_COLOR);
		render_fill_polygon(game->renderer, (SDL_FPoint*)translate_poly2d(meterBG, (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT - 22}).vertices, meterBG.vert_count, MENU_COLOR);

// 	Text
//		shadowColor = 'black';
//		shadowBlur = 5;
		SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
		SDL_Rect text_rect = {SCREEN_WIDTH / 2 - 56, SCREEN_HEIGHT - 30, 15, 10};
		SDL_RenderFillRect(game->renderer, &text_rect);
		text_rect.x = SCREEN_WIDTH / 2 + 56;
		text_rect.y = SCREEN_HEIGHT - 30;
		SDL_RenderFillRect(game->renderer, &text_rect);
//		colorAlignedText(canvas.width / 2 - 56, canvas.height - 30, 'center', '10px Orbitron', 'white', 'Thrust Power');
//		colorAlignedText(canvas.width / 2 + 56, canvas.height - 30, 'center', '10px Orbitron', 'white', 'Weapon Temp');

//		font = '20px Orbitron';
//		textAlign = 'left';
//		fillStyle = 'white';
//		ctx.fillText('Score: ', 8, canvas.height - 8);

//		textAlign = 'right';
		draw_score(game);
		draw_player_lives(game);

		draw_player_lives(game);
		draw_thrust_meter(game);
		draw_weapon_heat(game);
		draw_score(game);
		draw_active_weapon(game);
	}
}