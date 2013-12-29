////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Single texture shader with no lighting

namespace octet {
  class cubemap_reflection_shader : public shader {

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;
    GLuint modelToWorldIndex_;

    // index for texture sampler
    GLuint samplerIndex_;
  public:
    void init() {
      // this is the vertex shader.
      // it is called for each corner of each triangle
      // it inputs pos and uv from each corner
      // it outputs gl_Position and uv_ to the rasterizer
      const char vertex_shader[] = SHADER_STR(
        varying vec3 normal_;

        attribute vec4 pos;
        attribute vec3 normal;
       
        uniform vec3 cameraPosition;
        uniform mat4 modelToProjection;
        uniform mat4 modelToWorld;

        void main() {
          normal_ = (modelToWorld * vec4(normal, 1)).xyz;
          gl_Position = modelToProjection * pos; 
        }
      );

      // this is the fragment shader
      // after the rasterizer breaks the triangle into fragments
      // this is called for every fragment
      // it outputs gl_FragColor, the color of the pixel and inputs uv_
      const char fragment_shader[] = SHADER_STR(
        varying vec3 normal_;

        uniform samplerCube sampler;

        void main() {
          gl_FragColor = textureCube(sampler, normal_);
        }
      );
    
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      modelToWorldIndex_ = glGetUniformLocation(program(), "modelToWorld");
      samplerIndex_ = glGetUniformLocation(program(), "sampler");
    }

    void render(const mat4t *modelToProjection, const mat4t *modelToWorld, int sampler) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniform1i(samplerIndex_, sampler);
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection->get());
      glUniformMatrix4fv(modelToWorldIndex_, 1, GL_FALSE, modelToWorld->get());
    }
  };

  class cubemap_sky_shader : public shader {
    GLuint modelToProjectionIndex_;
    GLuint cameraPositionIndex_;
    GLuint samplerIndex_;

  public:
    void init() {
      const char vertex_shader[] = SHADER_STR(
        attribute vec4 pos;

        uniform vec3 cameraPosition;
        uniform mat4 modelToProjection;

        varying vec3 uv_;

        void main() {
          uv_ = vec3(pos.x, -pos.yz);
          gl_Position = modelToProjection * vec4(pos.xyz + cameraPosition, 1.0); 
        }
      );

      const char fragment_shader[] = SHADER_STR(
        uniform samplerCube sampler;

        varying vec3 uv_;

        void main() {
          gl_FragColor = textureCube(sampler, uv_);
        }
      );
    
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      cameraPositionIndex_ = glGetUniformLocation(program(), "cameraPosition");
      samplerIndex_ = glGetUniformLocation(program(), "sampler");
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
    }

    void render(const mat4t *modelToProjection, const vec3 *cameraPosition, int sampler) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniform1i(samplerIndex_, sampler);
      glUniform3fv(cameraPositionIndex_, 3, cameraPosition->get());
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection->get());
    }
  };
}
