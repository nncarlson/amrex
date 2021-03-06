#include <AMReX_REAL.H>
#include "AMReX_LO_BCTYPES.H"

subroutine set_rhs(rhs, r_l1, r_l2, r_h1, r_h2, &
                   lo, hi, dx, a, b, sigma, w, ibnd) bind(C, name="set_rhs")

  implicit none

  integer          :: lo(2), hi(2), ibnd
  integer          :: r_l1, r_l2, r_h1, r_h2
  double precision :: rhs(r_l1:r_h1, r_l2:r_h2)
  double precision ::  dx(2), a, b, sigma, w

  integer i,j
  REAL_T  x,y,r, theta, beta, dbdrfac
  REAL_T  pi, fpi, tpi, fac

  pi = 4.d0 * datan(1.d0)
  tpi = 2.0d0 * pi
  fpi = 4.0d0 * pi
  fac = 2.0d0 * tpi**2
 
  theta = 0.5d0*log(3.0) / (w + 1.d-50)

  do j = lo(2), hi(2)
     y = (dble(j)+0.5d0)*dx(2)

     do i = lo(1), hi(1)
        x = (dble(i)+0.5d0)*dx(1)
            
        r = sqrt((x-0.5d0)**2 + (y-0.5d0)**2)

        beta = (sigma-1.d0)/2.d0*tanh(theta*(r-0.25d0)) + (sigma+1.d0)/2.d0
        beta = beta * b
        dbdrfac = (sigma-1.d0)/2.d0/(cosh(theta*(r-0.25d0)))**2 * theta/r
        dbdrfac = dbdrfac * b
        
        if (ibnd .eq. 0 .or. ibnd.eq. LO_NEUMANN) then

           rhs(i,j) = beta*fac*(cos(tpi*x) * cos(tpi*y) &
                              + cos(fpi*x) * cos(fpi*y)) &
                  + dbdrfac*((x-0.5d0)*(tpi*sin(tpi*x) * cos(tpi*y) &
                                       + pi*sin(fpi*x) * cos(fpi*y)) &
                           + (y-0.5d0)*(tpi*cos(tpi*x) * sin(tpi*y) &  
                                       + pi*cos(fpi*x) * sin(fpi*y))) &
                  + a * (                   cos(tpi*x) * cos(tpi*y) &
                                 + 0.25d0 * cos(fpi*x) * cos(fpi*y))

        else if (ibnd .eq. LO_DIRICHLET) then

           rhs(i,j) = beta*fac*(sin(tpi*x) * sin(tpi*y) &
                              + sin(fpi*x) * sin(fpi*y)) &
              + dbdrfac*((x-0.5d0)*(-tpi*cos(tpi*x) * sin(tpi*y) &  
                                    - pi*cos(fpi*x) * sin(fpi*y)) &
                       + (y-0.5d0)*(-tpi*sin(tpi*x) * cos(tpi*y) &  
                                    - pi*sin(fpi*x) * cos(fpi*y))) &
              + a * (                    sin(tpi*x) * sin(tpi*y) &
                              + 0.25d0 * sin(fpi*x) * sin(fpi*y))
        else
           print *, 'FORT_SET_RHS: unknown boundary type'
           stop
        endif
     end do
  end do
      
end subroutine set_rhs
