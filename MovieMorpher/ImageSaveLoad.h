#ifndef IMAGESAVELOAD_H
#define IMAGESAVELOAD_H

namespace ImageSaveLoadHelper
{
	void _FlipImage(unsigned char* image, unsigned int width, unsigned int height);

	unsigned char* LoadImageFromDisk(unsigned int &width, unsigned int &height, char* outFileName = NULL);
	bool           SaveImageToDisk();

	bool		   SelectVideoFromDisk(char* outFileName);

}

#endif
