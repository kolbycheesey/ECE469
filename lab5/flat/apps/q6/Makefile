default:
	cd q6test; make

clean:
	cd q6test; make clean

run:
	cd ../../os; make; cd ../bin; dlxsim -x os.dlx.obj -a -u q6test.dlx.obj; ee469_fixterminal
