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

# include <Siv3D/TexturedCircle.hpp>
# include <Siv3D/Renderer2D/IRenderer2D.hpp>
# include <Siv3D/Common/Siv3DEngine.hpp>

namespace s3d
{
	TexturedCircle::TexturedCircle(const Texture& _texture, const float l, const float t, const float r, const float b, const Circle& _circle)
		: circle{ _circle }
		, texture{ _texture }
		, uvRect{ l, t, r, b } {}

	TexturedCircle::TexturedCircle(const Texture& _texture, const FloatRect& _uvRect, const Circle& _circle)
		: circle{ _circle }
		, texture{ _texture }
		, uvRect{ _uvRect } {}
}
