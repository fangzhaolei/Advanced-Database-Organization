
Steps to execute Storage Manager ::
_______________________________________

1. Make the assignment by issuing ‘make’
2. The ‘make’ will result in two object files which are test_assign1 and test_assign1_2
3. test_assign1_2 are additional test cases written by us.


Memory leaks found in test_assign1_1.c at 2 places :
1. closePageFile() was not being invoked in testSinglePageContent() in the file test_assign1_1.c
2. free was not called upon the page handle - ph.
