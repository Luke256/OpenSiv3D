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

# include "D3D11Texture.hpp"
# include <Siv3D/Image.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/2DShapes.hpp>
# include <Siv3D/Texture/TextureCommon.hpp>

namespace s3d
{
	D3D11Texture::D3D11Texture(ID3D11Device* device, const Image& image, const TextureDesc desc)
		: m_desc{ image.size(),
			(detail::IsSRGB(desc) ? TextureFormat::R8G8B8A8_Unorm_SRGB : TextureFormat::R8G8B8A8_Unorm),
			desc,
			1,
			1, 0,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_SHADER_RESOURCE,
			0, 0 }
	{
		// [メインテクスチャ] を作成
		const D3D11_SUBRESOURCE_DATA initData{ image.data(), image.stride(), 0 };
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, &initData, &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Default;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(ID3D11Device* const device, const Image& image, const Array<Image>& mips, const TextureDesc desc)
		: m_desc{ image.size(),
			(detail::IsSRGB(desc) ? TextureFormat::R8G8B8A8_Unorm_SRGB : TextureFormat::R8G8B8A8_Unorm),
			desc,
			static_cast<uint32>(mips.size() + 1),
			1, 0,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_SHADER_RESOURCE,
			0, 0 }
	{
		Array<D3D11_SUBRESOURCE_DATA> initData(m_desc.mipLevels);
		{
			initData[0] = { image.data(), image.stride(), 0 };
			for (uint32 i = 0; i < mips.size(); ++i)
			{
				initData[i + 1] = { mips[i].data(), mips[i].stride(), 0 };
			}
		}

		// [メインテクスチャ] を作成
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, initData.data(), &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Default;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(Dynamic, ID3D11Device* const device, const Size& size, const void* pData, const uint32 stride, const TextureFormat& format, const TextureDesc desc)
		: m_desc{ size,
			format,
			desc,
			1,
			1, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0, 0 }
	{
		// [メインテクスチャ] を作成
		const D3D11_SUBRESOURCE_DATA initData{ pData, stride, 0 };
		{
			D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, (pData ? &initData : nullptr), &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D (D3D11_USAGE_DEFAULT). Error code: {:#X}"_fmt(hr));
				return;
			}

			// [ステージング・テクスチャ] を作成
			{
				d3d11Desc.Usage				= D3D11_USAGE_STAGING;
				d3d11Desc.BindFlags			= 0;
				d3d11Desc.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;

				if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, (pData ? &initData : nullptr), &m_stagingTexture);
					FAILED(hr))
				{
					LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D (D3D11_USAGE_STAGING). Error code: {:#X}"_fmt(hr));
					return;
				}
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Dynamic;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(Render, ID3D11Device* device, const Size& size, const TextureFormat& format, const TextureDesc desc)
		: m_desc{ size,
			format,
			desc,
			1,
			1, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			0, 0 }
	{
		// [メインテクスチャ] を作成
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, nullptr, &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [レンダー・ターゲット・ビュー] を作成
		{
			const D3D11_RENDER_TARGET_VIEW_DESC rtDesc = m_desc.makeD3D11_RENDER_TARGET_VIEW_DESC();
			if (HRESULT hr = device->CreateRenderTargetView(m_texture.Get(), &rtDesc, &m_renderTargetView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create RenderTargetView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Render;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(Render, ID3D11Device* device, const Image& image, const TextureFormat& format, const TextureDesc desc)
		: m_desc{ image.size(),
			format,
			desc,
			1,
			1, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			0, 0 }
	{
		// [メインテクスチャ] を作成
		const D3D11_SUBRESOURCE_DATA initData{ image.data(), image.stride(), 0 };
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, &initData, &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [レンダー・ターゲット・ビュー] を作成
		{
			const D3D11_RENDER_TARGET_VIEW_DESC rtDesc = m_desc.makeD3D11_RENDER_TARGET_VIEW_DESC();
			if (HRESULT hr = device->CreateRenderTargetView(m_texture.Get(), &rtDesc, &m_renderTargetView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create RenderTargetView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Render;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(Render, ID3D11Device* device, const Grid<float>& image, const TextureFormat& format, const TextureDesc desc)
		: m_desc{ image.size(),
			format,
			desc,
			1,
			1, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			0, 0 }
	{
		// [メインテクスチャ] を作成
		const D3D11_SUBRESOURCE_DATA initData{ image.data(), static_cast<UINT>(image.width() * sizeof(float)), 0 };
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, &initData, &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [レンダー・ターゲット・ビュー] を作成
		{
			const D3D11_RENDER_TARGET_VIEW_DESC rtDesc = m_desc.makeD3D11_RENDER_TARGET_VIEW_DESC();
			if (HRESULT hr = device->CreateRenderTargetView(m_texture.Get(), &rtDesc, &m_renderTargetView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create RenderTargetView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Render;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(Render, ID3D11Device* device, const Grid<Float2>& image, const TextureFormat& format, const TextureDesc desc)
		: m_desc{ image.size(),
			format,
			desc,
			1,
			1, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			0, 0 }
	{
		// [メインテクスチャ] を作成
		const D3D11_SUBRESOURCE_DATA initData{ image.data(), static_cast<UINT>(image.width() * sizeof(Float2)), 0 };
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, &initData, &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [レンダー・ターゲット・ビュー] を作成
		{
			const D3D11_RENDER_TARGET_VIEW_DESC rtDesc = m_desc.makeD3D11_RENDER_TARGET_VIEW_DESC();
			if (HRESULT hr = device->CreateRenderTargetView(m_texture.Get(), &rtDesc, &m_renderTargetView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create RenderTargetView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Render;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(Render, ID3D11Device* device, const Grid<Float4>& image, const TextureFormat& format, const TextureDesc desc)
		: m_desc{ image.size(),
			format,
			desc,
			1,
			1, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			0, 0 }
	{
		// [メインテクスチャ] を作成
		const D3D11_SUBRESOURCE_DATA initData{ image.data(), static_cast<UINT>(image.width() * sizeof(Float4)), 0 };
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, &initData, &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [レンダー・ターゲット・ビュー] を作成
		{
			const D3D11_RENDER_TARGET_VIEW_DESC rtDesc = m_desc.makeD3D11_RENDER_TARGET_VIEW_DESC();
			if (HRESULT hr = device->CreateRenderTargetView(m_texture.Get(), &rtDesc, &m_renderTargetView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create RenderTargetView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = m_desc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::Render;
		m_initialized = true;
	}

	D3D11Texture::D3D11Texture(MSRender, ID3D11Device* device, const Size& size, const TextureFormat& format, const TextureDesc desc)
		: m_desc{ size,
			format,
			desc,
			1,
			4, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_RENDER_TARGET,
			0, 0 }
	{
		// [マルチ・サンプルテクスチャ] を作成
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = m_desc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, nullptr, &m_multiSampledTexture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [レンダー・ターゲット・ビュー] を作成
		{
			const D3D11_RENDER_TARGET_VIEW_DESC rtDesc = m_desc.makeD3D11_RENDER_TARGET_VIEW_DESC();
			if (HRESULT hr = device->CreateRenderTargetView(m_multiSampledTexture.Get(), &rtDesc, &m_renderTargetView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create RenderTargetView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		const D3D11Texture2DDesc resolvedDesc{ size,
			format,
			desc,
			1,
			1, 0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0, 0 };

		// [メイン・テクスチャ] を作成
		{
			const D3D11_TEXTURE2D_DESC d3d11Desc = resolvedDesc.makeTEXTURE2D_DESC();
			if (HRESULT hr = device->CreateTexture2D(&d3d11Desc, nullptr, &m_texture);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create Texture2D. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		// [シェーダ・リソース・ビュー] を作成
		{
			const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = resolvedDesc.makeSHADER_RESOURCE_VIEW_DESC();
			if (HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_shaderResourceView);
				FAILED(hr))
			{
				LOG_FAIL(U"❌ D3D11Texture::D3D11Texture() : Failed to create ShaderResourceView. Error code: {:#X}"_fmt(hr));
				return;
			}
		}

		m_type = TextureType::MSRender;
		m_initialized = true;
	}


	bool D3D11Texture::isInitialized() const noexcept
	{
		return m_initialized;
	}

	const D3D11Texture2DDesc& D3D11Texture::getDesc() const noexcept
	{
		return m_desc;
	}

	ID3D11ShaderResourceView** D3D11Texture::getSRVPtr()
	{
		return m_shaderResourceView.GetAddressOf();
	}

	ID3D11RenderTargetView* D3D11Texture::getRTV()
	{
		return m_renderTargetView.Get();
	}

	bool D3D11Texture::fill(ID3D11DeviceContext* context, const ColorF& color, const bool wait)
	{
		if (m_type != TextureType::Dynamic)
		{
			return false;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			const UINT flag = (wait ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT);

			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_WRITE, flag, &mapped)))
			{
				return false;
			}

			if ((mapped.pData == nullptr)
				|| (mapped.RowPitch < m_desc.stride()))
			{
				context->Unmap(m_stagingTexture.Get(), 0);
				return false;
			}

			FillByColor(mapped.pData, m_desc.size, mapped.RowPitch, color, m_desc.format);

			context->Unmap(m_stagingTexture.Get(), 0);
		}

		context->CopyResource(m_texture.Get(), m_stagingTexture.Get());

		return true;
	}

	bool D3D11Texture::fillRegion(ID3D11DeviceContext* context, const ColorF& color, const Rect& rect)
	{
		if (m_type != TextureType::Dynamic)
		{
			return false;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_WRITE, 0, &mapped)))
			{
				return false;
			}

			if ((mapped.pData == nullptr)
				|| (mapped.RowPitch < m_desc.stride()))
			{
				context->Unmap(m_stagingTexture.Get(), 0);
				return false;
			}

			FillRegionByColor(mapped.pData, m_desc.size, mapped.RowPitch, rect, color, m_desc.format);

			context->Unmap(m_stagingTexture.Get(), 0);
		}

		context->CopyResource(m_texture.Get(), m_stagingTexture.Get());

		return true;
	}

	bool D3D11Texture::fill(ID3D11DeviceContext* context, const void* src, const uint32 stride, const bool wait)
	{
		if (m_type != TextureType::Dynamic)
		{
			return false;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			const UINT flag = (wait ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT);

			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_WRITE, flag, &mapped)))
			{
				return false;
			}

			if ((mapped.pData == nullptr)
				|| (mapped.RowPitch < m_desc.stride()))
			{
				context->Unmap(m_stagingTexture.Get(), 0);
				return false;
			}

			FillByImage(mapped.pData, m_desc.size, mapped.RowPitch, src, stride, m_desc.format);

			context->Unmap(m_stagingTexture.Get(), 0);
		}

		context->CopyResource(m_texture.Get(), m_stagingTexture.Get());

		return true;
	}

	bool D3D11Texture::fillRegion(ID3D11DeviceContext* context, const void* src, uint32 stride, const Rect& rect, const bool wait)
	{
		if (m_type != TextureType::Dynamic)
		{
			return false;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_WRITE, wait ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped)))
			{
				return false;
			}

			if ((mapped.pData == nullptr)
				|| (mapped.RowPitch < m_desc.stride()))
			{
				context->Unmap(m_stagingTexture.Get(), 0);
				return false;
			}

			FillRegionByImage(mapped.pData, m_desc.size, mapped.RowPitch, src, stride, rect, m_desc.format);

			context->Unmap(m_stagingTexture.Get(), 0);
		}

		const int32 leftX	= Clamp(rect.x, 0, m_desc.size.x);
		const int32 rightX	= Clamp(rect.x + rect.w, 0, m_desc.size.x);
		const int32 topY	= Clamp(rect.y, 0, m_desc.size.y);
		const int32 bottomY	= Clamp(rect.y + rect.h, 0, m_desc.size.y);
		const int32 width	= (rightX - leftX);
		const int32 height	= (bottomY - topY);

		if ((width == 0) || (height == 0))
		{
			return true;
		}

		D3D11_BOX box = CD3D11_BOX(leftX, topY, 0, rightX, bottomY, 1);
		context->CopySubresourceRegion(m_texture.Get(), 0, leftX, topY, 0, m_stagingTexture.Get(), 0, &box);

		return true;
	}

	void D3D11Texture::clearRT(ID3D11DeviceContext* context, const ColorF& color)
	{
		if ((m_type != TextureType::Render)
			&& (m_type != TextureType::MSRender))
		{
			return;
		}

		const Float4 clearColor = (m_desc.isSRGB() ? color.gamma(1.0 / 2.2).toFloat4() : color.toFloat4());

		context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor.getPointer());
	}

	void D3D11Texture::readRT(ID3D11Device* device, ID3D11DeviceContext* context, Image& image)
	{
		if ((m_type != TextureType::Render)
			&& (m_type != TextureType::MSRender))
		{
			return;
		}

		if ((m_desc.format != TextureFormat::R8G8B8A8_Unorm)
			&& (m_desc.format != TextureFormat::R8G8B8A8_Unorm_SRGB)) // RGBA8 形式以外なら失敗
		{
			LOG_FAIL(U"D3D11Texture::readRT(): Image is not supported in this format");
			return;
		}

		// テクスチャのサイズ
		const Size size = m_desc.size;

		// [ステージング・テクスチャ] が未作成の場合、作成
		if (not m_stagingTexture)
		{
			const D3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC(
				static_cast<DXGI_FORMAT>(m_desc.format.DXGIFormat()),
				size.x, size.y,
				1, 1,
				0,
				D3D11_USAGE_STAGING,
				D3D11_CPU_ACCESS_READ,
				1, 0,
				0);

			if (FAILED(device->CreateTexture2D(&desc, nullptr, &m_stagingTexture)))
			{
				return;
			}
		}

		// [メインテクスチャ] から [ステージング・テクスチャ] にコピー
		context->CopyResource(m_stagingTexture.Get(), m_texture.Get());

		// Image をテクスチャのサイズにリサイズ
		image.resize(size);

		// [ステージング・テクスチャ] から Image にコピー
		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
			{
				return;
			}

			if (not mapped.pData)
			{
				return;
			}

			const size_t srcStride = mapped.RowPitch;
			const size_t dstStride = image.stride();

			if (srcStride == dstStride)
			{
				std::memcpy(image.data(), mapped.pData, image.size_bytes());
			}
			else
			{
				const size_t width = image.width();
				const size_t height = image.height();

				const uint8* pSrc = static_cast<const uint8*>(mapped.pData);
				Color* pDst = image.data();

				for (size_t y = 0; y < height; ++y)
				{
					std::memcpy(pDst, pSrc, dstStride);
					pDst += width;
					pSrc += srcStride;
				}
			}

			context->Unmap(m_stagingTexture.Get(), 0);
		}
	}

	void D3D11Texture::readRT(ID3D11Device* device, ID3D11DeviceContext* context, Grid<float>& image)
	{
		if ((m_type != TextureType::Render)
			&& (m_type != TextureType::MSRender))
		{
			return;
		}

		if (m_desc.format != TextureFormat::R32_Float) // R32F 形式以外なら失敗
		{
			LOG_FAIL(U"D3D11Texture::readRT(): Grid<float> is not supported in this format");
			return;
		}

		// テクスチャのサイズ
		const Size size = m_desc.size;

		// [ステージング・テクスチャ] が未作成の場合、作成
		if (not m_stagingTexture)
		{
			const D3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC(
				static_cast<DXGI_FORMAT>(m_desc.format.DXGIFormat()),
				size.x, size.y,
				1, 1,
				0,
				D3D11_USAGE_STAGING,
				D3D11_CPU_ACCESS_READ,
				1, 0,
				0);

			if (FAILED(device->CreateTexture2D(&desc, nullptr, &m_stagingTexture)))
			{
				return;
			}
		}

		// [メインテクスチャ] から [ステージング・テクスチャ] にコピー
		context->CopyResource(m_stagingTexture.Get(), m_texture.Get());

		// Image をテクスチャのサイズにリサイズ
		image.resize(size);

		// [ステージング・テクスチャ] から Image にコピー
		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
			{
				return;
			}

			if (!mapped.pData)
			{
				return;
			}

			const size_t srcStride = mapped.RowPitch;
			const size_t dstStride = image.width() * sizeof(float);

			if (srcStride == dstStride)
			{
				std::memcpy(image.data(), mapped.pData, image.size_bytes());
			}
			else
			{
				const size_t width = image.width();
				const size_t height = image.height();

				const uint8* pSrc = static_cast<const uint8*>(mapped.pData);
				float* pDst = image.data();

				for (size_t y = 0; y < height; ++y)
				{
					std::memcpy(pDst, pSrc, dstStride);
					pDst += width;
					pSrc += srcStride;
				}
			}

			context->Unmap(m_stagingTexture.Get(), 0);
		}
	}

	void D3D11Texture::readRT(ID3D11Device* device, ID3D11DeviceContext* context, Grid<Float2>& image)
	{
		if ((m_type != TextureType::Render)
			&& (m_type != TextureType::MSRender))
		{
			return;
		}

		if (m_desc.format != TextureFormat::R32G32_Float) // RG32F 形式以外なら失敗
		{
			LOG_FAIL(U"D3D11Texture::readRT(): Grid<Float2> is not supported in this format");
			return;
		}

		// テクスチャのサイズ
		const Size size = m_desc.size;

		// [ステージング・テクスチャ] が未作成の場合、作成
		if (not m_stagingTexture)
		{
			const D3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC(
				static_cast<DXGI_FORMAT>(m_desc.format.DXGIFormat()),
				size.x, size.y,
				1, 1,
				0,
				D3D11_USAGE_STAGING,
				D3D11_CPU_ACCESS_READ,
				1, 0,
				0);

			if (FAILED(device->CreateTexture2D(&desc, nullptr, &m_stagingTexture)))
			{
				return;
			}
		}

		// [メインテクスチャ] から [ステージング・テクスチャ] にコピー
		context->CopyResource(m_stagingTexture.Get(), m_texture.Get());

		// Image をテクスチャのサイズにリサイズ
		image.resize(size);

		// [ステージング・テクスチャ] から Image にコピー
		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
			{
				return;
			}

			if (not mapped.pData)
			{
				return;
			}

			const size_t srcStride = mapped.RowPitch;
			const size_t dstStride = image.width() * sizeof(Float2);

			if (srcStride == dstStride)
			{
				std::memcpy(image.data(), mapped.pData, image.size_bytes());
			}
			else
			{
				const size_t width = image.width();
				const size_t height = image.height();

				const uint8* pSrc = static_cast<const uint8*>(mapped.pData);
				Float2* pDst = image.data();

				for (size_t y = 0; y < height; ++y)
				{
					std::memcpy(pDst, pSrc, dstStride);
					pDst += width;
					pSrc += srcStride;
				}
			}

			context->Unmap(m_stagingTexture.Get(), 0);
		}
	}

	void D3D11Texture::readRT(ID3D11Device* device, ID3D11DeviceContext* context, Grid<Float4>& image)
	{
		if ((m_type != TextureType::Render)
			&& (m_type != TextureType::MSRender))
		{
			return;
		}

		if (m_desc.format != TextureFormat::R32G32B32A32_Float) // RGBA32F 形式以外なら失敗
		{
			LOG_FAIL(U"D3D11Texture::readRT(): Grid<Float4> is not supported in this format");
			return;
		}

		// テクスチャのサイズ
		const Size size = m_desc.size;

		// [ステージング・テクスチャ] が未作成の場合、作成
		if (not m_stagingTexture)
		{
			const D3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC(
				static_cast<DXGI_FORMAT>(m_desc.format.DXGIFormat()),
				size.x, size.y,
				1, 1,
				0,
				D3D11_USAGE_STAGING,
				D3D11_CPU_ACCESS_READ,
				1, 0,
				0);

			if (FAILED(device->CreateTexture2D(&desc, nullptr, &m_stagingTexture)))
			{
				return;
			}
		}

		// [メインテクスチャ] から [ステージング・テクスチャ] にコピー
		context->CopyResource(m_stagingTexture.Get(), m_texture.Get());

		// Image をテクスチャのサイズにリサイズ
		image.resize(size);

		// [ステージング・テクスチャ] から Image にコピー
		D3D11_MAPPED_SUBRESOURCE mapped;
		{
			if (FAILED(context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
			{
				return;
			}

			if (not mapped.pData)
			{
				return;
			}

			const size_t srcStride = mapped.RowPitch;
			const size_t dstStride = image.width() * sizeof(Float4);

			if (srcStride == dstStride)
			{
				std::memcpy(image.data(), mapped.pData, image.size_bytes());
			}
			else
			{
				const size_t width = image.width();
				const size_t height = image.height();

				const uint8* pSrc = static_cast<const uint8*>(mapped.pData);
				Float4* pDst = image.data();

				for (size_t y = 0; y < height; ++y)
				{
					std::memcpy(pDst, pSrc, dstStride);
					pDst += width;
					pSrc += srcStride;
				}
			}

			context->Unmap(m_stagingTexture.Get(), 0);
		}
	}

	void D3D11Texture::resolveMSRT(ID3D11DeviceContext* context)
	{
		if (m_type != TextureType::MSRender)
		{
			return;
		}

		context->ResolveSubresource(m_texture.Get(), 0,
			m_multiSampledTexture.Get(), 0, DXGI_FORMAT(m_desc.format.DXGIFormat()));
	}
}
