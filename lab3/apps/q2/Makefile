default:
	cd makeprocs; make
	cd producers; make
	cd consumers; make

clean:
	cd makeprocs; make clean
	cd producers; make clean
	cd producers; make clean

run:
	cd ../../bin; dlxsim -D b -x os.dlx.obj -a -u makeprocs.dlx.obj 1 4; ee469_fixterminal
