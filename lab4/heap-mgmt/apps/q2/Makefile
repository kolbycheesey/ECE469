default:
	cd makeprocs; make
	cd spawn_me; make
	cd stack; make

clean:
	cd makeprocs; make clean
	cd spawn_me; make clean
	cd stack; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 5; ee469_fixterminal
