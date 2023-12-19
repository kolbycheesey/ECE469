default:
	cd userprog; make

clean:
	cd userprog; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -D t -u userprog.dlx.obj 1 3; ee469_fixterminal
