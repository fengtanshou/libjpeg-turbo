
#ifndef __V4L2CAMERA_H__
#define __V4L2CAMERA_H__

#include <stdint.h>
#include <linux/videodev2.h>

#define PIXEL_FORMAT_MJPEG		V4L2_PIX_FMT_MJPEG
#define PIXEL_FORMAT_YUV		V4L2_PIX_FMT_YUYV

class V4l2Camera {
public:
	V4l2Camera();
	~V4l2Camera();

public:
	int32_t		Init( const char *pDevice, int32_t iWidth, int32_t iHeight, int32_t iPixelFormat, int32_t iNumOfBuffer );
	void		Deinit( void );

	int32_t		StreamControl( int32_t bStreamOn );

	int32_t		QueueBuffer( int32_t iIndex );
	int32_t		QueueBuffer( void *pBuf );
	int32_t		DequeueBuffer( int32_t *iIndex, void **ppBUf, int32_t *iBufSize );

private:
	enum {	MAX_REQ_BUFFER = 16	};	

	int32_t		m_bInit;					//	Device Initialized State
	int32_t		m_bStreamOn;				//	Device Stream On
	int32_t		m_bUseGrabMethod;			//	En/Disalbe Grab Method( Default : Enable )

	int32_t		m_hFd;						//	V4L2 File Descriptor
	char*		m_pDeviceName;				//	Camera Device Node's Name

	int32_t 	m_iWidth;					//	Capture Width
	int32_t		m_iHeight;					//	Capture Height
	int32_t		m_iPixelFormat;				//	PixelFormat
	int32_t		m_iNumOfBuffer;				//	Number Of Buffer

	struct v4l2_capability	m_Capability;	//	Capability
	struct v4l2_format		m_Format;		//	Format

	struct v4l2_requestbuffers m_ReqBuf;	//	Requested Buffer Information
	void* 		m_pBuf[MAX_REQ_BUFFER];		//	Mapped Buffer's Address
	int32_t		m_iBufSize;

private:
	V4l2Camera (V4l2Camera &Ref);
	V4l2Camera &operator=(V4l2Camera &Ref);			
};

#endif	// __V4L2CAMERA_H__
