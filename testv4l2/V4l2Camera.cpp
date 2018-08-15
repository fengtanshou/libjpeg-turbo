
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <V4l2Camera.h>

#define NX_V4L2_SYS_PATH	"/sys/class/video4linux"

// #define NX_DBG_OFF

#define NX_DTAG		"[V4l2Camera]"
#define DbgMsg(...)
//------------------------------------------------------------------------------
V4l2Camera::V4l2Camera()
	: m_bInit( false )
	, m_bStreamOn( false )
	, m_bUseGrabMethod( true )
	, m_hFd( -1 )
	, m_pDeviceName( NULL )
	, m_iPixelFormat( V4L2_PIX_FMT_MJPEG )
	, m_iNumOfBuffer( 0 )
{

}

//------------------------------------------------------------------------------
V4l2Camera::~V4l2Camera()
{
	if( m_pDeviceName ) {
		free( m_pDeviceName );
		m_pDeviceName = NULL;
	}

	if( m_hFd > 0 ) {
		close( m_hFd );
		m_hFd = -1;
	}
}

//------------------------------------------------------------------------------
int32_t V4l2Camera::Init( const char *pDevice, int32_t iWidth, int32_t iHeight, int32_t iPixelFormat, int32_t iNumOfBuffer )
{
	//DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );

	int32_t ret = 0;

	m_pDeviceName = strdup(pDevice);

	m_hFd = open( m_pDeviceName, O_RDWR );
	if( 0 > m_hFd) {
		//DbgMsg( NX_DBG_ERR, "Fali, Open V4L2 interface. ( %s )\n", m_pDeviceName );
		goto ERROR;
	}

	memset( &m_Capability, 0, sizeof(m_Capability) );
	
	ret = ioctl( m_hFd, VIDIOC_QUERYCAP, &m_Capability );
	if( 0 > ret )
	{
		DbgMsg( NX_DBG_ERR, "Fail, Unable to query device.\n" );
		goto ERROR;
	}

	if( (m_Capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 )
	{
		DbgMsg( NX_DBG_ERR, "Fail, Video Capture is not supported.\n" );
		goto ERROR;
	}

	if( m_bUseGrabMethod )
	{
		if( !(m_Capability.capabilities & V4L2_CAP_STREAMING) )
		{
			DbgMsg( NX_DBG_ERR, "Fail, Streaming I/O is not supported.\n");
			goto ERROR;
		}
	}
	else
	{
		if( !(m_Capability.capabilities & V4L2_CAP_READWRITE) )
		{
			DbgMsg( NX_DBG_ERR, "Fail, Read I/O is not supported.\n");
			goto ERROR;
		}
	}

	memset( &m_Format, 0, sizeof(m_Format) );
	m_Format.type					= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_Format.fmt.pix.width			= iWidth;
	m_Format.fmt.pix.height			= iHeight;
	m_Format.fmt.pix.pixelformat	= iPixelFormat;
	m_Format.fmt.pix.field			= V4L2_FIELD_ANY;

	ret = ioctl( m_hFd, VIDIOC_S_FMT, &m_Format);
	if( 0 > ret )
	{
		DbgMsg( NX_DBG_ERR, "Fail, Unable to Set format.\n" );
		goto ERROR;
	}
	if( (m_Format.fmt.pix.width != (uint32_t)iWidth) || (m_Format.fmt.pix.height != (uint32_t)iHeight) )
	{
		DbgMsg( NX_DBG_WARN, "Fail, Not Support Resolution( %d x %d ). Maybe %d x %d !?\n", iWidth, iHeight, m_Format.fmt.pix.width, m_Format.fmt.pix.height );
		goto ERROR;
	}

	// Request Buffers
	memset( &m_ReqBuf, 0, sizeof(m_ReqBuf) );
	m_ReqBuf.count	= iNumOfBuffer;
	m_ReqBuf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_ReqBuf.memory	= V4L2_MEMORY_MMAP;
	ret = ioctl( m_hFd, VIDIOC_REQBUFS, &m_ReqBuf );
	if( 0 > ret )
	{
		DbgMsg( NX_DBG_ERR, "Fail, Unable to Allocate Buffers.\n" );
		goto ERROR;
	}

	// Map the Buffers
	struct v4l2_buffer buf;
	for( int32_t i = 0; i < iNumOfBuffer; i++ )
	{
		memset( &buf, 0, sizeof(buf) );
		buf.index	= i;
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		
		ret = ioctl( m_hFd, VIDIOC_QUERYBUF, &buf );
		if( 0 > ret )
		{
			DbgMsg( NX_DBG_ERR, "Fail, Unable to query buffer.\n" );
			goto ERROR;
		}

		m_pBuf[i]	= mmap( 0 /* start anywhere */, buf.length, PROT_READ, MAP_SHARED, m_hFd, buf.m.offset );
		m_iBufSize	= buf.length;

		if( m_pBuf[i] == MAP_FAILED )
		{
			DbgMsg( NX_DBG_ERR, "Fail, Unable to map buffer.\n" );
			goto ERROR;
		}

		DbgMsg( NX_DBG_DEBUG, "alloc buffer: index(%d), virAddr(%p), length(%d), offeset(%d)\n", i, m_pBuf[i], buf.length, buf.m.offset );
	}

	for( int32_t i = 0; i < iNumOfBuffer; i++ )
	{
		memset( &buf, 0, sizeof(buf) );
		buf.index	= i;
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		ret = ioctl( m_hFd, VIDIOC_QBUF, &buf );
		if( 0 > ret )
		{
			DbgMsg( NX_DBG_ERR, "Fail, Unable to queue buffer.");
			goto ERROR;
		}
	}

	m_bInit			= true;
	m_iWidth		= iWidth;
	m_iHeight		= iHeight;
	m_iPixelFormat	= iPixelFormat;
	m_iNumOfBuffer	= iNumOfBuffer;

	DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;

ERROR:
	if( m_hFd > 0 )
	{
		close( m_hFd );
		m_hFd = -1;
	}

	if( m_pDeviceName )
	{
		free( m_pDeviceName );
		m_pDeviceName = NULL;
	}

	DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return -1;
}

//------------------------------------------------------------------------------
void	V4l2Camera::Deinit( void )
{
	DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );

	if( m_bStreamOn )
	{
		StreamControl( false );
		m_bStreamOn = false;
	}

	for( int32_t i = 0; i < m_iNumOfBuffer; i++ ) 
	{
		if( m_pBuf[i] )
		{
			DbgMsg( NX_DBG_DEBUG, "free buffer: index(%d), virAddr(%p)\n", i, m_pBuf[i] );
			munmap(m_pBuf[i], m_iBufSize);
			m_pBuf[i] = NULL;
		}
	}

	if( 0 < m_hFd )
	{
		close( m_hFd );
		m_hFd = -1;
	}

	if( m_pDeviceName )
	{
		free( m_pDeviceName );
		m_pDeviceName = NULL;
	}

	m_bInit = false;
	DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
}

//------------------------------------------------------------------------------
int32_t	V4l2Camera::StreamControl( int32_t bStreamOn )
{
	DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );

	int32_t iRet = 0;
	int32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	iRet = ioctl( m_hFd, bStreamOn ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, &type );
	if( 0 > iRet )
	{
		DbgMsg( NX_DBG_ERR, "Unable to %s capture: %d.\n", bStreamOn ? "start" : "stop", iRet );
		DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
		return iRet;
	}

	m_bStreamOn = bStreamOn ? true : false;

	DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t	V4l2Camera::QueueBuffer( int32_t iIndex )
{
	int32_t ret;
	struct v4l2_buffer buf;

	memset( &buf, 0, sizeof(buf) );
	buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory	= V4L2_MEMORY_MMAP;
	buf.index	= iIndex;

	ret = ioctl( m_hFd, VIDIOC_QBUF, &buf );
	if( 0 > ret )
	{
		//DbgMsg( NX_DBG_ERR, "Unable to queue buffer( %d ).\n", ret );
	}

	return ret;
}

//------------------------------------------------------------------------------
int32_t V4l2Camera::QueueBuffer( void *pBuf )
{
	int32_t ret, iSlotIndex = -1;
	struct v4l2_buffer buf;

	for( int32_t i = 0; i < m_iNumOfBuffer; i++ )
	{
		if( m_pBuf[i] == pBuf ) {
			iSlotIndex = i;
			break;
		}
	}

	if( iSlotIndex == -1 ) {
		//DbgMsg( NX_DBG_ERR, "Unable to find index.\n" );
		return -1;
	}

	memset( &buf, 0, sizeof(buf) );
	buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory	= V4L2_MEMORY_MMAP;
	buf.index	= iSlotIndex;

	ret = ioctl( m_hFd, VIDIOC_QBUF, &buf );
	if( 0 > ret )
	{
		//DbgMsg( NX_DBG_ERR, "Unable to queue buffer( %d ).\n", ret );
	}

	return ret;
}

//------------------------------------------------------------------------------
int32_t V4l2Camera::DequeueBuffer( int32_t *iIndex, void **ppBUf, int32_t *iBufSize )
{
	int32_t ret;
	struct v4l2_buffer buf;

	memset( &buf, 0, sizeof(buf) );
	buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory	= V4L2_MEMORY_MMAP;

	ret = ioctl( m_hFd, VIDIOC_DQBUF, &buf );
	if( 0 > ret )
	{
		//DbgMsg( NX_DBG_ERR, "Unable to dequeue buffer( %d ).\n", ret );
		return ret;
	}
	else
	{
		*iIndex		= buf.index;
		*ppBUf		= m_pBuf[buf.index];
		*iBufSize	= buf.bytesused;
	}

	return ret;
}
