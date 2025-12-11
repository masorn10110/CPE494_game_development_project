#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>

class UIManager {
public:
    unsigned int VAO, VBO;
    Shader* uiShader;
    glm::mat4 projection;

    UIManager(unsigned int screenWidth, unsigned int screenHeight) {
        // Orthographic projection for 2D UI
        projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
        
        uiShader = new Shader("ui.vs", "ui.fs");
        
        // Setup quad for rendering textures
        float vertices[] = {
            // pos      // tex
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        };
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    void DrawTexture(unsigned int texture, glm::vec2 position, glm::vec2 size, 
                     glm::vec3 color = glm::vec3(1.0f), float alpha = 1.0f) {
        uiShader->use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));
        
        uiShader->setMat4("projection", projection);
        uiShader->setVec3("color", color);
        uiShader->setFloat("alpha", alpha);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    
    // Simple rect for buttons without texture
    void DrawRect(glm::vec2 position, glm::vec2 size, glm::vec3 color, float alpha = 1.0f) {
        // สร้าง white texture ถ้ายังไม่มี
        static unsigned int whiteTexture = 0;
        if (whiteTexture == 0) {
            unsigned char white[] = {255, 255, 255, 255};
            glGenTextures(1, &whiteTexture);
            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        }
        DrawTexture(whiteTexture, position, size, color, alpha);
    }
    
    ~UIManager() {
        delete uiShader;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

#endif