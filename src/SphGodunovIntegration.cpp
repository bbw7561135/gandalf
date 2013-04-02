// ============================================================================
// SphGodunovIntegration.cpp
// ..
// ============================================================================


#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include "Dimensions.h"
#include "Sph.h"
#include "SphKernel.h"
#include "SphIntegration.h"
#include "SphParticle.h"
#include "EOS.h"
#include "Debug.h"
using namespace std;



// ============================================================================
// SphGodunovIntegration::SphGodunovIntegration()
// ============================================================================
template <int ndim>
SphGodunovIntegration<ndim>::SphGodunovIntegration(
			       DOUBLE accel_mult_aux, 
			       DOUBLE courant_mult_aux) :
  SphIntegration<ndim>(accel_mult_aux, courant_mult_aux)
{
}



// ============================================================================
// SphGodunovIntegration::~SphGodunovIntegration()
// ============================================================================
template <int ndim>
SphGodunovIntegration<ndim>::~SphGodunovIntegration()
{
}



// ============================================================================
// SphGodunovIntegration::AdvanceParticles
// Integrate particle positions to 2nd order, and particle velocities to 1st
// order from the beginning of the step to the current simulation time, i.e. 
// r(t+dt) = r(t) + v(t)*dt + 0.5*a(t)*dt^2, 
// v(t+dt) = v(t) + a(t)*dt.
// Also set particles at the end of step as 'active' in order to compute 
// the end-of-step force computation.
// ============================================================================
template <int ndim>
void SphGodunovIntegration<ndim>::AdvanceParticles(int n, int level_step, int Nsph,
				      SphParticle<ndim> *sph, FLOAT timestep)
{
  int i;                                // Particle counter
  int k;                                // Dimension counter
  int nstep;                            // Particle (integer) step size
  FLOAT dt;                             // Timestep since start of step

  debug2("[SphGodunovIntegration::AdvanceParticles]");

  // --------------------------------------------------------------------------
#pragma omp parallel for default(shared) private(dt,k,nstep)
  for (i=0; i<Nsph; i++) {

    // Compute time since beginning of current step
    nstep = pow(2,level_step - sph[i].level);
    if (n%nstep == 0) dt = timestep*(FLOAT) nstep;
    else dt = timestep*(FLOAT) (n%nstep);

    // Advance particle positions and velocities
    for (k=0; k<ndim; k++) sph[i].r[k] = sph[i].r0[k] + 
      (sph[i].v0[k] + 0.5*sph[i].a[k]*sph[i].dt)*dt;
    for (k=0; k<vdim; k++) sph[i].v[k] = sph[i].v0[k] + sph[i].a0[k]*dt;

    // Set particle as active at end of step
    if (n%nstep == 0) sph[i].active = true;
    else sph[i].active = false;
  }
  // --------------------------------------------------------------------------

  return;
}
 


// ============================================================================
// SphGodunovIntegration::CorrectionTerms
// No correction terms to apply.
// ============================================================================
template <int ndim>
void SphGodunovIntegration<ndim>::CorrectionTerms(int n, int level_step, int Nsph,
					    SphParticle<ndim> *sph, FLOAT timestep)
{
  return;
}



// ============================================================================
// SphGodunovIntegration::EndTimestep
// Record all important SPH particle quantities at the end of the step for the 
// start of the new timestep.
// ============================================================================
template <int ndim>
void SphGodunovIntegration<ndim>::EndTimestep(int n, int level_step,
				 int Nsph, SphParticle<ndim> *sph)
{
  int i;                                // Particle counter
  int k;                                // Dimension counter
  int nstep;                            // Particle (integer) step size

  debug2("[SphGodunovIntegration::EndTimestep]");

  // --------------------------------------------------------------------------
#pragma omp parallel for default(shared) private(k,nstep)
  for (i=0; i<Nsph; i++) {
    nstep = pow(2,level_step - sph[i].level);
    if (n%nstep == 0) {
      for (k=0; k<ndim; k++) sph[i].r0[k] = sph[i].r[k];
      for (k=0; k<ndim; k++) sph[i].v0[k] = sph[i].v[k];
      for (k=0; k<ndim; k++) sph[i].a0[k] = sph[i].a[k];
      //sph[i].active = false;
      sph[i].active = true;
    }
  }
  // --------------------------------------------------------------------------

  return;
}

// ============================================================================
// SphIntegration::Timestep
// Default timestep size for SPH particles.  Takes the minimum of : 
// (i)  const*h/(sound_speed + h*|div_v|)    (Courant condition)
// (ii) const*sqrt(h/|a|)                    (Acceleration condition)
// ============================================================================
template <int ndim>
DOUBLE SphGodunovIntegration<ndim>::Timestep(SphParticle<ndim> &part, int hydro_forces)
{
  DOUBLE timestep;
  //DOUBLE amag;

  // Courant condition
  timestep = this->courant_mult*part.h/(part.sound + small_number_dp);

  // Local convergence/divergence condition
  timestep = min(timestep,this->courant_mult/(fabs(part.div_v) + small_number_dp));

  //Acceleration condition
  //amag = sqrt(DotProduct(part.a,part.a,ndim));
  //timestep = min(timestep, accel_mult*sqrt(part.h/(amag + small_number_dp)));

  return timestep;
}

template class SphGodunovIntegration<1>;
template class SphGodunovIntegration<2>;
template class SphGodunovIntegration<3>;