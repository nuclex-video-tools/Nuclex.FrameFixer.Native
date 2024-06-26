#pragma region Apache License 2.0
/*
Nuclex Frame Fixer
Copyright (C) 2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "./RgbGradientMatrix.h"

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
  template<typename TRgbGradientMatrix>
  constexpr std::size_t getRowArrayOffset() {
    constexpr std::size_t extraByteCount = (
      sizeof(TRgbGradientMatrix) %
      alignof(Nuclex::FrameFixer::Algorithm::RgbGradient *)
    );
    return (
      sizeof(TRgbGradientMatrix) + (
        (extraByteCount == 0) ?
        (0) :
        (sizeof(TRgbGradientMatrix) - extraByteCount)
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
      alignof(Nuclex::FrameFixer::Algorithm::RgbGradient)
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
  /// <typeparam name="TRgbGradientMatrix">
  ///   Type that will be allocated, expected to derive from ResourceManifest
  /// </typeparam>
  /// <remarks>
  ///   Normally, a non-templated implementation of this allocator would seem to suffice,
  ///   but <code>std::allocate_shared()</code> implementations will very likely
  ///   (via the type-changing copy constructor) allocate a type inherited from our
  ///   <see cref="RgbGradientMatrix" /> that packages the reference counter of
  ///   the <code>std::shared_ptr</code> together with the instance.
  /// </remarks>
  template<class TRgbGradientMatrix>
  class RgbGradientMatrixAllocator {

    /// <summary>Type of element the allocator is for, required by standard</summary>
    public: typedef TRgbGradientMatrix value_type;

    /// <summary>Initializes a new allocator using the specified appended list size</summary>
    /// <param name="width">Width of the gradient matrix (number of columns)</param>
    /// <param name="height">Height of the gradient matrix (number of rows)</param>
    public: RgbGradientMatrixAllocator(std::size_t width, std::size_t height) :
      width(width),
      height(height) {}

    /// <summary>
    ///   Creates this allocator as a clone of an allocator for a different type
    /// </summary>
    /// <typeparam name="TOther">Type the existing allocator is allocating for</typeparam>
    /// <param name="other">Existing allocator whose attributes will be copied</param>
    public: template<class TOther> RgbGradientMatrixAllocator(
      const RgbGradientMatrixAllocator<TOther> &other
    ) :
      width(other.width),
      height(other.height) {}

    /// <summary>Allocates memory for the specified number of elements (must be 1)</summary>
    /// <param name="count">Number of elements to allocate memory for (must be 1)</param>
    /// <returns>The allocated (but not initialized) memory for the requested type</returns>
    public: TRgbGradientMatrix *allocate(std::size_t count) {
      using Nuclex::FrameFixer::Algorithm::RgbGradient;

      NUCLEX_FRAMEFIXER_NDEBUG_UNUSED(count);
      assert(count == 1);

      std::size_t totalByteCount = getRowArrayOffset<TRgbGradientMatrix>();
      totalByteCount += sizeof(RgbGradient **[2]) * height / 2;
      totalByteCount = getMatrixElementsOffset(totalByteCount);
      totalByteCount += sizeof(RgbGradient[2]) * width * height / 2;

      return reinterpret_cast<TRgbGradientMatrix *>(
        new(std::align_val_t(alignof(TRgbGradientMatrix))) std::uint8_t[totalByteCount]
      );
    }

    /// <summary>Frees memory for the specified element (count must be 1)</summary>
    /// <param name="instance">Instance for which memory will be freed</param>
    /// <param name="count">Number of instances that will be freed (must be 1)</param>
    public: void deallocate(TRgbGradientMatrix *instance, std::size_t count) {
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
  class DefaultConstructibleRgbGradientMatrix :
    public Nuclex::FrameFixer::Algorithm::RgbGradientMatrix {
    /// <summary>Leaves the gradient with uninitialized attributes</summary>
    public: DefaultConstructibleRgbGradientMatrix() = default;
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  RgbGradientMatrix::RgbGradientMatrix(std::size_t width, std::size_t height) :
    M(nullptr),
    width(width),
    height(height),
    memory(nullptr) {

    std::size_t matrixElementsOffset = getMatrixElementsOffset(
      sizeof(RgbGradient **[2]) * height / 2
    );
    std::size_t totalByteCount = (
      matrixElementsOffset + (sizeof(RgbGradient[2]) * width * height / 2)
    );

    std::unique_ptr<std::uint8_t[]> buffer(
      new(std::align_val_t(alignof(RgbGradient *))) std::uint8_t[totalByteCount]
    );
    initializePointersWithBuffer(buffer.get(), matrixElementsOffset);
    this->memory = buffer.release();
  }

  // ------------------------------------------------------------------------------------------- //

  RgbGradientMatrix::~RgbGradientMatrix() {
    if(this->memory != nullptr) {
      delete[] this->memory;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<RgbGradientMatrix> RgbGradientMatrix::Create(
    std::size_t width, std::size_t height
  ) {
    std::shared_ptr<RgbGradientMatrix> gradientMatrix = (
      std::allocate_shared<DefaultConstructibleRgbGradientMatrix>(
        RgbGradientMatrixAllocator<DefaultConstructibleRgbGradientMatrix>(width, height)
      )
    );

    std::uint8_t *rowArrayStart = (
      reinterpret_cast<std::uint8_t *>(gradientMatrix.get()) +
      getRowArrayOffset<RgbGradientMatrix>()
    );
    std::size_t matrixElementsOffset = getMatrixElementsOffset(
      rowArrayStart + (sizeof(RgbGradient **[2]) * height / 2) -
      reinterpret_cast<std::uint8_t *>(gradientMatrix.get())
    );     

    gradientMatrix->width = width;
    gradientMatrix->height = height;
    gradientMatrix->memory = nullptr;
    gradientMatrix->initializePointersWithBuffer(rowArrayStart, matrixElementsOffset);

    return gradientMatrix;
  }

  // ------------------------------------------------------------------------------------------- //

  void RgbGradientMatrix::FillAll(float value) {
    for(std::size_t y = 0; y < this->height; ++y) {
      RgbGradient *row = this->M[y];
      for(std::size_t x = 0; x < this->width; ++x) {
        row[x].RedHorizontal = value;
        row[x].RedVertical = value;
        row[x].GreenHorizontal = value;
        row[x].GreenVertical = value;
        row[x].BlueHorizontal = value;
        row[x].BlueVertical = value;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void RgbGradientMatrix::DivideAllBy(float value) {
    for(std::size_t y = 0; y < this->height; ++y) {
      RgbGradient *row = this->M[y];
      for(std::size_t x = 0; x < this->width; ++x) {
        row[x].RedHorizontal /= value;
        row[x].RedVertical /= value;
        row[x].GreenHorizontal /= value;
        row[x].GreenVertical /= value;
        row[x].BlueHorizontal /= value;
        row[x].BlueVertical /= value;
      }
    }
  }


  // ------------------------------------------------------------------------------------------- //

  void RgbGradientMatrix::Multiply(const RgbGradientMatrix &other) {
    bool dimensionsMath = (
      (this->width == other.width) &&
      (this->height == other.height)
    );
    if(!dimensionsMath) {
      throw std::invalid_argument(u8"Gradient matrices must have the same size");
    }

    
  }

  // ------------------------------------------------------------------------------------------- //

  void RgbGradientMatrix::initializePointersWithBuffer(
    std::uint8_t *buffer, std::size_t matrixElementsOffset
  ) {
    std::size_t misalignment = reinterpret_cast<std::uintptr_t>(buffer) % alignof(RgbGradient *);
    if(misalignment > 0) {
      buffer += (alignof(RgbGradient *) - misalignment);
    }
    this->M = reinterpret_cast<RgbGradient **>(buffer);

    std::uint8_t *row = buffer + matrixElementsOffset;
    for(std::size_t index = 0; index < this->height; ++index) {
      this->M[index] = reinterpret_cast<RgbGradient *>(row);
      row += sizeof(RgbGradient[2]) * this->width / 2; 
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm
