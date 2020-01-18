#ifndef KN_GAME_H
#define KN_GAME_H

typedef struct {
	bool (*init)(void);
	void (*shutdown)(void);
	bool (*tick)(void);
	void (*draw)(void);
} Game;

void Game_Init(const char* target);
void Game_Draw(void);
void Game_Tick(uint64_t dt);
void Game_Shutdown();

#endif /* KN_GAME_H */
