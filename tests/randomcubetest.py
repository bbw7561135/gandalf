from seren.analysis.facade import *
import time

newsim('randomcube.dat')
subfigure(2,2,1)
plot("x","y")
subfigure(2,2,2)
plot("x","rho")
subfigure(2,2,3)
plot("x","vy")
subfigure(2,2,4)
plot("x","u")
run()
block()


