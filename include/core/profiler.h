#ifndef PROFILER_H
#define PROFILER_H

#include <SDL2/SDL.h>

// Rolling average over 60 frames (~1 second at 60fps)
#define ROLLING_AVG_FRAMES 60
// 10-second average at 100Hz (dt=0.01)
#define FPS_10S_FRAMES 1000

typedef struct Profiler {
    float physics_times[ROLLING_AVG_FRAMES];
    float render_times[ROLLING_AVG_FRAMES];
    float frame_times[ROLLING_AVG_FRAMES];
    float fps_history[FPS_10S_FRAMES];
    int current_index;
    int frame_count;
    Uint64 last_frame_start;
    Uint64 physics_start;
    Uint64 render_start;
    float avg_physics_ms;
    float avg_render_ms;
    float avg_frame_ms;
    float current_fps;
    float avg_fps_10s;
} Profiler;

void profiler_init(Profiler* prof);
void profiler_start_frame(Profiler* prof);
void profiler_start_physics(Profiler* prof);
void profiler_end_physics(Profiler* prof);
void profiler_start_render(Profiler* prof);
void profiler_end_render(Profiler* prof);
void profiler_end_frame(Profiler* prof);
void profiler_get_metrics(Profiler* prof, float* physics_ms, float* render_ms, float* frame_ms, float* fps);

// Simple text rendering for metrics overlay
void profiler_draw_metrics(SDL_Renderer* renderer, Profiler* prof, int particle_count);

#endif
