/*****************************
// NDPRead.h - NDP data access library
// (c) Copyright 2009 Hamamatsu Photonics K.K.
// Version 1.1.16
//***************************/

// define FAR to be nothing just to satisfy the compiler
//#define FAR

//typedef const wchar_t* LPCWSTR;
//typedef wchar_t* LPWSTR;

#define NDPREAD_CHANNELORDER_UNDEFINED	(0)
#define NDPREAD_CHANNELORDER_BGR		(1)
#define NDPREAD_CHANNELORDER_RGB		(2)
#define NDPREAD_CHANNELORDER_Y			(3)

#ifdef _UNICODE

#define	GetImageWidth		GetImageWidthW
#define	GetImageHeight		GetImageHeightW
#define	GetImageBitDepth	GetImageBitDepthW
#define	GetNoChannels		GetNoChannelsW
#define GetChannelOrder		GetChannelOrderW
#define	GetImageData		GetImageDataW
#define	GetImageData16		GetImageData16W
#define	GetSourceLens		GetSourceLensW
#define	GetSourcePixelSize	GetSourcePixelSizeW
#define	GetReference		GetReferenceW
#define	GetMap				GetMapW
#define	GetSlideImage		GetSlideImageW
#define	GetZRange			GetZRangeW
#define	GetLastErrorMessage	GetLastErrorMessageW
#define	GetLowLevelParam	GetLowLevelParamW

#endif //#ifdef _UNICODE

extern "C"
{	
#ifdef _UNICODE
	
	long	GetImageWidthW(LPCWSTR i_strImageID);
	long	GetImageHeightW(LPCWSTR i_strImageID);
	long	GetImageBitDepthW(LPCWSTR i_strImageID);
	long	GetNoChannelsW(LPCWSTR i_strImageID);
	long	GetChannelOrder(LPCWSTR i_strImageID);
	long	GetImageDataW(LPCWSTR i_strImageID, long i_nPhysicalXPos, long i_nPhysicalYPos, long i_nPhysicalZPos, float i_fMag, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize);
	long	GetImageData16W(LPCWSTR i_strImageID, long i_nPhysicalXPos, long i_nPhysicalYPos, long i_nPhysicalZPos, float i_fMag, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize);
	long	GetImageDataInSourceBitDepthW(LPCWSTR i_strImageID, long i_nPhysicalXPos, long i_nPhysicalYPos, long i_nPhysicalZPos, float i_fMag, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize);
	float	GetSourceLensW(LPCWSTR i_strImageID);
	long	GetSourcePixelSizeW(LPCWSTR i_strImageID, long FAR* o_nWidth, long FAR* o_nHeight);
	long	GetReferenceW(LPCWSTR i_strImageID, LPWSTR o_strReference, long i_nBufferLength);
	long	GetMapW(LPCWSTR i_strImageID, long FAR* o_nPhysicalX, long FAR* o_nPhysicalY, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize, long FAR* o_nPixelWidth, long FAR* o_nPixelHeight) ;
	long	GetSlideImageW(LPCWSTR i_strImageID, long FAR* o_nPhysicalX, long FAR* o_nPhysicalY, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize, long FAR* o_nPixelWidth, long FAR* o_nPixelHeight);
	long	GetZRangeW(LPCWSTR i_strImageID, long FAR* o_nMin, long FAR* o_nMax, long FAR* o_nStep);
	LPCWSTR	GetLastErrorMessageW();
	long	GetLowLevelParam(LPCWSTR i_strImageID, LPCWSTR i_strParamID, LPWSTR o_strValue, long i_nBufferLength);
	
#else
	
	long	GetImageWidth(LPCSTR i_strImageID);
	long	GetImageHeight(LPCSTR i_strImageID);
	long	GetImageBitDepth(LPCSTR i_strImageID);
	long	GetNoChannels(LPCSTR i_strImageID);
	long	GetChannelOrder(LPCSTR i_strImageID);
	long	GetMap(LPCSTR i_strImageID, long FAR* o_nPhysicalX, long FAR* o_nPhysicalY, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize, long FAR* o_nPixelWidth, long FAR* o_nPixelHeight);
	long	GetSlideImage(LPCSTR i_strImageID, long FAR* o_nPhysicalX, long FAR* o_nPhysicalY, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize, long FAR* o_nPixelWidth, long FAR* o_nPixelHeight);
	long	GetZRange(LPCSTR i_strImageID, long FAR* o_nMin, long FAR* o_nMax, long FAR* o_nStep);
	long	GetImageData(LPCSTR i_strImageID, long i_nPhysicalXPos, long i_nPhysicalYPos, long i_nPhysicalZPos, float i_fMag, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize);
	long	GetImageData16(LPCSTR i_strImageID, long i_nPhysicalXPos, long i_nPhysicalYPos, long i_nPhysicalZPos, float i_fMag, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize);
	long	GetImageDataInSourceBitDepth(LPCSTR i_strImageID, long i_nPhysicalXPos, long i_nPhysicalYPos, long i_nPhysicalZPos, float i_fMag, long FAR* o_nPhysicalWidth, long FAR* o_nPhysicalHeight, void *i_pBuffer, long *io_nBufferSize);
	float	GetSourceLens(LPCSTR i_strImageID);
	long	GetSourcePixelSize(LPCSTR i_strImageID, long FAR* o_nWidth, long FAR* o_nHeight);
	long	GetReference(LPCSTR i_strImageID, LPSTR o_strReference, long i_nBufferLength);
	LPCSTR	GetLastErrorMessage();
	long	GetLowLevelParam(LPCSTR i_strImageID, LPCSTR i_strParamID, LPSTR o_strValue, long i_nBufferLength);
	
#endif //#ifdef _UNICODE
	
	long	SetCameraResolution(long i_nWidth, long i_nHeight);
	long	CleanUp();
}
