
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <iostream>
#include "../intern/render/context.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_video.h"
int main(int argc, char* argv[]){
    std::cout << "SDL_main" << std::endl;
    SDL_Window* window;
    
    /* 初始化 SDL 音频文件失败 */
    if (SDL_Init(SDL_INIT_VIDEO)){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "MyRender3D",
        1920,
        1080,
        SDL_WINDOW_VULKAN
    );

    if (!window) {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR, 
            "Could not create window: %s\n", 
            SDL_GetError()
        );
        SDL_Quit();
        return 1;
    }

    /* SDL 事件循环 */
    bool bDone = false;
    while(!bDone) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            /* 触发了退出事件 */
            if (event.type == SDL_EVENT_QUIT) {
                bDone = true;
            }
        }
        /* 游戏逻辑 和 渲染逻辑 */
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
