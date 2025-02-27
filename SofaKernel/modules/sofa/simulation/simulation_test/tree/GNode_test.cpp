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

#include "../Node_test.h"
#include <gtest/gtest.h>
#include <SofaSimulationTree/GNode.h>

using sofa::simulation::tree::GNode;

TEST(GNodeTest, objectDestruction_singleObject)
{
    Node_test_objectDestruction_singleObject<GNode>();
}

TEST(GNodeTest, objectDestruction_multipleObjects)
{
    Node_test_objectDestruction_multipleObjects<GNode>();
}

TEST(GNodeTest, objectDestruction_childNode_singleObject)
{
    Node_test_objectDestruction_childNode_singleObject<GNode>();
}

TEST(GNodeTest, objectDestruction_childNode_complexChild)
{
    Node_test_objectDestruction_childNode_complexChild<GNode>();
}
