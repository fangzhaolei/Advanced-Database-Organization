
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testReadWrite(void);
static void testAppendEnsureCapMetaData(void);

/* main function running all tests */
int main (void) {
	testName = "";

	initStorageManager();

	testReadWrite();

	testAppendEnsureCapMetaData();

	return 0;
}

void testReadWrite(void) {
	SM_FileHandle fh;
	SM_PageHandle ph;

	int i;

	ph = (SM_PageHandle) malloc(PAGE_SIZE);

	testName = "test read and write methods";

	TEST_CHECK(createPageFile (TESTPF));

	TEST_CHECK(openPageFile (TESTPF, &fh));
	ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
	ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
	ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");

	//Writes A, B, C, D, E, F, G, H from 0th page to 7th page
	for(i = 0; i < 8; i ++) {
		memset(ph, 'A' + i, PAGE_SIZE);
		TEST_CHECK(writeBlock (i, &fh, ph));
		printf("writing %d th block\n", fh.curPagePos);
	}

	// read first page into handle  i.e. A
	TEST_CHECK(readFirstBlock (&fh, ph));
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 'A'), "expected A");
	printf("first block contains A\n");

	// read last page into handle   i.e. H
	TEST_CHECK(readLastBlock (&fh, ph));
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 'H'), "expected H");
	printf("last block contains H\n");

	// read first page into handle  i.e. A
	readFirstBlock (&fh, ph);

	// read next page into handle   i.e. B
	TEST_CHECK(readNextBlock (&fh, ph));
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 'B'), "expected B");
	printf("block contains B\n");

	readNextBlock (&fh, ph);        //C
	readNextBlock (&fh, ph);        //D

	// read previous page into handle	i.e. C
	TEST_CHECK(readPreviousBlock (&fh, ph));
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 'C'), "expected C");
	printf("block contains C\n");

	readNextBlock (&fh, ph);        //D
	readNextBlock (&fh, ph);        //E

	// read current page into handle       i.e. E
	TEST_CHECK(readCurrentBlock (&fh, ph));
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 'E'), "expected E");
	printf("block contains E\n");

	readPreviousBlock (&fh, ph);        //D

	//Replace D with Z
	memset(ph, 'Z', PAGE_SIZE);
	TEST_CHECK(writeCurrentBlock (&fh, ph));
	printf("writing D with Z\n");

	// read current page into handle        i.e. Z
	TEST_CHECK(readCurrentBlock (&fh, ph));
	for (i=0; i < PAGE_SIZE; i++){
		ASSERT_TRUE((ph[i] == 'Z'), "expected Z");
	}
	printf("block contains Z\n");

	TEST_CHECK(closePageFile (&fh));
	TEST_CHECK(destroyPageFile (TESTPF));

	free(ph);


	TEST_DONE();
}

void getString(int someNumber,char * reversedArray)
{
	char array[4];
	memset(array, '\0', 5);
	int i = 0;
	while (someNumber != 0)
	{
		array[i++] = (someNumber % 10) + '0';
		someNumber /= 10;
	}
	array[i] = '\0';

	//char reversedArray[4];
	int j=0;
	int x;
	for(x = strlen(array)-1;x>=0;x--)
	{
		reversedArray[j++] = array[x];
	}
	reversedArray[j]='\0';
}
void readMetaDataFromFile(char *fileName,char *readMetaFromFile)
{
	FILE *fp;
	fp = fopen(fileName, "r");
	char currentCharacter;
	if(fp != NULL)
	{
		int i = 0;
		while((currentCharacter = fgetc(fp))!='\0')
			readMetaFromFile[i++] = currentCharacter;
		readMetaFromFile[i] = '\0';
		fclose(fp);
	}
}
void testAppendEnsureCapMetaData()
{
	SM_FileHandle fh;
	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);

	TEST_CHECK(createPageFile (TESTPF));
	TEST_CHECK(openPageFile (TESTPF, &fh));

	//Append an empty block to the file.
	appendEmptyBlock(&fh);
	//Check whether the appended block has only 4096 '\0' in the currentBlock.
	readBlock(getBlockPos(&fh),&fh,ph);
	int i;
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
	printf("Appended Block was empty\n");

	//Page File should contain only 2 blocks.first block during createPage and second during appendBlock
	ASSERT_TRUE((fh.totalNumPages == 2), "Number of Blocks : 2");

	 //Current Block postion should be 1
	ASSERT_TRUE((fh.curPagePos == 1), "Current Page Position is 1");

	//add 3 more blocks to the Page File.
	ensureCapacity(5,&fh);

	//Verify whether the freshly added 3 blocks are of '\0' characters
	//[START]
	readBlock(2,&fh,ph);
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");

	readBlock(3,&fh,ph);
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");

	readBlock(4,&fh,ph);
	for (i=0; i < PAGE_SIZE; i++)
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
	printf("Freshly appended 3 blocks are empty\n");
	//[END]

	//Page File should contain only 5 blocks, as we have called ensureCapacity(5)
	ASSERT_TRUE((fh.totalNumPages == 5), "Number of Blocks : 5");

	//Current Block postion should be 4
	ASSERT_TRUE((fh.curPagePos == 4), "Current Page Position is 4");

	//Store the metaData into the file and close the pagefile.
	int totalNoOfPages = fh.totalNumPages;
	char fileName[100];
	memset(fileName,'\0',100);
	strcpy(fileName,fh.fileName);
	char metaDataFromFile[100];
	memset(metaDataFromFile,'\0',100);
	closePageFile(&fh);

	//Verify whether the written  MetaData is correct or not
	//[START]
	char metaDataToBeVerified[100];
	memset(metaDataToBeVerified,'\0',100);
	char returnData[100];
	memset(returnData,'\0',100);
	metaDataToBeVerified[0]= 'P';metaDataToBeVerified[1]= 'S';metaDataToBeVerified[2]= ':';metaDataToBeVerified[3]= '\0';
	getString(PAGE_SIZE,returnData);
	strcat(metaDataToBeVerified,returnData);
	strcat(metaDataToBeVerified,";");
	memset(returnData,'\0',100);
	strcat(metaDataToBeVerified,"NP:");
	getString(totalNoOfPages,returnData);
	strcat(metaDataToBeVerified,returnData);
	strcat(metaDataToBeVerified,";");
	readMetaDataFromFile(fileName,metaDataFromFile);
	ASSERT_TRUE((strcmp(metaDataToBeVerified, metaDataFromFile) == 0), "MetaData read from file is correct");
	printf("Read Meta Data from file is :: %s\n",metaDataToBeVerified);
	//[END]
	TEST_CHECK(destroyPageFile(TESTPF));
	free(ph);
	TEST_DONE();
}
