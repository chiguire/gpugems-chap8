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

    vec3 camera_position;
    vec3 camera_rotation;

    float rotateAngle;
    mat4t cameraToWorld;
    mat4t modelToWorld;

    float rough;
    float spacing;
    vec4 hiliteColor;
    vec3 lightPosition;
    
    bool rotating;
    bool just_press_rotating;

    enum model {
      MODEL_CD,
      MODEL_CUBE
    };
    model current_model;
    bool just_press_change_model;

    // helper to rotate camera about scene
    //mouse_ball ball;

    GLuint cubeMapTex;
    GLuint leMapTex;

    cubemap_fragdiffraction_shader cubeMapDiffractionShader;
    cubemap_sky_shader cubeMapSkyShader;

    color_shader cshader;
    mesh ring;

  public:
    // this is called when we construct the class
    engine(int argc, char **argv)
    : app(argc, argv)
    , camera_position(0.0f, 0.0f, 3.0f)
    , camera_rotation(0.0f, 0.0f, 0.0f)
    //, ball()
    {

    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shaders
      cubeMapDiffractionShader.init();
      //cubeMapReflectionShader.init();
      cubeMapSkyShader.init();
      cshader.init();

      cameraToWorld.loadIdentity();
      modelToWorld.loadIdentity();

      rough = 50.0f;
      spacing = 10.0f;
      rotating = true;
      just_press_rotating = false;

      current_model = MODEL_CD;
      just_press_change_model = false;

      lightPosition = vec3(0.0f, 0.0f, 1.0f);
      hiliteColor = vec4(1.0f, 0.7f, 0.3f, 1.0f);

      mesh_builder mb;
      mb.add_one_CD(0.3f, 2.0f, 1.0f, 1.0f);
      mb.get_mesh(ring);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      rotateAngle = 0;

      cubeMapTex = resources::get_cubemap_texture_handle(GL_RGBA, "Tenerife4",
        "assets/cubemaps/Tenerife4/posx.jpg", "assets/cubemaps/Tenerife4/posy.jpg", "assets/cubemaps/Tenerife4/posz.jpg",
        "assets/cubemaps/Tenerife4/negx.jpg", "assets/cubemaps/Tenerife4/negy.jpg", "assets/cubemaps/Tenerife4/negz.jpg");

      printf("Cube map tex: %d\n", cubeMapTex);
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx, vy;
      get_viewport_size(vx, vy);
      // set a viewport - includes whole window area
      glViewport(0, 0, vx, vy);

      // clear the background to black
      glClearColor(0.5f, 0.2f, 0.2f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      renderSky();

      if (current_model == MODEL_CD) {
        renderCD();
      } else {
        renderCube();
      }
      
     
      if (rotating) rotateAngle += 1.0f;

      if (is_key_down('W')) {
        camera_position[1] += 0.25f * (camera_position[2]/5.0f);
      } else if (is_key_down('S')) {
        camera_position[1] -= 0.25f * (camera_position[2]/5.0f);
      }

      if (is_key_down('A')) {
        camera_position[0] -= 0.25f * (camera_position[2]/5.0f);
      } else if (is_key_down('D')) {
        camera_position[0] += 0.25f * (camera_position[2]/5.0f);
      }

      if (is_key_down('Q')) {
        camera_position[2] -= 0.25f;
        if (camera_position[2] < 2.0f) camera_position[2] = 2.0f;
      } else if (is_key_down('E')) {
        camera_position[2] += 0.25f;
        if (camera_position[2] > 200.0f) camera_position[2] = 200.0f;
      }

       if (is_key_down('F')) {
        camera_rotation[1] += 5.0f;
        if (camera_rotation[1] >= 360.0f) camera_rotation[1] -= 360.0f;
      } else if (is_key_down('H')) {
        camera_rotation[1] -= 5.0f;
        if (camera_rotation[1] < 0.0f) camera_rotation[1] += 360.0f;
      }

      if (is_key_down('G')) {
        camera_rotation[0] -= 5.0f;
        if (camera_rotation[0] < -60.0f) camera_rotation[0] = -60.0f;
      } else if (is_key_down('T')) {
        camera_rotation[0] += 5.0f;
        if (camera_rotation[0] > 60.0f) camera_rotation[0] = 60.0f;
      }

      if (is_key_down('Z')) {
        rough -= 1.0f;
        if (rough < 1.0f) rough = 1.0f;
      } else if (is_key_down('X')) {
        rough += 1.0f;
        if (rough > 500.0f) rough = 500.0f;
      }

      if (is_key_down('C')) {
        spacing -= 1.0f;
        if (spacing < 1.0f) spacing = 1.0f;
      } else if (is_key_down('V')) {
        spacing += 1.0f;
        if (spacing > 500.0f) spacing = 500.0f;
      }

      if (is_key_down(key_space) && !just_press_rotating) {
        rotating = !rotating;
        just_press_rotating = true;
      } else {
        just_press_rotating = false;
      }

      if (is_key_down('B') && !just_press_change_model) {
        printf("Change model to %s.\n", current_model == MODEL_CD? "cube": "cd");
        current_model = current_model == MODEL_CD? MODEL_CUBE: MODEL_CD;
        just_press_change_model = true;
      } else {
        just_press_change_model = false;
      }
    }

    void renderSky() {
      glDisable(GL_DEPTH_TEST);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      mat4t skyModelToWorld;
      skyModelToWorld.loadIdentity();
      mat4t skyCameraToWorld;
      skyCameraToWorld.loadIdentity();
      skyCameraToWorld.rotate(camera_rotation[1], 0.0f, 1.0f, 0.0f);
      skyCameraToWorld.rotate(camera_rotation[0], 1.0f, 0.0f, 0.0f);

      vec3 cameraPositionNormalized = camera_position.normalize();

      mat4t skyModelToProjection = mat4t::build_projection_matrix(skyModelToWorld, skyCameraToWorld);

      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_CUBE_MAP);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);

      cubeMapSkyShader.render(&skyModelToProjection, &cameraPositionNormalized, 0);

      float skyCubeVertices[] =
      {    // x, y, z
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        
        -1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f,  1.0f,
        -1.0f,  -1.0f,  1.0f,

        -1.0f, 1.0f,  1.0f,
        1.0f, 1.0f,  1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        1.0f, -1.0f,  1.0f,        
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
      };

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, skyCubeVertices);

      glEnableVertexAttribArray(attribute_pos);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

      glDisableVertexAttribArray(attribute_pos);
    }

    void renderCube() {
      glEnable(GL_TEXTURE_CUBE_MAP);
      glEnable(GL_DEPTH_TEST);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      cameraToWorld.loadIdentity();
      cameraToWorld.rotate(camera_rotation[1], 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(camera_rotation[0], 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(camera_position.x(), camera_position.y(), camera_position.z());
      
      modelToWorld.loadIdentity();
      modelToWorld.rotate(rotateAngle, 0.0f, 1.0f, 0.0f);

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
      mat4t modelToWorldIT = modelToWorld.inverse4x4().transpose4x4();

      vec3 cameraPositionNormalized = camera_position.normalize();

      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_CUBE_MAP);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
      
      cubeMapDiffractionShader.render(modelToProjection, modelToWorld, modelToWorldIT, rough, spacing, hiliteColor, lightPosition, cameraPositionNormalized, 0);

      float vertices[] = {
        1.0f,  1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -1.0f,  1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f,

        -1.0f,  1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        1.0f,  1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        
        1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        
        -1.0f,  1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f
      };

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), vertices);
      glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_TRUE, 9*sizeof(float), vertices+3);
      glVertexAttribPointer(attribute_tangent, 3, GL_FLOAT, GL_TRUE, 9*sizeof(float), vertices+6);

      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_normal);
      glEnableVertexAttribArray(attribute_tangent);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
      glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

      glDisableVertexAttribArray(attribute_pos);
      glDisableVertexAttribArray(attribute_normal);
      glDisableVertexAttribArray(attribute_tangent);
    }

    void renderCD() {
      glEnable(GL_TEXTURE_CUBE_MAP);
      glEnable(GL_DEPTH_TEST);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      cameraToWorld.loadIdentity();
      cameraToWorld.rotate(camera_rotation[1], 0.0f, 1.0f, 0.0f);
      cameraToWorld.rotate(camera_rotation[0], 1.0f, 0.0f, 0.0f);
      cameraToWorld.translate(camera_position.x(), camera_position.y(), camera_position.z());
      
      modelToWorld.loadIdentity();
      modelToWorld.rotate(rotateAngle, 0.0f, 1.0f, 0.0f);

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
      mat4t modelToWorldIT = modelToWorld.inverse4x4().transpose4x4();

      vec3 cameraPositionNormalized = camera_position.normalize();

      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_CUBE_MAP);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
      
      cubeMapDiffractionShader.render(modelToProjection, modelToWorld, modelToWorldIT, rough, spacing, hiliteColor, lightPosition, cameraPositionNormalized, 0);

      ring.render();
    }
     
  };
}
