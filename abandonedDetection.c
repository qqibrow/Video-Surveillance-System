#include "abandonedDetection.h"
#include "math.h"
#include "Draw.h"


#define MAX_BLOCK_HEIGHT 70
#define MAX_BLOCK_WIDTH 70
#define MIN_BLOCK_WIDTH 30
#define MIN_BLOCK_WIDTH 30
#define MIN_RGB_DISTANCE 20


void exchangeTowMode( GMM* mode1, GMM* mode2)
{
	float tempMu;

	tempMu = mode1->muB;
	mode1->muB = mode2->muB;
	mode2->muB = tempMu;

	tempMu = mode1->muR;
	mode1->muR = mode2->muR;
	mode2->muR = tempMu;

	tempMu = mode1->muG;
	mode1->muG = mode2->muG;
	mode2->muG = tempMu;

	tempMu = mode1->variance;
	mode1->variance = mode2->variance;
	mode2->variance = tempMu;
    
//	mode1->weight = 1.0f;
//	mode2->weight = 0.0f;

	tempMu = mode1->weight;
	mode1->weight = mode2->weight;
	mode2->weight = tempMu;

	tempMu = mode1->significants;
	mode1->significants = mode2->significants;
	mode2->significants = tempMu;


}

void updateStaticIdInfo( StaticObject* idInfo, GrimsonGMM* bgs, unsigned char* staticImage)
{
	int width, height;
	long mode1_pos, mode2_pos;

	int right = idInfo->currentX + idInfo->width/2;
	int left = idInfo->currentX - idInfo->width/2;
	int top = idInfo->currentY - idInfo->height/2;
	int bottom = idInfo->currentY + idInfo->height/2;
	
	for( height = top; height <= bottom; height++)
		for ( width = left; width <= right; width++)
		{
			
			// get the model position
			mode1_pos = height*bgs->m_width + width;

			if(staticImage[mode1_pos] == FOREGROUND)
			{
				mode1_pos = mode1_pos*bgs->m_max_modes;
				mode2_pos = mode1_pos + 1;
				exchangeTowMode(&bgs->m_modes[mode1_pos], &bgs->m_modes[mode2_pos]);


			}
		}

}



void pushAlertObjectIntoBackground(StaticObject* staticObjects[], int size, GrimsonGMM* bgs, unsigned char* staticImage)
{
	int i;
	StaticObject* object = NULL;
	for( i = 0; i < size; i++)
	{
		object = staticObjects[i];
		if( object->id != A_MAXID && object->status != NONE)
			updateStaticIdInfo(object, bgs, staticImage);
	}	
}
BOOL IfInnerObjectIsHomogeneous(StaticObject* object, unsigned char* image, int imageWidth, int imageHeight)
{
	int outerGrayMean, innerGrayMean;
	int i;
	int d;
	int x;
	int oright = object->currentX + object->width/2;
	int oleft = object->currentX - object->width/2;
	int otop = object->currentY - object->height/2;
	int obottom = object->currentY + object->height/2;
	
	int iright = (1/3.0)*oright + (2/3.0)*object->currentX;
	int ileft = ( (1/3.0)*oleft + (2/3.0)*object->currentX);
	int itop = ( (1/3.0)*otop + (2/3.0)*object->currentY);
	int ibottom = ((1/3.0)*obottom + (2/3.0)*object->currentY);
	
	outerGrayMean = getEdgeGrayMean( image, imageWidth, imageHeight, otop, obottom, oright, oleft);
//	x = getGrayMean( image, imageWidth, imageHeight, otop, obottom, oright, oleft);
//	innerGrayMean = getGrayMean( image, imageWidth, imageHeight, itop, ibottom, iright, ileft);
	innerGrayMean = getCenterMean( image, imageWidth, imageHeight, object->currentX, object->currentY);
	

	if( abs(outerGrayMean - innerGrayMean) > MIN_RGB_DISTANCE)
	{
		//object->status = ABANDONDED; 
		return FALSE;
	}
	else
		return TRUE;
		//object->status = REMOVED;
}

int getMean(unsigned char* image,int imageWidth, int imageHeight, int rgbIndex, int top, int bottom, int right, int left)
{
	int height, width;
	long pos;
	float sum = 0.0f;

	if(  top < 0 || bottom > imageHeight || left < 0 || right > imageHeight)
		return -1;

		for( height = top; height <= bottom; height++)
			for ( width = left; width <= right; width++)
			{
				pos = (height*imageWidth + width)*3 + rgbIndex;
				sum += (int)image[pos];

			}
	return sum/(float)(( bottom - top)*( right - left));

}


int getGrayMean(unsigned char* image,int imageWidth, int imageHeight, int top, int bottom, int right, int left)
{
	int height, width,ch;
	long pos;
	float sum = 0.0f;

	if(  top < 0 || bottom > imageHeight || left < 0 || right > imageHeight)
		return -1;

		for( height = top; height <= bottom; height++)
			for ( width = left; width <= right; width++)
			{
				pos = (height*imageWidth + width)*3;
				sum += (19595*image[pos] + 38469*image[pos + 1] + 7472*image[pos + 2])>> 16;

			}
	return sum/(float)(( bottom - top)*( right - left));

}
int getEdgeGrayMean(unsigned char* image,int imageWidth, int imageHeight, int top, int bottom, int right, int left)
{
	int height, width;
	long pos;
	int gray;
	float sum = 0.0f;

	int points = 0;
	if(  top < 0 || bottom > imageHeight || left < 0 || right > imageHeight)
		return -1;

		for( height = top; height <= bottom; height++)
			for ( width = left; width <= right; width++)
			{
				if( height == top || height == bottom || width == left || width == right)
				{
					pos = (height*imageWidth + width)*3;
					gray = (299*(int)image[pos+2] + 587*(int)image[pos + 1] + 114*(int)image[pos] + 500)/1000;
					sum += gray;
					points++;
				}

			}
	return sum/(float)(points);

}


int getCenterMean(unsigned char* image,int imageWidth, int imageHeight, int centerX, int centerY)
{

	Colour color;

	
	int width, height;
	long pos;
	float sum = 0.0f;
	int gray;
	int points = 0;

	color.b = 255;
	color.g = 0;
	color.r = 0;

	for( height = centerY - 5; height < centerY + 5; height++)
		for( width =centerX - 5; width < centerX + 5; width++)
		{
				pos = (height*imageWidth + width)*3;
					gray = (299*(int)image[pos+2] + 587*(int)image[pos + 1] + 114*(int)image[pos] + 500)/1000;
					sum += gray;
					points++;
		}

		DrawRectangle(image, imageHeight, imageWidth, centerY - 5, centerY + 5, centerX - 5, centerX + 5, color);
		return sum/(float)(points);
}
void clearStaticObjects(StaticObject* staticObjects[], int size)
{
	int i;
	for( i = 0; i < size; i++)
	{
		free( staticObjects[i] );
		staticObjects[i] = NULL;
	}
}

BOOL ifObjectIsInSize( StaticObject* object, int height, int width)
{
	if( object ->height <= height /*&& object->height >= height/2 */&& object->width <= width /*&& object->width >= width/2*/)
		return TRUE;
	else
		return FALSE;
}
void writeTextOnScreen( char* text, IplImage* srcimage, Point location)
{
	CvFont font;
	CvPoint point;
	point.x = location.x;
	point.y = location.y;
	cvInitFont( &font, CV_FONT_HERSHEY_SIMPLEX, 0.6, 0.6, 0, 1, 8 );
	cvPutText( srcimage, text, point, &font, CV_RGB( 255, 0, 255) );

}

void checkStaticObjectStatus(StaticObject* object, unsigned char* image, int imageWidth, int imageHeight)
{
	if( object->frames > ALERT_TIME) // the size is stable
	{
		object->status = OVERTIME;

		if(ifObjectIsInSize(object, MAX_BLOCK_HEIGHT,MAX_BLOCK_WIDTH)) // is a object, not a human or anything else
		{
			if(IfInnerObjectIsHomogeneous( object, image, imageWidth, imageHeight )) // is a texture
				object->status = REMOVED;
			else	
				object->status = ABANDONDED;
		}
		
		// post processing
	

	
	}
}

void StaticObjectAlert(StaticObject* staticObjects[], int size, unsigned char* image, int imageWidth, int imageHeight)
{
	int i;
	StaticObject* object;
	int right;
	int left;
	int top;
	int bottom;
	char text[50];
	Colour color;

	color.b = 255;
	color.g = 0;
	color.r = 0;

	for( i = 0; i < size; i++)
	{
		object = staticObjects[i];

		if( object->id != A_MAXID && object->status == NONE)
			checkStaticObjectStatus(object, image, imageWidth, imageHeight);

		if(object->status != NONE)
		{
			if( object->status == REMOVED)
				printf(" the object (%d, %d) has been REMOVED \n", object->currentX, object->currentY);
			else if( object->status == ABANDONDED)
				printf(" the object (%d, %d) has been ABANDONDED \n", object->currentX, object->currentY);
			else
				printf(" the object (%d, %d) is not an object \n", object->currentX, object->currentY);

			object->id = A_MAXID;
			object->currentObject = NULL;
			object->frames = 0;
			object->status = NONE;
			

		/*	right = object->currentX + object->width/2;
			left = object->currentX - object->width/2;
			top = object->currentY - object->height/2;
			bottom = object->currentY + object->height/2;

			DrawRectangle(image, imageHeight, imageWidth, top, bottom, left, right, color);*/

		}
		

		//post-processing push to pushAlertObjectIntoBackground

	}
}

void initStaticObject(StaticObject* info, Object* object, Point* centerPoint,int frameNum)
{
	static int d = 0;
	info->id = ++d;
	info->currentX = centerPoint->x;
	info->currentY = centerPoint->y;
	info->height = info->preHeight = object->bottom - object->top;
	info->width = info->preWidth = object->right - object->left;
	info->currentObject = object;
	info->frames = 0;

	
}




StaticObject* initEmptyStaticObject()
{
	StaticObject* staticObject = NULL;
	staticObject = (StaticObject*)malloc(sizeof(StaticObject));
	staticObject->id = A_MAXID;
	staticObject->currentObject = NULL;
	staticObject->frames = 0;
	staticObject->status = NONE;
	return staticObject;
}

int getEmptyStaticObject(StaticObject* StaticObjects[], int size)
{
	int i;

	for( i = 0; i < size; i++)
	{
		if(StaticObjects[i]->id == A_MAXID)
			return i;
	}
	return -1;
}

Point computerOCenterPoint(Object* object)
{
	Point centerPoint;

	centerPoint.y = (object->bottom + object->top) / 2;
	centerPoint.x = (object->left + object->right) / 2;

	return centerPoint;
}
int getODistance( StaticObject* idInfo, Object* object, Point* centerPoint)
{
	int dx;
	int dy;
	dx = idInfo->currentX - centerPoint->x;
	dy = idInfo->currentY - centerPoint->y;
	return dx*dx + dy*dy; 
}


void updateStaticObjects( StaticObject* idInfo, Object* object, Point* centerPoint)
{
	// put into idInfo's queue
	idInfo->currentX = centerPoint->x;
	idInfo->currentY = centerPoint->y;

	idInfo->preHeight = idInfo->height;
	idInfo->preWidth = idInfo->width;


	idInfo->height = object->bottom - object->top;
	idInfo->width = object->right - object->left;

	
	
	if( abs(idInfo->height - idInfo->preHeight) <= DELTA && 
		abs(idInfo->width - idInfo->preWidth) <= DELTA)
		idInfo->frames++;
	else
		idInfo->frames = 0;
	

	idInfo->currentObject = object;

	//if( idInfo->frames > ALERT_TIME)

	
}
int compareStaticObjectIndex(const void* _t1, const void* _t2)
{
	const StaticObject* t1 = _t1;
	const StaticObject* t2 = _t2;

	return t1->id - t2->id;
}

int compareStaticObjectIndex_ptr(const void* _t1, const void* _t2)
{
	StaticObject** t1 = _t1;
	StaticObject** t2 = _t2;

	return compareObjectIndex(*t1, *t2);

}
void linkObjectToStaticObjects(StaticObject* staticObjects[],  Target* target, int frameNum)
{
	unsigned int objectIndex, i;
	Point* centerPoints = NULL;
	StaticObject* idInfo = NULL;
	int distance;
	int tempDistance;
	int index;

	BOOL ifdetectedTargets[TARGETNUM];
	BOOL* ifdetected;
	//init

	if(target->targetnum == 0)
		return;

	memset(ifdetectedTargets, 0, sizeof(BOOL) * TARGETNUM);
	ifdetected = (BOOL*)malloc( target->targetnum * sizeof(BOOL));
	memset(ifdetected, 0, target->targetnum*sizeof(BOOL));

	centerPoints = (Point*)malloc(target->targetnum * sizeof(Point));
	

	for( i = 0; i < target->targetnum; i++)
		centerPoints[i] = computerOCenterPoint(&target->objects[i]);

	qsort(staticObjects, TARGETNUM, sizeof(StaticObject*), compareStaticObjectIndex_ptr);

	for(  i = 0; i < TARGETNUM; i++)
	{
		if( staticObjects[i]->id != A_MAXID)  // indict that it is a already exist one
		{
			idInfo = staticObjects[i];

			distance = 0x0fffffff;
			index = -1;

			for( objectIndex = 0; objectIndex < target->targetnum; objectIndex++ )
			{
				tempDistance = getODistance(idInfo, &target->objects[objectIndex], &centerPoints[objectIndex]);
				
				if(tempDistance > SMALLEST_DISTANCE)
					continue;

				if( distance > tempDistance )
				{
					distance = tempDistance;
					index = objectIndex;
				}
			}
			
			// find a link
			if(index != -1 && ifdetected[index] == FALSE)
			{
				ifdetected[index] = TRUE;
				ifdetectedTargets[i] = TRUE;
				if(idInfo->status == NONE)
					updateStaticObjects(idInfo, &target->objects[index], &centerPoints[index]);
			}
			

		}
	}

	// to those disappear target, disappeartime ++
	for( i = 0; i < TARGETNUM; i++)	
	{
		if(staticObjects[i]->id != A_MAXID && ifdetectedTargets[i] == FALSE)
		{
			staticObjects[i]->currentObject = NULL;
		}
	}

	// handle those objects who still cannot get a link, maybe a new one  
	for( objectIndex = 0; objectIndex < target->targetnum; objectIndex++ )
	{
		if(ifdetected[objectIndex] == FALSE) // indict that this object is a new one
		{	
			i = getEmptyStaticObject(staticObjects, TARGETNUM);
			if( i == -1)
			{
				printf(" the staticObjects is full\n");
				exit(-1);
			}
			else
			{
				initStaticObject( staticObjects[i], &target->objects[objectIndex], &centerPoints[objectIndex], frameNum);
			}
		}
	}
	

	// to those who disappear for a long time, clear them
	
//	StaticObjectAlert(staticObjects, TARGETNUM);
	free(centerPoints), centerPoints = NULL;
	free(ifdetected), ifdetected = NULL;
}
