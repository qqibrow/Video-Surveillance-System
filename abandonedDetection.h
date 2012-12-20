#ifndef _ABANDONED_
#define _ABANDONED_
#include "Shape.h"
#include "BackgroundSubtractModel.h"
#include <cv.h>


#define NULL 0
#define SMALLEST_DISTANCE 600

#define A_MAXID 0x0fffffff



typedef enum _STATUS { ABANDONDED, REMOVED, NONE, OVERTIME}STATUS;
typedef struct _StaticObject
{
	int id;
	int currentX;
	int currentY;
	int width;
	int height;
	int preWidth;
	int preHeight;
	int frames;
	STATUS status;
	Object* currentObject;
	
}StaticObject;

void exchangeTowMode( GMM* mode1, GMM* mode2);

void updateStaticIdInfo( StaticObject* idInfo, GrimsonGMM* bgs, unsigned char* staticImage);

void pushAlertObjectIntoBackground(StaticObject* staticObjects[], int size, GrimsonGMM* bgs, unsigned char* staticImage);

void clearStaticObjects(StaticObject* staticObjects[], int size);

BOOL IfInnerObjectIsHomogeneous(StaticObject* object, unsigned char* image, int imageWidth, int imageHeight);

void writeTextOnScreen( char* text, IplImage* srcimage, Point location);

int getMean(unsigned char* image,int imageWidth, int imageHeight, int rgbIndex, int top, int bottom, int right, int left);

int getGrayMean(unsigned char* image,int imageWidth, int imageHeight, int top, int bottom, int right, int left);

int getEdgeGrayMean(unsigned char* image,int imageWidth, int imageHeight, int top, int bottom, int right, int left);
int getCenterMean(unsigned char* image,int imageWidth, int imageHeight, int centerX, int centerY);
BOOL ifObjectIsInSize( StaticObject* object, int height, int width);

void checkStaticObjectStatus(StaticObject* object, unsigned char* image, int imageWidth, int imageHeight);
// alert those have reached alert condition
void StaticObjectAlert(StaticObject* staticObjects[], int size, unsigned char* image, int imageWidth, int imageHeight);

void initStaticObject(StaticObject* info, Object* object, Point* centerPoint,int frameNum);

StaticObject* initEmptyStaticObject();

int getEmptyStaticObject(StaticObject* StaticObjects[], int size);

Point computerOCenterPoint(Object* object);

int getODistance( StaticObject* idInfo, Object* object, Point* centerPoint);

void updateStaticObjects( StaticObject* idInfo, Object* object, Point* centerPoint);

int compareStaticObjectIndex(const void* _t1, const void* _t2);

int compareStaticObjectIndex_ptr(const void* _t1, const void* _t2);

// main function in linking the targetInfo with 
void linkObjectToStaticObjects(StaticObject* staticObjects[],  Target* target, int frameNum);

#endif