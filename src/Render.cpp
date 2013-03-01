// ============================================================================
// Rendergrid.cpp
// ============================================================================


#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <math.h>
#include <cstdio>
#include <cstring>
#include "SphParticle.h"
#include "Sph.h"
#include "SphSnapshot.h"
#include "SphKernel.h"
#include "Exception.h"
#include "Parameters.h"
#include "InlineFuncs.h"
#include "Debug.h"
#include "Render.h"
using namespace std;



// ============================================================================
// Render::Render
// ============================================================================
Render::Render()
{
}



// ============================================================================
// Render::~Render
// ============================================================================
Render::~Render()
{
}




// ============================================================================
// Render::CreateRenderingGrid
// ============================================================================
int Render::CreateRenderingGrid(int ixgrid, int iygrid, string xstring, 
				string ystring, string renderstring, string renderunit,
				float &scaling_factor, float xmin, float xmax,
				float ymin, float ymax, float **gridvalues,
				SphSnapshot &snap, Sph *sph)
{
  int arraycheck = 1;
  int c;
  int i;
  int j;
  int k;
  int idummy;
  int Ngrid = ixgrid*iygrid;
  float dr[2];
  float drsqd;
  float drmag;
  float *xvalues;
  float *yvalues;
  float *rendervalues;
  float *mvalues;
  float *rhovalues;
  float *hvalues;
  float *rendernorm;
  float wnorm;
  float invh;
  float wkern;
  float hrangesqd;
  string dummystring = "";
  float dummyfloat = 0.0;
  float *rgrid;
  float *values = *gridvalues;

  int ndim = snap.ndim;

  // First, verify x, y and render strings are valid
  snap.ExtractArray(xstring,&xvalues,&idummy,dummyfloat,dummystring); arraycheck *= idummy;
  snap.ExtractArray(ystring,&yvalues,&idummy,dummyfloat,dummystring); arraycheck *= idummy;
  snap.ExtractArray(renderstring,&rendervalues,&idummy,scaling_factor,renderunit); arraycheck *= idummy;
  snap.ExtractArray("m",&mvalues,&idummy,dummyfloat,dummystring); arraycheck *= idummy;
  snap.ExtractArray("rho",&rhovalues,&idummy,dummyfloat,dummystring); arraycheck *= idummy;
  snap.ExtractArray("h",&hvalues,&idummy,dummyfloat,dummystring); arraycheck *= idummy;

  // If any are invalid, exit here with failure code
  if (arraycheck == 0) return -1;

  rendernorm = new float[snap.Nsph];

  // Create grid positions here
  c = 0;
  rgrid = new float[2*Ngrid];
  for (i=0; i<ixgrid; i++) {
	  for (j=0; j<iygrid; j++) {
		  rgrid[2*c] = xmin + ((float) i + 0.5f)*(xmax - xmin)/(float)ixgrid;
		  rgrid[2*c + 1] = ymin + ((float) j + 0.5f)*(ymax - ymin)/(float)iygrid;
	  }
  }

  for (c=0; c<Ngrid; c++) values[c] = 0.0f;
  for (c=0; c<Ngrid; c++) rendernorm[c] = 0.0f;


  // Loop over all particles in snapshot
  // -----------------------------------------------------------------------------
  for (i=0; i<snap.Nsph; i++) {

    invh = 1.0f/hvalues[i];
    wnorm = mvalues[i]/rhovalues[i]*pow(invh,ndim);
    hrangesqd = sph->kernp->kernrangesqd*hvalues[i]*hvalues[i];

    // Now loop over all pixels and add current particles
    // ---------------------------------------------------------------------------
    for (c=0; c<Ngrid; c++) {

      dr[0] = rgrid[2*c] - xvalues[i];
      dr[1] = rgrid[2*c + 1] - yvalues[i];
      drsqd = dr[0]*dr[0] + dr[1]*dr[1];
      
      if (drsqd > hrangesqd) continue;

      drmag = sqrt(hrangesqd);
      wkern = float(sph->kernp->w0((FLOAT) (drmag*invh)));

      values[c] += wnorm*rendervalues[i]*wkern;
      rendernorm[c] += wnorm*wkern;
    }
    // ---------------------------------------------------------------------------

  }
  // -----------------------------------------------------------------------------

  // Normalise all grid cells
  for (c=0; c<Ngrid; c++)
    if (rendernorm[c] > 1.e-10) values[c] /= rendernorm[c];


  return 1;
}
