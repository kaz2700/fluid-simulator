#include "gpu_compute.hpp"

// Phase 14: Compute shader for density calculation
const char* GPUCompute::densityShaderSource = R"(#version 430 core

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

// SSBOs
layout(std430, binding = 0) readonly buffer Positions {
    vec2 positions[];
};

layout(std430, binding = 1) writeonly buffer Densities {
    float densities[];
};

// Uniform parameters
layout(std430, binding = 5) readonly buffer Params {
    float h;
    float m;
    float rho0;
    float B;
    float mu;
    float gamma;
    float dt;
    float gravity;
    int numParticles;
    float domainMin;
    float domainMax;
    float damping;
    float padding;
} params;

const float PI = 3.14159265359;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= params.numParticles) return;
    
    float h = params.h;
    float h2 = h * h;
    float h6 = h2 * h2 * h2;
    float h9 = h6 * h2 * h;
    float poly6Coeff = 315.0 / (64.0 * PI * h9);
    float selfContribution = params.m * poly6Coeff * h2 * h2 * h2;
    
    float density = 0.0;
    vec2 pi = positions[i];
    
    // Brute force neighbor search (simple but O(n²))
    // In production, use spatial hash on GPU
    for (int j = 0; j < params.numParticles; j++) {
        vec2 pj = positions[j];
        vec2 diff = pi - pj;
        float r2 = dot(diff, diff);
        
        if (r2 < h2) {
            float d = h2 - r2;
            float d3 = d * d * d;
            density += params.m * poly6Coeff * d3;
        }
    }
    
    density += selfContribution;
    densities[i] = density;
}
)";

// Phase 14: Compute shader for pressure calculation
const char* GPUCompute::pressureShaderSource = R"(#version 430 core

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 2) readonly buffer Densities {
    float densities[];
};

layout(std430, binding = 3) writeonly buffer Pressures {
    float pressures[];
};

layout(std430, binding = 5) readonly buffer Params {
    float h;
    float m;
    float rho0;
    float B;
    float mu;
    float gamma;
    float dt;
    float gravity;
    int numParticles;
    float domainMin;
    float domainMax;
    float damping;
    float padding;
} params;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= params.numParticles) return;
    
    float ratio = densities[i] / params.rho0;
    float pressure = params.B * (pow(ratio, params.gamma) - 1.0);
    pressures[i] = max(pressure, 0.0);
}
)";

// Phase 14: Compute shader for force calculation (pressure + viscosity)
const char* GPUCompute::forcesShaderSource = R"(#version 430 core

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) readonly buffer Positions {
    vec2 positions[];
};

layout(std430, binding = 1) readonly buffer Velocities {
    vec2 velocities[];
};

layout(std430, binding = 2) readonly buffer Densities {
    float densities[];
};

layout(std430, binding = 3) readonly buffer Pressures {
    float pressures[];
};

layout(std430, binding = 4) writeonly buffer Accelerations {
    vec2 accelerations[];
};

layout(std430, binding = 5) readonly buffer Params {
    float h;
    float m;
    float rho0;
    float B;
    float mu;
    float gamma;
    float dt;
    float gravity;
    int numParticles;
    float domainMin;
    float domainMax;
    float damping;
    float padding;
} params;

const float PI = 3.14159265359;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= params.numParticles) return;
    
    float h = params.h;
    float h2 = h * h;
    float spikyCoeff = -45.0 / (PI * h * h * h * h * h * h);
    float viscosityCoeff = 45.0 / (PI * h * h * h * h * h);
    
    vec2 f_pressure = vec2(0.0);
    vec2 f_viscosity = vec2(0.0);
    
    vec2 pi = positions[i];
    vec2 vi = velocities[i];
    float rhoi = densities[i];
    float Pi = pressures[i];
    
    // Brute force neighbor search
    for (int j = 0; j < params.numParticles; j++) {
        if (i == j) continue;
        
        vec2 pj = positions[j];
        vec2 r_vec = pi - pj;
        float r2 = dot(r_vec, r_vec);
        
        if (r2 < h2 && r2 > 1e-8) {
            float r = sqrt(r2);
            float r_h = h - r;
            
            // Pressure force (spiky gradient)
            float pressure_term = (Pi + pressures[j]) / (2.0 * densities[j]);
            vec2 gradW = spikyCoeff * r_h * r_h / r * r_vec;
            f_pressure -= params.m * pressure_term * gradW;
            
            // Viscosity force (laplacian)
            float laplacian = viscosityCoeff * (h - r);
            vec2 v_diff = velocities[j] - vi;
            f_viscosity += params.m * v_diff / densities[j] * laplacian;
        }
    }
    
    f_viscosity *= params.mu;
    
    accelerations[i] = (f_pressure + f_viscosity) / rhoi;
}
)";

// Phase 14: Compute shader for integration
const char* GPUCompute::integrateShaderSource = R"(#version 430 core

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer Positions {
    vec2 positions[];
};

layout(std430, binding = 1) buffer Velocities {
    vec2 velocities[];
};

layout(std430, binding = 4) readonly buffer Accelerations {
    vec2 accelerations[];
};

layout(std430, binding = 5) readonly buffer Params {
    float h;
    float m;
    float rho0;
    float B;
    float mu;
    float gamma;
    float dt;
    float gravity;
    int numParticles;
    float domainMin;
    float domainMax;
    float damping;
    float padding;
} params;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= params.numParticles) return;
    
    vec2 acc = accelerations[i];
    
    // Apply gravity
    acc.y += params.gravity;
    
    // Clamp acceleration
    float maxAcc = 50.0;
    float accMag = length(acc);
    if (accMag > maxAcc) {
        acc = (acc / accMag) * maxAcc;
    }
    
    // Velocity Verlet integration
    velocities[i] += 0.5 * acc * params.dt;
    positions[i] += velocities[i] * params.dt;
    velocities[i] += 0.5 * acc * params.dt;
}
)";

// Phase 14: Compute shader for boundary handling
const char* GPUCompute::boundaryShaderSource = R"(#version 430 core

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer Positions {
    vec2 positions[];
};

layout(std430, binding = 1) buffer Velocities {
    vec2 velocities[];
};

layout(std430, binding = 5) readonly buffer Params {
    float h;
    float m;
    float rho0;
    float B;
    float mu;
    float gamma;
    float dt;
    float gravity;
    int numParticles;
    float domainMin;
    float domainMax;
    float damping;
    float padding;
} params;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= params.numParticles) return;
    
    vec2 pos = positions[i];
    vec2 vel = velocities[i];
    
    if (pos.x < params.domainMin) {
        pos.x = params.domainMin;
        vel.x *= -params.damping;
    } else if (pos.x > params.domainMax) {
        pos.x = params.domainMax;
        vel.x *= -params.damping;
    }
    
    if (pos.y < params.domainMin) {
        pos.y = params.domainMin;
        vel.y *= -params.damping;
    } else if (pos.y > params.domainMax) {
        pos.y = params.domainMax;
        vel.y *= -params.damping;
    }
    
    positions[i] = pos;
    velocities[i] = vel;
}
)";

GPUCompute::GPUCompute()
    : available(false), maxParticles(0), numParticles(0),
      positionsSSBO(0), velocitiesSSBO(0), accelerationsSSBO(0),
      densitiesSSBO(0), pressuresSSBO(0), paramsSSBO(0),
      densityProgram(0), pressureProgram(0), forcesProgram(0),
      integrateProgram(0), boundaryProgram(0) {
}

GPUCompute::~GPUCompute() {
    if (!available) return;
    
    // Delete SSBOs
    if (positionsSSBO) glDeleteBuffers(1, &positionsSSBO);
    if (velocitiesSSBO) glDeleteBuffers(1, &velocitiesSSBO);
    if (accelerationsSSBO) glDeleteBuffers(1, &accelerationsSSBO);
    if (densitiesSSBO) glDeleteBuffers(1, &densitiesSSBO);
    if (pressuresSSBO) glDeleteBuffers(1, &pressuresSSBO);
    if (paramsSSBO) glDeleteBuffers(1, &paramsSSBO);
    
    // Delete programs
    if (densityProgram) glDeleteProgram(densityProgram);
    if (pressureProgram) glDeleteProgram(pressureProgram);
    if (forcesProgram) glDeleteProgram(forcesProgram);
    if (integrateProgram) glDeleteProgram(integrateProgram);
    if (boundaryProgram) glDeleteProgram(boundaryProgram);
}

bool GPUCompute::initialize(size_t maxParticles_) {
    maxParticles = maxParticles_;
    
    // Check OpenGL version
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    
    if (major < 4 || (major == 4 && minor < 3)) {
        std::cerr << "[GPU] OpenGL 4.3+ required for compute shaders (found " 
                  << major << "." << minor << ")" << std::endl;
        return false;
    }
    
    std::cout << "[GPU] OpenGL " << major << "." << minor << " detected" << std::endl;
    
    // Create SSBOs
    glGenBuffers(1, &positionsSSBO);
    glGenBuffers(1, &velocitiesSSBO);
    glGenBuffers(1, &accelerationsSSBO);
    glGenBuffers(1, &densitiesSSBO);
    glGenBuffers(1, &pressuresSSBO);
    glGenBuffers(1, &paramsSSBO);
    
    // Initialize SSBOs with max capacity
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelerationsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pressuresSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUParams), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    // Create compute programs
    densityProgram = createComputeProgram(densityShaderSource);
    pressureProgram = createComputeProgram(pressureShaderSource);
    forcesProgram = createComputeProgram(forcesShaderSource);
    integrateProgram = createComputeProgram(integrateShaderSource);
    boundaryProgram = createComputeProgram(boundaryShaderSource);
    
    if (!densityProgram || !pressureProgram || !forcesProgram || 
        !integrateProgram || !boundaryProgram) {
        std::cerr << "[GPU] Failed to create compute programs" << std::endl;
        return false;
    }
    
    available = true;
    std::cout << "[GPU] Compute shaders initialized with capacity for " 
              << maxParticles << " particles" << std::endl;
    
    return true;
}

void GPUCompute::uploadParticles(const std::vector<glm::vec2>& positions,
                                  const std::vector<glm::vec2>& velocities,
                                  const std::vector<glm::vec2>& accelerations,
                                  const std::vector<float>& densities,
                                  const std::vector<float>& pressures) {
    if (!available) return;
    
    numParticles = positions.size();
    if (numParticles > maxParticles) {
        std::cerr << "[GPU] Warning: Particle count (" << numParticles 
                  << ") exceeds max capacity (" << maxParticles << ")" << std::endl;
        numParticles = maxParticles;
    }
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(glm::vec2), positions.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(glm::vec2), velocities.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelerationsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(glm::vec2), accelerations.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(float), densities.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pressuresSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(float), pressures.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GPUCompute::downloadParticles(std::vector<glm::vec2>& positions,
                                    std::vector<glm::vec2>& velocities,
                                    std::vector<glm::vec2>& accelerations,
                                    std::vector<float>& densities,
                                    std::vector<float>& pressures) {
    if (!available) return;
    
    positions.resize(numParticles);
    velocities.resize(numParticles);
    accelerations.resize(numParticles);
    densities.resize(numParticles);
    pressures.resize(numParticles);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(glm::vec2), positions.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(glm::vec2), velocities.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelerationsSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(glm::vec2), accelerations.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(float), densities.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pressuresSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numParticles * sizeof(float), pressures.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GPUCompute::setParams(const GPUParams& params) {
    if (!available) return;
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUParams), &params);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GPUCompute::computeDensities() {
    if (!available) return;
    
    glUseProgram(densityProgram);
    
    // Bind SSBOs
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, densitiesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, paramsSSBO);
    
    // Dispatch
    GLuint numGroups = (numParticles + 255) / 256;
    glDispatchCompute(numGroups, 1, 1);
    
    // Barrier to ensure writes complete
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GPUCompute::computePressures() {
    if (!available) return;
    
    glUseProgram(pressureProgram);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, densitiesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pressuresSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, paramsSSBO);
    
    GLuint numGroups = (numParticles + 255) / 256;
    glDispatchCompute(numGroups, 1, 1);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GPUCompute::computeForces() {
    if (!available) return;
    
    glUseProgram(forcesProgram);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, densitiesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pressuresSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, accelerationsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, paramsSSBO);
    
    GLuint numGroups = (numParticles + 255) / 256;
    glDispatchCompute(numGroups, 1, 1);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GPUCompute::integrate() {
    if (!available) return;
    
    glUseProgram(integrateProgram);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, accelerationsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, paramsSSBO);
    
    GLuint numGroups = (numParticles + 255) / 256;
    glDispatchCompute(numGroups, 1, 1);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GPUCompute::applyBoundaries() {
    if (!available) return;
    
    glUseProgram(boundaryProgram);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, paramsSSBO);
    
    GLuint numGroups = (numParticles + 255) / 256;
    glDispatchCompute(numGroups, 1, 1);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GPUCompute::step(float dt) {
    if (!available) return;
    
    // Update the timestep parameter for the GPU shaders
    GPUParams params;
    glGetBufferSubData(paramsSSBO, 0, sizeof(GPUParams), &params);
    params.dt = dt;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUParams), &params);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    computeDensities();
    computePressures();
    computeForces();
    integrate();
    applyBoundaries();
}

GLuint GPUCompute::createComputeProgram(const char* source) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "[GPU] Compute shader compilation failed:\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "[GPU] Program linking failed:\n" << infoLog << std::endl;
        glDeleteProgram(program);
        glDeleteShader(shader);
        return 0;
    }
    
    glDeleteShader(shader);
    return program;
}

void GPUCompute::checkGLError(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[GPU] OpenGL error in " << operation << ": " << error << std::endl;
    }
}
