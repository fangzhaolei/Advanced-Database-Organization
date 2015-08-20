________________________________________________________________________________
Assignment 2: Buffer Manager
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
1. Change the directory to the assignemnt directory.(i.e. assignment2)
2. Issue "make" in the terminal to make the Object files.
3. For Mandatory test cases issue:
	./test_assign2_1
4. For Additional test cases issue:
	./test_assign2_2 
5. Issue "make clean" in the terminal to clean all the object files.
________________________________________________________________________________

________________________________________________________________________________

2)Additional functionalities
----------------------------

1.We have implemented additional page replacement strategy 
  		a. LFU(Least Frequently used).
  		b. CLOCK
2.We have included additional error checks.
3.We have ensured that program has 0 memory leaks.
________________________________________________________________________________

________________________________________________________________________________

3)Description of functions used
-------------------------------

                ------------------------- 
		//Buffer Pool Functions//
		-------------------------
	
	initBufferPool Function
	-----------------------
	Creates a buffer pool of size numPages. Creates a Map ramPageMap 
to store frame and their corresponding disc PageNumbers.Initially the Map 
will have -1 as the discPageNumbers.Creates a Map pageAndDirtyBitIndicator 
to store frame and their corresponding dirty Bit. Creates a Map 
pageAndFixCount to store frame and their corresponding fix count.


	shutdownBufferPool Function
	---------------------------
	It destroys the buffer pool. It frees all the data structures that 
is ramPageMap, pageAndDirtyBitIndicator and pageAndFixCount used to 
implement this project.

	forceFlushPool Function
	-----------------------
	It writes the content of the frame to the page file on disk , if 
that frame is set as dirty.


                -----------------------------
		//Page Management Functions//
		----------------------------

	pinPage Function
	----------------
	It pins the page with page number pageNum. If the page is already 
on the buffer pool then return that frame. If the page is not the buffer 
pool and there is a empty frame, then copy the page from the disk to the 
buffer pool. If there is no empty frame, then use required page replacement 
strategy to replace the page from the memory. Data structures are updated 
every time a page is pinned.
	
	unpinPage Function
	------------------
	It unpins the page page. All the data structures are updated every 
time a page is unpinned. 
	
	markDirty Function
	------------------
	It marks a page as dirty. The data structure pageAndDirtyBitIndicator 
is updated.

	forcePage Function
	------------------
	It writes the current content of the page back to the page file 
on disk.


                ------------------------
		//Statistics Functions//
		-----------------------

	getFrameContents Function
	-------------------------
	This function returns an array of PageNumbers (of size numPages) 
where the ith element is the number of the page stored in the ith page frame. 
An empty page frame is represented using the constant NO_PAGE.

	getDirtyFlags Function
	----------------------
	This function returns an array of bools (of size numPages) where the 
ith element is TRUE if the page stored in the ith page frame is dirty. Empty
page frames are considered as clean.

	getFixCounts Function	
	---------------------
	This function returns an array of ints (of size numPages) where the 
ith element is the fix count of the page stored in the ith page frame.Return 
0 for empty page frames.

	getNumReadIO Function
	---------------------
	This function returns the number of pages that have been read from 
disk since a buffer pool has been initialized. 

	getNumWriteIO Function
	----------------------
	It returns the number of pages written to the page file since the 
buffer pool has been initialized.	


________________________________________________________________________________

________________________________________________________________________________

4)Additional Test Cases and Additional error checks
---------------------------------------------------

	Test Cases
	----------

  a) We have tested Least Frequency Used (LFU) algorithm.
  b) We have tested CLOCK alogorithm.
  b) We have included test_assign2_2.c which tests :	
						1. CLOCK
						2. LFU
	
	
	Error Checks
	------------

	The following are the additional error checks.

  a) #define RC_BUFFER_POOL_INIT_ERROR 500  //  Buffer Initialization error
  b) #define RC_NO_FRAME 501	       	    //  No free frames RC
  c) #define RC_DIRTY_UPDATE_FAILED 502     //  Dirty update failure
  d) #define RC_UNPIN_FAILED 503            //  Unpin Failure error
  e) #define RC_SHUT_DOWN_ERROR 504	    //  Buffer Pool Shutdown error

________________________________________________________________________________

