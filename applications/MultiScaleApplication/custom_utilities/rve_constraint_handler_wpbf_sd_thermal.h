/*
==============================================================================
KratosMultiScaleApplication
A library based on:
Kratos
A General Purpose Software for Multi-Physics Finite Element Analysis
Version 1.0 (Released on march 05, 2007).

Copyright 2007
Pooyan Dadvand, Riccardo Rossi, Janosch Stascheit, Felix Nagel
pooyan@cimne.upc.edu
rrossi@cimne.upc.edu
janosch.stascheit@rub.de
nagel@sd.rub.de
- CIMNE (International Center for Numerical Methods in Engineering),
Gran Capita' s/n, 08034 Barcelona, Spain
- Ruhr-University Bochum, Institute for Structural Mechanics, Germany


Permission is hereby granted, free  of charge, to any person obtaining
a  copy  of this  software  and  associated  documentation files  (the
"Software"), to  deal in  the Software without  restriction, including
without limitation  the rights to  use, copy, modify,  merge, publish,
distribute,  sublicense and/or  sell copies  of the  Software,  and to
permit persons to whom the Software  is furnished to do so, subject to
the following condition:

Distribution of this code for  any  commercial purpose  is permissible
ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNERS.

The  above  copyright  notice  and  this permission  notice  shall  be
included in all copies or substantial portions of the Software.

THE  SOFTWARE IS  PROVIDED  "AS  IS", WITHOUT  WARRANTY  OF ANY  KIND,
EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT  SHALL THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY
CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER IN AN  ACTION OF CONTRACT,
TORT  OR OTHERWISE, ARISING  FROM, OUT  OF OR  IN CONNECTION  WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

==============================================================================
*/
//
//   Project Name:        Kratos
//   Last Modified by:    $Author: Massimo Petracca $
//   Date:                $Date: 2013-11-04 12:00:00 $
//   Revision:            $Revision: 1.00 $
//
//

#if !defined(RVE_CONSTRAINT_HANDLER_WPBF_SD_THERMAL_H_INCLUDED)
#define RVE_CONSTRAINT_HANDLER_WPBF_SD_THERMAL_H_INCLUDED

#include "rve_constraint_handler.h"
#include "../custom_conditions/rve_weak_periodic_condition_2D2N.h"

namespace Kratos
{

	/** \brief RveConstraintHandler_WPBF_SD_THERMAL
	*
	* Rve Constrain Handler for Periodic Boundary Displacement Fluctuations
	* in Small Displacement formulation
	*/
	template<class TSparseSpace, 
			 class TDenseSpace>
	class RveConstraintHandler_WPBF_SD_THERMAL : public RveConstraintHandler<TSparseSpace, TDenseSpace>
	{

	public:

		KRATOS_CLASS_POINTER_DEFINITION( RveConstraintHandler_WPBF_SD_THERMAL );

		typedef RveConstraintHandler<TSparseSpace, TDenseSpace> BaseType;

		typedef typename ModelPart::DofsArrayType DofsArrayType;

	public:
		
		RveConstraintHandler_WPBF_SD_THERMAL()
			: BaseType()
		{
		}

		virtual ~RveConstraintHandler_WPBF_SD_THERMAL()
		{
		}

	public:
		
		virtual void AddConditions(ModelPart& mp, 
								   const RveGeometryDescriptor& geom)
		{
			if(geom.Dimension() == 2)
			{
				Properties::Pointer cnd_prop(new Properties(0));
				size_t cnd_id = 0;

				// master node id
				//ModelPart::NodeType::Pointer master_node = mp.pGetNode(geom.CornerNodesIDs()[0]);

				//// create a constraint condition for each boundary edge
				//for(RveGeometryDescriptor::ContainerOfIndexContainerType::const_iterator it =
				//	geom.BoundaryEdgesIDs().begin(); it != geom.BoundaryEdgesIDs().end(); ++it)
				//{
				//	const RveGeometryDescriptor::IndexContainerType& edge_nodes = *it;
				//	Element::GeometryType::PointsArrayType nodes;
				//	for(size_t i = 0; i < edge_nodes.size(); i++)
				//	{
				//		ModelPart::NodeType::Pointer inode = mp.pGetNode(edge_nodes[i]);
				//		nodes.push_back(inode);
				//	}
				//	nodes.push_back(master_node);

				//	// constraint enforcing (3 equations for 2D, 6 equations for 3D):
				//	// int{ (sym_grad(u))dV } = 0
				//	// or
				//	// int{ (ni*uj + nj*ui) dS } = 0
				//	Element::GeometryType::Pointer cnd_geom( new Element::GeometryType(nodes) );
				//	RveWeakPeriodicCondition2D2N::Pointer cnd(new 
				//		RveWeakPeriodicCondition2D2N(cnd_id++, cnd_geom, cnd_prop));
				//	mp.AddCondition(cnd);

				//	// constraint enforcing (1 equation for 2D, 3 equations for 3D):
				//	// int{ (skew_grad(u))dV } = 0
				//	// or
				//	// int{ (ni*uj - nj*ui) dS } = 0
				//	Element::GeometryType::PointsArrayType nodes_skew = nodes;
				//	Element::GeometryType::Pointer cnd_geom_skew( new Element::GeometryType(nodes_skew) );
				//	RveWeakPeriodicCondition2D2N::Pointer cnd_skew(new 
				//		RveWeakPeriodicCondition2D2N(cnd_id++, cnd_geom_skew, cnd_prop));
				//	cnd_skew->IsSkewSymmetricConstraint() = true;
				//	mp.AddCondition(cnd_skew);

				//	// TODO:
				//	// these 2 constraints can be collected into a unified condition!
				//}



				//ModelPart::NodeType::Pointer master_node_x = mp.pGetNode(geom.CornerNodesIDs()[0]);
				//ModelPart::NodeType::Pointer master_node_y = mp.pGetNode(geom.CornerNodesIDs()[1]);

				//for(RveGeometryDescriptor::ContainerOfIndexContainerType::const_iterator it =
				//	geom.BoundaryEdgesIDs().begin(); it != geom.BoundaryEdgesIDs().end(); ++it)
				//{
				//	const RveGeometryDescriptor::IndexContainerType& edge_nodes = *it;
				//	Element::GeometryType::PointsArrayType nodes;
				//	int edge_id = 0;
				//	for(size_t i = 0; i < edge_nodes.size(); i++)
				//	{
				//		ModelPart::NodeType::Pointer inode = mp.pGetNode(edge_nodes[i]);
				//		if(std::find(geom.CornerNodesIDs().begin(),geom.CornerNodesIDs().end(), inode->GetId()) == geom.CornerNodesIDs().end()) {
				//			RveGeometryDescriptor::NodeToEdgeIDMapType::const_iterator eid_iter = geom.NodeToEdgeIDMap().find(inode->GetId());
				//			if(eid_iter != geom.NodeToEdgeIDMap().end()) {
				//				edge_id = eid_iter->second;
				//			}
				//		}
				//		nodes.push_back(inode);
				//	}
				//	if(edge_id > 0)
				//	{
				//		if(edge_id == 1 || edge_id == 2) {
				//			nodes.push_back(master_node_x);
				//		}
				//		else {
				//			nodes.push_back(master_node_y);
				//		}
				//		Element::GeometryType::Pointer cnd_geom( new Element::GeometryType(nodes) );
				//		RveWeakPeriodicCondition2D2N::Pointer cnd(new 
				//			RveWeakPeriodicCondition2D2N(cnd_id++, cnd_geom, cnd_prop));
				//		mp.AddCondition(cnd);

				//		// skew
				//		/*Element::GeometryType::PointsArrayType nodes_skew;
				//		for(unsigned int i=0; i<3; i++)
				//			nodes_skew.push_back(nodes[i]);
				//		nodes_skew.push_back(master_node_x);
				//		Element::GeometryType::Pointer cnd_geom_skew( new Element::GeometryType(nodes_skew) );
				//		RveWeakPeriodicCondition2D2N::Pointer cnd_skew(new 
				//			RveWeakPeriodicCondition2D2N(cnd_id++, cnd_geom_skew, cnd_prop));
				//		cnd_skew->IsSkewSymmetricConstraint() = true;
				//		mp.AddCondition(cnd_skew);*/
				//	}
				//}


				ModelPart::NodeType::Pointer master_node_1 = mp.pGetNode(geom.CornerNodesIDs()[0]);
				ModelPart::NodeType::Pointer master_node_2 = mp.pGetNode(geom.CornerNodesIDs()[1]);
				ModelPart::NodeType::Pointer master_node_3 = mp.pGetNode(geom.CornerNodesIDs()[2]);
				ModelPart::NodeType::Pointer master_node_4 = mp.pGetNode(geom.CornerNodesIDs()[3]);

				for(RveGeometryDescriptor::ContainerOfIndexContainerType::const_iterator it =
					geom.BoundaryEdgesIDs().begin(); it != geom.BoundaryEdgesIDs().end(); ++it)
				{
					const RveGeometryDescriptor::IndexContainerType& edge_nodes = *it;
					Element::GeometryType::PointsArrayType nodes;
					int edge_id = 0;
					for(size_t i = 0; i < edge_nodes.size(); i++)
					{
						ModelPart::NodeType::Pointer inode = mp.pGetNode(edge_nodes[i]);
						if(std::find(geom.CornerNodesIDs().begin(),geom.CornerNodesIDs().end(), inode->GetId()) == geom.CornerNodesIDs().end()) {
							RveGeometryDescriptor::NodeToEdgeIDMapType::const_iterator eid_iter = geom.NodeToEdgeIDMap().find(inode->GetId());
							if(eid_iter != geom.NodeToEdgeIDMap().end()) {
								edge_id = eid_iter->second;
							}
						}
						nodes.push_back(inode);
					}
					if(edge_id > 0)
					{
						if(edge_id==1)
							nodes.push_back(master_node_1);
						else if(edge_id==2)
							nodes.push_back(master_node_2);
						else if(edge_id==3)
							nodes.push_back(master_node_3);
						else
							nodes.push_back(master_node_4);
						Element::GeometryType::Pointer cnd_geom( new Element::GeometryType(nodes) );
						RveWeakPeriodicCondition2D2N::Pointer cnd(new 
							RveWeakPeriodicCondition2D2N(cnd_id++, cnd_geom, cnd_prop));
						mp.AddCondition(cnd);

						// skew
						/*Element::GeometryType::PointsArrayType nodes_skew;
						for(unsigned int i=0; i<3; i++)
							nodes_skew.push_back(nodes[i]);
						nodes_skew.push_back(master_node_1);
						Element::GeometryType::Pointer cnd_geom_skew( new Element::GeometryType(nodes_skew) );
						RveWeakPeriodicCondition2D2N::Pointer cnd_skew(new 
							RveWeakPeriodicCondition2D2N(cnd_id++, cnd_geom_skew, cnd_prop));
						cnd_skew->IsSkewSymmetricConstraint() = true;
						mp.AddCondition(cnd_skew);*/
					}
				}
			}
			else
			{
				std::cout << "3D not implemented in WPBF constraint handler" << std::endl;
				exit(-1);
			}
		}

		virtual void ApplyMacroScaleData(ModelPart& mp, 
										 const RveGeometryDescriptor& geom,
										 const RveMacroscaleData& macroScaleData)
		{
			const ProcessInfo& pinfo = mp.GetProcessInfo();
			Vector gTemp = -macroScaleData.StrainVector();
			std::vector< Vector > grad_T;
			for (ModelPart::ElementIterator elem_iter = mp.ElementsBegin(); elem_iter != mp.ElementsEnd(); ++elem_iter)
			{
				Element& ielem = *elem_iter;
				Element::GeometryType& igeom = ielem.GetGeometry();
				Element::IntegrationMethod intmethod = ielem.GetIntegrationMethod();
				const Element::GeometryType::IntegrationPointsArrayType& ipts = igeom.IntegrationPoints(intmethod);
				if (grad_T.size() != ipts.size())
					grad_T.resize(ipts.size());

				std::fill(grad_T.begin(), grad_T.end(), gTemp);
				ielem.SetValueOnIntegrationPoints(INITIAL_TEMP_GRAD, grad_T, pinfo);
			}

			if(geom.Dimension() == 2)
			{
				ModelPart::NodeType& n00 = mp.GetNode( geom.CornerNodesIDs()[0] );
				ModelPart::NodeType& n10 = mp.GetNode( geom.CornerNodesIDs()[1] );
				n00.Fix(TEMPERATURE);
				n10.Fix(TEMPERATURE);
			}
			else
			{
				std::cout << "3D not implemented in WPBF constraint handler" << std::endl;
				exit(-1);
			}
		}

		virtual void FinalizeSolutionStep(ModelPart& mp, 
										 const RveGeometryDescriptor& geom,
										 const RveMacroscaleData& macroScaleData)
		{
			//const ModelPart::NodeType& ref_node = mp.GetNode(geom.ReferenceNodeID());
			const double T_M = macroScaleData.Mean_Temp();
			Vector grad_T_M = macroScaleData.StrainVector();
			double T_m;
			if (geom.Dimension() == 2)
			{
				double x0 = ref_node.X0();
				double y0 = ref_node.Y0();
				double gT_x = grad_T_M[0];
				double gT_y = grad_T_M[1];
				for (ModelPart::NodeIterator it = mp.NodesBegin(); it != mp.NodesEnd(); ++it)
				{
					ModelPart::NodeType& inode = *it;
					if (inode.SolutionStepsDataHas(RVE_FULL_TEMPERATURE))
					{
						double x = inode.X0() - x0;
						double y = inode.Y0() - y0;
						const double& T_f = inode.FastGetSolutionStepValue(TEMPERATURE);
						T_m = T_f + T_M + gT_x * x + gT_y * y;
						inode.FastGetSolutionStepValue(RVE_FULL_TEMPERATURE) = T_m;
					}
				}
			}
			else
			{
				double x0 = ref_node.X0();
				double y0 = ref_node.Y0();
				double z0 = ref_node.Z0();
				double gT_x = grad_T_M[0];
				double gT_y = grad_T_M[1];
				double gT_z = grad_T_M[2];
				for (ModelPart::NodeIterator it = mp.NodesBegin(); it != mp.NodesEnd(); ++it)
				{
					ModelPart::NodeType& inode = *it;
					if (inode.SolutionStepsDataHas(RVE_FULL_TEMPERATURE))
					{
						double x = inode.X0() - x0;
						double y = inode.Y0() - y0;
						double z = inode.Z0() - z0;
						const double& T_f = inode.FastGetSolutionStepValue(TEMPERATURE);
						T_m = T_f + T_M + gT_x * x + gT_y * y + gT_z * z;
						inode.FastGetSolutionStepValue(RVE_FULL_TEMPERATURE) = T_m;
					}
				}
			}
		}

	};

} // namespace Kratos



#endif // RVE_CONSTRAINT_HANDLER_WPBF_SD_THERMAL_H_INCLUDED
