#ifndef AOE_INDICATOR_H
#define AOE_INDICATOR_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>
#include <vector>

// Shader Code อย่างง่ายสำหรับ Indicator (ฝังในโค้ดเลยเพื่อความสะดวก)
const char *indicatorVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char *indicatorFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color; // รับสี (r, g, b, a)
void main() {
    FragColor = color;
}
)";

class AoEIndicator
{
private:
    unsigned int quadVAO, quadVBO;
    unsigned int coneVAO, coneVBO;
    int coneVertexCount;
    Shader shader;

    void initQuad()
    {
        // สี่เหลี่ยมแบนราบ (XZ plane) ขนาด 1x1 จุดศูนย์กลางอยู่ที่ (0,0,0) หรือปรับตามต้องการ
        // เพื่อให้ง่าย เราจะให้จุด (0,0) อยู่ที่กึ่งกลางด้านล่าง หรือกึ่งกลางเลย แล้วแต่การใช้ transform
        // แบบนี้: สี่เหลี่ยมขนาด 1x1 เริ่มจาก -0.5 ถึง 0.5
        float vertices[] = {
            0.5f, 0.01f, 0.5f,   // top right
            0.5f, 0.01f, -0.5f,  // bottom right
            -0.5f, 0.01f, -0.5f, // bottom left
            -0.5f, 0.01f, 0.5f   // top left
        };
        unsigned int indices[] = {0, 1, 3, 1, 2, 3};

        unsigned int EBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void initCone(int segments = 30)
    {
        std::vector<float> vertices;
        // จุดศูนย์กลาง (Center) ที่ตัวละครยืน
        vertices.push_back(0.0f);
        vertices.push_back(0.01f);
        vertices.push_back(0.0f);

        float angleStep = glm::radians(90.0f) / (segments - 1); // มุมกว้าง 90 องศา
        float startAngle = glm::radians(-45.0f);                // เริ่มที่ -45 องศา (เพื่อให้ 0 คือตรงกลาง)

        // สร้างส่วนโค้ง
        for (int i = 0; i < segments; ++i)
        {
            float angle = startAngle + i * angleStep;
            // Z คือทิศหน้า (ใน OpenGL ปกติ -Z คือหน้า แต่เราหมุนโมเดลตามใจชอบได้)
            // สมมติรัศมี 1.0
            float x = sin(angle);
            float z = cos(angle); // ให้ Z เป็นแกนหลักในการพุ่งไปข้างหน้า (หรือใช้ -cos ถ้าตาม OpenGL)
            // แต่เพื่อให้ง่ายกับการ Transform เราจะสร้างให้ชี้ไปทาง +Z หรือ -Z แล้วหมุนเอา
            // ลองสร้างชี้ไปทาง +Z
            vertices.push_back(x);
            vertices.push_back(0.01f);
            vertices.push_back(z);
        }
        coneVertexCount = vertices.size() / 3;

        glGenVertexArrays(1, &coneVAO);
        glGenBuffers(1, &coneVBO);
        glBindVertexArray(coneVAO);
        glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

public:
    AoEIndicator(const char *vsPath, const char *fsPath)
        : shader(vsPath, fsPath) // 🔴 เรียก Constructor ของ Shader
    {
        initQuad();
        initCone();
    }

    void DrawBoxAtCenter(const glm::mat4 &view, const glm::mat4 &proj,
                         glm::vec3 centerPos, glm::vec3 forwardDir,
                         float width, float length)
    {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);
        shader.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 0.6f)); // สีแดง 60%

        glm::mat4 model = glm::mat4(1.0f);

        // 1. ตำแหน่ง: ใช้จุดกึ่งกลางที่ส่งมาเลย
        // ยกขึ้นนิดหน่อย (0.02f) เพื่อไม่ให้ซ้อนกับพื้นหรือ AoE อื่น
        centerPos.y = 0.02f;
        model = glm::translate(model, centerPos);

        // 2. หมุน: หันตามทิศทางที่ส่งมา
        float angle = atan2(forwardDir.x, forwardDir.z);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        // 3. ขนาด:
        // width = ด้านกว้าง (ซ้าย-ขวา)
        // length = ด้านยาว (หน้า-หลัง)
        model = glm::scale(model, glm::vec3(width, 1.0f, length));

        shader.setMat4("model", model);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);
    }

    void DrawRectangle(const glm::mat4 &view, const glm::mat4 &proj, glm::vec3 pos, glm::vec3 forward, float width, float length)
    {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);
        // สีแดง Opacity 60%
        shader.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 0.6f));

        glm::mat4 model = glm::mat4(1.0f);

        // 1. ตำแหน่ง: วางไว้ข้างหน้าตัวละครครึ่งหนึ่งของความยาว (เพื่อให้ตัวละครอยู่ที่ขอบ)
        // หรือถ้าโมเดล Quad เราสร้างไว้ Center (0,0) เราต้องเลื่อนมันไปข้างหน้า
        // แต่ถ้าเราใช้ตำแหน่ง "เป้าหมาย" ที่คำนวณมาแล้ว (center ของสกิล) ก็ใช้ pos ได้เลย

        // กรณี Fireball/Wall: pos คือตำแหน่ง Demon
        // เราต้องการสี่เหลี่ยมยื่นไปข้างหน้า
        glm::vec3 rectCenter = pos + forward * (length / 2.0f);
        model = glm::translate(model, rectCenter);

        // 2. หมุน: ให้หันตาม Demon
        // หาหมุม Yaw จาก forward vector
        float angle = atan2(forward.x, forward.z);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        // 3. ขนาด
        model = glm::scale(model, glm::vec3(width, 1.0f, length));

        shader.setMat4("model", model);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);
    }

    void DrawCone(const glm::mat4 &view, const glm::mat4 &proj, glm::vec3 pos, glm::vec3 forward, float length, float angleSpread)
    {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);
        shader.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 0.6f)); // สีแดงโปร่งใส

        glm::mat4 model = glm::mat4(1.0f);

        // 1. ตำแหน่ง: จุดยอดกรวย
        model = glm::translate(model, pos);

        // 2. หมุน: หันหน้าไปตาม forward vector
        float angle = atan2(forward.x, forward.z);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        // 3. 🌟 คำนวณ Scale ตาม angleSpread
        // แปลงมุมเป็นเรเดียนและหาร 2
        float halfAngleRad = glm::radians(angleSpread / 2.0f);

        // คำนวณรัศมีของฐานกรวยที่ระยะ length
        float coneRadius = glm::tan(halfAngleRad) * length;

        // Scale:
        // - แกน X (กว้าง): coneRadius
        // - แกน Y (สูง/หนา): 1.0f (หรือปรับตามต้องการเพื่อให้แบนราบกับพื้น)
        // - แกน Z (ยาว): length (ทิศทางไปข้างหน้า)

        // หมายเหตุ: หากโมเดล Cone ของคุณถูกสร้างให้ชี้ไปทาง Z, การ Scale แกน Z จะเพิ่มความยาว
        // และการ Scale แกน X จะเพิ่มความกว้างของมุม

        // สมมติว่า Cone Model แบนราบ (Y=0) และชี้ไปทาง Z
        model = glm::scale(model, glm::vec3(coneRadius, 1.0f, length));

        shader.setMat4("model", model);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(coneVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, coneVertexCount);

        glDisable(GL_BLEND);
    }
};

#endif