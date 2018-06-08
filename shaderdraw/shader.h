#ifndef SHADER_H
#define SHADER_H

#include "GL/glew.h"
#include <vector>
#include <string>
#include <iterator>// std::istreambuf_iterator
#include <fstream>
#include <iostream>

struct ShaderFile
{
    GLenum shaderType;
    const char *filePath;

    ShaderFile(GLenum type, const char *path)
        : shaderType(type), filePath(path)
    {}
};

class Shader
{
public:
    Shader(const char *vertexPath, const char *fragPath)
        : programId(0)
    {
        std::vector<ShaderFile> fileVec;
        fileVec.push_back(ShaderFile(GL_VERTEX_SHADER, vertexPath));
        fileVec.push_back(ShaderFile(GL_FRAGMENT_SHADER, fragPath));
        loadFromFile(fileVec);
    }

    Shader(const char *vertexPath, const char *fragPath, const char *geometryPath)
        : programId(0)
    {
        std::vector<ShaderFile> fileVec;
        fileVec.push_back(ShaderFile(GL_VERTEX_SHADER, vertexPath));
        fileVec.push_back(ShaderFile(GL_FRAGMENT_SHADER, fragPath));
        fileVec.push_back(ShaderFile(GL_GEOMETRY_SHADER, geometryPath));
        loadFromFile(fileVec);
    }

    void use()
    {
        glUseProgram(programId);
    }

    ~Shader()
    {
        if (programId) {
            glDeleteProgram(programId);
        }
    }

    GLuint programId;

private:
    /*
    * 从文件加载顶点和片元着色器
    * 传递参数为 [(着色器文件类型，着色器文件路径)+]
    */
    void loadFromFile(std::vector<ShaderFile> &shaderFileVec)
    {
        std::vector<GLuint> shaderObjectIdVec;
        std::string vertexSource, fragSource;
        std::vector<std::string> sourceVec;
        size_t shaderCount = shaderFileVec.size();
        // 读取文件源代码
        for (size_t i = 0; i < shaderCount; ++i) {
            std::string shaderSource;
            if (!loadShaderSource(shaderFileVec[i].filePath, shaderSource)) {
                std::cout << "Error::Shader could not load file:" << shaderFileVec[i].filePath << std::endl;
                return;
            }
            sourceVec.push_back(shaderSource);
        }
        bool bSuccess = true;
        // 编译shader object
        for (size_t i = 0; i < shaderCount; ++i) {
            GLuint shaderId = glCreateShader(shaderFileVec[i].shaderType);
            const char *c_str = sourceVec[i].c_str();
            glShaderSource(shaderId, 1, &c_str, NULL);
            glCompileShader(shaderId);
            GLint compileStatus = 0;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus); // 检查编译状态
            if (compileStatus == GL_FALSE) { // 获取错误报告
                GLint maxLength = 0;
                glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
                std::vector<GLchar> errLog(maxLength);
                glGetShaderInfoLog(shaderId, maxLength, &maxLength, &errLog[0]);
                std::cout << "Error::Shader file [" << shaderFileVec[i].filePath << " ] compiled failed,"
                          << &errLog[0] << std::endl;
                bSuccess = false;
            }
            shaderObjectIdVec.push_back(shaderId);
        }
        // 链接shader program
        if (bSuccess) {
            programId = glCreateProgram();
            for (size_t i = 0; i < shaderCount; ++i) {
                glAttachShader(programId, shaderObjectIdVec[i]);
            }
            glLinkProgram(programId);
            GLint linkStatus;
            glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
            if (linkStatus == GL_FALSE) {
                GLint maxLength = 0;
                glGetProgramiv(this->programId, GL_INFO_LOG_LENGTH, &maxLength);
                std::vector<GLchar> errLog(maxLength);
                glGetProgramInfoLog(this->programId, maxLength, &maxLength, &errLog[0]);
                std::cout << "Error::shader link failed," << &errLog[0] << std::endl;
            }
        }
        // 链接完成后detach 并释放shader object
        for (size_t i = 0; i < shaderCount; ++i) {
            if (programId != 0) {
                glDetachShader(programId, shaderObjectIdVec[i]);
            }
            glDeleteShader(shaderObjectIdVec[i]);
        }
    }

    /*
    * 读取着色器程序源码
    */
    bool loadShaderSource(const char *filePath, std::string &source)
    {
        source.clear();
        std::ifstream in(filePath);
        if (!in) {
            return false;
        }
        source.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()); // 文件流迭代器构造字符串
		return true;
    }
};

#endif // SHADER_H