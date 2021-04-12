#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>

#include "grid.h"

struct PixelRGB24 {
  unsigned char R;
  unsigned char G;
  unsigned char B;
  unsigned char __padding;
};

typedef std::vector<PixelRGB24> ImageRGB24;
typedef ImageRGB24 Image;

Image draw_grid(const Grid& grid, size_t img_width, size_t img_height);
Image draw_grid(const std::string filename, const Grid& grid, size_t img_width, size_t img_height);
Image draw_domains(const Grid& grid, size_t img_width, size_t img_height);
Image draw_domains(const std::string filename, const Grid& grid, size_t img_width, size_t img_height);
Image draw_spins(const Grid& grid, size_t img_width, size_t img_height);
Image draw_spins(const std::string filename, const Grid& grid, size_t img_width, size_t img_height);

void init_video_lib();

struct VideoEncoderState;

class VideoEncoder
{
public:
  explicit VideoEncoder(const std::string &filename, int width, int height, int fps);
  ~VideoEncoder();

  void add_frame(const Image &frame);
  void save();

private:
  std::string filename;
  int width;
  int height;
  int fps;
  int frame_counter = 0;
  struct VideoEncoderState *state;
};

#endif
