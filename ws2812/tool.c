#include <stdio.h>
#include <fcntl.h>
#include <linux/ioctl.h>

typedef struct
{
    unsigned int profile_type;
    unsigned int color_Value;
    unsigned int interval;
}profile_t;


int main(void)
{

   profile_t pt;
   int retVal;
   int fd = open( "/dev/ws2812c_dev", O_RDWR );

   if( fd < 0 )
   {
       printf("Cannot open device \t");
       printf(" fd = %d \n",fd);
       return 0;
   }


   pt.profile_type = 0;
   pt.color_Value = 0;

   while (1)
   {
     retVal = ioctl(fd, 77 , &pt);
     if (retVal < 0)
     {
       printf("Error: %d\n", retVal);
     }

     sleep(1);
     if( pt.color_Value>= 0x0 && pt.color_Value<= 0xff) pt.color_Value += 0x10;
     else if( pt.color_Value> 0xff && pt.color_Value<= 0xffff) pt.color_Value += 0x1000;
     else if( pt.color_Value> 0xffff && pt.color_Value<= 0xffffff) pt.color_Value += 0x100000;
     else if( pt.color_Value> 0xffffff) pt.color_Value = 0;
   }

    if( 0 != close(fd) )
    {
        printf("Could not close device\n");
    }

    return 0;
}
