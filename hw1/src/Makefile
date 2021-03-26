All:
	gcc AcDc.c -o AcDc -Wall -O2
clean:
	rm AcDc output*
test:
	./AcDc ../test/sample5.ac output5
	./AcDc ../test/sample6.ac output6
	./AcDc ../test/sample7.ac output7
	./AcDc ../test/sample8.ac output8
	./AcDc ../test/test_constant_folding.ac output_const 
	./AcDc ../test/test_div_constant.ac output_div 