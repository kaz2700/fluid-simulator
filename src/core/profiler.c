#include "core/profiler.h"
#include <stdio.h>

void profiler_init(Profiler* prof) {
    prof->current_index = 0;
    prof->frame_count = 0;
    prof->avg_physics_ms = 0.0f;
    prof->avg_render_ms = 0.0f;
    prof->avg_frame_ms = 0.0f;
    prof->current_fps = 0.0f;
    prof->avg_fps_10s = 0.0f;
    prof->last_frame_start = SDL_GetPerformanceCounter();
    
    for (int i = 0; i < ROLLING_AVG_FRAMES; i++) {
        prof->physics_times[i] = 0.0f;
        prof->render_times[i] = 0.0f;
        prof->frame_times[i] = 0.0f;
    }
    
    for (int i = 0; i < FPS_10S_FRAMES; i++) {
        prof->fps_history[i] = 0.0f;
    }
}

void profiler_start_frame(Profiler* prof) {
    prof->last_frame_start = SDL_GetPerformanceCounter();
}

void profiler_start_physics(Profiler* prof) {
    prof->physics_start = SDL_GetPerformanceCounter();
}

void profiler_end_physics(Profiler* prof) {
    Uint64 end = SDL_GetPerformanceCounter();
    Uint64 elapsed = end - prof->physics_start;
    float ms = (float)(elapsed * 1000.0) / SDL_GetPerformanceFrequency();
    prof->physics_times[prof->current_index] = ms;
}

void profiler_start_render(Profiler* prof) {
    prof->render_start = SDL_GetPerformanceCounter();
}

void profiler_end_render(Profiler* prof) {
    Uint64 end = SDL_GetPerformanceCounter();
    Uint64 elapsed = end - prof->render_start;
    float ms = (float)(elapsed * 1000.0) / SDL_GetPerformanceFrequency();
    prof->render_times[prof->current_index] = ms;
}

void profiler_end_frame(Profiler* prof) {
    Uint64 end = SDL_GetPerformanceCounter();
    Uint64 elapsed = end - prof->last_frame_start;
    float ms = (float)(elapsed * 1000.0) / SDL_GetPerformanceFrequency();
    prof->frame_times[prof->current_index] = ms;
    
    // Calculate rolling averages
    float physics_sum = 0.0f, render_sum = 0.0f, frame_sum = 0.0f;
    int count = (prof->frame_count < ROLLING_AVG_FRAMES) ? prof->frame_count + 1 : ROLLING_AVG_FRAMES;
    
    for (int i = 0; i < count; i++) {
        physics_sum += prof->physics_times[i];
        render_sum += prof->render_times[i];
        frame_sum += prof->frame_times[i];
    }
    
    prof->avg_physics_ms = physics_sum / count;
    prof->avg_render_ms = render_sum / count;
    prof->avg_frame_ms = frame_sum / count;
    prof->current_fps = (prof->avg_frame_ms > 0.0f) ? 1000.0f / prof->avg_frame_ms : 0.0f;
    
    // Store FPS for 10s average
    int fps_index = prof->frame_count % FPS_10S_FRAMES;
    prof->fps_history[fps_index] = prof->current_fps;
    
    // Calculate 10s rolling average
    float fps_sum = 0.0f;
    int fps_count = (prof->frame_count < FPS_10S_FRAMES) ? prof->frame_count + 1 : FPS_10S_FRAMES;
    for (int i = 0; i < fps_count; i++) {
        fps_sum += prof->fps_history[i];
    }
    prof->avg_fps_10s = fps_sum / fps_count;
    
    prof->current_index = (prof->current_index + 1) % ROLLING_AVG_FRAMES;
    prof->frame_count++;
}

void profiler_get_metrics(Profiler* prof, float* physics_ms, float* render_ms, float* frame_ms, float* fps) {
    if (physics_ms) *physics_ms = prof->avg_physics_ms;
    if (render_ms) *render_ms = prof->avg_render_ms;
    if (frame_ms) *frame_ms = prof->avg_frame_ms;
    if (fps) *fps = prof->current_fps;
}

// Simple 3x5 pixel font for digits (1 = pixel, 0 = empty)
// Each digit is 3 wide x 5 tall, stored as 5 rows of 3 bits
static const unsigned char digit_patterns[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111},  // 0
    {0b010, 0b110, 0b010, 0b010, 0b111},  // 1
    {0b111, 0b001, 0b111, 0b100, 0b111},  // 2
    {0b111, 0b001, 0b111, 0b001, 0b111},  // 3
    {0b101, 0b101, 0b111, 0b001, 0b001},  // 4
    {0b111, 0b100, 0b111, 0b001, 0b111},  // 5
    {0b111, 0b100, 0b111, 0b101, 0b111},  // 6
    {0b111, 0b001, 0b001, 0b010, 0b010},  // 7
    {0b111, 0b101, 0b111, 0b101, 0b111},  // 8
    {0b111, 0b101, 0b111, 0b001, 0b111},  // 9
};

static void draw_digit(SDL_Renderer* renderer, int x, int y, int digit, int scale) {
    if (digit < 0 || digit > 9) return;
    
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green text
    
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            if (digit_patterns[digit][row] & (1 << (2 - col))) {
                SDL_Rect rect = {
                    x + col * scale,
                    y + row * scale,
                    scale,
                    scale
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

static void draw_number(SDL_Renderer* renderer, int x, int y, int number, int scale) {
    if (number == 0) {
        draw_digit(renderer, x, y, 0, scale);
        return;
    }
    
    // Handle negative numbers
    if (number < 0) {
        // Draw minus sign
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect rect = {x, y + 2 * scale, 3 * scale, scale};
        SDL_RenderFillRect(renderer, &rect);
        x += 4 * scale;
        number = -number;
    }
    
    // Count digits and draw from right to left
    int temp = number;
    int num_digits = 0;
    while (temp > 0) {
        num_digits++;
        temp /= 10;
    }
    
    temp = number;
    int digit_x = x + (num_digits - 1) * 4 * scale;
    while (temp > 0) {
        draw_digit(renderer, digit_x, y, temp % 10, scale);
        temp /= 10;
        digit_x -= 4 * scale;
    }
}

static void draw_string(SDL_Renderer* renderer, int x, int y, const char* str, int scale) {
    int current_x = x;
    
    while (*str) {
        char c = *str++;
        if (c >= '0' && c <= '9') {
            draw_digit(renderer, current_x, y, c - '0', scale);
        } else if (c == '.') {
            // Draw dot
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect rect = {current_x, y + 4 * scale, scale, scale};
            SDL_RenderFillRect(renderer, &rect);
        } else if (c == ' ') {
            // Space
        } else if (c == 'F') {
            // Draw F for FPS
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect rects[] = {
                {current_x, y, 3 * scale, scale},
                {current_x, y, scale, 5 * scale},
                {current_x, y + 2 * scale, 2 * scale, scale},
            };
            for (int i = 0; i < 3; i++) {
                SDL_RenderFillRect(renderer, &rects[i]);
            }
        } else if (c == 'P') {
            // Draw P for Physics
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect rects[] = {
                {current_x, y, 3 * scale, scale},
                {current_x, y, scale, 5 * scale},
                {current_x + 2 * scale, y + scale, scale, 2 * scale},
                {current_x, y + 2 * scale, 3 * scale, scale},
            };
            for (int i = 0; i < 4; i++) {
                SDL_RenderFillRect(renderer, &rects[i]);
            }
        } else if (c == 'R') {
            // Draw R for Render
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect rects[] = {
                {current_x, y, 3 * scale, scale},
                {current_x, y, scale, 5 * scale},
                {current_x + 2 * scale, y + scale, scale, 2 * scale},
                {current_x, y + 2 * scale, 3 * scale, scale},
                {current_x + scale, y + 3 * scale, scale, scale},
                {current_x + 2 * scale, y + 4 * scale, scale, scale},
            };
            for (int i = 0; i < 6; i++) {
                SDL_RenderFillRect(renderer, &rects[i]);
            }
        } else if (c == 'M') {
            // Draw M for ms
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect rects[] = {
                {current_x, y, scale, 5 * scale},
                {current_x + 2 * scale, y, scale, 5 * scale},
                {current_x + scale, y + scale, scale, scale},
            };
            for (int i = 0; i < 3; i++) {
                SDL_RenderFillRect(renderer, &rects[i]);
            }
        } else if (c == 's') {
            // Draw s
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect rects[] = {
                {current_x, y, 3 * scale, scale},
                {current_x, y + 2 * scale, 3 * scale, scale},
                {current_x, y + 4 * scale, 3 * scale, scale},
                {current_x, y + scale, scale, scale},
                {current_x + 2 * scale, y + 3 * scale, scale, scale},
            };
            for (int i = 0; i < 5; i++) {
                SDL_RenderFillRect(renderer, &rects[i]);
            }
        } else if (c == ':') {
            // Draw colon
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect rects[] = {
                {current_x + scale, y + scale, scale, scale},
                {current_x + scale, y + 3 * scale, scale, scale},
            };
            for (int i = 0; i < 2; i++) {
                SDL_RenderFillRect(renderer, &rects[i]);
            }
        }
        
        current_x += 4 * scale;
    }
}

void profiler_draw_metrics(SDL_Renderer* renderer, Profiler* prof, int particle_count) {
    // Draw semi-transparent background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect bg = {10, 10, 280, 120};
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    int scale = 2;
    int y = 15;
    int x = 15;

    // FPS: XX.X
    draw_string(renderer, x, y, "F:", scale);
    char fps_str[16];
    int fps_whole = (int)prof->current_fps;
    int fps_frac = (int)((prof->current_fps - fps_whole) * 10);
    snprintf(fps_str, sizeof(fps_str), "%d.%d", fps_whole, fps_frac);
    draw_string(renderer, x + 20, y, fps_str, scale);

    y += 20;

    // 10s FPS avg: XX.X
    draw_string(renderer, x, y, "F10:", scale);
    int fps10_whole = (int)prof->avg_fps_10s;
    int fps10_frac = (int)((prof->avg_fps_10s - fps10_whole) * 10);
    snprintf(fps_str, sizeof(fps_str), "%d.%d", fps10_whole, fps10_frac);
    draw_string(renderer, x + 36, y, fps_str, scale);

    y += 20;

    // Physics: XX.X ms
    draw_string(renderer, x, y, "P:", scale);
    int phys_whole = (int)prof->avg_physics_ms;
    int phys_frac = (int)((prof->avg_physics_ms - phys_whole) * 10);
    snprintf(fps_str, sizeof(fps_str), "%d.%d M", phys_whole, phys_frac);
    draw_string(renderer, x + 20, y, fps_str, scale);

    y += 20;

    // Render: XX.X ms
    draw_string(renderer, x, y, "R:", scale);
    int rend_whole = (int)prof->avg_render_ms;
    int rend_frac = (int)((prof->avg_render_ms - rend_whole) * 10);
    snprintf(fps_str, sizeof(fps_str), "%d.%d M", rend_whole, rend_frac);
    draw_string(renderer, x + 20, y, fps_str, scale);

    y += 20;

    // Particles: XXXXX
    draw_string(renderer, x, y, "P:", scale);
    snprintf(fps_str, sizeof(fps_str), "%d", particle_count);
    draw_string(renderer, x + 20, y, fps_str, scale);
}
