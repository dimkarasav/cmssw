// -*- C++ -*-
//
// Package:     Vertexs
// Class  :     FWVertexProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Dec  2 14:17:03 EST 2008
// $Id: FWVertexProxyBuilder.cc,v 1.12 2012/03/02 04:56:39 amraktad Exp $
//
// user include files// user include files
#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWProxyBuilderConfiguration.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWParameters.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "Fireworks/Vertices/interface/TEveEllipsoid.h"

#include "TEvePointSet.h"
#include "TMatrixDEigen.h"
#include "TMatrixDSym.h"
#include "TDecompSVD.h"
#include "TVectorD.h"
#include "TEveTrans.h"
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"
#include "TEveStraightLineSet.h"
#include "TEveBoxSet.h"
#include "TGeoSphere.h"
#include "TEveGeoNode.h"
#include "TEveVSDStructs.h"

class FWVertexProxyBuilder : public FWSimpleProxyBuilderTemplate<reco::Vertex> 
{
public:
   FWVertexProxyBuilder() {}
   virtual ~FWVertexProxyBuilder() {}
   
   virtual void setItem(const FWEventItem* iItem)
   {
      FWProxyBuilderBase::setItem(iItem);
      if (iItem)
      {
         iItem->getConfig()->assertParam("Draw Tracks", false);
         iItem->getConfig()->assertParam("Draw Ellipse", false);
         iItem->getConfig()->assertParam("Scale Ellipse", 3l, 1l, 100l);
      }
   }
   
   REGISTER_PROXYBUILDER_METHODS();
   
private:
   FWVertexProxyBuilder(const FWVertexProxyBuilder&); // stop default
   const FWVertexProxyBuilder& operator=(const FWVertexProxyBuilder&); // stop default

   virtual void build(const reco::Vertex& iData, unsigned int iIndex,TEveElement& oItemHolder, const FWViewContext*);
   // virtual bool haveSingleProduct() const { return false; }
   // virtual void buildViewType(const reco::Vertex& iData, unsigned int iIndex, TEveElement& oItemHolder, FWViewType::EType type , const FWViewContext*);

};


void
FWVertexProxyBuilder::build(const reco::Vertex& iData, unsigned int iIndex, TEveElement& oItemHolder , const FWViewContext*)
{
   const reco::Vertex & v = iData;
  
   // marker
   TEveGeoManagerHolder gmgr(TEveGeoShape::GetGeoMangeur());
   TEvePointSet* pointSet = new TEvePointSet();
   pointSet->SetNextPoint( v.x(), v.y(), v.z() );  
   setupAddElement(pointSet, &oItemHolder);
  

   // ellipse
   if ( item()->getConfig()->value<bool>("Draw Ellipse"))
   {
    
      TEveEllipsoid* eveEllipsoid = new TEveEllipsoid(Form("Ellipsod %d", iIndex), "vtx"); 

      eveEllipsoid->RefPos().Set(v.x(),v.y(),v.z());

      reco::Vertex::Error e= v.error();      
      TMatrixDSym m(3);
      for(int i=0;i<3;i++)
         for(int j=0;j<3;j++)
         {
            m(i,j) = e(i,j);
            eveEllipsoid->RefEMtx()(i+1, j+1) =  e(i,j);
         }

      // external scaling
      double ellipseScale = 1.;
      if ( item()->getConfig()->value<long>("Scale Ellipse"))
         ellipseScale = item()->getConfig()->value<long>("Scale Ellipse");
     
      eveEllipsoid->SetScale(ellipseScale);

      // cache 3D extend used in eval bbox and render 3D
      TMatrixDEigen eig(m);
      TVectorD vv ( eig.GetEigenValuesRe());
      eveEllipsoid->RefExtent3D().Set(sqrt(vv(0))*ellipseScale,sqrt(vv(1))*ellipseScale,sqrt(vv(2))*ellipseScale); 

      setupAddElement(eveEllipsoid, &oItemHolder);
   }

   // tracks
   if ( item()->getConfig()->value<bool>("Draw Tracks")) 
   {    
      for(reco::Vertex::trackRef_iterator it = v.tracks_begin() ;
          it != v.tracks_end()  ; ++it)
      {
         float w = v.trackWeight(*it);
         if (w < 0.5) continue;
      
         const reco::Track & track = *it->get();
         TEveRecTrack t;
         t.fBeta = 1.;
         t.fV = TEveVector(track.vx(), track.vy(), track.vz());
         t.fP = TEveVector(track.px(), track.py(), track.pz());
         t.fSign = track.charge();
         TEveTrack* trk = new TEveTrack(&t, context().getTrackPropagator());
         trk->SetMainColor(item()->defaultDisplayProperties().color());
         trk->MakeTrack(); 
         setupAddElement(trk, &oItemHolder);
      }
   }
  
}

//
// static member functions
//
REGISTER_FWPROXYBUILDER(FWVertexProxyBuilder, reco::Vertex, "Vertices", FWViewType::k3DBit | FWViewType::kAllRPZBits);
