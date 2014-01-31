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

    bool is_frag_shader_mode;
    bool just_press_frag_shader_mode;

    bool just_press_dump_info;
    
    bool normals_visible;
    bool just_press_normals_visible;

    bool tangents_visible;
    bool just_press_tangents_visible;

    bool show_help;
    bool just_press_show_help;

    enum model {
      MODEL_CD,
      MODEL_CUBE
    };
    model current_model;
    bool just_press_change_model;

    // helper to rotate camera about scene
    //mouse_ball ball;

    GLuint cubeMapTex;
    GLuint helpTex;

    cubemap_fragdiffraction_shader cubeMapDiffractionShader;
    cubemap_diffraction_shader cubeMapVertexDifractionShader;
    cubemap_sky_shader cubeMapSkyShader;
    texture_shader tShader;
    color_shader cshader;

    mesh ring;
    mesh ring_normals;
    mesh ring_tangents;

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
      cubeMapVertexDifractionShader.init();
      cubeMapSkyShader.init();
      cshader.init();
      tShader.init();

      cameraToWorld.loadIdentity();
      modelToWorld.loadIdentity();

      rough = 50.0f;
      spacing = 10.0f;
      rotating = true;
      just_press_rotating = false;

      is_frag_shader_mode = true;
      just_press_frag_shader_mode = false;

      current_model = MODEL_CD;
      just_press_change_model = false;

      just_press_dump_info = false;

      normals_visible = false;
      just_press_normals_visible = false;

      tangents_visible = false;
      just_press_tangents_visible = false;

      show_help = true;
      just_press_show_help = false;

      lightPosition = vec3(0.0f, -1.0f*sin(10*3.14159f/180.0f), 1.0f*cos(10*3.14159f/180.0f));
      hiliteColor = vec4(1.0f, 0.7f, 0.3f, 1.0f);

      mesh_builder mb;
      mb.add_one_CD(0.3f, 2.0f, 1.0f, 1.0f);
      mb.get_mesh(ring);

      ring_normals.init();
      ring_normals.make_normal_visualizer(ring, 1.0f, attribute_normal);

      ring_tangents.init();
      ring_tangents.make_normal_visualizer(ring, 1.0f, attribute_tangent);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      rotateAngle = 0;

      cubeMapTex = resources::get_cubemap_texture_handle(GL_RGBA, "Tenerife4",
        "assets/cubemaps/Tenerife4/posx.jpg", "assets/cubemaps/Tenerife4/posy.jpg", "assets/cubemaps/Tenerife4/posz.jpg",
        "assets/cubemaps/Tenerife4/negx.jpg", "assets/cubemaps/Tenerife4/negy.jpg", "assets/cubemaps/Tenerife4/negz.jpg");

      helpTex = resources::get_texture_handle(GL_RGBA, "assets/help.gif");

      //printf("Cube map tex: %d\n", cubeMapTex);
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

      renderHelp();
      
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
      }
      if (!is_key_down(key_space)) {
        just_press_rotating = false;
      }

      if (is_key_down('B') && !just_press_change_model) {
        printf("Change model to %s.\n", current_model == MODEL_CD? "cube": "cd");
        current_model = current_model == MODEL_CD? MODEL_CUBE: MODEL_CD;
        just_press_change_model = true;
      }
      if (!is_key_down('B')) {
        just_press_change_model = false;
      }

      if (is_key_down('N') && !just_press_change_model) {
        printf("Change shader to %s.\n", is_frag_shader_mode? "vertex-based": "fragment-based");
        is_frag_shader_mode = !is_frag_shader_mode;
        just_press_frag_shader_mode = true;
      }
      if (!is_key_down('N')) {
        just_press_frag_shader_mode = false;
      }

      if (is_key_down('M') && !just_press_dump_info) {
        printf("Current shader is %s.\n", is_frag_shader_mode? "fragment-based": "vertex-based");
        printf("Current model is %s.\n", current_model == MODEL_CD? "CD": "cube");
        printf("Rough: %.2f.\n", rough);
        printf("Spacing: %.2f.\n", spacing);
        printf("Light position: (%.2f, %2.f, %.2f)\n", lightPosition[0], lightPosition[1], lightPosition[2]);
        printf("Hilite color: (%.2f, %.2f, %.2f, %.2f)\n", hiliteColor[0], hiliteColor[1], hiliteColor[2], hiliteColor[3]);
        printf("Camera position: (%.2f, %.2f, %.2f), Rotation: (%.2f, %.2f, %.2f)\n\n", camera_position[0], camera_position[1], camera_position[2], camera_rotation[0], camera_rotation[1], camera_rotation[2]);
        just_press_dump_info = true;
      }
      if (!is_key_down('N')) {
        just_press_dump_info = false;
      }

      if (is_key_down('R') && !just_press_normals_visible) {
        normals_visible = !normals_visible;
        printf("Normals visible: %s.\n", normals_visible? "yes": "no");
        just_press_normals_visible = true;
      }
      if (!is_key_down('R')) {
        just_press_normals_visible = false;
      }

      if (is_key_down('Y') && !just_press_tangents_visible) {
        tangents_visible = !tangents_visible;
        printf("Tangents visible: %s.\n", tangents_visible? "yes": "no");
        just_press_tangents_visible = true;
      }
      if (!is_key_down('Y')) {
        just_press_tangents_visible = false;
      }
      
      if (is_key_down('J') && !just_press_show_help) {
        show_help = !show_help;
        just_press_show_help = true;
      }
      if (!is_key_down('J')) {
        just_press_show_help = false;
      }
    }

    void renderSky() {
      glDisable(GL_DEPTH_TEST);
      glDepthMask(false);
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
      glDepthMask(true);
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

      vec3 cameraPos = vec4(0.0f, 0.0f, 0.0f, 1.0f)*cameraToWorld;

      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_CUBE_MAP);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
      
      if (is_frag_shader_mode) {
        cubeMapDiffractionShader.render(modelToProjection, modelToWorld, modelToWorldIT, rough, spacing, hiliteColor, lightPosition, cameraPos, 0);
      } else {
        cubeMapVertexDifractionShader.render(modelToProjection, modelToWorld, modelToWorldIT, rough, spacing, hiliteColor, lightPosition, cameraPos, 0);
      }

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
      
      vec3 cameraPos = vec4(0.0f, 0.0f, 0.0f, 1.0f)*cameraToWorld;
       
      modelToWorld.loadIdentity();
      modelToWorld.rotate(rotateAngle, 0.0f, 1.0f, 0.0f);

      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
      mat4t modelToWorldIT = modelToWorld.inverse4x4().transpose4x4();

      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_CUBE_MAP);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
      
      if (is_frag_shader_mode) {
        cubeMapDiffractionShader.render(modelToProjection, modelToWorld, modelToWorldIT, rough, spacing, hiliteColor, lightPosition, cameraPos, 0);
      } else {
        cubeMapVertexDifractionShader.render(modelToProjection, modelToWorld, modelToWorldIT, rough, spacing, hiliteColor, lightPosition, cameraPos, 0);
      }

      ring.render();

      if (normals_visible) {
        cshader.render(modelToProjection, vec4(0.0f, 0.0f, 1.0f, 1.0f));
        ring_normals.render();
      }

      if (tangents_visible) {
        cshader.render(modelToProjection, vec4(1.0f, 0.0f, 0.0f, 1.0f));
        ring_tangents.render();
      }
    }
     
    void renderHelp() {

      if (!show_help) return;

      glDisable(GL_DEPTH_TEST);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      mat4t help_text_matrix;
      help_text_matrix.loadIdentity();

      mat4t help_cam_matrix;
      help_cam_matrix.loadIdentity();

      mat4t modelToProjection = mat4t::build_projection_matrix(help_text_matrix, help_cam_matrix);
      
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      tShader.render(modelToProjection, 0);

      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_CUBE_MAP);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, helpTex);
      
      float vertices[] = {
        -0.9f,  -0.9f, -1.0f, 0.0f, 0.0f,
        0.9f,  -0.9f,  -1.0f, 1.0f, 0.0f,
        0.9f, -0.6f,  -1.0f, 1.0f, 1.0f,
        -0.9f, -0.6f, -1.0f, 0.0f, 1.0f
      };

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), vertices);
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), vertices+3);

      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_uv);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

      glDisableVertexAttribArray(attribute_pos);
      glDisableVertexAttribArray(attribute_uv);
    }
  };
}
