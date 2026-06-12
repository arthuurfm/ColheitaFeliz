#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <nlohmann/json.hpp>
#include <iostream>
using namespace std;

// janela do jogo e frames.
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr int FPS = 60;
constexpr int TIME_FRAME_MS = 1000 / FPS;

// tiles da fazenda.
constexpr int TILE_WIDTH = 64;
constexpr int TILE_HEIGHT = 32;

// grid da fazenda.
constexpr int GRID_COLUMNS = 7;
constexpr int GRID_ROWS = 5;

// centraliza a fazenda na janela.
constexpr int OFFSET_X = WINDOW_WIDTH / 2;
constexpr int OFFSET_Y = 200;

// canteiros iniciais.
constexpr int INITIAL_SITES = 6;

// onde os tiles da fazendo vão renderizar em x e y.
int isoScreenX(int column, int row) {
  return (column - row) * (TILE_WIDTH / 2) + OFFSET_X;
}
int isoScreenY(int column, int row) {
  return (column + row) * (TILE_HEIGHT / 2) + OFFSET_Y;
}

// posição da coluna em que o mouse clicou.
float screenGridColumn(int mouseX, int mouseY) {
  float relX = (float)(mouseX - OFFSET_X); 
  float relY = (float)(mouseY - OFFSET_Y);

  return (relX / (TILE_WIDTH / 2.0f) + relY / (TILE_HEIGHT / 2.0f)) / 2.0f;
}

// posição da linha em que o mouse clicou.
float screenGridRow(int mouseX, int mouseY) {
  float relX = (float)(mouseX - OFFSET_X); 
  float relY = (float)(mouseY - OFFSET_Y);

  return (relY / (TILE_HEIGHT / 2.0f) + relX / (TILE_HEIGHT / 2.0f)) / 2.0f;
}

enum siteStatus {
  LOCKED = 0,
  EMPTY = 1,
  PLANTED = 2,
  RIPE = 3,
  REMAINS = 4
};

struct Site {
  int column;
  int row;
  siteStatus status;
};

// desenha o losango.
void drawFilledDiamond(SDL_Renderer* renderer, int centerX, int centerY,
int r, int g, int b, int width = TILE_WIDTH, int height = TILE_HEIGHT) {
  SDL_SetRenderDrawColor(renderer, r, g, b, 255); // cor do losango.

  for (int dy =-height / 2; dy <= height; dy++) {
    int halfWidth = (height / 2 - abs(dy)) * width / height;
    SDL_RenderDrawLine(renderer, centerX - halfWidth, centerY + dy, centerX + halfWidth,
    centerY - dy);
  }
}

// desenha o contorno do losango.
void drawOutlineDiamond(SDL_Renderer* renderer, int centerX, int centerY,
int r, int g, int b, int width = TILE_WIDTH, int height = TILE_HEIGHT) {
  int topX = centerX;
  int topY = centerY - height / 2;

  int rightX = centerX + width / 2;
  int rightY = centerY / 2;

  int leftX = centerX;
  int leftY = centerY / 2;

  int bottomX = centerX - width / 2;
  int bottomY = centerY;

  SDL_SetRenderDrawColor(renderer, r, g, b, 255); // cor do contorno.
  SDL_RenderDrawLine(renderer, topX, topY, rightX, rightY);
  SDL_RenderDrawLine(renderer, rightX, rightY, bottomX, bottomY);
  SDL_RenderDrawLine(renderer, bottomX, bottomY, leftX, leftY);
  SDL_RenderDrawLine(renderer, leftX, leftY, topX, topY);
}

int main(int agrc, char* agrv[]) {
  // checa se o áudio ou vídeo incializam.
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
    cout << "Erro ao inicializar SDL2: " << SDL_GetError() << endl;
    return 1;
  }

  // checa se o PNG (imagem) inicializa.
  if (!IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) {
    cout << "Erro ao inicializar o SDL2_Image: " << IMG_GetError() << endl;
    SDL_Quit(); // limpa o SDL da memória.
    return 1;
  }

  // checa se o TTF inicializa.
  if (TTF_Init() == -1) {
    cout << "Erro ao inicializar o SDL2_ttf: " << TTF_GetError() << endl;
    IMG_Quit(); // limpa o IMG da memória.
    SDL_Quit(); // limpa o SDL da memória.
    return 1;
  }

  // checa se o mixer inicializa. params: frequência, formato, canais e buffer.
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)) {
    cout << "Erro ao inicializar o SDL2_Mixer: " << Mix_GetError() << endl;
    TTF_Quit(); // limpa o TTF da memória.
    IMG_Quit(); // limpa o IMG da memória.
    SDL_Quit(); // limpa o SDL da memória.
    return 1;
  }
  
  // Cria a janela do jogo.
  SDL_Window* window = SDL_CreateWindow(
    "Colheita Feliz",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_SHOWN // mostra a janela imediatamente após ser criada.
  );

  if (!window) {
    cout << "Erro ao criar a janela" << SDL_GetError() << endl;
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  // objeto que cria pixels na janela.
  SDL_Renderer* renderer = SDL_CreateRenderer(
    window, 
    -1,
    SDL_RENDERER_ACCELERATED || SDL_RENDERER_PRESENTVSYNC
  );

  if (!renderer) {
    cout << "Erro ao criar o renderer" << SDL_GetError() << endl;
    SDL_DestroyWindow(window); // limpa a janela da memória.
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  array<Site, GRID_COLUMNS * GRID_ROWS> sites;
  int unlocked = 0;

  for (int row = 0; row < GRID_ROWS; row++) {
    for (int column = 0; column < GRID_COLUMNS; column++) {
      int index = row * GRID_COLUMNS + column;
      sites[index].column = column;
      sites[index].row = row;
      sites[index].status = (unlocked < INITIAL_SITES) ? EMPTY : LOCKED;
      if (unlocked < INITIAL_SITES) unlocked++;
    }
  }

  cout << "Fazenda: " << GRID_COLUMNS << "x" << GRID_ROWS << "(" << INITIAL_SITES << "desbloqueados)" << endl;

  bool is_running = true;
  SDL_Event event; // controla todos os eventos.
  Uint32 previous_time = SDL_GetTicks(); // pega todo o tempo desde a inicialização.
  float deltaTime = 0.0f;
  int siteHover = -1; // canteiro onde o mouse está em cima.
  int mouseX = 0;
  int mouseY = 0;

  while (is_running) {
    Uint32 current_time = SDL_GetTicks();
    deltaTime = (current_time - previous_time) / 1000.0f;
    previous_time = current_time;

    // se tiver eventos na fila.
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = false;
      }

      // guarda os valores de onde mouse está.
      if (event.type == SDL_MOUSEMOTION) {
        mouseX = event.motion.x;
        mouseY = event.motion.y;
      }

      // passando o mouse sobre o canteiro e passando o estado dele.
      if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (siteHover >= 0) {
          Site& s = sites[siteHover];
          cout << "Clique (" << s.column << "," << s.row << ")" << endl;
          switch (s.status) {
            case LOCKED: cout << "BLOQUEADO" << endl; break;
            case EMPTY: cout << "VAZIO" << endl; break;
            case PLANTED: cout << "PLANTADO" << endl; break;
            case RIPE: cout << "MADURO" << endl; break;
            case REMAINS: cout << "RESTOS" << endl; break;
          }
        }
      }

      if (event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_ESCAPE) {
          is_running = false;
        }
      }
    }

    // desenha uma cor.
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    // preenche a tela com a cor definida.
    SDL_RenderClear(renderer);
    // mostra tudo que foi desenhado, evita piscadas.
    SDL_RenderPresent(renderer);
  
    Uint32 frame_time = SDL_GetTicks() - current_time;
    if (frame_time < TIME_FRAME_MS) {
      // se os frames estiverem muito rápidos, ele espera/diminui.
      SDL_Delay(TIME_FRAME_MS - frame_time);
    }
  }

  SDL_DestroyRenderer(renderer); // limpa o renderer da memória.
  SDL_DestroyWindow(window); // limpa a janela da memória.
  Mix_CloseAudio(); // limpa o mixer da memória.
  TTF_Quit(); // limpa o TTF da memória.
  IMG_Quit(); // limpa o IMG da memória.
  SDL_Quit(); // limpa o SDL da memória.

  cout << "Jogo encerrado com sucesso!" << endl;

  return 0;
}