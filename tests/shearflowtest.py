#==============================================================================
# soundwavetest.py
# Run isothermal soundwave test using initial conditions file 'soundwave.dat'
#==============================================================================
from seren.analysis.facade import *
import time

# Create new simulation object and generate initial conditions
sim = newsim('shearflow.dat')
setupsim()

# Plot density distribution along with analytical solution
plot("y","vx")

# Use 'sleep' hack to refresh plot window.  Then run simulation.
time.sleep(1)
run()
block()
