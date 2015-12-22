#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include "main.h"
#include "physics.h"
#include "model.h"
#include "color.h"
#include "light.h"
#include "string.h"
#include "logic.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define MOUSE_SPEED 0.1
#define MAX_LIGHTS 10

extern GLuint loadShaders(const char * vertex_file_path, const char * fragment_file_path);
extern GLuint loadTextureBMP(const char * texture_file_path);

extern vec3 movementDecay;
extern int useMeshing;

typedef enum ProgramType_E {
    NORMAL_PROGRAM,
    SHADOW_PROGRAM,
    PLAIN_PROGRAM,
    TEXTURE_PROGRAM,
    SKYBOX_PROGRAM
} ProgramType;

static void windowResizeFunc(GLFWwindow* window, int width, int height);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

static void initInputs(GLFWwindow* window);
static void readInputs(GLFWwindow* window);

static void initMeshes();
static void updateColorCrosshair();
static void updateColorRect();

static void useProgram(ProgramType prog);

static void sendProjectionMatrix(mat4 data);
static void sendViewMatrix(mat4 data);
static void sendModelMatrix(mat4 data);
static void sendUniformData();

// static void makeLight(vec3 position, vec3 color, float size, float radius);

// static void renderShadowMap();
static void renderWorld(mat4 view, mat4 projection);

int frame_buffer_width = 0;
int frame_buffer_height = 0;
double deltaTime = 0.0;

// don't change, obviously
static mat4 identityMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

static ProgramType currProgram;

static GLuint normalProgram, /*shadowProgram,*/ plainProgram, textureProgram, skyboxProgram;
static GLuint vao;
static GLuint /*normalLightUBO,*/ normalMaterialsUBO;
static GLuint normalModelUniformID, normalViewUniformID,
              normalProjectionUniformID, /*normalShadowMapUniformID,
              normalLightCountUniformID,*/

              /*shadowLightSourceUniformID, shadowModelUniformID,
              shadowViewUniformID, shadowProjectionUniformID,
              shadowLightRadiusUniformID,*/

              plainModelUniformID, plainViewUniformID,
              plainProjectionUniformID,

              textureModelUniformID, textureViewUniformID,
              textureProjectionUniformID, textureTextureUniformID,

              skyboxModelUniformID, skyboxViewUniformID,
              skyboxProjectionUniformID, skyboxSkyboxUniformID;

static mat4 viewMatrix;
static mat4 projectionMatrix;

static Player player;

static double currTime = 0.0;

static double mousex, mousey;
static int mouseButtons[2] = {0, 0};
static int control, inertia, wireframe;
static Selection selection;
static int placeModel = 0;
static float colorx = 0.5, colory = 0;
static Color currColor;

// static Light *light[MAX_LIGHTS];
// static int lightCount = 0;
static World *world;
static Mesh *playerModel;
static Mesh *selectedFrame;
static Mesh *crosshair;
static Model *model1;

static int ccparams[] = {80, 20, 5, 5};
static Mesh *colorChooser, *colorCrosshair, *colorRect;

static void windowResizeFunc(GLFWwindow* window, int width, int height) {
    frame_buffer_width = width;
    frame_buffer_height = height;

    // fix stuff

    identity_m4(projectionMatrix);
    perspective(projectionMatrix, (float)frame_buffer_width/frame_buffer_height, 60, 0.01, 100);

    glViewport(0, 0, frame_buffer_width, frame_buffer_height);

    freeMesh(selectedFrame);
    freeMesh(crosshair);
    freeMesh(colorChooser);
    freeMesh(colorCrosshair);

    initMeshes();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_ESCAPE:
            control = 0;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_LEFT_CONTROL:
            placeModel = action ^ GLFW_RELEASE;
            break;
        case GLFW_KEY_1:
            if (action == GLFW_RELEASE) {
                Chunk *chunk = getChunk(world, (int)(player.position[0] / BLOCK_WIDTH) >> LOG_CHUNK_SIZE,
                                            (int)(player.position[1] / BLOCK_WIDTH) >> LOG_CHUNK_SIZE,
                                            (int)(player.position[2] / BLOCK_WIDTH) >> LOG_CHUNK_SIZE);
                char* name = malloc(40);
                sprintf(name, "models/model%ld", time(NULL));
                writeModel(chunk, name);
                puts("Saved model");
                model1 = createModel();
                readModel(model1, name);
                free(name);
            }
            break;
        case GLFW_KEY_2:
            if (action == GLFW_RELEASE) {
                writeWorld(world, "worlds/saved");
                puts("Saved world");
            }
            break;
        case GLFW_KEY_3:
            if (action == GLFW_RELEASE) {
                copyChunk(getChunk(world, (int)(player.position[0] / BLOCK_WIDTH) >> LOG_CHUNK_SIZE,
                                          (int)(player.position[1] / BLOCK_WIDTH) >> LOG_CHUNK_SIZE,
                                          (int)(player.position[2] / BLOCK_WIDTH) >> LOG_CHUNK_SIZE),
                          model1->chunk);
            }
            break;
        case GLFW_KEY_E:
            if (selection.selected_active) {
                Block* selected = selectedBlock(world, &selection);
                currColor = selected->color;
                if (selected->data) {
                    model1 = selected->data;
                }
                updateColorRect();
            }
            break;
        case GLFW_KEY_GRAVE_ACCENT:
            if (action == GLFW_PRESS) {
                if (wireframe)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                else
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                wireframe = !wireframe;
            }
            break;
        case GLFW_KEY_EQUAL:
            if (action == GLFW_PRESS) {
                useMeshing = !useMeshing;
                int i;
                renderModel(model1);
                for (i = 0; i < world->num_chunks; i++) {
                    renderChunk(world->chunks[i]);
                }
            }
            break;
        case GLFW_KEY_L:
            if (action == GLFW_PRESS && selection.selected_active) {
                Block* selected = selectedBlock(world, &selection);
                if (selected->logic) {
                    selected->logic->type++;
                    selected->logic->type %= NUM_GATES;
                } else {
                    selected->logic = calloc(1, sizeof(Logic));
                    selected->logic->auto_orient = 1;
                }
                if (selected->logic->auto_orient)
                    autoOrient(selected);
            }
            break;
        case GLFW_KEY_I:
            if (action == GLFW_PRESS && selection.selected_active) {
                Block* selected = selectedBlock(world, &selection);
                if (selected->logic) {
                    selected->logic->roll++;
                    selected->logic->auto_orient = 0;
                }
            }
            break;
        case GLFW_KEY_O:
            if (action == GLFW_PRESS && selection.selected_active) {
                Block* selected = selectedBlock(world, &selection);
                if (selected->logic) {
                    selected->logic->yaw++;
                    selected->logic->auto_orient = 0;
                }
            }
            break;
        case GLFW_KEY_P:
            if (action == GLFW_PRESS && selection.selected_active) {
                Block* selected = selectedBlock(world, &selection);
                if (selected->logic) {
                    selected->logic->pitch++;
                    selected->logic->auto_orient = 0;
                }
            }
            break;
        case GLFW_KEY_G:
            if (action == GLFW_PRESS)
                inertia = !inertia;
            break;
        default:
            break;
    }
}

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    colorx += xoffset * ((0.0025 * ccparams[1]) * ccparams[1]) / ccparams[0];
    colory += yoffset * (0.0025 * ccparams[1]);
    if (colorx < 0) colorx = 0;
    if (colorx > 1) colorx = 1;
    if (colory < 0) colory = 0;
    if (colory > 1) colory = 1;

    currColor = getCoordinateColor(colorx, colory);

    updateColorRect();
    updateColorCrosshair();
}

static void initInputs(GLFWwindow* window) {
    control = 1;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(window, &mousex, &mousey);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, windowResizeFunc);
    glfwSetScrollCallback(window, scrollCallback);
}

static void readInputs(GLFWwindow* window) {
    if (control) {

        // mouse buttons

        int newMouseButtons[2] = {
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) ^ GLFW_RELEASE,
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) ^ GLFW_RELEASE
        };

        if (newMouseButtons[0] && !mouseButtons[0])
            if (selection.selected_active) {
                setBlock(getChunk(world, selection.selected_chunk_x, selection.selected_chunk_y, selection.selected_chunk_z),
                         selection.selected_block_x, selection.selected_block_y, selection.selected_block_z, (Block){0, {.all=0}, NULL, NULL});
            }

        if (newMouseButtons[1] && !mouseButtons[1])
            if (selection.previous_active) {
                if (placeModel) {
                    Block block;
                    insertModel(model1, &block);
                    setBlock(getChunk(world, selection.previous_chunk_x, selection.previous_chunk_y, selection.previous_chunk_z),
                             selection.previous_block_x, selection.previous_block_y, selection.previous_block_z, block);
                } else
                    setBlock(getChunk(world, selection.previous_chunk_x, selection.previous_chunk_y, selection.previous_chunk_z),
                             selection.previous_block_x, selection.previous_block_y, selection.previous_block_z, (Block){1, currColor, NULL, NULL});
            }

        // mouse position

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        mouseButtons[0] = newMouseButtons[0];
        mouseButtons[1] = newMouseButtons[1];

        player.horizontalAngle += MOUSE_SPEED * deltaTime * (mousex - xpos);
        player.verticalAngle += MOUSE_SPEED * deltaTime * (mousey - ypos);

        if (player.verticalAngle > PI/2)
            player.verticalAngle = PI/2;
        if (player.verticalAngle < -PI/2)
            player.verticalAngle = -PI/2;

        mousex = xpos;
        mousey = ypos;

        player.direction[0] = cos(player.horizontalAngle + PI/2) * cos(player.verticalAngle);
        player.direction[1] = sin(player.verticalAngle);
        player.direction[2] = sin(player.horizontalAngle + PI/2) * cos(player.verticalAngle);

        movePlayer(&player, world,
                   glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS,
                   glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS,
                   glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS,
                   glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS,
                   glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS,
                   glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS,
                   inertia);

        identity_m4(viewMatrix);
        translate_m4(viewMatrix, VALUES(-player.position));
        rotate_Y_m4(viewMatrix, player.horizontalAngle);
        rotate_X_m4(viewMatrix, player.verticalAngle);
    } else {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            control = 1;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &mousex, &mousey);
        }
    }
}

static void initMeshes() {
    makeBox(playerModel, VALUES(player.bounding_box.min), VALUES(player.bounding_box.size), (vec3){0.4, 0.6, 0.9});
    buildBlockFrame(selectedFrame);
    makeCrosshair(crosshair, 2, 20, 2);
    makeColorChooser(colorChooser, ccparams[0], ccparams[1], ccparams[2], ccparams[3]);
    makeCrosshair(colorCrosshair, 1, 8, 1);

    currColor = getCoordinateColor(colorx, colory);

    updateColorRect();
    identity_m4(colorChooser->modelMatrix);
    translate_m4(colorChooser->modelMatrix, - PIXEL_X((ccparams[0] * ccparams[2]) / 2), -1, 0);
    updateColorCrosshair();
}

static void updateColorCrosshair() {
    identity_m4(colorCrosshair->modelMatrix);
    translate_m4(colorCrosshair->modelMatrix,
                 - (0.5f - colorx) * PIXEL_X(ccparams[0] * ccparams[2]),
                 -1.0f + (1.0f - colory) * PIXEL_Y(ccparams[1] * ccparams[2]) + PIXEL_Y(ccparams[3]), 0);
}

static void updateColorRect() {
    rect(colorRect, PIXEL_X(-(ccparams[0]*ccparams[2])/2 - ccparams[3]), -1,
                    PIXEL_X( (ccparams[0]*ccparams[2])/2 + ccparams[3]), PIXEL_Y(ccparams[1]*ccparams[2]+2*ccparams[3])-1,
                    0, (vec3){(float)currColor.r/255, (float)currColor.g/255, (float)currColor.b/255});
}

static void useProgram(ProgramType prog) {
    currProgram = prog;

    switch(prog) {
        case NORMAL_PROGRAM:
            glUseProgram(normalProgram);
            break;
        // case SHADOW_PROGRAM:
        //     glUseProgram(shadowProgram);
        //     break;
        case PLAIN_PROGRAM:
            glUseProgram(plainProgram);
            break;
        case TEXTURE_PROGRAM:
            glUseProgram(textureProgram);
            break;
        case SKYBOX_PROGRAM:
            glUseProgram(skyboxProgram);
            break;
        default:
            break;
    }
}

static void sendProjectionMatrix(mat4 data) {
    switch (currProgram) {
        case NORMAL_PROGRAM:
            glUniformMatrix4fv(normalProjectionUniformID, 1, GL_TRUE, data);
            break;
        // case SHADOW_PROGRAM:
        //     glUniformMatrix4fv(shadowProjectionUniformID, 1, GL_TRUE, data);
        //     break;
        case PLAIN_PROGRAM:
            glUniformMatrix4fv(plainProjectionUniformID, 1, GL_TRUE, data);
            break;
        case TEXTURE_PROGRAM:
            glUniformMatrix4fv(textureProjectionUniformID, 1, GL_TRUE, data);
            break;
        case SKYBOX_PROGRAM:
            glUniformMatrix4fv(skyboxProjectionUniformID, 1, GL_TRUE, data);
            break;
        default:
            break;
    }
}

static void sendViewMatrix(mat4 data) {
    switch (currProgram) {
        case NORMAL_PROGRAM:
            glUniformMatrix4fv(normalViewUniformID, 1, GL_TRUE, data);
            break;
        // case SHADOW_PROGRAM:
        //     glUniformMatrix4fv(shadowViewUniformID, 1, GL_TRUE, data);
        //     break;
        case PLAIN_PROGRAM:
            glUniformMatrix4fv(plainViewUniformID, 1, GL_TRUE, data);
            break;
        case TEXTURE_PROGRAM:
            glUniformMatrix4fv(textureViewUniformID, 1, GL_TRUE, data);
            break;
        case SKYBOX_PROGRAM:
            glUniformMatrix4fv(skyboxViewUniformID, 1, GL_TRUE, data);
            break;
        default:
            break;
    }
}

static void sendModelMatrix(mat4 data) {
    switch (currProgram) {
        case NORMAL_PROGRAM:
            glUniformMatrix4fv(normalModelUniformID, 1, GL_TRUE, data);
            break;
        // case SHADOW_PROGRAM:
        //     glUniformMatrix4fv(shadowModelUniformID, 1, GL_TRUE, data);
        //     break;
        case PLAIN_PROGRAM:
            glUniformMatrix4fv(plainModelUniformID, 1, GL_TRUE, data);
            break;
        case TEXTURE_PROGRAM:
            glUniformMatrix4fv(textureModelUniformID, 1, GL_TRUE, data);
            break;
        case SKYBOX_PROGRAM:
            glUniformMatrix4fv(skyboxModelUniformID, 1, GL_TRUE, data);
            break;
        default:
            break;
    }
}

static void sendUniformData() {
    // GLfloat lightData[8 * MAX_LIGHTS];
    // int i;

    switch (currProgram) {
        case NORMAL_PROGRAM:
            // for (i = 0; i < lightCount; i++)
            //     memcpy(&lightData[i * 8], light[i], 8 * sizeof(GLfloat));
            // glBindBuffer(GL_UNIFORM_BUFFER, normalLightUBO);
            // glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLfloat) * 8 * MAX_LIGHTS, lightData);
            // glBindBuffer(GL_UNIFORM_BUFFER, 0);
            // glUniform1i(normalLightCountUniformID, lightCount);
            break;
        // case SHADOW_PROGRAM:
        //     // glUniform3f(shadowLightSourceUniformID, VALUES(light[0]->position));
        //     // glUniform1f(shadowLightRadiusUniformID, light[0]->radius);
        //     break;
        default:
            break;
    }
}

// static void makeLight(vec3 position, vec3 color, float size, float radius) {
//     if (lightCount < MAX_LIGHTS)
//         light[lightCount++] = createLight(position, color, size, radius);
// }

void drawMesh(Mesh * mesh) {
    // don't render an empty mesh :p
    if (mesh->size == 0)
        return;

    sendModelMatrix(mesh->modelMatrix);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexvbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->normalvbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->colorvbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    if (mesh->texvbo) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->texvbo);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    if (mesh->buffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffer);
        glDrawElements(mesh->type, mesh->size, GL_UNSIGNED_INT, NULL);
    } else {
        glDrawArrays(mesh->type, 0, mesh->size);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void init(GLFWwindow *window) {
    initInputs(window);

    glfwGetFramebufferSize(window, &frame_buffer_width, &frame_buffer_height);

    glEnableClientState(GL_VERTEX_ARRAY);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* load shaders */

    normalProgram  = loadShaders("shaders/normalShader.vert", "shaders/normalShader.frag");
    // shadowProgram  = loadShaders("shaders/shadowShader.vert", "shaders/shadowShader.frag");
    plainProgram   = loadShaders("shaders/plainShader.vert", "shaders/plainShader.frag");
    textureProgram = loadShaders("shaders/textureShader.vert", "shaders/textureShader.frag");
    skyboxProgram  = loadShaders("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");

    useProgram(NORMAL_PROGRAM);

    /* benchmark */

    #if 0
    useMeshing = 1;
    float before = glfwGetTime();
    int i, j;
    for (i = 0; i < 100; i++) {
        for (j = 0; j < world->num_chunks; j++) {
            renderChunk(world->chunks[j]);
        }
    }
    float delta = glfwGetTime() - before;
    printf("Benchmark took %.2f seconds\n", delta);
    useMeshing = 0;
    #endif

    /* meshes, etc. */

    // world = createWorld(6);
    // fillWorld(world);

    world = readWorld("worlds/saved");

    float world_width = world->size * CHUNK_WIDTH;

    player = (Player){{world_width/2, world_width, world_width/2}, {0, 0, -1}, {0, 0, 0},
                      0.0, 0.0,
                      {{-.4f*BLOCK_WIDTH, -2.5f*BLOCK_WIDTH, -.4f*BLOCK_WIDTH},
                      {.8f*BLOCK_WIDTH, 2.8f*BLOCK_WIDTH, .8f*BLOCK_WIDTH}}};

    // makeLight((vec3){0, 0, 0}, (vec3){1, 1, 1}, BLOCK_WIDTH, CHUNK_WIDTH*2);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, light[0]->shadowMapTex);

    playerModel    = malloc(sizeof(Mesh));
    selectedFrame  = malloc(sizeof(Mesh));
    crosshair      = malloc(sizeof(Mesh));
    colorChooser   = malloc(sizeof(Mesh));
    colorCrosshair = malloc(sizeof(Mesh));
    colorRect      = malloc(sizeof(Mesh));

    initMeshes();

    initLogicModels();

    model1 = createModel();
    readModel(model1, "models/model1");

    /* control / option variables */

    control = 1;
    wireframe = 0;
    inertia = 1;

    selection.selected_active = selection.previous_active = 0;

    /* get shader uniforms */

    // glGenBuffers(1, &normalLightUBO);
    glGenBuffers(1, &normalMaterialsUBO);

    // glBindBuffer(GL_UNIFORM_BUFFER, normalLightUBO);
    // glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat) * 8 * MAX_LIGHTS, NULL, GL_STREAM_DRAW);
    // glUniformBlockBinding(normalProgram, glGetUniformBlockIndex(normalProgram, "Light"), 0);
    // glBindBufferBase(GL_UNIFORM_BUFFER, 0, normalLightUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, normalMaterialsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat) * 12, NULL, GL_STATIC_DRAW);
    glUniformBlockBinding(normalProgram, glGetUniformBlockIndex(normalProgram, "Materials"), 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, normalMaterialsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLfloat) * 12, (GLfloat[]) {
        0.3, 0.3, 0.3, 0, // diffuse
        0.7, 0.7, 0.7, 0, // ambient
        0.1, 0.1, 0.1, 0, // specular
    });

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    normalModelUniformID      = glGetUniformLocation(normalProgram, "modelMatrix");
    normalViewUniformID       = glGetUniformLocation(normalProgram, "viewMatrix");
    normalProjectionUniformID = glGetUniformLocation(normalProgram, "projectionMatrix");
    // normalLightCountUniformID = glGetUniformLocation(normalProgram, "lightCount");
    // normalShadowMapUniformID  = glGetUniformLocation(normalProgram, "shadowMap");

    // shadowLightSourceUniformID = glGetUniformLocation(shadowProgram, "lightSource");
    // shadowModelUniformID       = glGetUniformLocation(shadowProgram, "modelMatrix");
    // shadowViewUniformID        = glGetUniformLocation(shadowProgram, "viewMatrix");
    // shadowProjectionUniformID  = glGetUniformLocation(shadowProgram, "projectionMatrix");
    // shadowLightRadiusUniformID = glGetUniformLocation(shadowProgram, "lightRadius");

    plainModelUniformID      = glGetUniformLocation(plainProgram, "modelMatrix");
    plainViewUniformID       = glGetUniformLocation(plainProgram, "viewMatrix");
    plainProjectionUniformID = glGetUniformLocation(plainProgram, "projectionMatrix");

    textureModelUniformID      = glGetUniformLocation(textureProgram, "modelMatrix");
    textureViewUniformID       = glGetUniformLocation(textureProgram, "viewMatrix");
    textureProjectionUniformID = glGetUniformLocation(textureProgram, "projectionMatrix");
    textureTextureUniformID    = glGetUniformLocation(textureProgram, "currTexture");

    skyboxModelUniformID      = glGetUniformLocation(skyboxProgram, "modelMatrix");
    skyboxViewUniformID       = glGetUniformLocation(skyboxProgram, "viewMatrix");
    skyboxProjectionUniformID = glGetUniformLocation(skyboxProgram, "projectionMatrix");
    skyboxSkyboxUniformID     = glGetUniformLocation(skyboxProgram, "skybox");

    /* view options */

    perspective(projectionMatrix, (float)frame_buffer_width/frame_buffer_height, 60, 0.01, 100);

    glEnable(GL_TEXTURE_CUBE_MAP);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // sky blue
    glClearColor(0.6, 0.9, 1.0, 1.0);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, frame_buffer_width, frame_buffer_height);

    runLogicThread(world);

    // currColor.all=0xFF09FF42;
}

void tick(GLFWwindow *window) {
    double newTime = glfwGetTime();

    deltaTime = newTime - currTime;
    currTime = newTime;

    // moveLight(light[0], (vec3) {
    //     player.position[0] + (cos(currTime)*2)*BLOCK_WIDTH*5,
    //     player.position[1],// + BLOCK_WIDTH*2,
    //     player.position[2] + (sin(currTime)*2)*BLOCK_WIDTH*5
    // });

    /*moveLight(light, (vec3) {
        BLOCK_WIDTH*20,
        BLOCK_WIDTH*10,
        BLOCK_WIDTH*20
    });*/

    readInputs(window);

    vec3 pos;
    copy_v3(pos, player.position);
    scale_v3(pos, 1.0f/BLOCK_WIDTH);

    selection = selectBlock(world, pos, player.direction, CHUNK_SIZE);

    identity_m4(playerModel->modelMatrix);
    rotate_Y_m4(playerModel->modelMatrix, -player.horizontalAngle);
    translate_m4(playerModel->modelMatrix, VALUES(player.position));

    for (int i = 0; i < world->num_chunks; i++) {
        if (world->chunks[i]->needsUpdate) {
            world->chunks[i]->needsUpdate = 0;
            renderChunk(world->chunks[i]);
        }
    }

    render();
}

// static void renderShadowMap() {
//     mat4 projection;
//
//     // 90 degree FOV for cube faces
//     perspective(projection, 1, 90, 0.01, 100);
//
//     glBindFramebuffer(GL_FRAMEBUFFER, light[0]->shadowMapFBO);
//     glViewport(0, 0, SHADOW_RES, SHADOW_RES);
//
//     // HOW THE HELL IS A CUBEMAP ORIENTED >_>
//     // DONOTTOUCH. DON'T EVENT THINK ABOUT IT.
//     // but I guess you can note that the translation is accounted for to cut down
//     // on matrix calculations. It's as if we applied the transformation
//     // -light->position and then applied the transformations mentioned.
//     // STILL DON'T TOUCH. LIKE, AT ALL. EVER.
//     mat4 faces[6] = {
//         { 0,  0, -1,  light[0]->position[2],
//           0, -1,  0,  light[0]->position[1],
//           1,  0,  0, -light[0]->position[0],
//           0,  0,  0,  1}, //rotation_Y(-PI/2), flip_y
//
//         { 0,  0,  1, -light[0]->position[2],
//           0, -1,  0,  light[0]->position[1],
//          -1,  0,  0,  light[0]->position[0],
//           0,  0,  0,  1}, //rotation_Y( PI/2), flip_y
//
//         { 1,  0,  0, -light[0]->position[0],
//           0,  0,  1, -light[0]->position[2],
//           0,  1,  0, -light[0]->position[1],
//           0,  0,  0,  1}, //rotation_X(-PI/2), flip_z
//
//         { 1,  0,  0, -light[0]->position[0],
//           0,  0, -1,  light[0]->position[2],
//           0, -1,  0,  light[0]->position[1],
//           0,  0,  0,  1}, //rotation_X( PI/2), flip_z
//
//         { 1,  0,  0, -light[0]->position[0],
//           0, -1,  0,  light[0]->position[1],
//           0,  0,  1, -light[0]->position[2],
//           0,  0,  0,  1}, //rotation_Y(    0), flip_y
//
//         {-1,  0,  0,  light[0]->position[0],
//           0, -1,  0,  light[0]->position[1],
//           0,  0, -1,  light[0]->position[2],
//           0,  0,  0,  1}, //rotation_Y(   PI), flip_y
//     };
//
//     // to avoid shadow acne
//     glCullFace(GL_FRONT);
//
//     int i;
//     for (i = 0; i < 6; i++) {
//         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, light[0]->shadowMapTex, 0);
//         glClear(GL_DEPTH_BUFFER_BIT);
//
//         renderWorld(faces[i], projection);
//     }
//
//     glCullFace(GL_BACK);
//
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
//     glViewport(0, 0, frame_buffer_width, frame_buffer_height);
// }

static void renderWorld(mat4 view, mat4 projection) {
    sendViewMatrix(view);
    sendProjectionMatrix(projection);

    // draw the light source
    // int i;
    // for (i = 0; i < lightCount; i++)
    //     drawMesh(light[i]->mesh);

    // draw the player
    drawMesh(playerModel);

    // draw the world chunks
    drawWorld(world, view, projection);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw the shadow map
    // useProgram(SHADOW_PROGRAM);
    //
    //     sendUniformData();
    //
    //     renderShadowMap();

    // draw the world
    useProgram(NORMAL_PROGRAM);

        sendUniformData();
        // glUniform1i(normalShadowMapUniformID, 0);

        renderWorld(viewMatrix, projectionMatrix);

    // draw GUI, etc.
    useProgram(PLAIN_PROGRAM);

        sendViewMatrix(viewMatrix);
        sendProjectionMatrix(projectionMatrix);

        if (selection.selected_active) {
            translation_m4(selectedFrame->modelMatrix,
                           selection.selected_chunk_x * CHUNK_WIDTH + selection.selected_block_x * BLOCK_WIDTH,
                           selection.selected_chunk_y * CHUNK_WIDTH + selection.selected_block_y * BLOCK_WIDTH,
                           selection.selected_chunk_z * CHUNK_WIDTH + selection.selected_block_z * BLOCK_WIDTH);
            drawMesh(selectedFrame);
        }

        sendViewMatrix(identityMatrix);
        sendProjectionMatrix(identityMatrix);

        glDisable(GL_DEPTH_TEST);
        drawMesh(crosshair);
        drawMesh(colorRect);
        drawMesh(colorChooser);
        drawMesh(colorCrosshair);
        glEnable(GL_DEPTH_TEST);

    /*useProgram(TEXTURE_PROGRAM);

        sendViewMatrix(identityMatrix);
        sendProjectionMatrix(identityMatrix);

        glUniform1i(textureTextureUniformID, 0);
        drawMesh(cubeMapLayout);
    */
    /*useProgram(SKYBOX_PROGRAM);

        sendViewMatrix(identityMatrix);
        sendProjectionMatrix(identityMatrix);

        glUniform1i(skyboxSkyboxUniformID, 0);
        drawMesh(skybox);
    */
}

void finish() {
    stopLogicThread();
    freeLogicModels();

    // int i;
    // for (i = 0; i < lightCount; i++)
    //     freeLight(light[i]);
    freeWorld(world);

    // freeMesh(selectedFrame);
    free(selectedFrame);

    freeMesh(crosshair);
    free(crosshair);

    freeMesh(colorChooser);
    free(colorChooser);

    // freeModel(model1);

    glfwTerminate();
}

int main(void) {
    GLFWwindow* window;

    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Test", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return EXIT_FAILURE;
    }

    init(window);

    double lastTime = glfwGetTime();
    int frames = 0;

    while (!glfwWindowShouldClose(window)) {
        tick(window);
        glfwSwapBuffers(window);
        glfwPollEvents();

        frames++;
        // ms/f counter
        if (currTime - lastTime >= 1.0) {
            printf("%f ms / frame\n", 1000.0/(float)frames);
            frames = 0;
            lastTime += 1.0;
        }
    }

    finish();
    return EXIT_SUCCESS;
}
