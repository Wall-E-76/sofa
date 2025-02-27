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



#include <sofa/simulation/VelocityThresholdVisitor.h>
#include <sofa/core/behavior/BaseMechanicalState.h>
#include <iostream>

namespace sofa
{

namespace simulation
{

VelocityThresholdVisitor::VelocityThresholdVisitor( const core::ExecParams* params, core::MultiVecId v, SReal t  )
    : Visitor(params), vid(v), threshold(t)
{
}

Visitor::Result VelocityThresholdVisitor::processNodeTopDown(simulation::Node* node)
{
    sofa::core::behavior::BaseMechanicalState* state = node->mechanicalState;

    if (state != NULL)
    {
        state->vThreshold(vid.getId(state),threshold);
    }
    return Visitor::RESULT_CONTINUE;
}

} // namespace simulation

} // namespace sofa

