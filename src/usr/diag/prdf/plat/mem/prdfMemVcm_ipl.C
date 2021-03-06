/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/plat/mem/prdfMemVcm_ipl.C $                 */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/** @file prdfMemVcm_ipl.C */

// Platform includes
#include <prdfMemVcm.H>
#include <prdfMemScrubUtils.H>
#include <prdfPlatServices.H>
#include <prdfP9McaExtraSig.H>

using namespace TARGETING;

namespace PRDF
{

using namespace PlatServices;

//------------------------------------------------------------------------------

template<>
uint32_t VcmEvent<TYPE_MCA>::nextStep( STEP_CODE_DATA_STRUCT & io_sc,
                                       bool & o_done )
{
    #define PRDF_FUNC "[VcmEvent<TYPE_MCA>::nextStep] "

    uint32_t o_rc = SUCCESS;
    o_done = false;

    // add iv_mark to the callout list
    MemoryMru memmru( iv_chip->getTrgt(), iv_rank, iv_mark.getSymbol() );
    io_sc.service_data->SetCallout( memmru );

    do
    {
        //phase 0
        if ( TD_PHASE_0 == iv_phase )
        {
            //start VCM phase 1
            o_rc = startVcmPhase1<TYPE_MCA>( iv_chip, iv_rank );
            if ( SUCCESS != o_rc )
            {
                PRDF_ERR( PRDF_FUNC "Call to 'startVcmPhase1 failed on chip: "
                          "0x%08x", iv_chip->getHuid() );
                break;
            }
            iv_phase = TD_PHASE_1;
        }
        //phase 1
        else if ( TD_PHASE_1 == iv_phase )
        {
            //get the ecc attentions
            uint32_t eccAttns;
            o_rc = checkEccFirs<TYPE_MCA>( iv_chip, eccAttns );
            if ( SUCCESS != o_rc )
            {
                PRDF_ERR( PRDF_FUNC "Call to 'checkEccFirs' failed on chip: "
                          "0x%08x", iv_chip->getHuid() );
                break;
            }

            //if there was a UE or IUE
            if ( (eccAttns & MAINT_UE) || (eccAttns & MAINT_IUE) )
            {
                //UE
                if ( eccAttns & MAINT_UE )
                {
                    io_sc.service_data->setSignature( iv_chip->getHuid(),
                                                      PRDFSIG_MaintUE );
                }
                //IUE
                else
                {
                    io_sc.service_data->setSignature( iv_chip->getHuid(),
                                                      PRDFSIG_MaintIUE );
                }

                //Add the rank to the callout list (via MemoryMru)
                MemoryMru memmru(iv_chip->getTrgt(), iv_rank,
                        MemoryMruData::CALLOUT_RANK);
                io_sc.service_data->SetCallout( memmru );

                //Make the error log predictive
                io_sc.service_data->setServiceCall();

                //Leave the mark in place
                //Abort this procedure
                o_done = true;
            }
            else
            {
                //Start VCM phase 2
                o_rc = startVcmPhase2<TYPE_MCA>( iv_chip, iv_rank );
                if ( SUCCESS != o_rc )
                {
                    PRDF_ERR( PRDF_FUNC "Call to 'startVcmPhase2 failed on "
                              "chip: 0x%08x", iv_chip->getHuid() );
                    break;
                }
                iv_phase = TD_PHASE_2;
            }
        }
        //phase 2
        else if ( TD_PHASE_2 == iv_phase )
        {
            //get the ecc attentions
            uint32_t eccAttns;
            o_rc = checkEccFirs<TYPE_MCA>( iv_chip, eccAttns );
            if ( SUCCESS != o_rc )
            {
                PRDF_ERR( PRDF_FUNC "Call to 'checkEccFirs' failed on chip: "
                          "0x%08x", iv_chip->getHuid() );
                break;
            }

            //If DRAM repairs is disabled (via MNFG policy)
            if ( areDramRepairsDisabled() )
            {
                //Make the error log predictive
                io_sc.service_data->setServiceCall();
            }

            //if there was a UE or IUE
            if ( (eccAttns & MAINT_UE) || (eccAttns & MAINT_IUE) )
            {
                //UE
                if ( eccAttns & MAINT_UE )
                {
                    io_sc.service_data->setSignature( iv_chip->getHuid(),
                                                      PRDFSIG_MaintUE );
                }
                //IUE
                else
                {
                    io_sc.service_data->setSignature( iv_chip->getHuid(),
                                                      PRDFSIG_MaintIUE );
                }

                //Add the rank to the callout list (via MemoryMru)
                MemoryMru memmru(iv_chip->getTrgt(), iv_rank,
                        MemoryMruData::CALLOUT_RANK);
                io_sc.service_data->SetCallout( memmru );

                //Make the error log predictive
                io_sc.service_data->setServiceCall();

                //Leave the mark in place
            }
            //else if there was a MCE
            else if ( eccAttns & MAINT_MCE )
            {
                //The chip mark is verified
                io_sc.service_data->setSignature( iv_chip->getHuid(),
                                                  PRDFSIG_VcmVerified );

                //Update the DRAM repairs VPD to indicate the entire DRAM is bad
                //TODO ?????? - updating VPD not yet supported

            }
            //else - verification failed
            else
            {
                io_sc.service_data->setSignature( iv_chip->getHuid(),
                                                  PRDFSIG_VcmFalseAlarm );

                //Remove the chip mark
                o_rc = MarkStore::clearChipMark<TYPE_MCA>( iv_chip, iv_rank );
                if ( SUCCESS != o_rc )
                {
                    PRDF_ERR( PRDF_FUNC "Failure to clear chip mark on chip: "
                              "0x%08x", iv_chip->getHuid() );
                }

            }
            //Abort the procedure
            o_done = true;
        }
        else
        {
            PRDF_ASSERT( false ); //invalid value in iv_phase
        }
    }while(0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

// TODO: RTC 157888 Actual implementation of this procedure will be done later.
template<>
uint32_t VcmEvent<TYPE_MBA>::nextStep( STEP_CODE_DATA_STRUCT & io_sc,
                                       bool & o_done )
{
    #define PRDF_FUNC "[VcmEvent<TYPE_MBA>::nextStep] "

    uint32_t o_rc = SUCCESS;

    o_done = true;

    PRDF_ERR( PRDF_FUNC "function not implemented yet" );

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

} // end namespace PRDF

