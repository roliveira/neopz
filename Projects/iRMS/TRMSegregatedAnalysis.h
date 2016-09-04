//
//  TRMSegregatedAnalysis.hpp
//  PZ
//
//  Created by Omar on 7/18/16.
//
//

#ifndef TRMSegregatedAnalysis_h
#define TRMSegregatedAnalysis_h

#include <stdio.h>

#include <stdio.h>
#include "pzanalysis.h"
#include "pzstepsolver.h"
#include "pzbuildmultiphysicsmesh.h"
#include "TRMSimulationData.h"
#include "TRMBuildTransfers.h"
#include "TRMFluxPressureAnalysis.h"
#include "TRMTransportAnalysis.h"

class TRMSegregatedAnalysis : public TPZAnalysis {
    
private:
    
    /** @brief define the simulation data */
    TRMSimulationData * fSimulationData;
    
    /** @brief define the transfer matrices */
    TRMBuildTransfers * fTransfer;
    
    /** @brief define the parabolic system */
    TRMFluxPressureAnalysis * fParabolic;
    
    /** @brief define the hyperbolic system */
    TRMTransportAnalysis * fHyperbolic;
    
    /** @brief Residue error for flux - pressure */
    STATE ferror_flux_pressure;
    
    /** @brief Residue error for saturations */
    STATE ferror_saturation;
    
    /** @brief Correction variation for flux - pressure */
    STATE fdx_norm_flux_pressure;
    
    /** @brief Correction variation for saturations */
    STATE fdx_norm_saturation;

    
public:
    
    /** @brief default constructor  */
    TRMSegregatedAnalysis();
    
    /** @brief default desconstructor  */
    ~TRMSegregatedAnalysis();
    
    /** @brief Copy constructor $ */
    TRMSegregatedAnalysis(const TRMSegregatedAnalysis &copy);
    
    /** @brief Copy assignemnt operator $ */
    TRMSegregatedAnalysis &operator=(const TRMSegregatedAnalysis &other);
    
    /**
     * @defgroup Access Methods
     * @brief    Implements Access methods:
     * @{
     */
    
    
    /** @brief Set the simulation data */
    void SetSimulationData(TRMSimulationData * SimulationData)
    {
        fSimulationData = SimulationData;
    }
    
    /** @brief Get the space generator */
    TRMSimulationData * SimulationData()
    {
        return fSimulationData;
    }
    
    /** @brief Set the transfer object */
    void SetTransfer(TRMBuildTransfers *Transfer)
    {
        fTransfer = Transfer;
    }
    
    /** @brief Get the transfer object */
    TRMBuildTransfers * Transfer()
    {
        return fTransfer;
    }

    /** @brief Set the parabolic part */
    void SetParabolic(TRMFluxPressureAnalysis * parabolic)
    {
        fParabolic = parabolic;
    }
    
    /** @brief Get the parabolic part  */
    TRMFluxPressureAnalysis * Parabolic()
    {
        return fParabolic;
    }
    
    /** @brief Set the hyperbolic part */
    void SetHyperbolic(TRMTransportAnalysis * hyperbolic)
    {
        fHyperbolic = hyperbolic;
    }
    
    /** @brief Get the hyperbolic part  */
    TRMTransportAnalysis * Hyperbolic()
    {
        return fHyperbolic;
    }
    
    /** @brief Resize and fill residue and solution vectors */
    void AdjustVectors();
    
    // @}
    
    /**
     * @defgroup Time foward methods
     * @{
     */
    
    /** @brief Execute a euler method step */
    void ExcecuteOneStep(bool IsActiveQ);
    
    /** @brief Execute a newton iteration  */
    void NewtonIteration();
    
    /** @brief Execute a segregated iteration  */
    void SegregatedIteration(bool IsActiveQ);
    
    /** @brief PostProcess results */
    void PostProcessStep(bool IsActiveQ);

    /** @brief Update memory using the Transfer object at state n */
    void UpdateFluxes_at_n();
    
    /** @brief Update memory using the Transfer object at state n */
    void UpdateMemory_at_n();
    
    /** @brief Update memory using the Transfer object */
    void UpdateMemory();
    
    
    // @}
    
    
    
    
};


#endif /* TRMSegregatedAnalysis_h */