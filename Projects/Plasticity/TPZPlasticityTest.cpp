#include <iostream>
#include <cstdlib>


#include "TPZPlasticityTest.h"

/// Default constructor
TPZPlasticityTest::TPZPlasticityTest() : fZStressKnown(false), fRStressKnown(false), fPoreStressRZ(2,0.), fPoreStrainRZ(2,0.), fStressRZInput(0,2,0.),
                fStrainRZInput(0,2,0.), fStressRZSimulated(0,2,0.), fStrainRZSimulated(0,2,0.)
{
    fNumSteps = 0;
    fPoreClosureIndex = 0;
}

/// Get the stress to the pore stress
void TPZPlasticityTest::ApplyInitialStress()
{
    // load hydrostatic till I1 is in the middle of the ellips
    STATE I1 = fPoreStressRZ[0]*2.+fPoreStressRZ[1];
    STATE I1Initial = I1-fSandler.fYC.fA*fSandler.fYC.fR;
    
cout << "----------------------------> " << I1Initial << " " << I1 << " " << fSandler.fYC.fA << " " << fSandler.fYC.fR << " " << fPoreStressRZ[0] << " " << fPoreStressRZ[1] << endl;
    
    // should improve!!defLateral->value(i)
    TPZTensor<STATE> hidro, epstotal;
    for (STATE i1 = I1Initial/10.; i1>=I1Initial; i1 += I1Initial/10.) {
        hidro.XX() = i1/3.;
        hidro.YY() = i1/3.;
        hidro.ZZ() = i1/3.;
cout << "----------------------------> i1 " << i1  << " " << hidro.XX() << endl; 
        fSandler.ApplyLoad(hidro, epstotal);
    }
    
    // load elastic to the initial stress
    TPZTensor<STATE> porestress;
    porestress.XX() = fPoreStressRZ[0];
    porestress.YY() = fPoreStressRZ[0];
    porestress.ZZ() = fPoreStressRZ[1];
    fSandler.ApplyLoad(porestress, epstotal);
    fPoreStrainRZ[0] = epstotal.XX();
    if (fabs(epstotal.XX()-epstotal.YY()) > 1.e-8) {
        DebugStop();
    }
    fPoreStrainRZ[1] = epstotal.ZZ();
    
}

/// Evoluate the stress and strain to step ist
void TPZPlasticityTest::EvoluateToStep(TPZVec<STATE> &strainRZ, TPZVec<STATE> &stressRZ)
{
    TPZTensor<STATE> strain,stressgoal;
    TPZPlasticState<STATE> locstate;
    locstate = fSandler.GetState();
    STATE residual;
    
    strain.XX() = strainRZ[0];
    strain.YY() = strain.XX();
    strain.ZZ() = strainRZ[1];
    // if the stress is imposed, then this is its target value
    stressgoal.XX() = stressRZ[0];
    stressgoal.YY() = stressgoal.XX();
    stressgoal.ZZ() = stressRZ[1];
    do {
        
        fSandler.SetState(locstate);
        fSandler.fYC.fIsonCap = false;
        TPZFNMatrix<36,STATE> dep(6,6,0.);
        TPZTensor<STATE> stress;
        if(fRStressKnown || fZStressKnown)
        {
            fSandler.ApplyStrainComputeDep(strain, stress, dep);
        }
        else {
            fSandler.ApplyStrainComputeSigma(strain, stress);
        }
        residual = 0.;
        if (fRStressKnown) {
            residual += fabs(stress.XX()-stressgoal.XX());
        }
        if (fZStressKnown) {
            residual += fabs(stress.ZZ()-stressgoal.ZZ());
        }
        if (fabs(stress.XX()-stress.YY()) > 1.e-8) {
            DebugStop();
        }
        TPZFNMatrix<4,STATE> depsmall(2,2,0.);
        depsmall(0,0) = dep(_XX_,_XX_)+dep(_XX_,_YY_);
        depsmall(0,1) = dep(_XX_,_ZZ_);
        depsmall(1,0) = dep(_ZZ_,_XX_)+dep(_ZZ_,_YY_);
        depsmall(1,1) = dep(_ZZ_,_ZZ_);
        
        if (!fRStressKnown) {
            depsmall(0,0) = 1.;
            depsmall(0,1) = 0.;
            depsmall(1,0) = 0.;
            stress.XX() = stressgoal.XX();
        }
        if (!fZStressKnown) {
            depsmall(1,1) = 1.;
            depsmall(0,1) = 0.;
            depsmall(1,0) = 0.;
            stress.ZZ() = stressgoal.ZZ();
        }
        
        STATE det = depsmall(0,0)*depsmall(1,1)-depsmall(0,1)*depsmall(1,0);
        TPZFNMatrix<4,STATE> depsmallinv(2,2);
        depsmallinv(0,0) = depsmall(1,1)/det;
        depsmallinv(1,1) = depsmall(0,0)/det;
        depsmallinv(1,0) = -depsmall(1,0)/det;
        depsmallinv(0,1) = -depsmall(0,1)/det;
        
        TPZManVector<STATE,2> stressres(2,0.);
        stressres[0] = -stressgoal.XX()+stress.XX();
        stressres[1] = -stressgoal.ZZ()+stress.ZZ();
        
        strain.XX() -= depsmallinv(0,0)*stressres[0]+depsmallinv(0,1)*stressres[1];
        strain.YY() = strain.XX();
        strain.ZZ() -= depsmallinv(1,0)*stressres[0]+depsmallinv(1,1)*stressres[1];
        
        // maybe a line search is in order??
        
        
        
    } while (residual > 1.e-8);
    
    fSandler.SetState(locstate);
    TPZTensor<STATE> sigma;
    fSandler.ApplyStrainComputeSigma(strain, sigma);
    
    strainRZ[0] = strain.XX();
    strainRZ[1] = strain.ZZ();
    stressRZ[0] = sigma.XX();
    stressRZ[1] = sigma.ZZ();
    
}

/// compute the stress strain curve
void TPZPlasticityTest::PerformSimulation()
{
    ApplyInitialStress();
    int nloadsteps = fStressRZInput.Rows();
    fStrainRZSimulated.Redim(nloadsteps, 2);
    fStressRZSimulated.Redim(nloadsteps, 2);
    for (int istep = fPoreClosureIndex; istep < nloadsteps; istep++) {
        TPZManVector<STATE,2> strainRZ(2),stressRZ(2);
        strainRZ[0] = fStrainRZInput(istep,0);
        stressRZ[0] = fStressRZInput(istep,0);
        strainRZ[1] = fStrainRZInput(istep,1);
        stressRZ[1] = fStressRZInput(istep,1);
        
        strainRZ[0] -= fStrainRZInput(fPoreClosureIndex,0);
        strainRZ[1] -= fStrainRZInput(fPoreClosureIndex,1);
        strainRZ[0] += this->fPoreStrainRZ[0];
        strainRZ[1] += this->fPoreStrainRZ[1];
        if(istep == 1494)
        {
            std::cout << __FUNCTION__ << std::endl;
        }
        EvoluateToStep(strainRZ, stressRZ);
//        fStrainRZSimulated(istep,0) = strainRZ[0];
//        fStrainRZSimulated(istep,1) = strainRZ[1];
        fStrainRZSimulated(istep,0) = fStrainRZInput(istep,0);
        fStrainRZSimulated(istep,1) = fStrainRZInput(istep,1);
        fStressRZSimulated(istep,0) = stressRZ[0];
        fStressRZSimulated(istep,1) = stressRZ[1];
        
    }
    
    
}

/// read the input strain and stress from the laboratory file
void TPZPlasticityTest::ReadInputStrainStress(const std::string &filename)
{
    std::ifstream input(filename.c_str());
    if (!input) {
        DebugStop();
    }
    int numlines = 0;
    char buf[1024];
    input.getline(buf , 1024);
    STATE x, sig_ax_t, tempo, sig_ax_dev, sig_r, eps_ax, eps_r, eps_v;
    while (input) {
        input >> x >> sig_ax_t >> tempo >> sig_ax_dev >> tempo >> sig_r >> tempo >> eps_ax >> tempo >> eps_r >> tempo >> eps_v;
        if (!input) {
            break;
        }
        if(numlines >= fStressRZInput.Rows())
        {
            fStressRZInput.Resize(numlines+100, 2);
            fStrainRZInput.Resize(numlines+100, 2);
        }
        fStressRZInput(numlines,1) = -sig_ax_t;
        fStressRZInput(numlines,0) = -sig_r;
        fStrainRZInput(numlines,1) = -eps_ax/100.;
        fStrainRZInput(numlines,0) = -eps_r/100.;
	
	cout << "i= " << numlines << " " <<  fStressRZInput(numlines,1) << " " <<  fStressRZInput(numlines,0)<< " " << fStrainRZInput(numlines,1)<< " "<< fStrainRZInput(numlines,0) << endl;
	
        numlines++;
    }
    fStressRZInput.Resize(numlines, 2);
    fStrainRZInput.Resize(numlines, 2);
    
}
