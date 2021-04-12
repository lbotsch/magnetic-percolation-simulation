#include <exception>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include "graphics.h"

#include <cairommconfig.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

extern "C"{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

using namespace std;


void draw_grid(const Grid &grid, Cairo::RefPtr<Cairo::ImageSurface> surface)
{
  auto cr = Cairo::Context::create(surface);
  auto proj = grid.project_grid();
  int cell_W = (surface->get_width() - 20) / grid.dimensions().X;
  int cell_H = (surface->get_height() - 60) / grid.dimensions().Y;

  if (cell_W > cell_H) cell_W = cell_H;
  else cell_H = cell_W;

  int W = grid.dimensions().X*cell_W;
  int H = grid.dimensions().Y*cell_H;
  int off_X = (surface->get_width() - W)/2 + 10;
  int off_Y = (surface->get_height() - H)/2 + 50;

  // Fill with white
  cr->save();
  cr->set_source_rgb(1.0,1.0,1.0);
  cr->paint();
  cr->restore();

  // Draw title
  cr->save();
  cr->set_source_rgb(1.0,1.0,1.0);
  // TODO Write text
  cr->restore();

  // Draw grid
  cr->save();
  cr->set_line_width(10.0);
  cr->rectangle(0.0, 0.0, surface->get_width(), surface->get_height());
  cr->stroke();
}

void draw_domains(const Grid &grid, Cairo::RefPtr<Cairo::ImageSurface> surface)
{
  auto cr = Cairo::Context::create(surface);
  int cell_W = (surface->get_width() - 10) / grid.dimensions().X;
  int cell_H = (surface->get_height() - 20) / grid.dimensions().Y;

  if (cell_W > cell_H) cell_W = cell_H;
  else cell_H = cell_W;

  int W = grid.dimensions().X*cell_W;
  int H = grid.dimensions().Y*cell_H;
  int off_X = (surface->get_width() - W)/2 + 5;
  int off_Y = (surface->get_height() - H)/2 + 25;

  // Fill with white
  cr->save();
  cr->set_source_rgb(0.9,0.9,0.9);
  cr->paint();
  cr->restore();

  // Draw title
  stringstream ss;
  switch(grid.type()) {
  case Grid::GRID_SC: ss << "SC"; break;
  case Grid::GRID_HEX: ss << "Hex"; break;
  default: ss << "Unknown"; break;
  }
  ss << fixed << setprecision(4) << setfill('0')
     << " Grid ("
     << grid.dimensions().X << "x"
     << grid.dimensions().Y << "x"
     << grid.dimensions().Z << ") - P = "
     << grid.density();
  string title(ss.str());
  cr->save();
  cr->set_source_rgb(0.0,0.0,0.0);
  cr->move_to(surface->get_width()/2 - 220, 35);
  auto font = Cairo::ToyFontFace::create("Bitstream Charter",
                                         Cairo::FONT_SLANT_NORMAL,
                                         Cairo::FONT_WEIGHT_BOLD);
  cr->set_font_face(font);
  cr->set_font_size(30);
  cr->show_text(title);
  cr->restore();

  // Draw grid
  auto proj = grid.project_domains();
  mt19937 rng;
  uniform_real_distribution<double> color(0.0, 1.0);
  cr->save();
  cr->translate(off_X, off_Y);
  for (size_t xx = 0; xx < grid.dimensions().X; ++xx) {
    cr->save();
    for (size_t yy = 0; yy < grid.dimensions().Y; ++yy) {
      size_t idx = xx*grid.dimensions().Y + yy;
      if (proj[idx] != 0) {
        rng.seed(proj[idx]);
        cr->set_source_rgb(color(rng), color(rng), color(rng));
      } else {
        cr->set_source_rgb(0.0, 0.0, 0.0);
      }
      cr->rectangle(0.0, 0.0, cell_W, cell_H);
      cr->fill();
      cr->translate(cell_W, 0);
    }
    cr->restore();
    cr->translate(0, cell_H);
  }
  cr->restore();
}

void draw_spins(const Grid &grid, Cairo::RefPtr<Cairo::ImageSurface> surface)
{
  auto cr = Cairo::Context::create(surface);
  auto proj = grid.project_spins();
  // TODO Draw
}

Image draw_grid(const Grid& grid, size_t img_width, size_t img_height)
{
  Image pix_data(img_width*img_height);
  auto surface = Cairo::ImageSurface::create(static_cast<unsigned char *>(&pix_data[0].R),
                                             Cairo::FORMAT_RGB24, img_width, img_height,
                                             img_width*sizeof(PixelRGB24));
  draw_grid(grid, surface);
  surface->finish();
  return pix_data;
}

Image draw_grid(const std::string filename, const Grid& grid, size_t img_width, size_t img_height)
{
  Image pix_data(img_width*img_height);
  auto surface = Cairo::ImageSurface::create(static_cast<unsigned char *>(&pix_data[0].R),
                                             Cairo::FORMAT_RGB24, img_width, img_height,
                                             img_width*sizeof(PixelRGB24));
  draw_grid(grid, surface);
  surface->write_to_png(filename);
  surface->finish();
  return pix_data;
}

Image draw_domains(const Grid& grid, size_t img_width, size_t img_height)
{
  Image pix_data(img_width*img_height);
  auto surface = Cairo::ImageSurface::create(static_cast<unsigned char *>(&pix_data[0].R),
                                             Cairo::FORMAT_RGB24, img_width, img_height,
                                             img_width*sizeof(PixelRGB24));
  draw_domains(grid, surface);
  surface->finish();
  return pix_data;
}
Image draw_domains(const std::string filename, const Grid& grid, size_t img_width, size_t img_height)
{
  Image pix_data(img_width*img_height);
  int stride = img_width*sizeof(PixelRGB24);
  int stride2 = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_RGB24, img_width);
  assert(stride == stride2);
  auto surface = Cairo::ImageSurface::create(static_cast<unsigned char *>(&pix_data[0].R),
                                             Cairo::FORMAT_RGB24, img_width, img_height,
                                             stride);
  draw_domains(grid, surface);
  surface->write_to_png(filename);
  surface->finish();
  return pix_data;
}
Image draw_spins(const Grid& grid, size_t img_width, size_t img_height)
{
  Image pix_data(img_width*img_height);
  auto surface = Cairo::ImageSurface::create(static_cast<unsigned char *>(&pix_data[0].R),
                                             Cairo::FORMAT_RGB24, img_width, img_height,
                                             img_width*sizeof(PixelRGB24));
  draw_spins(grid, surface);
  surface->finish();
  return pix_data;
}
Image draw_spins(const std::string filename, const Grid& grid, size_t img_width, size_t img_height)
{
  Image pix_data(img_width*img_height);
  auto surface = Cairo::ImageSurface::create(static_cast<unsigned char *>(&pix_data[0].R),
                                             Cairo::FORMAT_RGB24, img_width, img_height,
                                             img_width*sizeof(PixelRGB24));
  draw_spins(grid, surface);
  surface->write_to_png(filename);
  surface->finish();
  return pix_data;
}


void init_video_lib()
{
  av_register_all();
  AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (codec == NULL) {
    // Could not find H264 encoder
    throw runtime_error("No H264 codec found.");
  }
}

struct VideoEncoderState
{
  struct SwsContext *convert_ctx;
  AVFormatContext *output_format_ctx;
  AVStream *output_stream;
  AVCodec *codec;
  AVCodecContext *codec_ctx;

  AVFrame *rgb_frame;
  AVFrame *yuv_frame;
  AVPacket pkt;
};

VideoEncoder::VideoEncoder(const string &filename, int width, int height, int fps) :
  filename(filename), width(width), height(height), fps(fps)
{
  int ret;
  state = new VideoEncoderState();
  state->convert_ctx = sws_getContext(width, height, AV_PIX_FMT_RGB24,
                                      width, height, AV_PIX_FMT_YUV420P,
                                      SWS_FAST_BILINEAR, NULL, NULL, NULL);
  AVOutputFormat * fmt = av_guess_format("mp4", NULL, NULL);
  avformat_alloc_output_context2(&state->output_format_ctx, NULL, NULL, filename.c_str());
  state->output_stream = avformat_new_stream(state->output_format_ctx, 0);
  state->output_stream->time_base=(AVRational){1, fps};
  state->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  AVDictionary *opt = NULL;
  av_dict_set(&opt, "preset", "slow", 0);
  av_dict_set(&opt, "crf", "20", 0);
  state->codec_ctx = avcodec_alloc_context3(state->codec);
  avcodec_parameters_to_context(state->codec_ctx, state->output_stream->codecpar);
  state->codec_ctx->width = width;
  state->codec_ctx->height = height;
  state->codec_ctx->time_base=(AVRational){1, fps};
  state->codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  // Some formats require a global header.
  if (state->output_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    state->codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  avcodec_open2(state->codec_ctx, state->codec, &opt);
  av_dict_free(&opt);
  //state->output_stream->codec = state->codec_ctx;
  av_dump_format(state->output_format_ctx, 0, filename.c_str(), 1);
  avio_open(&state->output_format_ctx->pb, filename.c_str(), AVIO_FLAG_WRITE);
  ret=avformat_write_header(state->output_format_ctx, &opt);
  av_dict_free(&opt);

  // Initialize frame data structures
  state->rgb_frame=av_frame_alloc();
  state->rgb_frame->format=AV_PIX_FMT_RGB24;
  state->rgb_frame->width=width;
  state->rgb_frame->height=height;
  ret=av_frame_get_buffer(state->rgb_frame, 1);

  state->yuv_frame=av_frame_alloc();
  state->yuv_frame->format=AV_PIX_FMT_YUV420P;
  state->yuv_frame->width=width;
  state->yuv_frame->height=height;
  ret=av_frame_get_buffer(state->yuv_frame, 1);
}

VideoEncoder::~VideoEncoder()
{
  // Freeing all the allocated memory:
  sws_freeContext(state->convert_ctx);
  av_frame_free(&state->rgb_frame);
  av_frame_free(&state->yuv_frame);
  avformat_free_context(state->output_format_ctx);
}

void VideoEncoder::add_frame(const Image &frame)
{
  int ret;
  int got_output;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // rgbpic->linesize[0] is equal to width.
      state->rgb_frame->data[0][y*state->rgb_frame->linesize[0]+3*x]=frame[x*y+y].R;
      state->rgb_frame->data[0][y*state->rgb_frame->linesize[0]+3*x+1]=frame[x*y+y].G;
      state->rgb_frame->data[0][y*state->rgb_frame->linesize[0]+3*x+2]=frame[x*y+y].B;
    }
  }
  // Not actually scaling anything, but just converting the RGB data to YUV and
  // store it in yuvpic.
  sws_scale(state->convert_ctx, state->rgb_frame->data, state->rgb_frame->linesize,
            0, height, state->yuv_frame->data, state->yuv_frame->linesize); 
  // The PTS of the frame are just in a reference unit, unrelated to the format
  // we are using. We set them, for instance, as the corresponding frame number.
  state->yuv_frame->pts = frame_counter++;
  ret=avcodec_send_frame(state->codec_ctx, state->yuv_frame);
  if (ret == 0) {
    ret=avcodec_receive_packet(state->codec_ctx, &state->pkt);
    if (ret == 0) {
      // We set the packet PTS and DTS taking in the account our FPS (second
      // argument) and the time base that our selected format uses (third
      // argument).
      av_packet_rescale_ts(&state->pkt, (AVRational){1, fps}, state->output_stream->time_base); 
      state->pkt.stream_index = state->output_stream->index;
      // Write the encoded frame to the mp4 file.
      av_interleaved_write_frame(state->output_format_ctx, &state->pkt); 
      av_packet_unref(&state->pkt);
    }
  }
}

void VideoEncoder::save()
{
  // Writing the delayed frames:
  while(avcodec_receive_packet(state->codec_ctx, &state->pkt) == 0) {
    av_packet_rescale_ts(&state->pkt, (AVRational){1, fps}, state->output_stream->time_base);
    state->pkt.stream_index = state->output_stream->index;
    av_interleaved_write_frame(state->output_format_ctx, &state->pkt);
    av_packet_unref(&state->pkt);
  }
  // Writing the end of the file.
  av_write_trailer(state->output_format_ctx);
  // Close the file.
  if (!(state->output_format_ctx->flags & AVFMT_NOFILE))
    avio_closep(&state->output_format_ctx->pb);
  avcodec_close(state->codec_ctx);
}
