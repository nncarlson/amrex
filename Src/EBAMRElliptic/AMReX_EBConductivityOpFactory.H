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

#ifndef _EBCONDUCTIVITYOPFACTORY_H___
#define _EBCONDUCTIVITYOPFACTORY_H___

#include <vector>
#include "AMReX_AMREBMultiGrid.H"
#include "AMReX_EBIndexSpace.H"
#include "AMReX_EBCellFAB.H"
#include "AMReX_EBCellFactory.H"
#include "AMReX_EBConductivityOp.H"
#include "AMReX_EBLevelDataOps.H"
#include "AMReX_ConductivityBaseEBBC.H"
#include "AMReX_ConductivityBaseDomainBC.H"

namespace amrex
{

  //! \class EBConductivityOpFactory
  //! Factory class to generate EBConductivityOps.  This follows the
  //! AMRLevelOpFactory interface.
  class EBConductivityOpFactory: public AMRLevelOpFactory<FabArray<EBCellFAB> >
  {
  public:

    ///
    /**
       Constructs a factory that builds EBConductivityOps 
    */
    EBConductivityOpFactory(const vector<EBLevelGrid>                           &  a_eblgs,
                            const Real                                          &  a_alpha,
                            const Real                                          &  a_beta,
                            const vector<shared_ptr<FabArray<EBCellFAB> > >     &  a_acoef,
                            const vector<shared_ptr<FabArray<EBFluxFAB> > >     &  a_bcoef,
                            const Real                                          &  a_dxCoarse,
                            const vector<int>                                   &  a_refRatio,
                            const shared_ptr<ConductivityBaseDomainBCFactory>   &  a_domainBCFactory,
                            const shared_ptr<ConductivityBaseEBBCFactory>       &  a_ebBcFactory,
                            const IntVect                                       &  a_ghostCellsPhi,
                            const IntVect                                       &  a_ghostCellsRhs,
                            int a_numLevels = -1);


    //! Destructor.
    virtual ~EBConductivityOpFactory();

    ///called by EBMultiGrid.H
    virtual EBConductivityOp*
    MGnewOp(const Box & a_FineindexSpace,
            int         a_depth,
            bool        a_homoOnly = true);

    ///called by AMREBMultiGrid.H
    virtual EBConductivityOp*
    AMRnewOp(const Box& a_FineindexSpace);


    ///does the real work
    EBConductivityOp*
    createOperator(const int&  a_amrlevel,
                   const int&  a_depth,
                   const bool  a_amrLevelOp);

  protected:

    vector<EBLevelGrid>                                  m_eblgs;
    Real                                                 m_alpha;
    Real                                                 m_beta;

    //! coefficients
    vector<shared_ptr<FabArray<EBCellFAB> > >        m_acoef;
    vector<shared_ptr<FabArray<EBFluxFAB> > >        m_bcoef;

    //inputs
    Real                                                 m_dxCoarse;
    vector<int>                                          m_refRatio;
    shared_ptr<ConductivityBaseDomainBCFactory>          m_domainBCFactory;
    shared_ptr<ConductivityBaseEBBCFactory>              m_ebBCFactory;
    int                                                  m_ghostCellsPhi;
    int                                                  m_ghostCellsRhs;
    int                                                  m_numLevels;
    vector<Real>                                         m_dx;
  private:
    ///weak construction bad
    EBConductivityOpFactory();

    //copy constructor and operator= disallowed for all the usual reasons
    EBConductivityOpFactory(const EBConductivityOpFactory& a_opin);

    //copy constructor and operator= disallowed for all the usual reasons
    void operator=(const EBConductivityOpFactory& a_opin);


}
#endif
