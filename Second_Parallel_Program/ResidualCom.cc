void computeResidual(float *presid, float *uresid, float *vresid, float *wresid,
		     const float *p,
		     const float *u, const float *v, const float *w,
		     float eta, float nu, float dx, float dy, float dz,
		     int ni, int nj, int nk, int kstart,
		     int iskip, int jskip) {
  // iskip is 1
  // i dimension goes in the +x coordinate direction
  // j dimension goes in the +y coordinate direction
  // k dimension goes in the +z coordinate direction
  const int kskip=1 ;
  // Loop through i faces of the mesh and compute fluxes in x direction
  // Add fluxes to cells that neighbor face
  omp_set_max_active_levels(3);
  #pragma omp parallel for
  for(int i=0;i<ni+1;++i) {
    const float vcoef = nu/dx ;
    const float area = dy*dz ;
    #pragma omp parallel for
    for(int j=0;j<nj;++j) {
      //Possible collapse?
      int offset = kstart+i*iskip+j*jskip;
      #pragma omp parallel for reduction(*:pflux)
      for(int k=0;k<nk;++k) {
	const int indx = k+offset ;
	// Compute the x direction inviscid flux
	// extract pressures from the stencil
	float ull = u[indx-2*iskip] ;
	float ul  = u[indx-iskip] ;
	float ur  = u[indx] ;
	float urr = u[indx+iskip] ;

	float vll = v[indx-2*iskip] ;
	float vl  = v[indx-iskip] ;
	float vr  = v[indx] ;
	float vrr = v[indx+iskip] ;

	float wll = w[indx-2*iskip] ;
	float wl  = w[indx-iskip] ;
	float wr  = w[indx] ;
	float wrr = w[indx+iskip] ;

	float pll = p[indx-2*iskip] ;
	float pl  = p[indx-iskip] ;
	float pr  = p[indx] ;
	float prr = p[indx+iskip] ;
	float pterm = (2./3.)*(pl+pr) - (1./12.)*(pl+pr+pll+prr) ;
	// x direction so the flux will be a function of u
	float udotn1 = ul+ur ;
	float udotn2 = ul+urr ;
	float udotn3 = ull+ur ;
	float pflux = eta*((2./3.)*udotn1 - (1./12.)*(udotn2+udotn3)) ;
	float uflux = ((1./3.)*(ul+ur)*udotn1 -
			(1./24.)*((ul+urr)*udotn2 + (ull+ur)*udotn3) +
			pterm) ;
	float vflux = ((1./3.)*(vl+vr)*udotn1 -
		       (1./24.)*((vl+vrr)*udotn2 + (vll+vr)*udotn3)) ;

	float wflux = ((1./3.)*(wl+wr)*udotn1 -
		       (1./24.)*((wl+wrr)*udotn2 + (wll+wr)*udotn3)) ;

	// Add in viscous fluxes integrate over face area
  //Possible reduction for pflux?
	pflux *= area ;
	uflux = area*(uflux - vcoef*((5./4.)*(ur-ul) - (1./12.)*(urr-ull))) ;
	vflux = area*(vflux - vcoef*((5./4.)*(vr-vl) - (1./12.)*(vrr-vll))) ;
	wflux = area*(wflux - vcoef*((5./4.)*(wr-wl) - (1./12.)*(wrr-wll))) ;
  //This is the race condition
  //tile method, if statement to check for boundary
  //Loop over the 1D direction of the array, do not subtract at the beginning and do not add at the end of boundary.
  //loops for each direction?
  for(int z; z<8 ; z++){

  }
  //insert if(at "right" boundary, do not add, reverse for left boundary)
	presid[indx-iskip] -= pflux ;
	presid[indx] += pflux ;
	uresid[indx-iskip] -= uflux ;
	uresid[indx] += uflux ;
	vresid[indx-iskip] -= vflux ;
	vresid[indx] += vflux ;
	wresid[indx-iskip] -= wflux ;
	wresid[indx] += wflux ;
      }
    }
  }

  // Loop through j faces of the mesh and compute fluxes in y direction
  // Add fluxes to cells that neighbor face
  // Couldn't this also be collapsed?

  #pragma omp parallel for
  for(int i=0;i<ni;++i) {
    const float vcoef = nu/dy ;
    const float area = dx*dz ;
    #pragma omp parallel for
    for(int j=0;j<nj+1;++j) {
      int offset = kstart+i*iskip+j*jskip;
      #pragma omp parallel for
      for(int k=0;k<nk;++k) {
	const int indx = k+offset ;
	// Compute the y direction inviscid flux
	// extract pressures and velocity from the stencil
	float ull = u[indx-2*jskip] ;
	float ul  = u[indx-jskip] ;
	float ur  = u[indx] ;
	float urr = u[indx+jskip] ;

	float vll = v[indx-2*jskip] ;
	float vl  = v[indx-jskip] ;
	float vr  = v[indx] ;
	float vrr = v[indx+jskip] ;

	float wll = w[indx-2*jskip] ;
	float wl  = w[indx-jskip] ;
	float wr  = w[indx] ;
	float wrr = w[indx+jskip] ;

	float pll = p[indx-2*jskip] ;
	float pl  = p[indx-jskip] ;
	float pr  = p[indx] ;
	float prr = p[indx+jskip] ;
	float pterm = (2./3.)*(pl+pr) - (1./12.)*(pl+pr+pll+prr) ;

	// y direction so the flux will be a function of v
	float udotn1 = vl+vr ;
	float udotn2 = vl+vrr ;
	float udotn3 = vll+vr ;

	float pflux = eta*((2./3.)*udotn1 - (1./12.)*(udotn2+udotn3)) ;
	float uflux = ((1./3.)*(ul+ur)*udotn1 -
		       (1./24.)*((ul+urr)*udotn2 + (ull+ur)*udotn3)) ;

	float vflux = ((1./3.)*(vl+vr)*udotn1 -
		       (1./24.)*((vl+vrr)*udotn2 + (vll+vr)*udotn3)
		       +pterm) ;

	float wflux = ((1./3.)*(wl+wr)*udotn1 -
		       (1./24.)*((wl+wrr)*udotn2 + (wll+wr)*udotn3)) ;

	// Add in viscous fluxes integrate over face area
  // Another possible reduction?
	pflux *= area ;

	uflux = area*(uflux - vcoef*((5./4.)*(ur-ul) - (1./12.)*(urr-ull))) ;
	vflux = area*(vflux - vcoef*((5./4.)*(vr-vl) - (1./12.)*(vrr-vll))) ;
	wflux = area*(wflux - vcoef*((5./4.)*(wr-wl) - (1./12.)*(wrr-wll))) ;

	presid[indx-jskip] -= pflux ;
	presid[indx] += pflux ;
	uresid[indx-jskip] -= uflux ;
	uresid[indx] += uflux ;
	vresid[indx-jskip] -= vflux ;
	vresid[indx] += vflux ;
	wresid[indx-jskip] -= wflux ;
	wresid[indx] += wflux ;
      }
    }
  }
  // Loop through k faces of the mesh and compute fluxes in z direction
  // Add fluxes to cells that neighbor face
  // More possible collapses?
  #pragma omp parallel for
  for(int i=0;i<ni;++i) {

    const float vcoef = nu/dz ;
    const float area = dx*dy ;
    #pragma omp parallel for
    for(int j=0;j<nj;++j) {

      int offset = kstart+i*iskip+j*jskip;
      #pragma omp parallel for
      for(int k=0;k<nk+1;++k) {

	const int indx = k+offset ;
	// Compute the y direction inviscid flux
	// extract pressures and velocity from the stencil
	float ull = u[indx-2*kskip] ;
	float ul  = u[indx-kskip] ;
	float ur  = u[indx] ;
	float urr = u[indx+kskip] ;

	float vll = v[indx-2*kskip] ;
	float vl  = v[indx-kskip] ;
	float vr  = v[indx] ;
	float vrr = v[indx+kskip] ;

	float wll = w[indx-2*kskip] ;
	float wl  = w[indx-kskip] ;
	float wr  = w[indx] ;
	float wrr = w[indx+kskip] ;

	float pll = p[indx-2*kskip] ;
	float pl  = p[indx-kskip] ;
	float pr  = p[indx] ;
	float prr = p[indx+kskip] ;
	float pterm = (2./3.)*(pl+pr) - (1./12.)*(pl+pr+pll+prr) ;

	// y direction so the flux will be a function of v
	float udotn1 = wl+wr ;
	float udotn2 = wl+wrr ;
	float udotn3 = wll+wr ;

	float pflux = eta*((2./3.)*udotn1 - (1./12.)*(udotn2+udotn3)) ;
	float uflux = ((1./3.)*(ul+ur)*udotn1 -
		       (1./24.)*((ul+urr)*udotn2 + (ull+ur)*udotn3)) ;

	float vflux = ((1./3.)*(vl+vr)*udotn1 -
		       (1./24.)*((vl+vrr)*udotn2 + (vll+vr)*udotn3)) ;

	float wflux = ((1./3.)*(wl+wr)*udotn1 -
		       (1./24.)*((wl+wrr)*udotn2 + (wll+wr)*udotn3)
		       + pterm) ;

	// Add in viscous fluxes integrate over face area
  // Another possible reduction
	pflux *= area ;
	uflux = area*(uflux - vcoef*((5./4.)*(ur-ul) - (1./12.)*(urr-ull))) ;
	vflux = area*(vflux - vcoef*((5./4.)*(vr-vl) - (1./12.)*(vrr-vll))) ;
	wflux = area*(wflux - vcoef*((5./4.)*(wr-wl) - (1./12.)*(wrr-wll))) ;

	presid[indx-kskip] -= pflux ;
	presid[indx] += pflux ;
	uresid[indx-kskip] -= uflux ;
	uresid[indx] += uflux ;
	vresid[indx-kskip] -= vflux ;
	vresid[indx] += vflux ;
	wresid[indx-kskip] -= wflux ;
	wresid[indx] += wflux ;
      }
    }
  }
  omp_set_max_active_levels(1);
}