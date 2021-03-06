/*
 *       {_       {__       {__{_______              {__      {__
 *      {_ __     {_ {__   {___{__    {__             {__   {__  
 *     {_  {__    {__ {__ { {__{__    {__     {__      {__ {__   
 *    {__   {__   {__  {__  {__{_ {__       {_   {__     {__     
 *   {______ {__  {__   {_  {__{__  {__    {_____ {__  {__ {__   
 *  {__       {__ {__       {__{__    {__  {_         {__   {__  
 * {__         {__{__       {__{__      {__  {____   {__      {__
 *
 */

#ifndef _BASEIFFAB_H_
#define _BASEIFFAB_H_

#include <cmath>
#include <cstdlib>
#include <vector>
#include "AMReX_IntVectSet.H"
#include "AMReX_FaceIndex.H"
#include "AMReX_BaseFab.H"
#include "AMReX_EBGraph.H"


namespace amrex
{

///
/**
   BaseIFFAB is a templated data holder
   defined over the Faces of an irregular domain.
*/
  template <class T>
  class BaseIFFAB
  {
  public:
    typedef T value_type;

///
/**
   Default constructor.  Constructs an uninitialized IFFAB.  You must
   subsequently call {\bf define} before using this IFFAB.
*/
    BaseIFFAB();

    
///
/**
   Defining constructor.  Specifies the irregular domain
   and the number of data components per face.  The irregular domain
   must lie completely within the EBGraph.  The
   contents are uninitialized.  Calls full define function.
*/
    BaseIFFAB(const IntVectSet& a_ivs,
              const EBGraph& a_ebgraph,
              const int& a_direction,
              const int& a_nvarin);

///
    ~BaseIFFAB();

///
/**
   Full define function.  Specifies the irregular domain and the
   number of data components per face.  The irregular domain
   must lie completely within the EBGraph.   The contents
   are uninitialized.  If it has previously been defined, the old definition
   data is overwritten and lost.
*/
    void define(const IntVectSet& a_ivs,
                const EBGraph& a_ebgraph,
                const int& a_direction,
                const int& a_nvarin);

///for AggStencil
    long offset(const FaceIndex& a_vof, const int& a_ivar) const
      {
        const T* conval = &((*this)(a_vof, a_ivar));
        long  roffset = (long)(conval-dataPtr(0));
        return roffset;
      }

///
/**
   Set a value at every data location.
*/
    void setVal(const T& value);

///
/**
   Set a value at every data location.
*/
    void setVal(int ivar, const T& value);

///
/**
   define over surrounding nodes of invectset (ivs is cell-centered).
*/
    BaseIFFAB<T>& copy(const BaseIFFAB<T> & src,
                       const Box&        srcbox,
                       int               srccomp,
                       const Box&        destbox,
                       int               destcomp,
                       int               numcomp);



    BaseIFFAB<T>& plus(const BaseIFFAB<T>& src,
                       const Box&        srcbox,
                       const Box&        destbox,
                       int               srccomp,
                       int               destcomp,
                       int               numcomp=1);
///
/**
   Remove all data from this BaseIFFAB.
   You must call {\bf define} again in order
   to use it.
*/
    void clear();

///invalid but necessary for leveldata to compile
    BaseIFFAB(const Box& a_region, int a_nVar)
      {
        amrex::Error("invalid constructor called for baseebcellfab");
      }

///
/**

   Tells whether this BaseIFFAB has been defined, either with a constructor
   or with {\bf define}.  It must be initialized in order to access its data.
*/
    bool
    isDefined() const
      {
        return (m_isDefined);
      }


///
/**
   Return the number of data components of this BaseIFFAB.
*/
    int nComp() const
      {
        return m_nComp;
      }

    int direction() const
      {
        return m_direction;
      }
///
/**
   Indexing operator.  Return a reference to the contents of this IFFAB,
   at the specified face and data component.  The first component is
   zero, the last is {\em nvar-1}.  The returned object is a modifiable
   lvalue.
*/
    T& operator() (const FaceIndex& a_face,const int& varlocin);
    const T& operator()(const FaceIndex& a_face,const int& varlocin) const;


///
    T* dataPtr(const int& a_comp);

    T* getIndex(const FaceIndex& a_face, const int& a_comp) const;
///
    const T* dataPtr(const int& a_comp) const;



    ///for copy and fillboundary where stuff is already defined
    std::size_t nBytes (const Box& bx, int start_comp, int ncomps) const;

    ///for copy and fillboundary where stuff is already defined
    std::size_t copyToMem (const Box& srcbox,
                           int        srccomp,
                           int        numcomp,
                           void*      dst) const;

    ///for copy and fillboundary where stuff is already defined
    std::size_t copyFromMem (const Box&  dstbox,
                             int         dstcomp,
                             int         numcomp,
                             const void* src);

    ///default false. can make this true with template specialization if you are sure.
    static bool preAllocatable () { return false; }

    ///includes all meta data
    std::size_t nBytesFull() const;

    ///includes all meta data
    std::size_t copyToMemFull(void* src) const;

    ///includes all meta data
    std::size_t copyFromMemFull(const void* src);

    ///
    int numFaces() const
      {
        return m_nFaces;
      }

    mutable bool    m_verbose;
///
    const std::vector<FaceIndex>& getFaces() const
      {
        return m_faces;
      }

    static bool s_verbose;

    EBGraph m_ebgraph;

  private:
    void   setDefaultValues();

    void   getFaceSubset(std::vector<FaceIndex>& a_subFaces, const Box& a_subBox) const;
  protected:

    //because my debugging functions get confounded  by templates
    void dumpFaces(const vector<FaceIndex>& a_faces) const;
    int m_nFaces;
    int m_nComp;
    int m_direction;
    IntVectSet m_ivs;
    std::vector<FaceIndex> m_faces;
    T* m_data;

    bool    m_isDefined;
  private:
//we disallow copy construction and assigment for data holders
    void operator= (const BaseIFFAB<T>&)
      {
        amrex::Error("invalid operator");
      }
    BaseIFFAB (const BaseIFFAB<T>&)
      {
        amrex::Error("invalid operator");
      }
  };

}
#include "AMReX_BaseIFFABI.H"


#endif
