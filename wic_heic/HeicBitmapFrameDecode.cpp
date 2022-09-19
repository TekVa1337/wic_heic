#include "stdafx.h"
#include "wic_heic.h"
#include "HeicBitmapFrameDecode.h"


CHeicBitmapFrameDecode::CHeicBitmapFrameDecode(heif::ImageHandle handle)
	: m_Count(1)
	, m_Handle(handle)
{
	Log("decoding");
	m_Image = handle.decode_image(heif_colorspace_RGB, heif_chroma_interleaved_24bit);
	Log("decoded");
}


CHeicBitmapFrameDecode::~CHeicBitmapFrameDecode()
{
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	} else if (IsEqualGUID(riid, IID_IWICBitmapSource)) {
		*ppvObject = static_cast<IWICBitmapSource*>(this);
	} else if (IsEqualGUID(riid, IID_IWICBitmapFrameDecode)) {
		*ppvObject = static_cast<IWICBitmapFrameDecode*>(this);
	} else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE CHeicBitmapFrameDecode::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CHeicBitmapFrameDecode::Release(void)
{
	uint32_t n = --m_Count;
	if (m_Count == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetSize(__RPC__out UINT *puiWidth, __RPC__out UINT *puiHeight)
{
	if (!puiWidth || !puiHeight) {
		return E_INVALIDARG;
	}

	*puiWidth = static_cast<UINT>(m_Handle.get_width());
	*puiHeight = static_cast<UINT>(m_Handle.get_height());

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetPixelFormat(__RPC__out WICPixelFormatGUID *pPixelFormat)
{
	if (!pPixelFormat) {
		return E_INVALIDARG;
	}

	m_Handle.has_alpha_channel();
	// GUID_WICPixelFormat24bppRGB
	// GUID_WICPixelFormat32bppR10G10B10A2HDR10
	*pPixelFormat = GUID_WICPixelFormat24bppRGB;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetResolution(__RPC__out double *pDpiX, __RPC__out double *pDpiY)
{
	if (pDpiX == nullptr || pDpiY == nullptr) {
		return E_INVALIDARG;
	}

	*pDpiX = 96;
	*pDpiY = 96;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::CopyPalette(__RPC__in_opt IWICPalette *pIPalette)
{
	return WINCODEC_ERR_PALETTEUNAVAILABLE;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::CopyPixels(__RPC__in_opt const WICRect *prc, UINT cbStride, UINT cbBufferSize, __RPC__out_ecount_full(cbBufferSize) BYTE *pbBuffer)
{
	if (!pbBuffer) {
		return E_INVALIDARG;
	}

	WICRect rc;
	rc.X = 0;
	rc.Y = 0;
	rc.Width = m_Handle.get_width();
	rc.Height = m_Handle.get_height();

	if (prc) {
		rc = *prc;
	}

	int inStride;
	int bpp = m_Image.get_bits_per_pixel(heif_channel_interleaved);
	uint8_t* plane = m_Image.get_plane(heif_channel_interleaved, &inStride);
	Log("CopyPixels: (X=%d,Y=%d,W=%d,H=%d), outStride=%u, inStride=%d",
		rc.X, rc.Y, rc.Width, rc.Height,
		cbStride, inStride);

	plane += (inStride * rc.Y);

	uint8_t* out = pbBuffer;
	for (int height = 0; height < rc.Height; height++)
	{
		memcpy_s(out, cbBufferSize, plane + (rc.X * bpp), cbStride);
		out += cbStride;
		plane += inStride;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetMetadataQueryReader(__RPC__deref_out_opt IWICMetadataQueryReader **ppIMetadataQueryReader)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetColorContexts(UINT cCount, __RPC__inout_ecount_full_opt(cCount) IWICColorContext **ppIColorContexts, __RPC__out UINT *pcActualCount)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetThumbnail(__RPC__deref_out_opt IWICBitmapSource **ppIThumbnail)
{
	return WINCODEC_ERR_CODECNOTHUMBNAIL;
}
