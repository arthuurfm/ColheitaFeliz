#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <nlohmann/json.hpp>
#include <iostream>
using namespace std;

int main(int agrc, char* agrv[]) {
  // checa se o áudio ou vídeo incializam.
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
    cout << "Erro ao inicializar SDL2: " << SDL_GetError() << endl;
    return 1;
  }
  
  cout << "SDL2 Inicializado com sucesso!" << endl;

  SDL_version versao;
  SDL_GetVersion(&versao); // popula 'versao'.
  cout << "Versão SDL2: " << int(versao.major) << "." << int(versao.minor) << int(versao.patch);

  // checa se o PNG (imagem) inicializa.
  if (!IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) {
    cout << "Erro ao inicializar o SDL2_Image: " << IMG_GetError() << endl;
    SDL_Quit(); // limpa o SDL da memória.
    return 1;
  }
  
  cout << "SDL2_Image (PNG) inicializado com sucesso!" << endl;

  // checa se o TTF inicializa.
  if (TTF_Init() == -1) {
    cout << "Erro ao inicializar o SDL2_ttf: " << TTF_GetError() << endl;
    IMG_Quit(); // limpa o IMG da memória.
    SDL_Quit(); // limpa o SDL da memória.
    return 1;
  }

  cout << "SDL2_ttf inicializado com sucesso!" << endl;

  // checa se o mixer inicializa. params: frequência, formato, canais e buffer.
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)) {
    cout << "Erro ao inicializar o SDL2_Mixer: " << Mix_GetError() << endl;
    TTF_Quit(); // limpa o TTF da memória.
    IMG_Quit(); // limpa o IMG da memória.
    SDL_Quit(); // limpa o SDL da memória.
    return 1;
  }

  cout << "SDL2_Mixer inicializado com sucesso!" << endl;

  nlohmann::json test;
  test["jogo"] = "Colheita Feliz";
  test["version"] = 1;

  cout << "nlohmann_json funcionando!" << endl;

  Mix_CloseAudio(); // limpa o mixer da memória.
  TTF_Quit(); // limpa o TTF da memória.
  IMG_Quit(); // limpa o IMG da memória.
  SDL_Quit(); // limpa o SDL da memória.

  cout << "\nTudo funcionando. Ambiente pronto!" << test.dump(2) << endl;

  return 0;
}