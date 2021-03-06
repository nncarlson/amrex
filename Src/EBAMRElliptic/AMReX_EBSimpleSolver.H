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

#ifndef _EBSIMPLESOLVER_H___
#define _EBSIMPLESOLVER_H___

#include "AMReX_FabArray.H"
#include "AMReX_LinearSolver.H"
#include "AMReX_MultiGrid.H"
#include "AMReX_EBCellFAB.H"

namespace amrex
{

  class EBSimpleSolver : public LinearSolver<FabArray<EBCellFAB> >
  {
  public:
    EBSimpleSolver();

    virtual ~EBSimpleSolver();

    virtual void setHomogeneous(bool a_homogeneous);

    virtual void define(LinearOp<FabArray<EBCellFAB> >* a_operator,
                        bool                             a_homogeneous);

    virtual void setNumSmooths(const int& a_numSmooths);

    virtual void solve(FabArray<EBCellFAB>&       a_phi,
                       const FabArray<EBCellFAB>& a_rhs);

  private:
    bool m_isDefined;

    MGLevelOp<FabArray<EBCellFAB> >* m_operator;

    bool m_homogeneous;
    int  m_numSmooths;
  };

}
#endif
