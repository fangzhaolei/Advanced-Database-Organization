/*
 * buffer_mgr.c
 *
 *  Created on: Mar 21, 2015
 *      Author: karthikComp
 */
#include "buffer_mgr.h"
#include "dberror.h"
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "bufferMgrDataStructures.h"

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {
	pageAndDirtyBitIndicator *temp;
	//here[START]
	ramPageMap *start = firstRamPageMapPtr;
	while(start->discPageFrameNumber != page->pageNum)
	{
		start = start->nextRamPageMap;
	}
	//here[END]
	temp = firstPageAndDirtyBitMap;
	//here
	//while(page->pageNum != temp->ramPageFrameNumber) {
	while(start->ramPageFrameNumber != temp->ramPageFrameNumber) {
		temp = temp->nextPageDirtyBit;
	}

	if(temp != NULL)
	{
		temp->isPageDirty = 1;
		return RC_OK;
	}
	else
	{
		return RC_DIRTY_UPDATE_FAILED;
	}
}

frameList *createBufferPool(int numPages)
{
		int counter = 1;
		frameList *currentFramePtr = NULL;
		frameList *previousFramePtr = NULL;

		while(counter <= numPages) //Ex : create 3 pages and link them to form a list.
		{
			currentFramePtr = (frameList *)malloc(sizeof(frameList));

			if(counter == 1)
				firstFramePtr = currentFramePtr; // firstFramePtr. deallocate Pool using this.
			else
				previousFramePtr->nextFramePtr = currentFramePtr;

			previousFramePtr = currentFramePtr;
			counter++;
		}
		currentFramePtr->nextFramePtr = NULL;
		return firstFramePtr;
}

ramPageMap *createRamPageMapList(int numPages)
{
	int counter = 0;
	ramPageMap *currentRamPagePtr = NULL;
	ramPageMap *previousRamPagePtr = NULL;
	ramPageMap *start = NULL;
	while(counter < numPages) //Ex : create 3 Maps and link them.
	{
		currentRamPagePtr = (ramPageMap *)malloc(sizeof(ramPageMap));

		if(counter == 0)
		{
			start = currentRamPagePtr; // firstFramePtr. deallocate Pool using this.
			clockPtr = start;
		}
		else
			previousRamPagePtr->nextRamPageMap = currentRamPagePtr;

		currentRamPagePtr->ramPageFrameNumber = counter;
		currentRamPagePtr->discPageFrameNumber = -1;
		currentRamPagePtr->clockReferenceBit = 0;

		previousRamPagePtr = currentRamPagePtr;
		counter++;
	}
	currentRamPagePtr->nextRamPageMap = NULL;
	return start;
}

pageAndDirtyBitIndicator *createPageAndDirtyBitMap(int numPages)
{
	int counter = 0;
	pageAndDirtyBitIndicator *currrentPageDirtyBitPtr = NULL;
	pageAndDirtyBitIndicator *previousPageDirtyBitPtr = NULL;
	pageAndDirtyBitIndicator *start = NULL;
	while(counter < numPages) //Ex : create 3 Maps and link them.
	{
		currrentPageDirtyBitPtr = (pageAndDirtyBitIndicator *)malloc(sizeof(pageAndDirtyBitIndicator));

		if(counter == 0)
			start = currrentPageDirtyBitPtr; // firstFramePtr. deallocate Pool using this.
		else
			previousPageDirtyBitPtr->nextPageDirtyBit = currrentPageDirtyBitPtr;

		currrentPageDirtyBitPtr->ramPageFrameNumber = counter;
		currrentPageDirtyBitPtr->isPageDirty = 0;

		previousPageDirtyBitPtr = currrentPageDirtyBitPtr;
		counter++;
	}
	currrentPageDirtyBitPtr->nextPageDirtyBit = NULL;
	return start;
}

pageAndFixCount *createPageAndFixCountMap(int numPages)
{
	int counter = 0;
	pageAndFixCount *currrentPageandFixCountPtr = NULL;
	pageAndFixCount *previousPageandFixCountPtr = NULL;
	pageAndFixCount *start = NULL;
	while(counter < numPages) //Ex : create 3 Maps and link them.
	{
		currrentPageandFixCountPtr = (pageAndFixCount *)malloc(sizeof(pageAndFixCount));

		if(counter == 0)
			start = currrentPageandFixCountPtr; // firstFramePtr. deallocate Pool using this.
		else
			previousPageandFixCountPtr->nextPageFixCount = currrentPageandFixCountPtr;

		currrentPageandFixCountPtr->ramPageFrameNumber = counter;
		currrentPageandFixCountPtr->fixCount = 0;

		previousPageandFixCountPtr = currrentPageandFixCountPtr;
		counter++;
	}
	currrentPageandFixCountPtr->nextPageFixCount = NULL;
	return start;
}
/*
 * Author 		: Aditya
 * Date			: 22.Mar.2015
 * Module		: initBufferPool
 * Description	: 1. Create a buffer pool of size numPages.
 * 				  2. Create a Map to store frame and their corresponding disc PageNumbers.
 	 	 	 	 	 Initially the Map will have -1 as the discPageNumbers
 * 				  3. Create a Map to store frame and their corresponding dirty Bit.
 * 				  4. Create a Map to store frame and their corresponding fix count.
 */
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
		  const int numPages, ReplacementStrategy strategy,
		  void *stratData)
{
	numberOfWrites = 0;
	numberOfReads  = 0;

	frameContentPtr = (PageNumber *)malloc(sizeof(PageNumber) * numPages);
	dirtyBitPtr = (bool *)malloc(sizeof(bool) * numPages);
	fixCountPtr = (int *)malloc(sizeof(int) * numPages);

	numberOfFrames = numPages;
	//fHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
	bm->mgmtData = createBufferPool(numPages); //gives address of the first Frame.
	firstRamPageMapPtr = createRamPageMapList(numPages);//give address of the first RamPage Map.
	firstPageAndDirtyBitMap = createPageAndDirtyBitMap(numPages);
	firstPageAndFixCountPtr = createPageAndFixCountMap(numPages);
	openPageFile((char *)pageFileName, &fHandle);

	bm->numPages = numPages;
	bm->pageFile = (char *)pageFileName;
	bm->strategy = strategy;
	if(bm->mgmtData != NULL && firstRamPageMapPtr !=NULL && firstPageAndDirtyBitMap != NULL && firstPageAndFixCountPtr != NULL)
		return RC_OK;
	else
		return RC_BUFFER_POOL_INIT_ERROR;
}

RC checkIfPagePresentInFramePageMaps(const PageNumber pageNum)
{

	ramPageMap *start = firstRamPageMapPtr;
	while(start != NULL)
	{
		if(start->discPageFrameNumber == pageNum)
			return start->ramPageFrameNumber;
		start = start->nextRamPageMap;
	}
	return RC_NO_FRAME;
}

void getFrameData(int frameNumber,BM_PageHandle * page)
{
	frameList *start = firstFramePtr;
	int counter = 0;
	while(counter < frameNumber)
	{
		start = start->nextFramePtr;
		counter++;
	}
	page->data = start->frameData;
}

void getFirstFreeFrameNumber(int *firstfreeFrameNumber,PageNumber PageNum)
{
	ramPageMap *start = firstRamPageMapPtr;
	while(start != NULL && start->discPageFrameNumber != -1)
	{
		start = start->nextRamPageMap;
	}
	if(start != NULL)
	{
		*firstfreeFrameNumber = start->ramPageFrameNumber;
		start->discPageFrameNumber = PageNum;
	}
	else
	{
		*firstfreeFrameNumber = -99;
	}

}

RC changeFixCount(int flag,int page)
{
	ramPageMap *startFramePtr = firstRamPageMapPtr;
	while(startFramePtr != NULL && startFramePtr->discPageFrameNumber != page)
	{
		startFramePtr = startFramePtr->nextRamPageMap;
	}

	pageAndFixCount *startFixCountPtr = firstPageAndFixCountPtr;
	while((startFixCountPtr != NULL) && (startFixCountPtr->ramPageFrameNumber != startFramePtr->ramPageFrameNumber))
	{
		startFixCountPtr = startFixCountPtr->nextPageFixCount;
	}
	if(flag == 1)
		startFixCountPtr->fixCount++;
	else
		startFixCountPtr->fixCount--;

	return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {

	/*
	 frameList *temp;
	ramPageMap *RPMtemp;
	RPMtemp = firstRamPageMapPtr;
	temp = firstFramePtr;
	*/

	/*
	int i = 0;
	for (i = 0; i <= page->pageNum; i ++) {
		temp = temp->nextFramePtr;
	}
	*/

	//here
	/*
	while (RPMtemp->ramPageFrameNumber != page->pageNum) {
		RPMtemp = RPMtemp->nextRamPageMap;
	}
	*/

	if(writeBlock (page->pageNum, &fHandle, page->data) == RC_OK)
	{
		numberOfWrites++;
		return RC_OK;
	}
	else
	{
		return RC_WRITE_FAILED;
	}
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	//If discNumber is passed, page->pageNum will be assigned with pageNumber
	//here
	/*
	if(page->pageNum > (bm->numPages-1))
	{
			ramPageMap *start = firstRamPageMapPtr;
			while(start->discPageFrameNumber != page->pageNum)
			{
				start = start->nextRamPageMap;
			}
			page->pageNum = start->ramPageFrameNumber;
	}
	*/

	ramPageMap *startFramePtr = firstRamPageMapPtr;
	while(startFramePtr != NULL && startFramePtr->discPageFrameNumber != page->pageNum)
	{
		startFramePtr = startFramePtr->nextRamPageMap;
	}
	RC status = changeFixCount(2,startFramePtr->discPageFrameNumber); //

	int frameNumber = startFramePtr->ramPageFrameNumber;//here

	//If dirty, call forcepage.

	pageAndDirtyBitIndicator *startDirtyPointer = firstPageAndDirtyBitMap;
	while(startDirtyPointer != NULL && startDirtyPointer->ramPageFrameNumber != frameNumber)
	{
		startDirtyPointer = startDirtyPointer->nextPageDirtyBit;
	}

	if(startDirtyPointer->isPageDirty == 1)
		forcePage(bm,page);
	if(status == RC_OK)
		return RC_OK;
	else
		return RC_UNPIN_FAILED;
}

RC FIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum,SM_PageHandle ph)
{

	pageAndFixCount *fixCountStart = firstPageAndFixCountPtr;
	//getFrameNumber of firstNode in ramPagePtr;
	//go to that fixCountPtr whose frameNumber = above frameNumber,
	//and check its fix count for > 0.
	ramPageMap *begin =firstRamPageMapPtr;
	while(fixCountStart != NULL && fixCountStart->ramPageFrameNumber != begin->ramPageFrameNumber)
		fixCountStart = fixCountStart->nextPageFixCount;


	if(fixCountStart != NULL && fixCountStart->fixCount > 0) //means node should'nt be deleted...
	{
		begin = firstRamPageMapPtr;
		int FrameNumberOfNewPage = begin->nextRamPageMap->ramPageFrameNumber;

		//make a new node ,add discPageDetails to it and make it last node

		//try writing this in a function...
		ramPageMap *currentRamPagePtr = (ramPageMap *)malloc(sizeof(ramPageMap));
		currentRamPagePtr->discPageFrameNumber = pageNum;
		currentRamPagePtr->ramPageFrameNumber = FrameNumberOfNewPage;
		currentRamPagePtr->nextRamPageMap = begin->nextRamPageMap->nextRamPageMap;
		free(begin->nextRamPageMap);
		begin->nextRamPageMap = currentRamPagePtr;

		//write to buffer..
		int counter = 0;
		frameList *beginFrame = firstFramePtr;
		while(counter < FrameNumberOfNewPage)
		{
			beginFrame = beginFrame->nextFramePtr;
			counter++;
		}
		memset(beginFrame->frameData,'\0',PAGE_SIZE + 1);
		if(ph != NULL)
			strcpy(beginFrame->frameData,ph);
		//strcpy(page->data,beginFrame->frameData);
		page->data = beginFrame->frameData;
		//here [START]
		//page->pageNum = FrameNumberOfNewPage;
		page->pageNum = pageNum;
		//here [END]
	}
	//get first node's frame Number;
	else
	{
		int frameNumberOfNewPage = firstRamPageMapPtr->ramPageFrameNumber;
		//remove the first node[START];
		ramPageMap *temp;
		temp = firstRamPageMapPtr;
		firstRamPageMapPtr = firstRamPageMapPtr->nextRamPageMap;
		free(temp);
		temp = NULL;
		//remove the first node[END];

		//make a new node ,add discPageDetails to it and make it last node
		ramPageMap *currentRamPagePtr = (ramPageMap *)malloc(sizeof(ramPageMap));
		currentRamPagePtr->discPageFrameNumber = pageNum;
		currentRamPagePtr->ramPageFrameNumber = frameNumberOfNewPage;
		currentRamPagePtr->nextRamPageMap = NULL;

		temp = firstRamPageMapPtr;
		while(temp->nextRamPageMap != NULL)

		{
			temp = temp->nextRamPageMap;
		}
		temp->nextRamPageMap = currentRamPagePtr;

		//writing data to buffer
		int counter = 0;
		frameList *beginFrame = firstFramePtr;
		while(counter < frameNumberOfNewPage)
		{
			beginFrame = beginFrame->nextFramePtr;
			counter++;
		}
		memset(beginFrame->frameData,'\0',PAGE_SIZE + 1);
		if(ph != NULL)
			strcpy(beginFrame->frameData,ph);
		//strcpy(page->data,beginFrame->frameData);
		page->data = beginFrame->frameData;
		//
		page->pageNum = pageNum;
	}
	return RC_OK;
}
//All these are dummy methods. Required functionality has to be added later[START]

void attachAtEndOfList(ramPageMap *temp)
{
	ramPageMap *start = firstRamPageMapPtr;
	while(start->nextRamPageMap != NULL)
		start = start->nextRamPageMap;
	start->nextRamPageMap = temp;
}
void sortFixCounts(int *intArray, int size)
{
      char flag = 'Y';
      int j = 0;
      int temp;
      while (flag == 'Y')
      {
            flag = 'N';j++;int i;
            for (i = 0; i <size-j; i++)
            {
                  if (intArray[i] > intArray[i+1])
                  {
                        temp = intArray[i];
                        intArray[i] = intArray[i+1];
                        intArray[i+1] = temp;
                        flag = 'Y';
                  }
            }
      }
}
void moveClockPtr()
{
	if(clockPtr->nextRamPageMap == NULL)
		clockPtr = firstRamPageMapPtr;
	else
		clockPtr = clockPtr->nextRamPageMap;
}
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
	SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);


	//call ensure capacity.
	ensureCapacity((pageNum + 1),&fHandle);
	/*Algorithm
	 * 1.If ith discNumber is present in our RamDisc DataStucture
	 *  {
	 * 		//It means required data is already present in the BufferPool.
	 * 		1.Fetch the corresponding ramPageNumber , paste it's pool content onto h
	 *  }
	 *  else
	 *  {
	 *  	//It means required data has to be brought from disc.
	 *  	1. if(ramDisc has free nodes)
	 *  		{
	 *  			a.Get first free node, paste ith value onto the free node's discNumber field.
	 *  			b.Get corresponding ramPage Number of ith disc from mapping table (say j)
	 *  			c.Go to jth frame in BufferPool and Paste readBlock(i) contents to ith frame
	 *  			d. Paste Bufferpool's jth frame's content on to h.
	 *  		}
	 *  	2. else
	 *  		{
	 *  			  RS comes here..
	 *  			//Refer the diagram written in GreenBook.
	 *  		}
	 *  }
	 */
	PageNumber frameNumber = checkIfPagePresentInFramePageMaps(pageNum);
	//memset(page->data,'\0',PAGE_SIZE);
	if(frameNumber != RC_NO_FRAME)
	{
		//here
		//int temp = page->pageNum;
		page->pageNum = frameNumber;
		getFrameData(frameNumber,page);
		//here
		page->pageNum = pageNum;
		if(bm->strategy == RS_LRU)
		{
			ramPageMap *temp = firstRamPageMapPtr;
			ramPageMap *prev = NULL;
			int counter = 0;

			while(temp !=NULL && temp->discPageFrameNumber != pageNum)
			{
				prev = temp;
				temp = temp->nextRamPageMap;
				counter ++;
			}

			if(temp != NULL)
			{
				if(counter == 0)
				{
					prev = firstRamPageMapPtr;
					firstRamPageMapPtr = firstRamPageMapPtr->nextRamPageMap;
					prev->nextRamPageMap = NULL;
					attachAtEndOfList(temp);
				}
				else
				{
					prev->nextRamPageMap = temp->nextRamPageMap;
					temp->nextRamPageMap = NULL;
					attachAtEndOfList(temp);
				}
			}

		}
		else if(bm->strategy == RS_CLOCK)
		{

			ramPageMap *start = firstRamPageMapPtr;
			while(frameNumber != start->ramPageFrameNumber)
			{
				start = start->nextRamPageMap;
			}
			start->clockReferenceBit = 1;
		}
	}
	else
	{
		int freeframeNumber = - 99;
		getFirstFreeFrameNumber(&freeframeNumber,pageNum);
		readBlock(pageNum, &fHandle,ph);
		numberOfReads++;
		if(freeframeNumber != -99)
		{
			//go to freeframeNumberTh frame and put ph contents onto it's frameData [START]

			int counter = 0;
			frameList *start = firstFramePtr;
			while(counter < freeframeNumber)
			{
				start = start->nextFramePtr;
				counter++;
			}
			memset(start->frameData,'\0',PAGE_SIZE+1);
			if(ph != NULL)
				strcpy(start->frameData,ph);
			//strcpy(page->data,start->frameData);
			page->data = start->frameData;
			//here
			//page->pageNum = freeframeNumber;
			page->pageNum = pageNum;


			ramPageMap *begin = firstRamPageMapPtr;
			while(begin->ramPageFrameNumber != freeframeNumber)
			{
				begin = begin->nextRamPageMap;
			}
			begin->discPageFrameNumber = pageNum;
			//go to freeframeNumberTh [END]
			//bufferPoool[freeFrameNumber] = readBlock(i);
			if(bm->strategy == RS_CLOCK)
			{
				clockPtr->clockReferenceBit = 0;
				moveClockPtr();
			}

		}
		else
		{
			if(bm->strategy == RS_FIFO || bm->strategy == RS_LRU)
			{
				FIFO(bm,page,pageNum,ph);
			}

			else if(bm->strategy == RS_LFU)
			{
				//get the frameNumber (f) which has the least fixCount
				//go to that f and
									//1.update fixCount to 0 in (FixCountPtr)
									//2.update discNum in (ramPageMap)
				pageAndFixCount *start = firstPageAndFixCountPtr;
				int sortedFixCountArray[bm->numPages];
				int index = 0;
				while(start != NULL)
				{
					sortedFixCountArray[index++] = start->fixCount;
					start = start->nextPageFixCount;
				}
				sortFixCounts(sortedFixCountArray,bm->numPages);

				start = firstPageAndFixCountPtr;
				while(start->fixCount != sortedFixCountArray[0])
				{
					start = start->nextPageFixCount;
				}
				start->fixCount = 0;

				ramPageMap *tempRPM = firstRamPageMapPtr;
				while(tempRPM->ramPageFrameNumber != start->ramPageFrameNumber)
				{
					tempRPM = tempRPM->nextRamPageMap;
				}
				tempRPM->discPageFrameNumber = pageNum;
				page->pageNum = pageNum;
			}
			else if(bm->strategy == RS_CLOCK)
			{
				while(clockPtr->clockReferenceBit == 1)
				{
					clockPtr->clockReferenceBit = 0;
					moveClockPtr();
				}
				clockPtr->discPageFrameNumber = pageNum;
				moveClockPtr();
				page->pageNum = pageNum;
			}
		}
		//if ramDisc has free nodes, get the first free node's frame number.
	}
	changeFixCount(1,pageNum);
	free(ph);
	ph = NULL;
	return RC_OK;
}

PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	ramPageMap *start = firstRamPageMapPtr;
	while(start != NULL)
	{
		frameContentPtr[start->ramPageFrameNumber] = start->discPageFrameNumber;
		start = start->nextRamPageMap;
	}
	return frameContentPtr;
}

bool *getDirtyFlags (BM_BufferPool *const bm)
{
	pageAndDirtyBitIndicator *start = firstPageAndDirtyBitMap;
	int i = 0;
	while(start != NULL)
	{
		if(start->isPageDirty == 1)
			dirtyBitPtr[i++] = true;
		else
			dirtyBitPtr[i++] = false;
		start = start->nextPageDirtyBit;
	}
	return dirtyBitPtr;
}
int *getFixCounts (BM_BufferPool *const bm)
{
	pageAndFixCount *start = firstPageAndFixCountPtr;
	int i = 0;
	while(start != NULL)
	{
		fixCountPtr[i++] = start->fixCount;
		start = start->nextPageFixCount;
	}
	return fixCountPtr;
}
RC shutdownBufferPool(BM_BufferPool *const bm)
{
	bool NonZeroFixIndicator = false;
	pageAndFixCount *start = firstPageAndFixCountPtr;
	while(start != NULL)
	{
		if(start->fixCount >0)
		{
			NonZeroFixIndicator = true;
			break;
		}
		start = start->nextPageFixCount;
	}
	start = NULL;

	if(NonZeroFixIndicator == false)
	{
		clockPtr = NULL;
		forceFlushPool(bm);

		free(frameContentPtr);
		free(dirtyBitPtr);
		free(fixCountPtr);

		int counter = 0;
		ramPageMap *RPMtemp = NULL;
		pageAndDirtyBitIndicator *PADBItemp = NULL;
		pageAndFixCount *PAFCtemp = NULL;
		frameList *FLtemp = NULL;
		frameList *firstFramePtr = NULL;
		firstFramePtr = (frameList*) bm->mgmtData;
		bm->mgmtData = NULL;


		while(counter < bm->numPages)
		{

			PAFCtemp = firstPageAndFixCountPtr;
			firstPageAndFixCountPtr = firstPageAndFixCountPtr->nextPageFixCount;
			free(PAFCtemp);


			RPMtemp = firstRamPageMapPtr;
			firstRamPageMapPtr = firstRamPageMapPtr->nextRamPageMap;
			free(RPMtemp);

			PADBItemp = firstPageAndDirtyBitMap;
			firstPageAndDirtyBitMap = firstPageAndDirtyBitMap->nextPageDirtyBit;
			free(PADBItemp);

			FLtemp = firstFramePtr;
			firstFramePtr = firstFramePtr->nextFramePtr;
			free(FLtemp);

			counter++;
		}
		firstPageAndFixCountPtr = NULL;
		firstRamPageMapPtr = NULL;
		firstPageAndDirtyBitMap = NULL;

		closePageFile(&fHandle);
	}
	if(firstPageAndFixCountPtr == NULL && firstRamPageMapPtr == NULL
			&& firstPageAndDirtyBitMap == NULL && bm->mgmtData ==NULL && NonZeroFixIndicator == false)
		return RC_OK;
	else
		return RC_SHUT_DOWN_ERROR;
}


int getNumWriteIO(BM_BufferPool *const bm)
{
	return numberOfWrites;
}

int getNumReadIO(BM_BufferPool *const bm)
{
	return numberOfReads;
}
RC forceFlushPool(BM_BufferPool *const bm)
{
	//If a frame is Dirty in DirtyBitIndicator,from BufferPool, write that frame onto disc.
	pageAndDirtyBitIndicator *dirtyStart = firstPageAndDirtyBitMap;
	frameList *frameStart = firstFramePtr;
	while (dirtyStart != NULL)
	{
		if(dirtyStart->isPageDirty == 1)
		{
			ramPageMap *ramPageStart = firstRamPageMapPtr;
			while(ramPageStart->ramPageFrameNumber != dirtyStart->ramPageFrameNumber)
			{
				ramPageStart = ramPageStart->nextRamPageMap;
			}
			writeBlock (ramPageStart->discPageFrameNumber, &fHandle,frameStart->frameData);
			dirtyStart->isPageDirty = 0;
		}
		dirtyStart = dirtyStart->nextPageDirtyBit;
		frameStart = frameStart->nextFramePtr;
	}
	//If a page is dirty, write it back to the disc.
	return RC_OK;
}
