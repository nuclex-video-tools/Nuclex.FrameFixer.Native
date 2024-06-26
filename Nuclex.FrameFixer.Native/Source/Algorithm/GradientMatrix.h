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

#ifndef NUCLEX_FRAMEFIXER_ALGORITHM_GRADIENTMATRIX_H
#define NUCLEX_FRAMEFIXER_ALGORITHM_GRADIENTMATRIX_H

#include "Nuclex/FrameFixer/Config.h"

#include "./Gradient.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for std::uint8_t
#include <memory> // for std::shared_ptr

namespace Nuclex::FrameFixer::Algorithm {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Two dimensional matrix of gradients</summary>
  class GradientMatrix {

    /// <summary>Initializes a new gradient matrix</summary>
    /// <param name="width">Width of the gradient matrix (number of columns)</param>
    /// <param name="height">Height of the gradient matrix (number of rows)</param>
    public: GradientMatrix(std::size_t width, std::size_t height);
    /// <summary>Frees all resources used by the gradient matrix</summary>
    public: ~GradientMatrix();

    /// <summary>Allocates a new gradient matrix</summary>
    /// <param name="width">Width of the gradient matrix (number of columns)</param>
    /// <param name="height">Height of the gradient matrix (number of rows)</param>
    /// <returns>The new gradient matrix</returns>
    /// <remarks>
    ///   This is slightly more efficient if you need to store the gradient matrix in
    ///   a shared_ptr anyway because it can allocate all of the memory as a single block.
    /// </remarks>
    public: static std::shared_ptr<GradientMatrix> Create(
      std::size_t width, std::size_t height
    );

    /// <summary>Width of the gradient matrix (number of columns)</summary>
    public: std::size_t GetWidth() const { return this->width; }

    /// <summary>Height of the gradient matrix (number of rows)</summary>
    public: std::size_t GetHeight() const { return this->height; }

    /// <summary>
    ///   Fills all dimensions of all gradient in the matrix with the specified value
    /// </summary>
    /// <param name="value">Value to which all of the gradient will be set</param>
    public: void FillAll(float value);

    /// <summary>
    ///   Divides all dimensions of all gradients in the matrix by the speified value
    /// </summary>
    /// <param name="value">Value by which all gradietns will be divided</param>
    public: void DivideAllBy(float value);

    /// <summary>Multiplies the matrix with another</summary>
    /// <param name="multiplicand">Other matrix this one will be multiplied with</param>
    public: void Multiply(const GradientMatrix &multiplicand);

    /// <summary>Needed for std::allocate_shared, don't use</summary>
    protected: explicit GradientMatrix() = default;

    /// <summary>Sets up the <see cref="M" /> array with the row start pointers</summary>
    /// <param name="buffer">
    ///   Memory address at which the <see cref="M" /> array should start
    /// </param>
    /// <param name="matrixElementsOffset">
    ///   Memory offset, in bytes, from the <see cref="M" /> array to the first matrix element
    /// </param>
    private: void initializePointersWithBuffer(std::uint8_t *buffer, std::size_t matrixElementsOffset);

    /// <summary>Provides access to the elements of the gradient matrix</summary>
    /// <remarks>
    ///   The outer dimension are the rows, each row can be indexed by column
    ///   to access an element in the matrix.
    /// </remarks>
    public: Gradient **M;

    /// <summary>Width of the matrix</summary>
    private: std::size_t width;
    /// <summary>Height of the matrix</summary>
    private: std::size_t height;
    /// <summary>Memory allocated to store the matrix' elements, can be nullptr</summary>
    private: std::uint8_t *memory;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Algorithm

#endif // NUCLEX_FRAMEFIXER_ALGORITHM_GRADIENTMATRIX_H
