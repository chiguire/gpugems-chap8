////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a container for named resources
//

namespace octet {
  // todo: kill this
  GLuint resources::get_texture_handle_internal(unsigned gl_kind, const char *url) {
    if (url[0] == '!') {
      return app_utils::get_stock_texture(gl_kind, url+1);
    } else if (url[0] == '#') {
      return app_utils::get_solid_texture(gl_kind, url+1);
    } else {
      dynarray<uint8_t> buffer;
      dynarray<uint8_t> image;
      app_utils::get_url(buffer, url);
      uint16_t format = 0;
      uint16_t width = 0;
      uint16_t height = 0;
      const unsigned char *src = &buffer[0];
      const unsigned char *src_max = src + buffer.size();
      if (buffer.size() >= 6 && !memcmp(&buffer[0], "GIF89a", 6)) {
        gif_decoder dec;
        dec.get_image(image, format, width, height, src, src_max);
      } else if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(image, format, width, height, src, src_max);
      } else if (buffer.size() >= 6 && buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 2) {
        tga_decoder dec;
        dec.get_image(image, format, width, height, src, src_max);
      } else {
        printf("warning: unknown texture format\n");
        return 0;
      }

      if (width > 0 && height > 0 && format) {
        return app_utils::make_texture(format, &image[0], image.size(), format, width, height);
      } else
      {
        return 0;
      }
    }
  };

  GLuint resources::get_cubemap_texture_handle_internal(unsigned gl_kind, const char *texName,
    const char *posx, const char *posy, const char *posz,
    const char *negx, const char *negy, const char *negz) {
    
      dynarray<uint8_t> buffer;
      dynarray<uint8_t> posx_image;
      dynarray<uint8_t> posy_image;
      dynarray<uint8_t> posz_image;
      dynarray<uint8_t> negx_image;
      dynarray<uint8_t> negy_image;
      dynarray<uint8_t> negz_image;

      posx_image.reset();
      posy_image.reset();
      posz_image.reset();
      negx_image.reset();
      negy_image.reset();
      negz_image.reset();

      uint16_t format = 0;
      uint16_t width = 0;
      uint16_t height = 0;

      buffer.reset();
      app_utils::get_url(buffer, posx);
      unsigned char *src = &buffer[0];
      unsigned char *src_max = src + buffer.size();
      if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(posx_image, format, width, height, src, src_max);
      } else {
        printf("warning: cube map only accepts JPEG format. posx.\n");
        return 0;
      }

      buffer.reset();
      app_utils::get_url(buffer, posy);
      src = &buffer[0];
      src_max = src + buffer.size();
      if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(posy_image, format, width, height, src, src_max);
      } else {
        printf("warning: cube map only accepts JPEG format. posy.\n");
        return 0;
      } 

      buffer.reset();
      app_utils::get_url(buffer, posz);
      src = &buffer[0];
      src_max = src + buffer.size();
      if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(posz_image, format, width, height, src, src_max);
      } else {
        printf("warning: cube map only accepts JPEG format. posz.\n");
        return 0;
      }

      buffer.reset();
      app_utils::get_url(buffer, negx);
      src = &buffer[0];
      src_max = src + buffer.size();
      if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(negx_image, format, width, height, src, src_max);
      } else {
        printf("warning: cube map only accepts JPEG format. negx.\n");
        return 0;
      } 

      buffer.reset();
      app_utils::get_url(buffer, negy);
      src = &buffer[0];
      src_max = src + buffer.size();
      if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(negy_image, format, width, height, src, src_max);
      } else {
        printf("warning: cube map only accepts JPEG format. negy.\n");
        return 0;
      }

      buffer.reset();
      app_utils::get_url(buffer, negz);
      src = &buffer[0];
      src_max = src + buffer.size();
      if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(negz_image, format, width, height, src, src_max);
      } else {
        printf("warning: cube map only accepts JPEG format. negz.\n");
        return 0;
      }

      if (width > 0 && height > 0 && format) {
        return app_utils::make_cubemap_texture(format, posx_image.size(), format, width, height,
          &posx_image[0], &posy_image[0], &posz_image[0], &negx_image[0], &negy_image[0], &negz_image[0]);
      } else
      {
        return 0;
      }
  };
}
