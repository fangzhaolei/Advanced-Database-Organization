#include "tables.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include <stdlib.h>
#include <string.h>


//Record Mgr Global DS [START]
SM_FileHandle fh_rec_mgr;
int tombStonedRIDsList[10000];//10000 is biggest numInserts Value
int currentScannedRecord = 0;
//Record Mgr Global DS [end]



/*
 * Author	:Aditya
 * Date		:21.Apr.2015
 * Module	:initRecordManager
 * Description:Initializes the record manager
 */
// table and manager
RC initRecordManager (void *mgmtData)
{
	int i = 0;
	for(;i<10000;i++)
		tombStonedRIDsList[i] = -99;
    return RC_OK;
}
/*
 * Author	:Aditya
 * Date		:21.Apr.2015
 * Module	:shutdownrecordManager
 * Description:Shutdowns the record manager
 */
RC shutdownRecordManager ()
{
    return RC_OK;
}
/*
 * Author	:Aditya
 * Date		:21.Apr.2015
 * Module	:createTable
 * Description:Creates a table. It creates the underlying page file
and store information about the schema, free-space and so on in the table information pages
 */
RC createTable (char *name, Schema *schema)
{
    SM_PageHandle ph;
    
    ph = (SM_PageHandle) malloc(PAGE_SIZE);
    
    createPageFile (name);                      //Create a new file with file name as table name.
    openPageFile (name, &fh_rec_mgr);           //Open the file.

    memset(ph,'\0',PAGE_SIZE);
    strcpy(ph,serializeSchema(schema)); // changed
    
    writeBlock (0, &fh_rec_mgr, ph);

    memset(ph,'\0',PAGE_SIZE);
    writeBlock (1, &fh_rec_mgr, ph);

    free(ph);
    
    return RC_OK;
}
/*
 * Author	:Aditya
 * Date		:21.Apr.2015
 * Module	:opentable
 * Description:It opens a table
 */
RC openTable (RM_TableData *rel, char *name)
{
    rel->schema = (Schema *)malloc(sizeof(Schema));//this will be deserialized schema.
    rel->name = (char *)malloc(sizeof(char)*100);
    SM_PageHandle ph;
    
    ph = (SM_PageHandle) malloc(PAGE_SIZE);

    if(fh_rec_mgr.fileName == NULL)
    	openPageFile(rel->name,&fh_rec_mgr);

    strcpy(rel->name,name);
    readBlock(0, &fh_rec_mgr, ph);
    deSerializeSchema(ph, rel->schema);//make changes
    rel->mgmtData = NULL; //not sure what's required here.
    
    free(ph);

    if(rel->schema != NULL && rel != NULL)
        return RC_OK;
    else
    	return RC_OPEN_TABLE_FAILED;

    closePageFile(&fh_rec_mgr);

    return RC_OK;
}
/*
 * Author	:Aditya
 * Date		:21.Apr.2015
 * Module	:closeTable
 * Description:It closes a table. It causes all outstanding
changes to the tables to be written to the page file
 */
RC closeTable (RM_TableData *rel)
{
    //Free Schema here
    //Free table later

	char freePagesList[PAGE_SIZE];
	memset(freePagesList,'\0',PAGE_SIZE);

	SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);
	readBlock(1,&fh_rec_mgr,ph);

	strcpy(freePagesList,ph);

	int x =0;

	char FreePageNumber[10];
	memset(FreePageNumber,'\0',10);

	char nullString[PAGE_SIZE];
	memset(nullString,'\0',PAGE_SIZE);
	for(;tombStonedRIDsList[x] != -99 ;x++)
	{
		sprintf(FreePageNumber,"%d",(tombStonedRIDsList[x]));
		strcat(freePagesList,FreePageNumber);
		strcat(freePagesList,";");

		writeBlock(1,&fh_rec_mgr,freePagesList);// update FSM block
		writeBlock(tombStonedRIDsList[x]+2,&fh_rec_mgr,nullString);//write deleted block

		memset(FreePageNumber,'\0',10);
	}

    closePageFile(&fh_rec_mgr);

    free(rel->schema);
    free(rel->name);
    return RC_OK;
}
/*
 * Author	:Aditya
 * Date		:22.Apr.2015
 * Module	:deleteTable
 * Description:It deletes the table
 */
RC deleteTable (char *name)
{
//    closePageFile(&fh_rec_mgr);
    destroyPageFile(name);
    return RC_OK;
}/*
 * Author	:Aditya
 * Date		:22.Apr.2015
 * Module	:getNumTuples
 * Description:It returns the number of tuples in the table
 */
int getNumTuples (RM_TableData *rel)
{
    int freePageCount = 0;
    int i = 0;
    
    SM_PageHandle ph;
    
    ph = (SM_PageHandle) malloc(PAGE_SIZE);
    
    readBlock(1, &fh_rec_mgr, ph);
    
    for (; ph[i] != NULL; i ++) {
        if (ph[i] == ';')
            freePageCount ++;
    }
    
    closePageFile(&fh_rec_mgr);
    openPageFile(rel->name, &fh_rec_mgr);
    return (fh_rec_mgr.totalNumPages - freePageCount);
}

/*
 * Author	:Karthik
 * Date		:22.Apr.2015
 * Module	:insertRecord
 * Description:It inserts a new record.This newly inserted record will
be assigned a RID and the record parameter passed is updated
 */
// handling records in a table
RC insertRecord (RM_TableData *rel, Record *record)
{
	if(fh_rec_mgr.fileName == NULL)
	{
		openPageFile(rel->name,&fh_rec_mgr);
	}

	int recordId;
    char *stringToBeWritten = (char *)malloc(sizeof(char) *PAGE_SIZE);
    memset(stringToBeWritten,'\0',PAGE_SIZE);
    strcpy(stringToBeWritten,record->data);

    char freePagesList[PAGE_SIZE];
	memset(freePagesList,'\0',PAGE_SIZE);

	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);
	readBlock(1, &fh_rec_mgr, ph);
	strcpy(freePagesList,ph); //read the 1st block content to fpl here

	free(ph);
	char areFreePagesPresent = 'N';
	int x;
	if(freePagesList != NULL)
	{
		for(x = 0; x < strlen(freePagesList);x++)
		{
			if(freePagesList[x] == ';')
			{
				areFreePagesPresent = 'Y';
				break;
			}
		}
	}
	if(areFreePagesPresent == 'Y')
	{
		char firstFreePageNumber[10];
		memset(firstFreePageNumber,'\0',10);
		for(x = 0; freePagesList[x] != ';';x++)
		{
			firstFreePageNumber[x] = freePagesList[x];
		}

		writeBlock(atoi(firstFreePageNumber) + 2,&fh_rec_mgr,stringToBeWritten);
		fh_rec_mgr.totalNumPages--; //because we replacing and not adding new blk
		int i = 0;
		char *newFreePagesList = (SM_PageHandle) malloc(PAGE_SIZE);;
		memset(newFreePagesList,'\0',PAGE_SIZE);
		for(x = (strlen(firstFreePageNumber) + 1) ;x < strlen(freePagesList);x++)
		{
			newFreePagesList[i++] = freePagesList[x];
		}
		writeBlock(1,&fh_rec_mgr,newFreePagesList);
		free(newFreePagesList);
		recordId = atoi(firstFreePageNumber);
	}
	else
	{
		writeBlock(fh_rec_mgr.totalNumPages,&fh_rec_mgr,stringToBeWritten);
		recordId = fh_rec_mgr.totalNumPages - 3;
	}

	record->id.page = recordId;
	record->id.slot = -99; //useless because we are storing 1 rec / page.

    free(stringToBeWritten);


    	return RC_OK;


}
/*
 * Author	:Vivek
 * Date		:22.Apr.2015
 * Module	:deleteRecord
 * Description:It deletes the record with a certain RID passed as an parameter
 */
RC deleteRecord (RM_TableData *rel, RID id)
{
	int i = 0 ;
	while(tombStonedRIDsList[i] != -99)
		i++;
	tombStonedRIDsList[i] = id.page;

    return RC_OK;
}
/*
 * Author	:Vivek
 * Date		:22.Apr.2015
 * Module	:updateRecord
 * Description:It updates a existing record with new values
 */
RC updateRecord (RM_TableData *rel, Record *record)
{

	openPageFile (rel->name, &fh_rec_mgr);
	writeBlock(record->id.page + 2,&fh_rec_mgr,record->data);
	closePageFile(&fh_rec_mgr);
    return RC_OK;
}
/*
 * Author	:Vivek
 * Date		:22.Apr.2015
 * Module	:getRecord
 * Description:It retrieve a record with certain RID passed as an parameter
 */
RC getRecord (RM_TableData *rel, RID id, Record *record)
{

    SM_PageHandle ph;

    SM_FileHandle fh;

    openPageFile (rel->name, &fh);

    ph = (SM_PageHandle) malloc(PAGE_SIZE);

    memset(ph,'\0',PAGE_SIZE);

    readBlock(id.page + 2, &fh, ph);

    record->id.page = id.page;

    strcpy(record->data, ph);
    //record->data = ph;

    closePageFile(&fh);
    free(ph);

    return RC_OK;
}

/*
 * Author	:UnniKrishnan
 * Date		:23.Apr.2015
 * Module	:startScan
 * Description:It initializes the RM_ScanHandle data structure passed as an
argument along with it
 */
// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	scan->rel = rel;
	scan->mgmtData = cond;
    return RC_OK;
}

/*
 * Author	:Karthik
 * Date		:23.Apr.2015
 * Module	:storeSemiColonPosition
 * Description: It stores the semicolon position
 */
void storeSemiColonPostion(char * record,int *p)
{
	int i;
	int j = 0;
	for(i = 0 ; i < strlen(record); i++)
		if(record[i] == ';')
			p[j++] = i;
}
/*
 * Author	:Karthik
 * Date		:23.Apr.2015
 * Module	:getColumndata
 * Description:It gets the columndata.
 */
void getColumnData(int columnNum,char * record,int *semiColonPosition,char * cellValue)
{
	int cellStartIndex;

	if(columnNum == 0)
		cellStartIndex = 0;
	else
		cellStartIndex = semiColonPosition[columnNum - 1] + 1;

	int i;
	int j = 0;
	for(i = cellStartIndex ; i < semiColonPosition[columnNum];i++)
		cellValue[j++] = record[i];

}
/*
 * Author	:Unnikrishnan
 * Date		:23.Apr.2015
 * Module	:next
 * Description:It returns the next tuple that fullfills the scan condition,
if NULL is passed as an scan condition,then all tuples of the table should is returned.
This function return RC_RM_NO_MORE_TUPLES once the scan is completed and RC_OK otherwise
unless an error occurs
 */
RC next (RM_ScanHandle *scan, Record *record)
{
	if(fh_rec_mgr.fileName == NULL)
		openPageFile(scan->rel->name,&fh_rec_mgr);

	Expr *expression = (Expr *) scan->mgmtData;
	while(currentScannedRecord < fh_rec_mgr.totalNumPages - 2)
	{
		SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);
		memset(ph,'\0',PAGE_SIZE);

		readBlock(currentScannedRecord+2,&fh_rec_mgr,ph);

		char cellValue[PAGE_SIZE];
		memset(cellValue,'\0',PAGE_SIZE);

		int semiColonPosition[3];  //scan->rel->schema->numAttr
		storeSemiColonPostion(ph,semiColonPosition);

		if(expression->expr.op->type == OP_COMP_EQUAL)
		{
			getColumnData(expression->expr.op->args[1]->expr.attrRef,ph,semiColonPosition,cellValue);
			if(expression->expr.op->args[0]->expr.cons->dt == DT_INT)
			{
				if(atoi(cellValue) == expression->expr.op->args[0]->expr.cons->v.intV)
				{
					strcpy(record->data,ph);
					currentScannedRecord++;
					free(ph);
					return RC_OK;
				}
			}
			else if(expression->expr.op->args[0]->expr.cons->dt == DT_STRING)
			{
				if(strcmp(cellValue,expression->expr.op->args[0]->expr.cons->v.stringV) == 0)
				{
					strcpy(record->data,ph);
					currentScannedRecord++;
					free(ph);
					return RC_OK;
				}
			}
		}
		else if(expression->expr.op->type == OP_BOOL_NOT)
		{
			getColumnData(expression->expr.op->args[0]->expr.op->args[0]->expr.attrRef,ph,semiColonPosition,cellValue);
			if(expression->expr.op->args[0]->expr.op[0].args[1]->expr.cons->v.intV <= atoi(cellValue))
			{
				strcpy(record->data,ph);
				currentScannedRecord++;
				free(ph);
				return RC_OK;
			}
		}
		currentScannedRecord++;
		free(ph);
	}
	currentScannedRecord = 0; // For next scan, this global variable has to be reset..
	return RC_RM_NO_MORE_TUPLES;
}
/*
 * Author	:Unnikrishnan
 * Date		:23.Apr.2015
 * Module	:closeScan
 * Description:It indicates to the record manager that all associated
resources can be cleaned up
 */
RC closeScan (RM_ScanHandle *scan)
{
    return RC_OK;
}

/*
 * Author	:Karthik
 * Date		:24.Apr.2015
 * Module	:getRecordSize
 * Description:It returns the size in bytes of record for a given schema
 */
// dealing with schemas
int getRecordSize (Schema *schema)
{
    int size = 0;
    int i;
    for (i = 0; i < schema->numAttr; i ++) {
        switch(schema->dataTypes[i]) {
            case DT_INT:
                size += sizeof(int);
                break;
            case DT_FLOAT:
                size += sizeof(float);
                break;
            case DT_BOOL:
                size += sizeof(bool);
                break;
            case DT_STRING:
                size += schema->typeLength[i];
                break;
        }
    }
    size = 8;
    return size;
}
/*
 * Author	:Karthik
 * Date		:24.Apr.2015
 * Module	:*createSchema
 * Description:it creates a new schema
 */
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	Schema *schema = (Schema *)malloc(sizeof(Schema));
	//free above schema isn't required because it will be freed during table DS free.

	schema->numAttr = numAttr;
	schema->attrNames = attrNames;
	schema->dataTypes = dataTypes;
	schema->typeLength = typeLength;
	schema->keyAttrs = keys;
	schema->keySize = keySize;

	return schema;
}
/*
 * Author	:Karthik
 * Date		:24.Apr.2015
 * Module	:freeSchema
 * Description: It free the schema
 */
RC freeSchema (Schema *schema)
{
    free(schema);
    
    return RC_OK;
}

/*
 * Author	:Karthik
 * Date		:24.Apr.2015
 * Module	:createRecord
 * Description:it creates a new record for a given schema and it allocates
enough memory to the data field to hold the binary representation for all attributes
of this record as determined by the schema
 */
// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema)
{
	*record = (Record *)malloc(sizeof(Record));
	//may be for record's data part,memory has to be allocated here..
	//(sizeof(int) + (sizeof(char)*4)+sizeof(int)+(sizeof(char)*10)
	 (*record)->data = (char *)malloc(PAGE_SIZE);
	 memset((*record)->data,'\0',PAGE_SIZE);

	 if(record != NULL)
		 return RC_OK;
	 else
		 return RC_CREATE_FAILED;
}
/*
 * Author	:Karthik
 * Date		:24.Apr.2015
 * Module	:freeRecord
 * Description:free a specified record
 */
RC freeRecord (Record *record)
{
    free(record);
    return RC_OK;
}
/*
 * Author	:Aditya
 * Date		:24.Apr.2015
 * Module	:getAttr
 * Description:It gets the attribute values of a record
 */
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
    char temp[PAGE_SIZE + 1];
    int tempCounter = 1;
    int dataTypeCounter = 0;

    memset(temp,'\0',PAGE_SIZE + 1);

    *value = (Value *)malloc(sizeof(Value) * schema->numAttr);
    
    int i;
    for (i = 0; i < PAGE_SIZE; i ++)
    {
        if ((record->data[i] == ';') || (record->data[i] == '\0'))
        {
            if (attrNum == dataTypeCounter) {

                switch (schema->dataTypes[dataTypeCounter]) {
                    case DT_INT:
                        temp[0] = 'i';
                        break;
                    case DT_FLOAT:
                        temp[0] = 'f';
                        break;
                    case DT_BOOL:
                        temp[0] = 'b';
                        break;
                    case DT_STRING:
                        temp[0] = 's';
                        break;
                }

                *value = stringToValue(temp);
                break;
            }
            dataTypeCounter ++;
            tempCounter = 1;
            memset(temp,'\0',PAGE_SIZE + 1);
        }
        else {
            temp[tempCounter ++] = record->data[i];
        }
    }
    return RC_OK;
}
/*
 * Author	:Karthik
 * Date		:24.Apr.2015
 * Module	:setAttr
 * Description:It sets the attribute values of a record
 */
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	 char * serializedColumnValue = serializeValue(value);

	 int noOfSemiColons = 0;
	 int i;
	 for(i = 0; i < strlen(record->data) ; i++)
		 if(record->data[i] == ';')
			 noOfSemiColons++;

	 if(noOfSemiColons == schema->numAttr)
	 {
		 int semiColonPosition[schema->numAttr];

		 int j = 0;
		 for(i = 0; i < strlen(record->data) ; i++)
			 if(record->data[i] == ';')
				 semiColonPosition[j++] = i;

		 if( attrNum == 0)
		 {
			 char endString[PAGE_SIZE];
			 memset(endString,'\0',PAGE_SIZE);
			 i = 0;
			 int j;
			 for(j = semiColonPosition[attrNum] ; j < strlen(record->data) ; j++ )
			 {
				 endString[i++] = record->data[j];
			 }
			 endString[i] = '\0';//here

			 memset(record->data,'\0',PAGE_SIZE);

			 strcpy(record->data,serializedColumnValue);
			 strcpy(record->data,endString);
		 }
		 else
		 {
			 char firstString[PAGE_SIZE];
			 char endString[PAGE_SIZE];

			 memset(firstString,'\0',PAGE_SIZE);
			 memset(endString,'\0',PAGE_SIZE);

			 for(i = 0 ; i <= semiColonPosition[attrNum - 1]; i++)
			 {
				 firstString[i] = record->data[i];
			 }
			 firstString[i] = '\0';

			 int j = 0;
			 for(i = semiColonPosition[attrNum] ; i < strlen(record->data) ; i++)
			 {
				 endString[j++] = record->data[i];
			 }
			 endString[j] = '\0';

			 strcat(firstString,serializedColumnValue);
			 strcat(firstString,endString);

			 memset(record->data,'\0',PAGE_SIZE);

			 strcpy(record->data,firstString);
		 }
	 }
	 else
	 {
		 strcat(record->data,serializedColumnValue);
		 strcat(record->data,";");
	 }

	 if(serializedColumnValue != NULL)
		 return RC_OK;
	 else
		 return RC_SET_ATTR_FAILED;
}

