////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
// (C) Ciro Duran, Bogdan Catana 2013, 2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Main engine. Employs a cube map viewer and a diffraction simulator shader.
//

namespace octet {
  class engine : public app {
    typedef mat4t mat4t;
    typedef vec4 vec4;

    float rotateAngle;
    mat4t cameraToWorld;
    mat4t modelToWorld;

    // helper to rotate camera about scene
    mouse_ball ball;

    GLuint cubeMapTex;

    cubemap_shader cubeMapShader;

  public:
    // this is called when we construct the class
    engine(int argc, char **argv) : app(argc, argv), ball() {

    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shaders
      cubeMapShader.init();

      cameraToWorld.loadIdentity();
      modelToWorld.loadIdentity();

      rotateAngle = 0;

      cubeMapTex = resources::get_cubemap_texture_handle(GL_RGBA, "Tenerife4",
        "assets/cubemaps/Tenerife4/posx.jpg", "assets/cubemaps/Tenerife4/posy.jpg", "assets/cubemaps/Tenerife4/posz.jpg",
        "assets/cubemaps/Tenerife4/negx.jpg", "assets/cubemaps/Tenerife4/negy.jpg", "assets/cubemaps/Tenerife4/negz.jpg");
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx, vy;
      get_viewport_size(vx, vy);
      // set a viewport - includes whole window area
      glViewport(0, 0, vx, vy);

      // clear the background to black
      glClearColor(0.5f, 0.5f, 0.5f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 3.0f);
      
      modelToWorld.loadIdentity();
      modelToWorld.rotate(rotateAngle, 0.0f, 1.0f, 0.0f);

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      cubeMapShader.render(modelToProjection, cubeMapTex);

      float vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f
      };

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), vertices);
      glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_TRUE, 6*sizeof(float), vertices+3);

      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_normal);
      
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

      rotateAngle += 1.0f;
    }
  };
}
