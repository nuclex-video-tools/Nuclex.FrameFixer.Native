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

#include "./GradientMatrix.h"

#include <cstdlib> // for std::aligned_alloc()
#include <cassert> // for assert()
#include <stdexcept> // for std::invalid_arguments

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the offset of the row array in the gradient matrix buffer</summary>
  /// <returns>
  ///   The offset, in bytes, from a pointer to a gradient matrix to the first
  ///   row pointer for the matrix elements.
  /// </returns>
  template<typename TGradientMatrix>
  constexpr std::size_t getRowArrayOffset() {
    constexpr std::size_t extraByteCount = (
      sizeof(TGradientMatrix) %
      alignof(Nuclex::FrameFixer::Algorithm::Gradient *)
    );
    return (
      sizeof(TGradientMatrix) + (
        (extraByteCount == 0) ?
        (0) :
        (sizeof(TGradientMatrix) - extraByteCount)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Calculates the offset of the matrix elements in the gradient matrix buffer
  /// </summary>
  /// <param name="rowArrayOffset">Offset of the row pointer array</param>
  /// <returns>
  ///   The offset, in bytes, from a pointer to a gradient matrix to the first
  ///   row pointer for the matrix elements.
  /// </returns>
  constexpr std::size_t getMatrixElementsOffset(std::size_t rowArrayEndOffset) {
    std::size_t extraByteCount = (
      rowArrayEndOffset %
      alignof(Nuclex::FrameFixer::Algorithm::Gradient)
    );
    return (
      rowArrayEndOffset + (
        (extraByteCount == 0) ?
        (0) :
        (rowArrayEndOffset - extraByteCount)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Custom alloctor that allocates a shared gradient matrix</summary>
  /// <typeparam name="TGradientMatrix">
  ///   Type that will be allocated, expected to derive from ResourceManifest
  /// </typeparam>
  /// <remarks>
  ///   Normally, a non-templated implementation of this allocator would seem to suffice,
  ///   but <code>std::allocate_shared()</code> implementations will very likely
  ///   (via the type-changing copy constructor) allocate a type inherited from our
  ///   <see cref="GradientMatrix" /> that packages the reference counter of
  ///   the <code>std::shared_ptr</code> together with the instance.
  /// </remarks>
  template<class TGradientMatrix>
  class GradientMatrixAllocator {

    /// <summary>Type of element the allocator is for, required by standard</summary>
    public: typedef TGradientMatrix value_type;

    /// <summary>Initializes a new allocator using the specified appended list size</summary>
    /// <param name="width">Width of the gradient matrix (number of columns)</param>
    /// <param name="height">Height of the gradient matrix (number of rows)</param>
    public: GradientMatrixAllocator(std::size_t width, std::size_t height) :
      width(width),
      height(height) {}

    /// <summary>
    ///   Creates this allocator as a clone of an allocator for a different type
    /// </summary>
    /// <typeparam name="TOther">Type the existing allocator is allocating for</typeparam>
    /// <param name="other">Existing allocator whose attributes will be copied</param>
    public: template<class TOther> GradientMatrixAllocator(
      const GradientMatrixAllocator<TOther> &other
    ) :
      width(other.width),
      height(other.height) {}

    /// <summary>Allocates memory for the specified number of elements (must be 1)</summary>
    /// <param name="count">Number of elements to allocate memory for (must be 1)</param>
    /// <returns>The allocated (but not initialized) memory for the requested type</returns>
    public: TGradientMatrix *allocate(std::size_t count) {
      using Nuclex::FrameFixer::Algorithm::Gradient;

      NUCLEX_FRAMEFIXER_NDEBUG_UNUSED(count);
      assert(count == 1);

      std::size_t totalByteCount = getRowArrayOffset<TGradientMatrix>();
      totalByteCount += sizeof(Gradient **[2]) * height / 2;
      totalByteCount = getMatrixElementsOffset(totalByteCount);
      totalByteCount += sizeof(Gradient[2]) * width * height / 2;

      return reinterpret_cast<TGradientMatrix *>(
        new(std::align_val_t(alignof(TGradientMatrix))) std::uint8_t[totalByteCount]
      );
    }

    /// <summary>Frees memory for the specified element (count must be 1)</summary>
    /// <param name="instance">Instance for which memory will be freed</param>
    /// <param name="count">Number of instances that will be freed (must be 1)</param>
    public: void deallocate(TGradientMatrix *instance, std::size_t count) {
      NUCLEX_FRAMEFIXER_NDEBUG_UNUSED(count);
      assert(count == 1);

      delete[] reinterpret_cast<std::uint8_t *>(instance);
    }

    /// <summary>Width of the matrix</summary>
    public: std::size_t width;
    /// <summary>Height of the matrix</summary>
    public: std::size_t height;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Variant of the gradient matrix with a default constructor to be constructible
  ///   via std::allocate_shared()
  /// </summary>
  class DefaultConstructibleGradientMatrix :
    public Nuclex::FrameFixer::Algorithm::GradientMatrix {
    /// <summary>Leaves the gradient with uninitialized attributes</summary>
    public: DefaultConstructibleGradientMatrix() = default;
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  GradientMatrix::GradientMatrix(std::size_t width, std::size_t height) :
    M(nullptr),
    width(width),
    height(height),
    memory(nullptr) {

    std::size_t matrixElementsOffset = getMatrixElementsOffset(
      sizeof(Gradient **[2]) * height / 2
    );
    std::size_t totalByteCount = (
      matrixElementsOffset + (sizeof(Gradient[2]) * width * height / 2)
    );

    std::unique_ptr<std::uint8_t[]> buffer(
      new(std::align_val_t(alignof(Gradient *))) std::uint8_t[totalByteCount]
    );
    initializePointersWithBuffer(buffer.get(), matrixElementsOffset);
    this->memory = buffer.release();
  }

  // ------------------------------------------------------------------------------------------- //

  GradientMatrix::~GradientMatrix() {
    if(this->memory != nullptr) {
      delete[] this->memory;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<GradientMatrix> GradientMatrix::Create(
    std::size_t width, std::size_t height
  ) {
    std::shared_ptr<GradientMatrix> gradientMatrix = (
      std::allocate_shared<DefaultConstructibleGradientMatrix>(
        GradientMatrixAllocator<DefaultConstructibleGradientMatrix>(width, height)
      )
    );

    std::uint8_t *rowArrayStart = (
      reinterpret_cast<std::uint8_t *>(gradientMatrix.get()) +
      getRowArrayOffset<GradientMatrix>()
    );
    std::size_t matrixElementsOffset = getMatrixElementsOffset(
      rowArrayStart + (sizeof(Gradient **[2]) * height / 2) -
      reinterpret_cast<std::uint8_t *>(gradientMatrix.get())
    );     

    gradientMatrix->width = width;
    gradientMatrix->height = height;
    gradientMatrix->memory = nullptr;
    gradientMatrix->initializePointersWithBuffer(rowArrayStart, matrixElementsOffset);

    return gradientMatrix;
  }

  // ------------------------------------------------------------------------------------------- //

  void GradientMatrix::FillAll(float value) {
    for(std::size_t y = 0; y < this->height; ++y) {
      Gradient *row = this->M[y];
      for(std::size_t x = 0; x < this->width; ++x) {
        row[x].Horizontal = value;
        row[x].Vertical = value;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void GradientMatrix::DivideAllBy(float value) {
    for(std::size_t y = 0; y < this->height; ++y) {
      Gradient *row = this->M[y];
      for(std::size_t x = 0; x < this->width; ++x) {
        row[x].Horizontal /= value;
        row[x].Vertical /= value;
      }
    }
  }


  // ------------------------------------------------------------------------------------------- //

  void GradientMatrix::Multiply(const GradientMatrix &other) {
    bool dimensionsMath = (
      (this->width == other.width) &&
      (this->height == other.height)
    );
    if(!dimensionsMath) {
      throw std::invalid_argument(u8"Gradient matrices must have the same size");
    }

    
  }

  // ------------------------------------------------------------------------------------------- //

  void GradientMatrix::initializePointersWithBuffer(
    std::uint8_t *buffer, std::size_t matrixElementsOffset
  ) {
    std::size_t misalignment = reinterpret_cast<std::uintptr_t>(buffer) % alignof(Gradient *);
    if(misalignment > 0) {
      buffer += (alignof(Gradient *) - misalignment);
    }
    this->M = reinterpret_cast<Gradient **>(buffer);

    std::uint8_t *row = buffer + matrixElementsOffset;
    for(std::size_t index = 0; index < this->height; ++index) {
      this->M[index] = reinterpret_cast<Gradient *>(row);
      row += sizeof(Gradient[2]) * this->width / 2; 
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm
