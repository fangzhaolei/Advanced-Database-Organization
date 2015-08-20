________________________________________________________________________________
Assignment 3: Record Manager

Team: 
	1. Karthik Bangalore Mani
	2. Vivek Basavarajappa
	3. Aditya Shirodkar
	4. Unnikrishnan Sasikumar

________________________________________________________________________________



________________________________________________________________________________



Contents:

---------



1)Instructions to run the code

2)Additional functionalities

3)Description of functions used

4)Additional Test Cases and Additional error checks

________________________________________________________________________________



________________________________________________________________________________



1)Instructions to run the code

-------------------------------



For Executing Test Cases:

1. Change the directory to the assignemnt directory.(i.e. assignment3)

2. Issue "make" in the terminal to make the Object files.

3. For test cases issue:

	./test_assign3
	./test_expr

4. Issue "make clean" in the terminal to clean all the object files.

________________________________________________________________________________


________________________________________________________________________________



2)Additional functionalities

----------------------------


1.We have implemented Tombstone concepts.
	For Tombstones ,we are grouping the RIDS of the records.
			we are using these groups of RIDS to flush the records at Bulk during CloseTable().
2.We have included additional error checks.


________________________________________________________________________________


________________________________________________________________________________



3)Description of functions used

-------------------------------


		--------------------------------------

		//Table and Record Manager Functions//
		--------------------------------------	

	initRecordManager Function
	--------------------------              	
		This Function initializes the record manager.
		
	shutdownRecordManager Function
	------------------------------
		This Function shutdowns the record manager.

	createTable Function
	--------------------
		This Functions creates a table. It creates the underlying page file 
and store information about the schema, free-space and so on in the table information pages.
		
	openTable Function
	------------------
		This Function is used to open a table.

	closeTable Function
	-------------------
		This Function is used to close a table. It causes all outstanding 
changes to the tables to be written to the page file. 

	deleteTable Function
	--------------------
		This Function deletes the table.
	
	getNumTuples Function
	---------------------
		This Function returns the number of tuples in the table.


		--------------------
		//Record Functions//
		--------------------

	insertRecord Function
	---------------------
		This Function inserts a new record.This newly inserted record will
be assigned a RID and the record parameter passed is updated. 

	deleteRecord Function
	---------------------
		This Function deletes the record with a certain RID passed as an parameter.

	updateRecord Function
	---------------------
		This Function updates a existing record with new values.

	getRecord Function
	------------------
		This Function retrieve a record with certain RID passed as an parameter.


		------------------
		//Scan Functions//
		------------------

	startScan Function
	------------------
		This Function initializes the RM_ScanHandle data structure passed as an
argument along with it.

	next Function
	-------------
		This Function returns the next tuple that fullfills the scan condition,
if NULL is passed as an scan condition,then all tuples of the table should is returned.
This function return RC_RM_NO_MORE_TUPLES once the scan is completed and RC_OK otherwise 
unless an error occurs. 

	closeScan Function
	------------------
		This Function indicates to the record manager that all associated 
resources can be cleaned up.

		--------------------
		//Schema Functions//
		--------------------

	getRecordSize Function
	----------------------
		This Function returns the size in bytes of record for a given schema.

	createSchema Function
	---------------------
		This Function creates a new Schema.

	freeSchema Function
	-------------------
		This function frees a given schema.
	

		-----------------------
		//Attribute Functions//
		-----------------------

	createRecord Function
	---------------------
		This Function creates a new record for a given schema and it allocates 
enough memory to the data field to hold the binary representation for all attributes 
of this record as determined by the schema.

	freeRecord Function
	-------------------
		This Function frees a specified record.

	getAttr Function
	----------------
		This Function gets the attribute values of a record.

	setAttr Function
	----------------
		This Function sets the attribute values of a record.
	



________________________________________________________________________________


________________________________________________________________________________



4)Additional error checks

---------------------------------------------------

1. RC_OPEN_TABLE_FAILED
2. RC_INSERT_ERROR
3. RC_CREATE_FAILED
4. RC_SET_ATTR_FAILED



  



________________________________________________________________________________



