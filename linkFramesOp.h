#ifndef _LINKFRAMES_
#define _LINKFRAMES_
#include "queue.h"
#include "Shape.h"
#include "abandonedDetection.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#define BUFFERSIZE 5
#define DISAPPEAR_THRESHOLD 7

#define SMALLEST_NEAR_DISTANCE   800
#define DISTANCE 1
#define OVERLAP 1
#define AREA  1
#define MOTION  1

#define MAXID 0x0fffffff
 
#define OPENCV_OUTPUT_TEXT 1


typedef struct _TargetIdInfo
{
	int id;
	int startFrame;
	int endFrame;
	int currentX;
	int currentY;
	int directionX;
	int directionY;
	int width;
	int height;
	int disappearFrames;
	int staticFrames;
	BOOL ishuman;
	BOOL alert;
	mytest_QUEUE queue;
	Object* currentObject;
	
}TargetIdInfo;


StaticObject* getStaticObjectFromTarget(TargetIdInfo* idInfo);
#ifdef OPENCV_OUTPUT_TEXT
void TargetsAlert(TargetIdInfo* staticObjects[], int size, unsigned char* image, int imageWidth, int imageHeight, IplImage* iplimage);
#else
void TargetsAlert(TargetIdInfo* staticObjects[], int size, unsigned char* image, int imageWidth, int imageHeight);
#endif


// clear all the target infos at last
void clearTargetInfos(TargetIdInfo* targetIdInfos[], int size);


// clear the targets which cannot find a correspond for a long time ( disappeartime > threshold)
void updateTargetInfos(TargetIdInfo* targetIdInfos[], int size);

// put object into the idInfo's queue to trace its path
void putObjectIntoQueue(TargetIdInfo* idInfo, Object* object, Point* centerPoint, int frameNum);

// init a taret who find a correspond object at the first time
void initTargetIdInfo(TargetIdInfo* idInfo, Object* object, Point* centerPoint,int frameNum);

// init the struct TargetIdInfo at the beginning of the program
TargetIdInfo* initEmptyTargetIdInfo();

// get first "empty" TargetIdInfo so that we can init to be a "real" one
int getEmptyTargetIdInfo(TargetIdInfo* targetIdInfos[], int size);

Point computerCenterPoint(Object* object);

int getDistance( TargetIdInfo* idInfo, Object* object, Point* centerPoint);

// get the ratio of the two rectangles, one of idInfo and the other of the object
// return (Area)Object/TargetIdInfo

float getAreaRatio( TargetIdInfo* idInfo, Object* object);

// get the overlap area ratio : overlapArea/ TargetIdInfo
float getAreaOverlapRatio( TargetIdInfo* idInfo, Object* object);


// get distance ratio 
// if the distance is larger than threshold, then return 0
// else return 1 - distance/threshold
float getDistanceRatio( int distance);

// get the angle ratio
// currently if the angle < 90, return 1
// else return 0  for reducing computing cost

float getMotionVectorAngle( TargetIdInfo* idInfo, Object* object, Point* centerPoint);

// update the TargetIdInfo, including the (x,y), motion vector, width, height, e.g.
void updateIdInfo( TargetIdInfo* idInfo, Object* object, Point* centerPoint, int frameNum);

// compare two targets 
int compareObjectIndex(const void* _t1, const void* _t2);

// compare two target ptr
int compareObjectIndex_ptr(const void* _t1, const void* _t2);


// main function in linking the targetInfo with 
void linkTargetToTargetIdInfos(TargetIdInfo* targetInfos[],  Target* target, int frameNum);

// draw rectangles 
IplImage* rectangle_drawing(IplImage* pimage, CvPoint point, CvSize area);


#endif 