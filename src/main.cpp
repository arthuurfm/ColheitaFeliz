#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <nlohmann/json.hpp>
#include <iostream>
using namespace std;

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr int FPS = 60;
constexpr int TIME_FRAME_MS = 1000 / FPS;

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

  bool is_running = true;
  SDL_Event event; // controla todos os eventos.
  Uint32 previous_time = SDL_GetTicks(); // pega todo o tempo desde a inicialização.
  float deltaTime = 0.0f;

  while (is_running) {
    Uint32 current_time = SDL_GetTicks();
    deltaTime = (current_time - previous_time) / 1000.0f;
    previous_time = current_time;

    // se tiver eventos na fila.
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = false;
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