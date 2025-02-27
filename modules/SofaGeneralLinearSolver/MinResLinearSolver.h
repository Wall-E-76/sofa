/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2019 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_LINEARSOLVER_MINRESLINEARSOLVER_H
#define SOFA_COMPONENT_LINEARSOLVER_MINRESLINEARSOLVER_H
#include "config.h"

#include <SofaBaseLinearSolver/MatrixLinearSolver.h>
#include <sofa/helper/map.h>

#include <cmath>


namespace sofa
{

namespace component
{

namespace linearsolver
{
//#define DISPLAY_TIME

/// Linear system solver using the MINRES iterative algorithm
/// @author Matthieu Nesme
/// @date 2013
template<class TMatrix, class TVector>
class MinResLinearSolver : public sofa::component::linearsolver::MatrixLinearSolver<TMatrix, TVector>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE2(MinResLinearSolver,TMatrix,TVector),SOFA_TEMPLATE2(sofa::component::linearsolver::MatrixLinearSolver,TMatrix,TVector));

    typedef TMatrix Matrix;
    typedef TVector Vector;
    typedef sofa::component::linearsolver::MatrixLinearSolver<TMatrix,TVector> Inherit;
    Data<unsigned> f_maxIter; ///< maximum number of iterations of the Conjugate Gradient solution
    Data<double> f_tolerance; ///< desired precision of the Conjugate Gradient Solution (ratio of current residual norm over initial residual norm)
    Data<bool> f_verbose; ///< Dump system state at each iteration
    Data<std::map < std::string, sofa::helper::vector<SReal> > > f_graph; ///< Graph of residuals at each iteration
#ifdef DISPLAY_TIME
    double time1;
    double time2;
    double timeStamp;
#endif
protected:
    MinResLinearSolver();

public:
    void resetSystem() override;
    void setSystemMBKMatrix(const sofa::core::MechanicalParams* mparams) override;

    /// Solve Mx=b
    void solve (Matrix& M, Vector& x, Vector& b) override;
};

#if !defined(SOFA_COMPONENT_LINEARSOLVER_MINRESLINEARSOLVER_CPP)
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< GraphScatteredMatrix, GraphScatteredVector >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< FullMatrix<SRreal>, FullVector<SRreal> >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< SparseMatrix<SRreal>, FullVector<SRreal> >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< CompressedRowSparseMatrix<SRreal>, FullVector<SRreal> >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< CompressedRowSparseMatrix<defaulttype::Mat<2,2,SRreal> >, FullVector<SRreal> >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< CompressedRowSparseMatrix<defaulttype::Mat<3,3,SRreal> >, FullVector<SRreal> >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< CompressedRowSparseMatrix<defaulttype::Mat<4,4,SRreal> >, FullVector<SRreal> >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< CompressedRowSparseMatrix<defaulttype::Mat<6,6,SRreal> >, FullVector<SRreal> >;
extern template class SOFA_GENERAL_LINEAR_SOLVER_API MinResLinearSolver< CompressedRowSparseMatrix<defaulttype::Mat<8,8,SRreal> >, FullVector<SRreal> >;
#endif

} // namespace linearsolver

} // namespace component

} // namespace sofa

#endif
