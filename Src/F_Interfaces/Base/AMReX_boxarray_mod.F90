
module amrex_boxarray_module

  use iso_c_binding

  use amrex_box_module

  implicit none

  private

  public :: amrex_boxarray_build, amrex_boxarray_destroy, amrex_print, amrex_allprint

  type, public :: amrex_boxarray
     logical     :: owner = .false.
     type(c_ptr) :: p = c_null_ptr
   contains
     generic   :: assignment(=) => amrex_boxarray_assign   ! shallow copy
     procedure :: clone         => amrex_boxarray_clone    ! deep copy
     procedure :: move          => amrex_boxarray_move     ! transfer ownership
     procedure :: maxSize       => amrex_boxarray_maxSize  ! make the boxes smaller
     procedure, private :: amrex_boxarray_assign
#if !defined(__GFORTRAN__) || (__GNUC__ > 4)
     final :: amrex_boxarray_destroy
#endif
  end type amrex_boxarray

  interface amrex_boxarray_build
     module procedure amrex_boxarray_build_bx
  end interface amrex_boxarray_build

  interface amrex_print
     module procedure amrex_boxarray_print
  end interface amrex_print

  interface amrex_allprint
     module procedure amrex_boxarray_allprint
  end interface amrex_allprint

  ! interfaces to cpp functions

  interface
     subroutine amrex_fi_new_boxarray (ba,lo,hi) bind(c)
       import
       type(c_ptr) :: ba
       integer(c_int), intent(in) :: lo(3), hi(3)
     end subroutine amrex_fi_new_boxarray

     subroutine amrex_fi_delete_boxarray (ba) bind(c)
       import
       type(c_ptr), value :: ba
     end subroutine amrex_fi_delete_boxarray

     subroutine amrex_fi_clone_boxarray (bao, bai) bind(c)
       import
       type(c_ptr) :: bao
       type(c_ptr), value :: bai
     end subroutine amrex_fi_clone_boxarray

     subroutine amrex_fi_boxarray_maxsize (ba,n) bind(c)
       import
       type(c_ptr), value :: ba
       integer(c_int), value :: n
     end subroutine amrex_fi_boxarray_maxsize

     subroutine amrex_fi_print_boxarray (ba, all) bind(c)
       import
       type(c_ptr), value :: ba
       integer(c_int), value :: all
     end subroutine amrex_fi_print_boxarray
  end interface

contains

  subroutine amrex_boxarray_build_bx (ba, bx)
    type(amrex_boxarray) :: ba
    type(amrex_box), intent(in ) :: bx
    ba%owner = .true.
    call amrex_fi_new_boxarray(ba%p, bx%lo, bx%hi)
  end subroutine amrex_boxarray_build_bx

  impure elemental subroutine amrex_boxarray_destroy (this)
    type(amrex_boxarray), intent(inout) :: this
    if (this%owner) then
       if (c_associated(this%p)) then
          call amrex_fi_delete_boxarray(this%p)
       end if
    end if
    this%owner = .false.
    this%p = c_null_ptr
  end subroutine amrex_boxarray_destroy

  subroutine amrex_boxarray_assign (dst, src)
    class(amrex_boxarray), intent(inout) :: dst
    type (amrex_boxarray), intent(in   ) :: src
    call amrex_boxarray_destroy(dst)
    dst%owner = .false.
    dst%p = src%p
  end subroutine amrex_boxarray_assign

  subroutine amrex_boxarray_clone (dst, src)
    class(amrex_boxarray), intent(inout) :: dst
    type (amrex_boxarray), intent(in   ) :: src
    dst%owner = .true.
    call amrex_fi_clone_boxarray(dst%p, src%p)
  end subroutine amrex_boxarray_clone

  subroutine amrex_boxarray_move (dst, src)
    class(amrex_boxarray) :: dst, src
    call amrex_boxarray_destroy(dst)
    dst%owner = src%owner
    dst%p = src%p
    src%owner = .false.
    src%p = c_null_ptr
  end subroutine amrex_boxarray_move

  subroutine amrex_boxarray_maxsize (this, sz)
    class(amrex_boxarray) this
    integer, intent(in)    :: sz
    call amrex_fi_boxarray_maxsize(this%p, sz)
  end subroutine amrex_boxarray_maxsize

  subroutine amrex_boxarray_print (ba)
    type(amrex_boxarray), intent(in) :: ba
    call amrex_fi_print_boxarray(ba%p, 0)
  end subroutine amrex_boxarray_print

  subroutine amrex_boxarray_allprint (ba)
    type(amrex_boxarray), intent(in) :: ba
    call amrex_fi_print_boxarray(ba%p, 1)
  end subroutine amrex_boxarray_allprint

end module amrex_boxarray_module