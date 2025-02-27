/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2019 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU General Public License as published by the Free  *
* Software Foundation; either version 2 of the License, or (at your option)   *
* any later version.                                                          *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
* more details.                                                               *
*                                                                             *
* You should have received a copy of the GNU General Public License along     *
* with this program. If not, see <http://www.gnu.org/licenses/>.              *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/

#include <sofa/core/objectmodel/Context.h>
#include <sofa/core/VecId.h>
using sofa::core::VecId;
#include <sofa/defaulttype/Vec.h>
#include <sofa/defaulttype/VecTypes.h>
using sofa::defaulttype::Vec3Types;
using sofa::defaulttype::ExtVec3Types;
using Coord3 = sofa::defaulttype::Vector3;
using VecCoord3 = sofa::helper::vector<Coord3>;
#include <sofa/gui/GUIManager.h>
#include <sofa/gui/Main.h>
#include <sofa/helper/ArgumentParser.h>
#include <sofa/helper/system/FileRepository.h>

#include <SofaComponentAdvanced/initComponentAdvanced.h>
#include <SofaComponentBase/initComponentBase.h>
#include <SofaComponentCommon/initComponentCommon.h>
#include <SofaComponentGeneral/initComponentGeneral.h>
#include <SofaComponentMisc/initComponentMisc.h>

#include <SofaBaseLinearSolver/CGLinearSolver.h>
using CGLinearSolver = sofa::component::linearsolver::CGLinearSolver<sofa::component::linearsolver::GraphScatteredMatrix, sofa::component::linearsolver::GraphScatteredVector>;
#include <SofaBaseMechanics/BarycentricMapping.h>
using BarycentricMapping3_to_Ext3 = sofa::component::mapping::BarycentricMapping<Vec3Types, ExtVec3Types>;
#include <SofaBaseMechanics/MechanicalObject.h>
using MechanicalObject3 = sofa::component::container::MechanicalObject<Vec3Types>;
#include <SofaBaseMechanics/UniformMass.h>
using UniformMass3 = sofa::component::mass::UniformMass<Vec3Types, SReal>;
#include <SofaBaseTopology/MeshTopology.h>
using sofa::component::topology::MeshTopology;
#include <SofaBaseVisual/VisualStyle.h>
using sofa::component::visualmodel::VisualStyle;
#include <SofaBoundaryCondition/FixedConstraint.h>
using FixedConstraint3 = sofa::component::projectiveconstraintset::FixedConstraint<Vec3Types>;
#include <SofaImplicitOdeSolver/EulerImplicitSolver.h>
using sofa::component::odesolver::EulerImplicitSolver;
#include <SofaOpenglVisual/OglModel.h>
using sofa::component::visualmodel::OglModel;
#include <SofaSimpleFem/TetrahedronFEMForceField.h>
using TetrahedronFEMForceField3 = sofa::component::forcefield::TetrahedronFEMForceField<Vec3Types>;
#include <SofaSimulationTree/init.h>
#include <SofaSimulationTree/GNode.h>
#include <SofaSimulationTree/TreeSimulation.h>

using sofa::core::objectmodel::Data;
using sofa::core::objectmodel::New;
using sofa::helper::ReadAccessor;
using sofa::helper::WriteAccessor;
using sofa::simulation::Node;

#include <iostream>
#include <fstream>


// ---------------------------------------------------------------------
// ---
// ---------------------------------------------------------------------
int main(int argc, char** argv)
{
    sofa::simulation::tree::init();
    ArgumentParser argParser(argc, argv);
    sofa::gui::GUIManager::RegisterParameters(&argParser);
    argParser.parse();
    sofa::gui::initMain();
    sofa::gui::GUIManager::Init(argv[0]);

    sofa::component::initComponentBase();
    sofa::component::initComponentCommon();
    sofa::component::initComponentGeneral();
    sofa::component::initComponentAdvanced();
    sofa::component::initComponentMisc();

    // The graph root node : gravity already exists in a GNode by default
    sofa::simulation::setSimulation(new sofa::simulation::tree::TreeSimulation());
    sofa::simulation::Node::SPtr groot = sofa::simulation::getSimulation()->createNewGraph("root");
    groot->setGravity( sofa::defaulttype::Vector3(0,-10,0) );

    // One solver for all the graph
    EulerImplicitSolver::SPtr solver = sofa::core::objectmodel::New<EulerImplicitSolver>();
    solver->setName("solver");
    solver->f_printLog.setValue(false);
    groot->addObject(solver);

    CGLinearSolver::SPtr linearSolver = New<CGLinearSolver>();
    linearSolver->setName("linearSolver");
    groot->addObject(linearSolver);


    // Tetrahedron degrees of freedom
    MechanicalObject3::SPtr DOF = sofa::core::objectmodel::New<MechanicalObject3>();
    groot->addObject(DOF);
    DOF->resize(4);
    DOF->setName("DOF");
    //get write access to the position vector of mechanical object DOF
    WriteAccessor<Data<VecCoord3> > x = *DOF->write(VecId::position());
    x[0] = Coord3(0,10,0);
    x[1] = Coord3(10,0,0);
    x[2] = Coord3(-10*0.5,0,10*0.866);
    x[3] = Coord3(-10*0.5,0,-10*0.866);
    DOF->showObject.setValue(true);
    DOF->showObjectScale.setValue(10.);


    // Tetrahedron uniform mass
    UniformMass3::SPtr mass = sofa::core::objectmodel::New<UniformMass3>();
    groot->addObject(mass);
    mass->setMass(2);
    mass->setName("mass");

    // Tetrahedron topology
    MeshTopology::SPtr topology = sofa::core::objectmodel::New<MeshTopology>();
    topology->setName("mesh topology");
    groot->addObject( topology );
    topology->addTetra(0,1,2,3);

    // Tetrahedron constraints
    FixedConstraint3::SPtr constraints = sofa::core::objectmodel::New<FixedConstraint3>();
    constraints->setName("constraints");
    groot->addObject(constraints);
    constraints->addConstraint(0);

    // Tetrahedron force field
    TetrahedronFEMForceField3::SPtr fem = sofa::core::objectmodel::New<TetrahedronFEMForceField3>();
    fem->setName("FEM");
    groot->addObject(fem);
    fem->setMethod("polar");
    fem->setUpdateStiffnessMatrix(true);
    fem->setYoungModulus(6);
    fem->setPoissonRatio(0.45);

    // Tetrahedron skin
    Node::SPtr skin = groot.get()->createChild("skin");
    // The visual model
    OglModel::SPtr visual = sofa::core::objectmodel::New<OglModel>();
    visual->setName( "visual" );
    visual->load(sofa::helper::system::DataRepository.getFile("mesh/liver-smooth.obj"), "", "");
    visual->setColor("red");
    visual->applyScale(0.7, 0.7, 0.7);
    visual->applyTranslation(1.2, 0.8, 0);
    skin->addObject(visual);

    // The mapping between the tetrahedron (DOF) and the liver (visual)
    BarycentricMapping3_to_Ext3::SPtr mapping = sofa::core::objectmodel::New<BarycentricMapping3_to_Ext3>();
    mapping->setModels(DOF.get(), visual.get());
    mapping->setName( "mapping" );
    skin->addObject(mapping);

    // Display Flags
    VisualStyle::SPtr style = sofa::core::objectmodel::New<VisualStyle>();
    groot->addObject(style);
    sofa::core::visual::DisplayFlags& flags = *style->displayFlags.beginEdit();
    flags.setShowNormals(false);
    flags.setShowInteractionForceFields(false);
    flags.setShowMechanicalMappings(false);
    flags.setShowCollisionModels(false);
    flags.setShowBoundingCollisionModels(false);
    flags.setShowMappings(false);
    flags.setShowForceFields(true);
    flags.setShowWireFrame(true);
    flags.setShowVisualModels(true);
    flags.setShowBehaviorModels(true);
    style->displayFlags.endEdit();

    // Init the scene
    sofa::simulation::tree::getSimulation()->init(groot.get());
    groot->setAnimate(false);


    //=======================================
    // Run the main loop
    sofa::gui::GUIManager::MainLoop(groot);

    sofa::simulation::tree::cleanup();
    return 0;
}
