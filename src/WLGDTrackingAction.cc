#include "WLGDTrackingAction.hh"
#include "WLGDTrackInformation.hh"
#include "WLGDTrajectory.hh"

#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"c
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4UnitsTable.hh"
#include <map>

WLGDTrackingAction::WLGDTrackingAction() = default;

void WLGDTrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{
  // Create trajectory for track if requested
  if(fpTrackingManager->GetStoreTrajectory() > 0)
  {
    fpTrackingManager->SetTrajectory(new WLGDTrajectory(aTrack));
  }

  if(aTrack->GetParticleDefinition()->GetParticleName() == "neutron")
  {
    auto tmp_vector = aTrack->GetVertexPosition();
    tmp_neutronXpos = tmp_vector.getX() / m;
    tmp_neutronYpos = tmp_vector.getY() / m;
    tmp_neutronZpos = tmp_vector.getZ() / m;
    tmp_vector      = aTrack->GetMomentumDirection();
    tmp_neutronXmom = tmp_vector.getX();
    tmp_neutronYmom = tmp_vector.getY();
    tmp_neutronZmom = tmp_vector.getZ();
    // G4cout << "Position of Neutron: " << tmp_neutronXpos << " " << tmp_neutronYpos << "
    // " << tmp_neutronZpos << G4endl;
    fEventAction->IncreaseByOne_NeutronInEvent();
    if(aTrack->GetLogicalVolumeAtVertex()->GetName() == "Lar_log" || aTrack->GetLogicalVolumeAtVertex()->GetName() == "ULar_log" || aTrack->GetLogicalVolumeAtVertex()->GetName() == "Ge_log" || aTrack->GetLogicalVolumeAtVertex()->GetName() == "Copper_log")// ULar_phys  Ge_phys
    {
      fRunAction->increaseTotalNumberOfNeutronsInLAr();
      fRunAction->addCoordinatsToFile(tmp_neutronXpos, tmp_neutronYpos, tmp_neutronZpos);
      fRunAction->addMomentumToFile(tmp_neutronXmom, tmp_neutronYmom, tmp_neutronZmom);
      fRunAction->addEnergyToFile(aTrack->GetKineticEnergy()/eV);
//	G4cout << "Neutron produced by: "  << (fEventAction->neutronProducerMap.find((int)aTrack->GetParentID())->second) << " " << aTrack->GetParentID() << G4endl;
      fRunAction->addParentParticleType(fEventAction->neutronProducerMap.find(aTrack->GetParentID())->second); 
    }
  }
}

void WLGDTrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{
  G4TrackVector* secondaries = fpTrackingManager->GimmeSecondaries();
  if(secondaries != nullptr)
  {
    WLGDTrackInformation* info  = (WLGDTrackInformation*) (aTrack->GetUserInformation());
    size_t                nSeco = secondaries->size();
    if(nSeco > 0)
    {
      for(size_t i = 0; i < nSeco; i++)
      {
        WLGDTrackInformation* infoNew = new WLGDTrackInformation(info);
        (*secondaries)[i]->SetUserInformation(infoNew);
        if((*secondaries)[i]->GetParticleDefinition()->GetParticleName() == "neutron"){// && aTrack->GetParticleDefinition()->GetParticleName() != "neutron"){
	 	//G4cout << aTrack->GetParticleDefinition()->GetPDGEncoding() << G4endl;
	       	fEventAction->neutronProducerMap.insert(std::make_pair((int)aTrack->GetTrackID(),(int)aTrack->GetParticleDefinition()->GetPDGEncoding()));
        }
      }
    }
  }
  if(aTrack->GetParticleDefinition()->GetParticleName() == "neutron")
  {
    // G4cout
    // << "Step Process: "
    // << aTrack->GetStep()->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName()
    // << G4endl;
    if(aTrack->GetStep()->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() ==
       "biasWrapper(nCapture)")
    {
      // G4cout << "Got through nCapture" << G4endl;
      int NumberOfSecundaries = aTrack->GetStep()->GetSecondaryInCurrentStep()->size();
      for(int i = 0; i < NumberOfSecundaries; i++)
      {
        // G4cout << "Looking for Ge77" << G4endl;
        if(aTrack->GetStep()
               ->GetSecondaryInCurrentStep()
               ->at(i)
               ->GetParticleDefinition()
               ->GetAtomicMass() == 77 &&
           aTrack->GetStep()
               ->GetSecondaryInCurrentStep()
               ->at(i)
               ->GetParticleDefinition()
               ->GetPDGCharge() == 32)
        {
          double tmp_x, tmp_y, tmp_z;
          tmp_x = aTrack->GetStep()->GetPostStepPoint()->GetPosition().getX() / m;
          tmp_y = aTrack->GetStep()->GetPostStepPoint()->GetPosition().getY() / m;
          tmp_z = aTrack->GetStep()->GetPostStepPoint()->GetPosition().getZ() / m;

          G4cout << "Got it!" << G4endl;
          G4cout << "Position: " << tmp_neutronXpos << " " << tmp_neutronYpos << " "
                 << tmp_neutronZpos << G4endl;
          G4cout << "Direction: " << tmp_neutronXmom << " " << tmp_neutronYmom << " "
                 << tmp_neutronZmom << G4endl;
          G4cout << "Energy: "
                 << aTrack->GetStep()->GetPreStepPoint()->GetKineticEnergy() / eV
                 << G4endl;
          G4cout << "Position of generated Ge-77: " << tmp_x << " " << tmp_y << " "
                 << tmp_z << G4endl;

          fEventAction->AddEkin(aTrack->GetStep()->GetPreStepPoint()->GetKineticEnergy() /
                                eV);
          fEventAction->AddNeutronxLoc(tmp_neutronXpos);
          fEventAction->AddNeutronyLoc(tmp_neutronYpos);
          fEventAction->AddNeutronzLoc(tmp_neutronZpos);
          fEventAction->AddNeutronxMom(tmp_neutronXmom);
          fEventAction->AddNeutronyMom(tmp_neutronYmom);
          fEventAction->AddNeutronzMom(tmp_neutronZmom);
        }
      }
    }
  }
}
