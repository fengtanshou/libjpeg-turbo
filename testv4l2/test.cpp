/*
 * =============================================================
 *
 *       Filename:  test.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2018年08月15日 16时15分08秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  sftan (Tan), fengtanshou@foxmail.com
 *   Organization:  
 *
 * =============================================================
 */
#include <stdlib.h>
#include "V4l2Camera.h"
#include <stdio.h>
#include <sys/time.h>
#include <turbojpeg.h>

using namespace std;

int write_file(char *name,char *buf,int size){
    FILE *jpegFile = NULL;
      if ((jpegFile = fopen(name, "wb")) != NULL){
      if (fwrite(buf, size, 1, jpegFile) > 0){
        fclose(jpegFile);  jpegFile = NULL;
      }
    }
}


int main(int argc, char **argv)
{
    V4l2Camera * p_CV4l2;

    p_CV4l2 = new V4l2Camera();
    p_CV4l2->Init("/dev/video1",1280,720,PIXEL_FORMAT_MJPEG,8);
    p_CV4l2->StreamControl(true);


    struct timeval tv1;
    struct timeval tv2;

    struct timezone tz1;
    struct timezone tz2;

    int32_t index;
    void *pbuf = NULL;
    int32_t sizex = 0 ;
    uint32_t temp = 0;
    tjhandle tjInstance = NULL;
    int width, height;
    int inSubsamp, inColorspace;
    void *imgBuf = (void*)malloc(1280*720);

      if ((tjInstance = tjInitDecompress()) == NULL)
      {
            printf("instance Decompress fail \n");
            return -1;
      }
	while(1)
    {
        gettimeofday(&tv1,&tz1);
        p_CV4l2->DequeueBuffer(&index,(void **)(&pbuf),&sizex);
        //jpeg_decode(&decode_buf,pbuf,&width, &height);
        
    tjDecompressHeader3(tjInstance, (const unsigned char*)pbuf, sizex, &width, &height,
                            &inSubsamp, &inColorspace);
    tjDecompress2(tjInstance, (const unsigned char*)pbuf, sizex, (unsigned char*)imgBuf, width, 0, height,
                      TJPF_GRAY, TJFLAG_ACCURATEDCT);
        write_file("xxx",(char *)imgBuf,1280*720);
       printf("index %d  size %d \n",index, sizex); 
        //memcpy(src_buf,decode_buf,src_size);
        //ff_converter_process(ff, src_buf,src_size,dst_buf,dst_size);
       // memcpy(outBuffer.bits,dst_buf,dst_size);

        p_CV4l2->QueueBuffer(index);
        gettimeofday(&tv2,&tz2);
        
        if(tv1.tv_usec > tv2.tv_usec)
            temp = tv2.tv_usec + 1000000 - tv1.tv_usec;
        else
            temp = tv2.tv_usec - tv1.tv_usec;

 //       printf(" %ld %ld : %ld : %ld\n", tv2.tv_sec, tv2.tv_usec, tv1.tv_sec,tv1.tv_usec);
          printf("time %ld \n",temp);
    }
    return 0;
}
