#pragma region CPL License
/*
Nuclex FrameFixer
Copyright (C) 2024 Nuclex Development Labs

This application is free software; you can redistribute it and/or modify it
under the terms of the IBM Common Public License as published by
the IBM Corporation; either version 1.0 of the License,
or (at your option) any later version.

This application is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the IBM Common Public License
for more details.

You should have received a copy of the IBM Common Public License
along with this library
*/
#pragma endregion // CPL License

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "./LibAvDeinterlacer.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception for the specified libav result code</summary>
  /// <param name="libavResult">
  ///   libav result code for which an error message will be provided in the exception
  /// </param>
  /// <param name="message">
  ///   Additional text that will be prefixed to the exception message
  /// </param>
  [[noreturn]] void throwExceptionForAvError(int libAvResult, const std::string &message) {
    char buffer[1024];
    int errorStringResult = ::av_strerror(libAvResult, buffer, sizeof(buffer));

    std::string combinedMessage(message);
    if(errorStringResult == 0) {
      combinedMessage.append(buffer);
    } else {
      combinedMessage.append(u8"unknown error ", 14);
      Nuclex::Support::Text::lexical_append(combinedMessage, libAvResult);
    }

    throw std::runtime_error(combinedMessage);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm::Deinterlacing {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<::AVFrame> LibAvDeinterlacerBase::AvFrameFromQImage(const QImage &image) {
    using Nuclex::FrameFixer::Platform::LibAvApi;

    std::shared_ptr<::AVFrame> frame = LibAvApi::NewAvFrame();

    frame->width = image.width();
    frame->height = image.height();
    
    // TODO: Cheap and insufficient decision between 16 bits per color channel
    //       and 8 bits per color channel. I only have the former kind of images
    //       currently, but this should compare the actual pixel formats!
    if(image.bytesPerLine() >= image.width() * 8) {
      frame->format = AV_PIX_FMT_RGBA64LE; // AV_PIX_FMT_BGR48LE

      LibAvApi::LockAvFrameBuffer(frame);

      std::uint8_t *frameData = frame->data[0];
      std::size_t frameHeight = static_cast<std::size_t>(frame->height);
      for(std::size_t lineIndex = 0; lineIndex < frameHeight; ++lineIndex) {
        std::copy_n(
          image.scanLine(lineIndex), // Will be QRgba64 (64 bit interleaved) pixels
          image.bytesPerLine(),
          frameData
        );
        frameData += frame->linesize[0];
      }
    } else {
      frame->format = AV_PIX_FMT_RGBA; // AV_PIX_FMT_ABGR;

      LibAvApi::LockAvFrameBuffer(frame);

      std::uint8_t *frameData = frame->data[0];
      std::size_t frameHeight = static_cast<std::size_t>(frame->height);
      for(std::size_t lineIndex = 0; lineIndex < frameHeight; ++lineIndex) {
        std::copy_n(
          image.scanLine(lineIndex), // Will be QRgb (32 bit interleaved) pixels
          image.bytesPerLine(),
          frameData
        );
        frameData += frame->linesize[0];
      }
    }

    return frame;
  }

  // ------------------------------------------------------------------------------------------- //

  void LibAvDeinterlacerBase::CopyAvFrameToQImage(
    const std::shared_ptr<::AVFrame> &frame, QImage &image
  ) {
    using Nuclex::FrameFixer::Platform::LibAvApi;

    bool dimensionsMatch = (
      (frame->width == image.width()) &&
      (frame->height == image.height())
    );
    if(!dimensionsMatch) {
      throw std::runtime_error(u8"Processed AV frame has different dimensions from QImage");
    }

    // TODO: Cheap and insufficient decision between 16 bits per color channel
    //       and 8 bits per color channel. I only have the former kind of images
    //       currently, but this should compare the actual pixel formats!
    if(image.bytesPerLine() >= image.width() * 8) {
      if(frame->data[0] == nullptr) {
        LibAvApi::LockAvFrameBuffer(frame);
      }
      if(frame->format == AV_PIX_FMT_GBRAP16LE) { // planar output (NNedi does this)
        const std::uint8_t *greenChannel = frame->data[0];
        const std::uint8_t *blueChannel = frame->data[1];
        const std::uint8_t *redChannel = frame->data[2];
        const std::uint8_t *alphaChannel = frame->data[3];

        std::size_t frameHeight = static_cast<std::size_t>(frame->height);
        for(std::size_t lineIndex = 0; lineIndex < frameHeight; ++lineIndex) {
          QRgba64 *targetScanline = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          std::size_t frameWidth = static_cast<std::size_t>(frame->width);
          for(std::size_t pixelIndex = 0; pixelIndex < frameWidth; ++pixelIndex) {
            targetScanline[pixelIndex] = QRgba64::fromRgba64(
              reinterpret_cast<const quint16 *>(redChannel)[pixelIndex],
              reinterpret_cast<const quint16 *>(greenChannel)[pixelIndex],
              reinterpret_cast<const quint16 *>(blueChannel)[pixelIndex],
              reinterpret_cast<const quint16 *>(alphaChannel)[pixelIndex]
            );
          }

          targetScanline += image.bytesPerLine();
          greenChannel += frame->linesize[0];
          blueChannel += frame->linesize[1];
          redChannel += frame->linesize[2];
          alphaChannel += frame->linesize[3];
        }
      } else if(frame->format == AV_PIX_FMT_GBRAP) { // planar output (Yadif does this)
        const std::uint8_t *greenChannel = frame->data[0];
        const std::uint8_t *blueChannel = frame->data[1];
        const std::uint8_t *redChannel = frame->data[2];
        const std::uint8_t *alphaChannel = frame->data[3];

        std::size_t frameHeight = static_cast<std::size_t>(frame->height);
        for(std::size_t lineIndex = 0; lineIndex < frameHeight; ++lineIndex) {
          QRgba64 *targetScanline = reinterpret_cast<QRgba64 *>(image.scanLine(lineIndex));
          std::size_t frameWidth = static_cast<std::size_t>(frame->width);
          for(std::size_t pixelIndex = 0; pixelIndex < frameWidth; ++pixelIndex) {
            targetScanline[pixelIndex] = QRgba64::fromRgba(
              reinterpret_cast<const quint8 *>(redChannel)[pixelIndex],
              reinterpret_cast<const quint8 *>(greenChannel)[pixelIndex],
              reinterpret_cast<const quint8 *>(blueChannel)[pixelIndex],
              reinterpret_cast<const quint8 *>(alphaChannel)[pixelIndex]
            );
          }

          targetScanline += image.bytesPerLine();
          greenChannel += frame->linesize[0];
          blueChannel += frame->linesize[1];
          redChannel += frame->linesize[2];
          alphaChannel += frame->linesize[3];
        }
      } else if(frame->format == AV_PIX_FMT_RGBA64LE) { // AV_PIX_FMT_BGR48LE
        const std::uint8_t *frameData = frame->data[0];
        std::size_t frameHeight = static_cast<std::size_t>(frame->height);
        for(std::size_t lineIndex = 0; lineIndex < frameHeight; ++lineIndex) {
          std::copy_n(
            frameData,
            frame->linesize[0],
            image.scanLine(lineIndex) // Will be QRgba64 (64 bit interleaved) pixels
          );
          frameData += frame->linesize[0];
        }
      } else {
        throw std::runtime_error(u8"Processed AV frame has different pixel format from QImage");
      }
    } else {
      if(frame->format != AV_PIX_FMT_RGBA) { // AV_PIX_FMT_ABGR;
        throw std::runtime_error(u8"Processed AV frame has different pixel format from QImage");
      }
      if(frame->data[0] == nullptr) {
        LibAvApi::LockAvFrameBuffer(frame);
      }

      const std::uint8_t *frameData = frame->data[0];
      std::size_t frameHeight = static_cast<std::size_t>(frame->height);
      for(std::size_t lineIndex = 0; lineIndex < frameHeight; ++lineIndex) {
        std::copy_n(
          frameData,
          frame->linesize[0],
          image.scanLine(lineIndex) // Will be QRgb (32 bit interleaved) pixels
        );
        frameData += frame->linesize[0];
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm::Deinterlacing
