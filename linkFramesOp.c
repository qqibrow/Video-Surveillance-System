#include "linkFramesOp.h"

//#define OUTPUT_PATH 1


StaticObject* getStaticObjectFromTarget(TargetIdInfo* idInfo)
{
	StaticObject* object = initEmptyStaticObject();
	object->id = idInfo->id;
	object->currentX = idInfo->currentX;
	object->currentY = idInfo->currentY;
	object->height = idInfo->height;
	object->width = idInfo->width;
	object->frames = idInfo->staticFrames;

	return object;
}
#ifdef OPENCV_OUTPUT_TEXT
void TargetsAlert(TargetIdInfo* targetIdInfos[], int size, unsigned char* image, int imageWidth, int imageHeight, IplImage* iplimage)
#else
void TargetsAlert(TargetIdInfo* targetIdInfos[], int size, unsigned char* image, int imageWidth, int imageHeight)
#endif
{
	int i;
	StaticObject* object;

#ifdef OPENCV_OUTPUT_TEXT
	char text[50];
	char objectinfo[10];
	Point textLoc;
#endif

	for( i = 0; i < size; i++)
	{
		if(targetIdInfos[i]->id != MAXID && targetIdInfos[i]->staticFrames > ALERT_TIME && targetIdInfos[i]->alert == FALSE)
		{
			object = getStaticObjectFromTarget( targetIdInfos[i] );
			checkStaticObjectStatus( object, image, imageWidth, imageHeight);

			if( object->status == REMOVED)
			{
				printf(" the object (%d, %d) has been REMOVED \n", object->currentX, object->currentY);

#ifdef OPENCV_OUTPUT_TEXT
				memset(text, 0, sizeof(char)*50);
				strcat(text,"object ");

				textLoc.x = 20;
				textLoc.y = 20;
						
				itoa(object->id, objectinfo, 10);
				strcat(text, objectinfo);
				strcat( text, " is REMOVED");
				writeTextOnScreen(text, iplimage, textLoc); 
#endif
			}
			else if( object->status == ABANDONDED)
			printf(" the object (%d, %d) has been ABANDONDED \n", object->currentX, object->currentY);
			else
			printf(" the object (%d, %d) is not an object \n", object->currentX, object->currentY);

			free(object), object = NULL;
			targetIdInfos[i]->alert = TRUE;
			// handle target
			//mytest_FreeQueue(&targetIdInfos[i]->queue);
			//targetIdInfos[i]->id = MAXID;
		}	
	}
}
void clearTargetInfos(TargetIdInfo* targetIdInfos[], int size)
{
	int i;
	mytest_QueueData* data;
	for( i = 0; i < size; i++)
	{
		if(targetIdInfos[i] != NULL)
		{
			printf("target ID: %d, startFrame : %d, endFrame : %d\n", targetIdInfos[i]->id, targetIdInfos[i]->startFrame, targetIdInfos[i]->endFrame);
			printf("path : ");
			 while(targetIdInfos[i]->queue.count > 0)
			 {
				 data = mytest_Front( &targetIdInfos[i]->queue );
				 printf("[frameNum:%d, (%d, %d)] ", data->frameNum, data->centerPoint.x, data->centerPoint.y);
			     mytest_Pop( &targetIdInfos[i]->queue );
				 if(targetIdInfos[i]->queue.count > 0)
					 printf(" -> ");
				 else
					 printf("\n");
			 }
			mytest_FreeQueue(&targetIdInfos[i]->queue);
			free( targetIdInfos[i] );
			targetIdInfos[i] = NULL;
		}
	}
}
void updateTargetInfos(TargetIdInfo* targetIdInfos[], int size)
{
	int i;
	mytest_QueueData* data;
	for( i = 0; i < size; i++ )
	{
		if( targetIdInfos[i] != NULL && targetIdInfos[i]->disappearFrames > DISAPPEAR_THRESHOLD)
		{
			//printf target disappear info on the screen
#ifdef OUTPUT_PATH
			printf("target ID: %d, startFrame : %d, endFrame : %d\n", targetIdInfos[i]->id, targetIdInfos[i]->startFrame, targetIdInfos[i]->endFrame);
			printf("path : ");
			 while(targetIdInfos[i]->queue.count > 0)
			 {
				 data = mytest_Front( &targetIdInfos[i]->queue );
				 printf("[frameNum:%d, (%d, %d)] ", data->frameNum, data->centerPoint.x, data->centerPoint.y);
			     mytest_Pop( &targetIdInfos[i]->queue );
				 if(targetIdInfos[i]->queue.count > 0)
					 printf(" -> ");
				 else
					 printf("\n");
			 }
#endif
			mytest_FreeQueue(&targetIdInfos[i]->queue);
			targetIdInfos[i]->id = MAXID;
		}
	}
}
void putObjectIntoQueue(TargetIdInfo* idInfo, Object* object, Point* centerPoint, int frameNum)
{
	mytest_QueueData data;
	memcpy( &data.centerPoint, centerPoint, sizeof(Point));
	data.frameNum = frameNum;
	mytest_Push( &idInfo->queue, &data);
}
void initTargetIdInfo(TargetIdInfo* idInfo, Object* object, Point* centerPoint,int frameNum)
{
	static int d = 0;
	idInfo->id = ++d;
	idInfo->currentX = centerPoint->x;
	idInfo->currentY = centerPoint->y;
	idInfo->directionX = 0;
	idInfo->directionY = 0;
	idInfo->alert = FALSE;
	idInfo->startFrame = frameNum;
	idInfo->disappearFrames = 0;
	idInfo->staticFrames = 0;
	idInfo->ishuman =object->ishuman;
	idInfo->height = object->bottom - object->top;
	idInfo->width = object->right - object->left;
	idInfo->currentObject = object;
	mytest_Init(&idInfo->queue);
	putObjectIntoQueue( idInfo, object, centerPoint, frameNum);
	
}
TargetIdInfo* initEmptyTargetIdInfo()
{
	TargetIdInfo* targetInfo = NULL;

	targetInfo = (TargetIdInfo*)malloc(sizeof(TargetIdInfo));
	targetInfo->id = MAXID;
	targetInfo->disappearFrames = 0;
	targetInfo->staticFrames = 0;
	targetInfo->endFrame = 0;
	targetInfo->ishuman = FALSE;
	targetInfo->startFrame = -1;
	targetInfo->currentObject = NULL;

	return targetInfo;
}

int getEmptyTargetIdInfo(TargetIdInfo* targetIdInfos[], int size)
{
	int i;

	for( i = 0; i < size; i++)
	{
		if(targetIdInfos[i]->id == MAXID)
			return i;
	}
	return -1;
}

Point computerCenterPoint(Object* object)
{
	Point centerPoint;

	centerPoint.y = (object->bottom + object->top) / 2;
	centerPoint.x = (object->left + object->right) / 2;

	return centerPoint;
}

int getDistance( TargetIdInfo* idInfo, Object* object, Point* centerPoint)
{
	int dx;
	int dy;
	dx = idInfo->currentX - centerPoint->x;
	dy = idInfo->currentY - centerPoint->y;
	return dx*dx + dy*dy; 
}

// area distance

float getAreaRatio( TargetIdInfo* idInfo, Object* object)
{
	float r;
	int t_area = idInfo->height * idInfo->width;
	int o_area = (object->bottom - object->top) * (object->right - object->left);
	r = o_area/(float)t_area;
	return r < 1 ? r: 1;
}

float getAreaOverlapRatio( TargetIdInfo* idInfo, Object* object)
{
	int width, height;
	float r;
	int right = idInfo->currentX + idInfo->width/2;
	int left = idInfo->currentX - idInfo->width/2;
	int top = idInfo->currentY - idInfo->height/2;
	int bottom = idInfo->currentY + idInfo->height/2;
	
	int t_area = idInfo->height * idInfo->width;

		//first check if the areas has overlaped
	if( left < object->right && right > object->left && bottom 
		&& object->bottom > top && bottom > object->top)
	{
		if( right > object->right)
		{
			if(left > object->left)
				width = object->right - left;
			else
				width = object->right - object->left;
		}
		else
		{
			if(left > object->left)
				width = right - left;
			else
				width = right - object->left;
		}

		if( bottom > object->bottom)
		{
			if( top > object->top)
				height = object->bottom - top;
			else
				height = object->bottom - object->top;
		}
		else
		{
			if( top > object->top)
				height = bottom - top;
			else
				height = bottom - object->top;
		}
		
		r = width*height/(float)t_area;
		return  r < 1 ? r : 1;
	}

	return 0;
}



float getDistanceRatio( int distance)
{
	if( SMALLEST_NEAR_DISTANCE < distance)
		return 0;
	else return 1 - distance/(float)SMALLEST_NEAR_DISTANCE;
}


float getMotionVectorAngle( TargetIdInfo* idInfo, Object* object, Point* centerPoint)
{
	int angle = idInfo->currentX*centerPoint->x + idInfo->currentY*centerPoint->y;
	if( angle > 0)
		return 1;
	else return 0;
}

void updateIdInfo( TargetIdInfo* idInfo, Object* object, Point* centerPoint, int frameNum)
{
	int currentHeight, currentWidth;

	currentHeight = object->bottom - object->top;
	currentWidth =  object->right - object->left;

	if( abs( idInfo->height - currentHeight) < DELTA  && abs( idInfo->width - currentWidth) < DELTA)
		idInfo->staticFrames++;
	else
	{
		idInfo->staticFrames = 0;
		idInfo->width = currentWidth;
		idInfo->height = currentHeight;
	}


	putObjectIntoQueue(idInfo, object, centerPoint, frameNum);

	// check is the idInfo is still
	// put into idInfo's queue
	idInfo->directionX = centerPoint->x - idInfo->currentX;
	idInfo->directionY = centerPoint->y - idInfo->currentY;
	idInfo->currentX = centerPoint->x;
	idInfo->currentY = centerPoint->y;
	idInfo->disappearFrames = 0;
	idInfo->endFrame = 0;
	idInfo->ishuman = idInfo->ishuman | object->ishuman;

	idInfo->currentObject = object;
}


int compareObjectIndex(const void* _t1, const void* _t2)
{
	const TargetIdInfo* t1 = _t1;
	const TargetIdInfo* t2 = _t2;

	return t1->id - t2->id;
}

int compareObjectIndex_ptr(const void* _t1, const void* _t2)
{
	TargetIdInfo** t1 = _t1;
	TargetIdInfo** t2 = _t2;

	return compareObjectIndex(*t1, *t2);

}
void linkTargetToTargetIdInfos(TargetIdInfo* targetInfos[],  Target* target, int frameNum)
{
	unsigned int objectIndex, i;
	Point* centerPoints = NULL;
	TargetIdInfo* SmallestDisIndex = NULL;   //record the smallest distance object index
	TargetIdInfo* idInfo = NULL;
	float distance;
	int tempDistance;
	int index;
	float tempRatioSum, ratioSum;

	float distanceRatio, areaRatio, motionVector, overlapRatio;
	BOOL ifdetectedTargets[TARGETNUM];
	BOOL* ifdetected;
	//init

	if(target->targetnum == 0)
	{
		for( i = 0; i < TARGETNUM; i++)
			if( targetInfos[i]->id != MAXID)
			{
				targetInfos[i]->disappearFrames++;
			}
		return;
	}
		

	memset(ifdetectedTargets, 0, sizeof(BOOL) * TARGETNUM);
	ifdetected = (BOOL*)malloc( target->targetnum * sizeof(BOOL));
	memset(ifdetected, 0, target->targetnum*sizeof(BOOL));

	centerPoints = (Point*)malloc(target->targetnum * sizeof(Point));
	

	for( i = 0; i < target->targetnum; i++)
		centerPoints[i] = computerCenterPoint(&target->objects[i]);

	qsort(targetInfos, TARGETNUM, sizeof(TargetIdInfo*), compareObjectIndex_ptr);

	//if(frameNum == 1050)
	//{
	//	for( i = 0; i < TARGETNUM; i++)
	//		printf("%d\t", targetInfos[i]->id);
	//	printf("\n");
	//}

	for(  i = 0; i < TARGETNUM; i++)
	{
		if( targetInfos[i]->id != MAXID)  // indict that it is a already exist one
		{
			idInfo = targetInfos[i];

			SmallestDisIndex = NULL;
			distance = 0;
			ratioSum = 0.0f;
			tempRatioSum = 0.0f;
			index = -1;


			for( objectIndex = 0; objectIndex < target->targetnum; objectIndex++ )
			{
				tempDistance = getDistance(idInfo, &target->objects[objectIndex], &centerPoints[objectIndex]);
				
				distanceRatio = getDistanceRatio( tempDistance );
				
				if(distanceRatio == 0)
					continue;

				areaRatio = getAreaRatio(idInfo,&target->objects[objectIndex]);

				motionVector = getMotionVectorAngle(idInfo, &target->objects[objectIndex], &centerPoints[objectIndex]);

				overlapRatio = getAreaOverlapRatio(idInfo,&target->objects[objectIndex]);
				
				tempRatioSum = DISTANCE*distanceRatio + AREA*areaRatio + MOTION*motionVector + OVERLAP*overlapRatio;

				if( ratioSum < tempRatioSum /*&& ...*/ )  // have changed
				{
					ratioSum = tempRatioSum;
					index = objectIndex;
				}
			}
			
			// find a link
			if(index != -1 && ifdetected[index] == FALSE)
			{
				ifdetected[index] = TRUE;
				ifdetectedTargets[i] = TRUE;
				updateIdInfo(idInfo, &target->objects[index], &centerPoints[index], frameNum);
			}
			

		}
	}

	// to those disappear target, disappeartime ++
	for( i = 0; i < TARGETNUM; i++)	
	{
		if(targetInfos[i]->id != MAXID && ifdetectedTargets[i] == FALSE)
		{
			targetInfos[i]->disappearFrames++;
			targetInfos[i]->currentObject = NULL;
		}
	}

	// handle those objects who still cannot get a link, maybe a new one  
	for( objectIndex = 0; objectIndex < target->targetnum; objectIndex++ )
	{
		if(ifdetected[objectIndex] == FALSE) // indict that this object is a new one
		{	
			i = getEmptyTargetIdInfo(targetInfos, TARGETNUM);
			if( i == -1)
			{
				printf(" the targetInfos is full\n");
				exit(-1);
			}
			else
			{
				initTargetIdInfo( targetInfos[i], &target->objects[objectIndex], &centerPoints[objectIndex], frameNum);
			}
		}
	}
	

	// to those who disappear for a long time, clear them
	updateTargetInfos(targetInfos, TARGETNUM);
	free(centerPoints), centerPoints = NULL;
	free(ifdetected), ifdetected = NULL;
}

IplImage* rectangle_drawing(IplImage* pimage, CvPoint point, CvSize area)
{
	if(point.x > area.width && point.x < pimage->width - area.width && point.y > area.height && point.y < pimage->height - area.height)
	{
		//using cvLine, more simpler

		cvLine(pimage, cvPoint(point.x - area.width, point.y - area.height), cvPoint(point.x + area.width, point.y - area.height), CV_RGB(255, 0, 0), 1, 8,0);
		cvLine(pimage, cvPoint(point.x - area.width, point.y - area.height), cvPoint(point.x - area.width, point.y + area.height), CV_RGB(255, 0, 0), 1, 8,0);
		cvLine(pimage, cvPoint(point.x + area.width, point.y + area.height), cvPoint(point.x + area.width, point.y - area.height), CV_RGB(255, 0, 0), 1, 8,0);
		cvLine(pimage, cvPoint(point.x + area.width, point.y + area.height), cvPoint(point.x - area.width, point.y + area.height), CV_RGB(255, 0, 0), 1, 8,0);
		//int rows = point.y-area.height;
		//for(int j=point.x-area.width;j<=point.x+area.width;j++)
		//pimage->imageData[pimage->nChannels*(rows*pimage->width+j)] = 255;
		//rows = point.y+area.height;
		//for(int j=point.x-area.width;j<=point.x+area.width;j++)
		//pimage->imageData[pimage->nChannels*(rows*pimage->width+j)] = 255;
		//int columes = point.x-area.width;
		//for(int i=point.y-area.height;i<=point.y+area.height;i++)
		//pimage->imageData[pimage->nChannels*(i*pimage->width+columes)] = 255;
		//columes = point.x+area.width;
		//for(int i=point.y-area.height;i<=point.y+area.height;i++)
		//pimage->imageData[pimage->nChannels*(i*pimage->width+columes)] = 255;
	}

	return pimage;
}