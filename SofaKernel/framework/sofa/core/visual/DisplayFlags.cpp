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
#include <sofa/core/visual/DisplayFlags.h>
#include <sofa/helper/logging/Messaging.h>

namespace sofa
{
namespace core
{
namespace visual
{

FlagTreeItem::FlagTreeItem(const std::string& showName, const std::string& hideName, FlagTreeItem* parent):
    m_showName({showName}),
    m_hideName({hideName}),
    m_state(tristate::neutral_value),
    m_parent(parent)
{
    if( m_parent ) m_parent->m_child.push_back(this);
}


void FlagTreeItem::addAliasShow(const std::string& newAlias)
{
    this->addAlias(this->m_showName, newAlias);
}


void FlagTreeItem::addAliasHide(const std::string& newAlias)
{
    this->addAlias(this->m_hideName, newAlias);
}


void FlagTreeItem::addAlias(sofa::helper::vector<std::string>& name, const std::string& newAlias)
{
    name.push_back(newAlias);
    return;
}


void FlagTreeItem::setValue(const tristate &state)
{
    this->m_state = state;
    propagateStateDown(this);
    propagateStateUp(this);
}

void FlagTreeItem::propagateStateDown(FlagTreeItem* origin)
{
    ChildIterator iter;
    for( iter = origin->m_child.begin(); iter != origin->m_child.end(); ++iter)
    {
        (*iter)->m_state = origin->m_state;
        propagateStateDown(*iter);
    }
}

void FlagTreeItem::propagateStateUp(FlagTreeItem* origin)
{
    FlagTreeItem* parent = origin->m_parent;
    if(!parent) return;

    tristate flag = origin->m_state;
    for( unsigned int i = 0 ; i < parent->m_child.size(); ++i)
    {
        FlagTreeItem* current = parent->m_child[i];
        flag = fusion_tristate(current->m_state,flag);
    }

    parent->m_state=flag;
    propagateStateUp(parent);
}

std::ostream& FlagTreeItem::write(std::ostream &os) const
{
    std::string s;
    write_recursive(this,s);
    s.erase(s.find_last_not_of(" \n\r\t")+1);
    os << s;
    return os;
}

std::istream& FlagTreeItem::read(std::istream &in)
{
    std::map<std::string, bool,  ci_comparison> parse_map;
    create_parse_map(this,parse_map);
    std::string token;
    while(in >> token)
    {
        if( parse_map.find(token) != parse_map.end() )
        {
            std::map<std::string,bool>::const_iterator iter;
            iter = parse_map.find(token);
            std::string string1 = iter->first;

            if(string1 != token)
            {
                msg_warning("DisplayFlags") << "Case of FlagTreeItem '" << token << "' is not correct, please use '"<< string1 <<"' instead";
            }

            parse_map[token] = true;
        }
        else
            msg_error("DisplayFlags") << "FlagTreeItem: unknown token " << token;
    }
    if( in.rdstate() & std::ios_base::eofbit ) { in.clear(); }

    read_recursive(this,parse_map);
    return in;
}



/*static*/
void FlagTreeItem::create_parse_map(FlagTreeItem *root, std::map<std::string, bool, ci_comparison> &map)
{
    size_t sizeShow = root->m_showName.size();
    size_t sizeHide = root->m_hideName.size();
    for(size_t i=0; i<sizeShow; i++)
    {
        map[root->m_showName[i]] = false;
    }
    for(size_t i=0; i<sizeHide; ++i)
    {
        map[root->m_hideName[i]] = false;
    }

    ChildIterator iter;
    for( iter = root->m_child.begin(); iter != root->m_child.end(); ++iter)
    {
        create_parse_map(*iter,map);
    }
}

void FlagTreeItem::read_recursive(FlagTreeItem *root, const std::map<std::string, bool, ci_comparison> &parse_map)
{
    ChildIterator iter;
    std::map<std::string,bool>::const_iterator iter_show;
    std::map<std::string,bool>::const_iterator iter_hide;
    for( iter = root->m_child.begin(); iter != root->m_child.end(); ++iter)
    {
        size_t sizeShow = (*iter)->m_showName.size();
        size_t sizeHide = (*iter)->m_hideName.size();

        bool found = false;

        for(size_t i=0; i<sizeHide; i++)
        {
            iter_hide = parse_map.find((*iter)->m_hideName[i]);
            if( iter_hide != parse_map.end() )
            {
                bool hide = iter_hide->second;
                if(hide)
                {
                    if(i != 0)
                    {
                        msg_warning("DisplayFlags") << "FlagTreeItem '" << (*iter)->m_hideName[i] << "' is deprecated, please use '"<<(*iter)->m_hideName[0]<<"' instead";
                    }

                    (*iter)->setValue(tristate::false_value);
                    found = true;
                }
            }
        }
        for(size_t i=0; i<sizeShow; i++)
        {
            iter_show = parse_map.find((*iter)->m_showName[i]);
            if( iter_show != parse_map.end() )
            {
                bool show  = iter_show->second;
                if(show)
                {
                    if(i != 0)
                    {
                        msg_warning("DisplayFlags") << "FlagTreeItem '" << (*iter)->m_showName[i] << "' is deprecated, please use '"<<(*iter)->m_showName[0]<<"' instead";
                    }

                    (*iter)->setValue(tristate::true_value);
                    found = true;
                }
            }
        }

        if(!found)
        {
            (*iter)->m_state = tristate::neutral_value;
            read_recursive(*iter,parse_map);
        }
    }
}

void FlagTreeItem::write_recursive(const FlagTreeItem* root, std::string& str )
{
    ChildConstIterator iter;
    for( iter = root->m_child.begin(); iter != root->m_child.end(); ++iter )
    {
        switch( (*iter)->m_state.state )
        {
        case tristate::true_value:
            str.append((*iter)->m_showName[0]);
            str.append(" ");
            break;
        case tristate::false_value:
            str.append((*iter)->m_hideName[0]);
            str.append(" ");
            break;
        case tristate::neutral_value:
            write_recursive(*iter,str);
        }
    }
}

DisplayFlags::DisplayFlags():
    m_root(FlagTreeItem("showRoot","hideRoot",NULL)),
    m_showAll(FlagTreeItem("showAll","hideAll",&m_root)),
    m_showVisual(FlagTreeItem("showVisual","hideVisual",&m_showAll)),
    m_showVisualModels(FlagTreeItem("showVisualModels","hideVisualModels",&m_showVisual)),
    m_showBehavior(FlagTreeItem("showBehavior","hideBehavior",&m_showAll)),
    m_showBehaviorModels(FlagTreeItem("showBehaviorModels","hideBehaviorModels",&m_showBehavior)),
    m_showForceFields(FlagTreeItem("showForceFields","hideForceFields",&m_showBehavior)),
    m_showInteractionForceFields(FlagTreeItem("showInteractionForceFields","hideInteractionForceFields",&m_showBehavior)),
    m_showCollision(FlagTreeItem("showCollision","hideCollision",&m_showAll)),
    m_showCollisionModels(FlagTreeItem("showCollisionModels","hideCollisionModels",&m_showCollision)),
    m_showBoundingCollisionModels(FlagTreeItem("showBoundingCollisionModels","hideBoundingCollisionModels",&m_showCollision)),
    m_showMapping(FlagTreeItem("showMapping","hideMapping",&m_showAll)),
    m_showVisualMappings(FlagTreeItem("showMappings","hideMappings",&m_showMapping)),
    m_showMechanicalMappings(FlagTreeItem("showMechanicalMappings","hideMechanicalMappings",&m_showMapping)),
    m_showOptions(FlagTreeItem("showOptions","hideOptions",&m_root)),
    m_showAdvancedRendering(FlagTreeItem("showAdvancedRendering","hideAdvancedRendering",&m_showOptions)),
    m_showWireframe(FlagTreeItem("showWireframe","hideWireframe",&m_showOptions)),
    m_showNormals(FlagTreeItem("showNormals","hideNormals",&m_showOptions))
{
    m_showVisualModels.setValue(tristate::neutral_value);
    m_showBehaviorModels.setValue(tristate::neutral_value);
    m_showForceFields.setValue(tristate::neutral_value);
    m_showInteractionForceFields.setValue(tristate::neutral_value);
    m_showCollisionModels.setValue(tristate::neutral_value);
    m_showBoundingCollisionModels.setValue(tristate::neutral_value);
    m_showVisualMappings.setValue(tristate::neutral_value);
    m_showMechanicalMappings.setValue(tristate::neutral_value);
    m_showAdvancedRendering.setValue(tristate::neutral_value);
    m_showWireframe.setValue(tristate::neutral_value);
    m_showNormals.setValue(tristate::neutral_value);

    m_showAdvancedRendering.addAliasShow("showRendering");
    m_showAdvancedRendering.addAliasHide("hideRendering");
}

DisplayFlags::DisplayFlags(const DisplayFlags & other):
    m_root(FlagTreeItem("showRoot","hideRoot",NULL)),
    m_showAll(FlagTreeItem("showAll","hideAll",&m_root)),
    m_showVisual(FlagTreeItem("showVisual","hideVisual",&m_showAll)),
    m_showVisualModels(FlagTreeItem("showVisualModels","hideVisualModels",&m_showVisual)),
    m_showBehavior(FlagTreeItem("showBehavior","hideBehavior",&m_showAll)),
    m_showBehaviorModels(FlagTreeItem("showBehaviorModels","hideBehaviorModels",&m_showBehavior)),
    m_showForceFields(FlagTreeItem("showForceFields","hideForceFields",&m_showBehavior)),
    m_showInteractionForceFields(FlagTreeItem("showInteractionForceFields","hideInteractionForceFields",&m_showBehavior)),
    m_showCollision(FlagTreeItem("showCollision","hideCollision",&m_showAll)),
    m_showCollisionModels(FlagTreeItem("showCollisionModels","hideCollisionModels",&m_showCollision)),
    m_showBoundingCollisionModels(FlagTreeItem("showBoundingCollisionModels","hideBoundingCollisionModels",&m_showCollision)),
    m_showMapping(FlagTreeItem("showMapping","hideMapping",&m_showAll)),
    m_showVisualMappings(FlagTreeItem("showMappings","hideMappings",&m_showMapping)),
    m_showMechanicalMappings(FlagTreeItem("showMechanicalMappings","hideMechanicalMappings",&m_showMapping)),
    m_showOptions(FlagTreeItem("showOptions","hideOptions",&m_root)),
    m_showAdvancedRendering(FlagTreeItem("showAdvancedRendering","hideAdvancedRendering",&m_showOptions)),
    m_showWireframe(FlagTreeItem("showWireframe","hideWireframe",&m_showOptions)),
    m_showNormals(FlagTreeItem("showNormals","hideNormals",&m_showOptions))
{
    m_showVisualModels.setValue(other.m_showVisualModels.state());
    m_showBehaviorModels.setValue(other.m_showBehaviorModels.state());
    m_showForceFields.setValue(other.m_showForceFields.state());
    m_showInteractionForceFields.setValue(other.m_showInteractionForceFields.state());
    m_showCollisionModels.setValue(other.m_showCollisionModels.state());
    m_showBoundingCollisionModels.setValue(other.m_showBoundingCollisionModels.state());
    m_showVisualMappings.setValue(other.m_showVisualMappings.state());
    m_showMechanicalMappings.setValue(other.m_showMechanicalMappings.state());
    m_showAdvancedRendering.setValue(other.m_showAdvancedRendering.state());
    m_showWireframe.setValue(other.m_showWireframe.state());
    m_showNormals.setValue(other.m_showNormals.state());

    m_showAdvancedRendering.addAliasShow("showRendering");
    m_showAdvancedRendering.addAliasHide("hideRendering");
}

DisplayFlags& DisplayFlags::operator =(const DisplayFlags& other)
{
    if( this != &other)
    {
        m_showVisualModels.setValue(other.m_showVisualModels.state());
        m_showBehaviorModels.setValue(other.m_showBehaviorModels.state());
        m_showForceFields.setValue(other.m_showForceFields.state());
        m_showInteractionForceFields.setValue(other.m_showInteractionForceFields.state());
        m_showCollisionModels.setValue(other.m_showCollisionModels.state());
        m_showBoundingCollisionModels.setValue(other.m_showBoundingCollisionModels.state());
        m_showVisualMappings.setValue(other.m_showVisualMappings.state());
        m_showMechanicalMappings.setValue(other.m_showMechanicalMappings.state());
        m_showAdvancedRendering.setValue(other.m_showAdvancedRendering.state());
        m_showWireframe.setValue(other.m_showWireframe.state());
        m_showNormals.setValue(other.m_showNormals.state());
    }
    return *this;
}

bool DisplayFlags::isNeutral() const
{
    return m_showVisualModels.state().state == tristate::neutral_value
            && m_showBehaviorModels.state().state == tristate::neutral_value
            && m_showForceFields.state().state  == tristate::neutral_value
            && m_showInteractionForceFields.state().state == tristate::neutral_value
            && m_showBoundingCollisionModels.state().state == tristate::neutral_value
            && m_showCollisionModels.state().state == tristate::neutral_value
            && m_showVisualMappings.state().state == tristate::neutral_value
            && m_showMechanicalMappings.state().state == tristate::neutral_value
            && m_showAdvancedRendering.state().state == tristate::neutral_value
            && m_showWireframe.state().state == tristate::neutral_value
            && m_showNormals.state().state == tristate::neutral_value
            ;
}

DisplayFlags merge_displayFlags(const DisplayFlags &previous, const DisplayFlags &current)
{
    DisplayFlags merge;
    merge.m_showVisualModels.setValue( merge_tristate(previous.m_showVisualModels.state(),current.m_showVisualModels.state()) );
    merge.m_showBehaviorModels.setValue( merge_tristate(previous.m_showBehaviorModels.state(),current.m_showBehaviorModels.state()) );
    merge.m_showForceFields.setValue( merge_tristate(previous.m_showForceFields.state(),current.m_showForceFields.state()) );
    merge.m_showInteractionForceFields.setValue( merge_tristate(previous.m_showInteractionForceFields.state(),current.m_showInteractionForceFields.state()) );
    merge.m_showCollisionModels.setValue( merge_tristate(previous.m_showCollisionModels.state(),current.m_showCollisionModels.state()) );
    merge.m_showBoundingCollisionModels.setValue( merge_tristate(previous.m_showBoundingCollisionModels.state(),current.m_showBoundingCollisionModels.state()) );
    merge.m_showVisualMappings.setValue( merge_tristate(previous.m_showVisualMappings.state(),current.m_showVisualMappings.state()) );
    merge.m_showMechanicalMappings.setValue( merge_tristate(previous.m_showMechanicalMappings.state(),current.m_showMechanicalMappings.state()) );
    merge.m_showAdvancedRendering.setValue( merge_tristate(previous.m_showAdvancedRendering.state(),current.m_showAdvancedRendering.state()) );
    merge.m_showWireframe.setValue( merge_tristate(previous.m_showWireframe.state(),current.m_showWireframe.state()) );
    merge.m_showNormals.setValue( merge_tristate(previous.m_showNormals.state(),current.m_showNormals.state()) );
    return merge;
}

DisplayFlags difference_displayFlags(const DisplayFlags& previous, const DisplayFlags& current)
{
    DisplayFlags difference;
    difference.m_showVisualModels.setValue( difference_tristate(previous.m_showVisualModels.state(),current.m_showVisualModels.state()) );
    difference.m_showBehaviorModels.setValue( difference_tristate(previous.m_showBehaviorModels.state(),current.m_showBehaviorModels.state()) );
    difference.m_showForceFields.setValue( difference_tristate(previous.m_showForceFields.state(),current.m_showForceFields.state()) );
    difference.m_showInteractionForceFields.setValue( difference_tristate(previous.m_showInteractionForceFields.state(),current.m_showInteractionForceFields.state()) );
    difference.m_showCollisionModels.setValue( difference_tristate(previous.m_showCollisionModels.state(),current.m_showCollisionModels.state()) );
    difference.m_showBoundingCollisionModels.setValue( difference_tristate(previous.m_showBoundingCollisionModels.state(),current.m_showBoundingCollisionModels.state()) );
    difference.m_showVisualMappings.setValue( difference_tristate(previous.m_showVisualMappings.state(),current.m_showVisualMappings.state()) );
    difference.m_showMechanicalMappings.setValue( difference_tristate(previous.m_showMechanicalMappings.state(),current.m_showMechanicalMappings.state()) );
    difference.m_showAdvancedRendering.setValue( difference_tristate(previous.m_showAdvancedRendering.state(),current.m_showAdvancedRendering.state()) );
    difference.m_showWireframe.setValue( difference_tristate(previous.m_showWireframe.state(),current.m_showWireframe.state()) );
    difference.m_showNormals.setValue( difference_tristate(previous.m_showNormals.state(),current.m_showNormals.state()) );
    return difference;
}
}

}

}
