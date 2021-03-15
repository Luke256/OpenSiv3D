﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2021 Ryo Suzuki
//	Copyright (c) 2016-2021 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include "Common.hpp"

namespace s3d
{
	struct WaveSampleS16;
	struct WaveSample;

	struct WaveSampleS16
	{
		int16 left;

		int16 right;

		WaveSampleS16() = default;

		explicit constexpr WaveSampleS16(int16 mono) noexcept;

		constexpr WaveSampleS16(int16 _left, int16 _right) noexcept;

		constexpr WaveSampleS16& set(int16 mono) noexcept;

		constexpr WaveSampleS16& set(int16 _left, int16 _right) noexcept;

		constexpr WaveSampleS16& set(WaveSampleS16 sample) noexcept;

		constexpr void swapChannel() noexcept;

		[[nodiscard]]
		constexpr WaveSample asWaveSample() const noexcept;

		[[nodiscard]]
		static constexpr WaveSampleS16 FromFloat32(float mono);

		[[nodiscard]]
		static constexpr WaveSampleS16 FromFloat32(float _left, float _right);

		[[nodiscard]]
		static constexpr WaveSampleS16 Zero();

		[[nodiscard]]
		static constexpr WaveSampleS16 Min();

		[[nodiscard]]
		static constexpr WaveSampleS16 Max();
	};

	struct WaveSample
	{
		float left;

		float right;

		WaveSample() = default;

		explicit constexpr WaveSample(float mono) noexcept;

		constexpr WaveSample(float _left, float _right) noexcept;

		explicit constexpr WaveSample(WaveSampleS16 sample) noexcept;

		constexpr WaveSample& operator =(const WaveSample& sample) = default;

		constexpr WaveSample& operator =(float mono) noexcept;

		constexpr WaveSample& operator =(WaveSampleS16 sample) noexcept;

		constexpr WaveSample& set(float mono) noexcept;

		constexpr WaveSample& set(float _left, float _right) noexcept;

		constexpr WaveSample& set(WaveSample sample) noexcept;

		constexpr WaveSample& set(WaveSampleS16 sample) noexcept;

		constexpr void swapChannel() noexcept;

		[[nodiscard]]
		constexpr WaveSampleS16 asWaveSampleS16() const noexcept;
		
		[[nodiscard]]
		static constexpr WaveSample FromInt16(int16 mono);

		[[nodiscard]]
		static constexpr WaveSample FromInt16(int16 _left, int16 _right);

		[[nodiscard]]
		static constexpr WaveSample Zero();

		[[nodiscard]]
		static constexpr WaveSample Min();

		[[nodiscard]]
		static constexpr WaveSample Max();
	};
}

# include "detail/WaveSample.ipp"