////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Single texture shader with no lighting

namespace octet {

  /* cubemap_fragdiffraction_shader - This is a variation of cubemap_diffraction_shader
   * that does the calculations in the fragment shader, so it does not depend of the
   * level of detail of the mesh.
   */
  class cubemap_fragdiffraction_shader : public shader {

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;
    GLuint modelToWorldIndex_;
    GLuint modelToWorldITIndex_;
    GLuint roughIndex_;
    GLuint spacingIndex_;
    GLuint hiliteColorIndex_;
    GLuint lightPositionIndex_;
    GLuint cameraPositionIndex_;

    // index for texture sampler
    GLuint samplerIndex_;

  public:
    void init() {
      // this is the vertex shader.
      // it is called for each corner of each triangle
      // it inputs pos and uv from each corner
      // it outputs gl_Position and uv_ to the rasterizer
      const char vertex_shader[] = SHADER_STR(
        varying vec3 position_;
        varying vec3 normal_;
        varying vec3 tangent_;

        attribute vec4 pos;
        attribute vec3 normal;
        attribute vec3 tangent;

        uniform mat4 modelToProjection;
        uniform mat4 modelToWorld;
        uniform mat3 modelToWorldIT;

        void main() {
          gl_Position = modelToProjection * pos; 
          position_ = gl_Position.xyz;
          normal_ = (modelToWorldIT * normal);
          tangent_ = (modelToWorldIT * tangent);
        }
      );

      // this is the fragment shader
      // after the rasterizer breaks the triangle into fragments
      // this is called for every fragment
      // it outputs gl_FragColor, the color of the pixel and inputs uv_
      const char fragment_shader[] = SHADER_STR(
        varying vec3 position_;
        varying vec3 normal_;
        varying vec3 tangent_;

        uniform samplerCube sampler;
        
        uniform mat4 modelToProjection;
        uniform mat4 modelToWorld;
        uniform mat3 modelToWorldIT;

        uniform float r;
        uniform float d;
        uniform vec4 hiliteColor;
        uniform vec3 lightPosition;
        uniform vec3 cameraPosition;
        
        vec3 blend3(vec3 x){
          vec3 y = 1.0 - x*x;
          y = max(y, vec3(0, 0, 0));
          return (y);
        }

        void main() {
          vec4 leColor = vec4(0.0, 0.0, 0.0, 1.0);

          vec3 P = position_;
          vec3 L = normalize(lightPosition - P);
          vec3 V = normalize(cameraPosition - P);
          vec3 H = L + V;
          vec3 N = normal_;
          vec3 T = tangent_;
          float u = dot(T, H) * d;
          float w = dot(N, H);
          float e = r * u / w;
          float c = exp(-e * e);
          vec4 anis = hiliteColor * vec4(c, c, c, 1.0);
          
          if (u < 0.0) u = -u;

          vec4 cdiff = vec4(0.0, 0.0, 0.0, 1.0);

          for (int n = 1; n < 8; n++) {
            float y = 2.0 * u / float(n) - 1.0;
            cdiff.xyz += blend3(vec3(4.0 * (y - 0.75), 4.0 * (y - 0.5), 4.0 * (y - 0.25)));
          }

          gl_FragColor = cdiff + anis;
          //textureCube(sampler, normal_);
        }
      );
    
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      modelToWorldIndex_ = glGetUniformLocation(program(), "modelToWorld");
      modelToWorldITIndex_ = glGetUniformLocation(program(), "modelToWorldIT");
      roughIndex_ = glGetUniformLocation(program(), "r");
      spacingIndex_ = glGetUniformLocation(program(), "d");
      hiliteColorIndex_ = glGetUniformLocation(program(), "hiliteColor");
      lightPositionIndex_ = glGetUniformLocation(program(), "lightPosition");
      cameraPositionIndex_ = glGetUniformLocation(program(), "cameraPosition");

      samplerIndex_ = glGetUniformLocation(program(), "sampler");
    }

    void render(const mat4t &modelToProjection, const mat4t &modelToWorld, mat4t &modelToWorldIT, 
      float rough, float spacing, vec4 &hiliteColor, vec3 &lightPosition, vec3 &cameraPosition, int sampler) {
      // tell openGL to use the program
      shader::render();

      dynarray<float> modelToWorldIT3x3;
      modelToWorldIT3x3.push_back(modelToWorldIT[0][0]);
      modelToWorldIT3x3.push_back(modelToWorldIT[1][0]);
      modelToWorldIT3x3.push_back(modelToWorldIT[2][0]);
      modelToWorldIT3x3.push_back(modelToWorldIT[0][1]);
      modelToWorldIT3x3.push_back(modelToWorldIT[1][1]);
      modelToWorldIT3x3.push_back(modelToWorldIT[2][1]);
      modelToWorldIT3x3.push_back(modelToWorldIT[0][2]);
      modelToWorldIT3x3.push_back(modelToWorldIT[1][2]);
      modelToWorldIT3x3.push_back(modelToWorldIT[2][2]);

      // customize the program with uniforms
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
      glUniformMatrix4fv(modelToWorldIndex_, 1, GL_FALSE, modelToWorld.get());
      glUniformMatrix3fv(modelToWorldITIndex_, 1, GL_FALSE, modelToWorldIT3x3.data());
      glUniform1f(roughIndex_, rough);
      glUniform1f(spacingIndex_, spacing);
      glUniform4fv(hiliteColorIndex_, 1, hiliteColor.get());
      glUniform3fv(lightPositionIndex_, 1, lightPosition.get());
      glUniform3fv(cameraPositionIndex_, 1, cameraPosition.get());
      glUniform1i(samplerIndex_, sampler);
    }
  };

  /*
   * cubemap_diffraction_shader - This is a port to GLSL for the diffraction shader
   * written in chapter 8 from GPU Gems. The calculations are done primarily in the
   * vertex shader, so it depends on the level of detail of the mesh rendered.
   */
  class cubemap_diffraction_shader : public shader {

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;
    GLuint modelToWorldIndex_;
    GLuint modelToWorldITIndex_;
    GLuint roughIndex_;
    GLuint spacingIndex_;
    GLuint hiliteColorIndex_;
    GLuint lightPositionIndex_;
    GLuint cameraPositionIndex_;

    // index for texture sampler
    GLuint samplerIndex_;

  public:
    void init() {
      // this is the vertex shader.
      // it is called for each corner of each triangle
      // it inputs pos and uv from each corner
      // it outputs gl_Position and uv_ to the rasterizer
      const char vertex_shader[] = SHADER_STR(
        varying vec4 color_;
        varying vec3 normal_;

        attribute vec4 pos;
        attribute vec3 normal;
        attribute vec3 tangent;

        uniform mat4 modelToProjection;
        uniform mat4 modelToWorld;
        uniform mat3 modelToWorldIT;

        uniform float r;
        uniform float d;
        uniform vec4 hiliteColor;
        uniform vec3 lightPosition;
        uniform vec3 cameraPosition;

        vec3 blend3(vec3 x){
          vec3 y = 1.0 - x*x;
          y = max(y, vec3(0, 0, 0));
          return (y);
        }

        void main() {
          vec3 P = (modelToWorld * pos).xyz;
          vec3 L = normalize(lightPosition - P);
          vec3 V = normalize(cameraPosition - P);
          vec3 H = L + V;
          vec3 N = modelToWorldIT * normal;
          vec3 T = modelToWorldIT * tangent;
          float u = dot(T, H) * d;
          float w = dot(N, H);
          float e = r * u / w;
          float c = exp(-e * e);
          vec4 anis = hiliteColor * vec4(c, c, c, 1.0);

          if (u < 0.0) u = -u;

          vec4 cdiff = vec4(0.0, 0.0, 0.0, 1.0);

          for (int n = 1; n < 8; n++) {
            float y = 2.0 * u / float(n) - 1.0;
            cdiff.xyz += blend3(vec3(4.0 * (y - 0.75), 4.0 * (y - 0.5), 4.0 * (y - 0.25)));
          }
          gl_Position = modelToProjection * pos; 
          color_ = cdiff + anis;
          normal_ = (modelToWorld * vec4(normal, 1)).xyz;
        }
      );

      // this is the fragment shader
      // after the rasterizer breaks the triangle into fragments
      // this is called for every fragment
      // it outputs gl_FragColor, the color of the pixel and inputs uv_
      const char fragment_shader[] = SHADER_STR(
        varying vec4 color_;

        uniform samplerCube sampler;

        void main() {
          gl_FragColor = color_; //textureCube(sampler, normal_);
        }
      );
    
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      modelToWorldIndex_ = glGetUniformLocation(program(), "modelToWorld");
      modelToWorldITIndex_ = glGetUniformLocation(program(), "modelToWorldIT");
      roughIndex_ = glGetUniformLocation(program(), "r");
      spacingIndex_ = glGetUniformLocation(program(), "d");
      hiliteColorIndex_ = glGetUniformLocation(program(), "hiliteColor");
      lightPositionIndex_ = glGetUniformLocation(program(), "lightPosition");
      cameraPositionIndex_ = glGetUniformLocation(program(), "cameraPosition");

      samplerIndex_ = glGetUniformLocation(program(), "sampler");
    }

    void render(const mat4t &modelToProjection, const mat4t &modelToWorld, mat4t &modelToWorldIT, 
      float rough, float spacing, vec4 &hiliteColor, vec3 &lightPosition, vec3 &cameraPosition, int sampler) {
      // tell openGL to use the program
      shader::render();

      dynarray<float> modelToWorldIT3x3;
      modelToWorldIT3x3.push_back(modelToWorldIT[0][0]);
      modelToWorldIT3x3.push_back(modelToWorldIT[1][0]);
      modelToWorldIT3x3.push_back(modelToWorldIT[2][0]);
      modelToWorldIT3x3.push_back(modelToWorldIT[0][1]);
      modelToWorldIT3x3.push_back(modelToWorldIT[1][1]);
      modelToWorldIT3x3.push_back(modelToWorldIT[2][1]);
      modelToWorldIT3x3.push_back(modelToWorldIT[0][2]);
      modelToWorldIT3x3.push_back(modelToWorldIT[1][2]);
      modelToWorldIT3x3.push_back(modelToWorldIT[2][2]);

      // customize the program with uniforms
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
      glUniformMatrix4fv(modelToWorldIndex_, 1, GL_FALSE, modelToWorld.get());
      glUniformMatrix3fv(modelToWorldITIndex_, 1, GL_FALSE, modelToWorldIT3x3.data());
      glUniform1f(roughIndex_, rough);
      glUniform1f(spacingIndex_, spacing);
      glUniform4fv(hiliteColorIndex_, 1, hiliteColor.get());
      glUniform3fv(lightPositionIndex_, 1, lightPosition.get());
      glUniform3fv(cameraPositionIndex_, 1, cameraPosition.get());
      glUniform1i(samplerIndex_, sampler);
    }
  };

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
          gl_Position = modelToProjection * normalize(vec4(pos.xyz + cameraPosition, 1.0)); 
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
