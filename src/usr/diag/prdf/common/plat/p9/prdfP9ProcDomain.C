/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/common/plat/p9/prdfP9ProcDomain.C $         */
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
//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <prdfGlobal.H>
#include <iipstep.h>
#include <iipsdbug.h>
#include <iipErrorRegister.h>
#include <iipServiceDataCollector.h>
#include <prdfP9ProcDomain.H>
#include <UtilHash.H>
#include <prdfPluginDef.H>

#include <prdfPlatServices.H>

#include <algorithm>

using namespace TARGETING;

namespace PRDF
{

using namespace PlatServices;

int32_t ProcDomain::Analyze(STEP_CODE_DATA_STRUCT & serviceData,
                              ATTENTION_TYPE attentionType)
{
    int32_t l_rc;
    l_rc = DomainContainer<RuleChip>::Analyze(serviceData, attentionType);


    if( l_rc == PRD_POWER_FAULT )
    {
        PRDF_ERR( "PrdfProcDomain::Analyze::Power Fault detected!" );
    }
    else
    {
        // Capture Global FIRs on xstp and recovered errors for domain.
        if ((attentionType == MACHINE_CHECK) || (attentionType == RECOVERABLE))
        {
            // start at 1 to skip analyzed
            for (uint32_t i = 1; i < GetSize(); ++i)
            {
                LookUp(i)->CaptureErrorData(
                                    serviceData.service_data->GetCaptureData(),
                                    Util::hashString("GlobalFIRs"));

                if (attentionType == MACHINE_CHECK)
                {
                    LookUp(i)->CaptureErrorData(
                                    serviceData.service_data->GetCaptureData(),
                                    Util::hashString("AllFIRs"));
                }
            }
        }
    }

    return l_rc;
}

void ProcDomain::Order(ATTENTION_TYPE attentionType)
{

    if (attentionType == MACHINE_CHECK)
    {
        SortForXstop();

    }
    else if (attentionType == RECOVERABLE)
    {
        SortForRecov();
    }
    else // Recovered or Special
    {
        SYSTEM_DEBUG_CLASS sysdbug;
        for (int32_t i = (GetSize() - 1); i >= 0; --i)
        {
            RuleChip * l_procChip = LookUp(i);
            TARGETING::TargetHandle_t l_pchipHandle =
                                                l_procChip->GetChipHandle();
            bool l_analysisPending =
                sysdbug.isActiveAttentionPending(l_pchipHandle, attentionType );
            if ( l_analysisPending )
            {
                MoveToFront(i);
                break;
            }
        }
    }
}


// Determine the proper sorting for a checkstop based on:
//         1. Find only a single chip with an internal checkstop
//         2. Graph reduction algorithm
//         3. WOF/TOD counters
void ProcDomain::SortForXstop()
{
    using namespace PluginDef;
    using namespace TARGETING;

    uint32_t l_internalOnlyCount = 0;
    int l_chip = 0;

    uint64_t l_externalDrivers[GetSize()];
    uint64_t l_wofValues[GetSize()];
    bool l_internalCS[GetSize()];

    memset( &l_externalDrivers[0], 0x00, sizeof(l_externalDrivers) );
    memset( &l_wofValues[0],       0x00, sizeof(l_wofValues)       );
    memset( &l_internalCS[0],      0x00, sizeof(l_internalCS)      );

    union { uint64_t * u; CPU_WORD * c; } ptr;
    SYSTEM_DEBUG_CLASS sysDebug;
    TargetHandle_t node = NULL;

    if( false == isSmpCoherent() )
    {
        // Before node stitching, any system CS is only limited
        // to a node. ATTN will only pass us proc chips belonging
        // to a single node. In this scenario, we should only consider
        // chips belonging to one node.
        TargetHandle_t proc = sysDebug.getTargetWithAttn( TYPE_PROC,
                                                          MACHINE_CHECK);
        node = getConnectedParent( proc, TYPE_NODE);
        if( NULL == node )
        {
            // We should never reach here. Even if we reach here, as this is
            // XSTOP, we would like to go ahead with analysis to have as much
            // data as possible. So just print a trace.
            PRDF_ERR("[ProcDomain::SortForXstop] Node is Null");
        }
    }

    // Get internal setting and external driver list for each chip.
    for(uint32_t i = 0; i < GetSize(); ++i)
    {
        l_externalDrivers[i] = 0;
        l_wofValues[i] = 0;

        RuleChip * l_procChip = LookUp(i);

        // if it is a node check stop, limit the scope of sorting only to the
        // node which is causing ATTN to invoke PRD.
        if ( ( NULL != node ) && ( node != getConnectedParent(
                                    l_procChip->GetChipHandle(), TYPE_NODE) ))
            continue;

        ptr.u = &l_externalDrivers[i];
        BitString l_externalChips(GetSize(), ptr.c);
        TargetHandleList l_tmpList;

        // Call "GetCheckstopInfo" plugin.
        ExtensibleChipFunction * l_extFunc
            = l_procChip->getExtensibleFunction("GetCheckstopInfo");

        (*l_extFunc)(l_procChip,
                     bindParm<bool &, TargetHandleList &, uint64_t &>
                        (l_internalCS[i],
                         l_tmpList,
                         l_wofValues[i]
                     )
        );


        // Update bit buffer.
        for (auto j = l_tmpList.begin();
             j != l_tmpList.end(); ++j)
        {
            for (uint32_t k = 0; k < GetSize(); k++)
                if ((*j) == LookUp(k)->GetChipHandle())
                    l_externalChips.Set(k);
        };

        // Check if is internal.
        if (l_internalCS[i])
        {
            l_internalOnlyCount++;
            l_chip = i;
        }
    }

    // Check if we are done... only one with an internal error.
    if (1 == l_internalOnlyCount)
    {
        MoveToFront(l_chip);
        return;
    }
    else if (0 == l_internalOnlyCount)
    {
        PRDF_TRAC("[ProcDomain::SortForXstop] continue "
                  "with analysis to determine which chip "
                  "sourced the error.");
    }

    // --- Do graph reduction ---
    // Currently does not do cycle elimination.

    // Get initial list (all chips).
    BIT_STRING_BUFFER_CLASS l_current(GetSize());
    l_current.Pattern(0,GetSize(),0xFFFFFFFF, 32); // turn on all bits.

    // Do reduction.
    // When done, l_prev will have the minimal list.
    BIT_STRING_BUFFER_CLASS l_prev(GetSize());
    l_prev.Clear();

    while ((!(l_current == l_prev)) && (!l_current.IsZero()))
    {
        l_prev = l_current;
        l_current.Clear();

        for (uint32_t i = 0; i < GetSize(); i++)
        {
            if (l_prev.IsSet(i)) // skip if this chip isn't in the pool.
                for (uint32_t j = 0; j < GetSize(); j++)
                {
                    ptr.u = &l_externalDrivers[i]; // zs01
                    if ( BitString(GetSize(), ptr.c).IsSet(j) )
                        l_current.Set(j);
                }
        }
    }

    // Hopefully, we got just one chip left...
    if (1 == l_prev.GetSetCount())
    {
        // Now find it.
        for (uint32_t i = 0; i < GetSize(); i++)
            if ((l_prev.IsSet(i)) &&
                (l_internalCS[i] || (0 == l_internalOnlyCount)))
            {
                MoveToFront(i);
                return;
            }
    }

    // --- Do WOF compare ---
    uint32_t l_minWof = 0;
    for (uint32_t i = 0; i < GetSize(); i++)
    {
        // Search for minimum WOF value.
        if (l_wofValues[i] < l_wofValues[l_minWof])
                // Only choose chips with internal checkstop,
                // unless no internals.
            if ((l_internalCS[i] || (0 == l_internalOnlyCount)))
                l_minWof = i;
    }
    MoveToFront(l_minWof);
    return;
}

namespace __prdfProcDomain
{
    // This function is used for the std::max_element function in SortForRecov
    // to ensure that elements towards the end of the list are favored (and
    // therefore preventing starvation of chips at the end of the domain list)
    inline bool lessThanOperator(uint32_t & l, uint32_t & r)
    {
        if (l == r)
        {
            return ((void *)&l) < ((void *)&r);
        }
        return l < r;
    }
};

void ProcDomain::SortForRecov()
{
    using namespace PluginDef;

    SYSTEM_DEBUG_CLASS sysdbug;
    uint32_t l_sev[GetSize()];
    std::fill(&l_sev[0], &l_sev[GetSize()], 0);

    // Loop through all chips.
    for ( uint32_t i = 0; i < GetSize(); ++i )
    {
        RuleChip * l_procChip = LookUp(i);
        TARGETING::TargetHandle_t l_pchipHandle = l_procChip->GetChipHandle();

        //check if chip has an attention which has not been analyzed as yet
        if( sysdbug.isActiveAttentionPending( l_pchipHandle, RECOVERABLE ) )
        {
            // Find severity level.
            ExtensibleChipFunction * l_extFunc
                    = l_procChip->getExtensibleFunction(
                                                "CheckForRecoveredSev");

            (*l_extFunc)(l_procChip, bindParm<uint32_t &>( l_sev[i] ));
        }

    }

    // Find item with highest severity.
    MoveToFront(std::distance(&l_sev[0],
                              std::max_element(&l_sev[0],
                                              &l_sev[GetSize()],
                             __prdfProcDomain::lessThanOperator)));
}

//Analyze a subset of chips in a Domain...
//This is a mini analysis of some of the chips in the Proc Domain.
int32_t ProcDomain::AnalyzeTheseChips(STEP_CODE_DATA_STRUCT & serviceData,
                                      ATTENTION_TYPE attentionType,
                                      TARGETING::TargetHandleList & i_chips)
{
    using namespace TARGETING ;
    PRDF_DENTER( "ProcDomain::AnalyzeTheseChips" );
    int32_t l_rc = ~SUCCESS;

    PRDF_DTRAC( "ProcDomain::AnalyzeTheseChips:: Domain ID = 0x%X", GetId() );

    if(i_chips.size() != 0)
    {
        OrderTheseChips(attentionType, i_chips);

        // After the Order function is called the first chip should contain the
        // chip to look at.
        RuleChip * l_procChip = FindChipInTheseChips(i_chips[0], i_chips);
        PRDF_DTRAC( "ProcDomain::AnalyzeTheseChips::Analyzing this one: 0x%X",
                    l_procChip->GetId() );
        if(NULL != l_procChip)
        {
            l_rc = l_procChip->Analyze(serviceData, attentionType);
        }
        else
        {
            PRDF_TRAC( "ProcDomain::AnalyzeTheseChips::l_procChip is NULL" );
            l_rc = ~SUCCESS;
        }
    }
    else
    {
        PRDF_TRAC( "ProcDomain::AnalyzeTheseChips::i_chips = %d",
                    i_chips.size() );
    }

    //Get Proc chip Global FIR data for FFDC
    for (auto i = i_chips.begin(); i != i_chips.end(); ++i)
    {
        RuleChip * l_procChip = FindChipInTheseChips(*i, i_chips);
        l_procChip->CaptureErrorData(
                                    serviceData.service_data->GetCaptureData(),
                                    Util::hashString("GlobalFIRs"));
    }

    PRDF_DEXIT( "ProcDomain::AnalyzeTheseChips" );
    return l_rc;
}

int32_t ProcDomain::OrderTheseChips(ATTENTION_TYPE attentionType,
                                      TARGETING::TargetHandleList & i_chips)
{
    using namespace PluginDef;
    using namespace TARGETING;
    PRDF_DENTER( "ProcDomain::OrderTheseChips" );

    uint32_t l_internalOnlyCount = 0;
    uint64_t l_externalDrivers[i_chips.size()];
    uint64_t l_wofValues[i_chips.size()];
    bool l_internalCS[i_chips.size()];

    union { uint64_t * u; CPU_WORD * c; } ptr;

    uint32_t l_chip = 0;
    uint32_t l_chipToFront = 0;
    // Get internal setting and external driver list for each chip.
    for (auto i = i_chips.begin();
         i != i_chips.end(); ++i)
    {

        RuleChip * l_procChip = FindChipInTheseChips(*i, i_chips);

        ptr.u = &l_externalDrivers[l_chip];
        BitString l_externalChips(i_chips.size(), ptr.c);
        TargetHandleList l_tmpList;

        if(l_procChip != NULL)
        {
            // Call "GetCheckstopInfo" plugin.
            ExtensibleChipFunction * l_extFunc
                = l_procChip->getExtensibleFunction("GetCheckstopInfo");

            (*l_extFunc)(l_procChip,
                     bindParm<bool &, TargetHandleList &, uint64_t &>
                        (l_internalCS[l_chip],
                         l_tmpList,
                         l_wofValues[l_chip]
                     )
            );
        }
        else
        {
            l_internalCS[l_chip] = false;
            PRDF_DTRAC( "ProcDomain::OrderTheseChips: l_procChip is NULL" );
        }

        //If we are just checking for internal errors then there is no need for
        //a list of what chips sent checkstops where.
        // Update bit buffer.
        for (auto j = l_tmpList.begin();
             j != l_tmpList.end();
             ++j)
        {
            for (uint32_t k = 0; k < i_chips.size(); k++)
                if ((*j) == LookUp(k)->GetChipHandle())
                    l_externalChips.Set(k);
        };

        // Check if is internal.
        if (l_internalCS[l_chip])
        {
            l_internalOnlyCount++;
            l_chipToFront = l_chip;
        }
        l_chip++;  //Move to next chip in the list.
    }

    // Check if we are done... only one with an internal error.
    if (1 == l_internalOnlyCount)
    {
        MoveToFrontInTheseChips(l_chipToFront, i_chips);
        return(SUCCESS);
    }

    PRDF_DEXIT( "ProcDomain::OrderTheseChips" );
    return(SUCCESS);
}

RuleChip * ProcDomain::FindChipInTheseChips(TargetHandle_t i_pchipHandle,
                                            TargetHandleList & i_chips)
{
    using namespace TARGETING;

    PRDF_DENTER( "ProcDomain::FindChipNumber" );
    RuleChip * l_procChip = NULL;
    TargetHandle_t l_tmpprocHandle= NULL;
    // Loop through all chips.
    for (auto iter = i_chips.begin();
         iter != i_chips.end(); ++iter)
    {
        for (uint32_t i = 0; i < GetSize(); ++i)
        {
            l_procChip = LookUp(i);
            l_tmpprocHandle = l_procChip->GetChipHandle();
            if( (l_tmpprocHandle == (*iter)) &&
                (l_tmpprocHandle == i_pchipHandle) ) return(l_procChip);
        }
    }

    PRDF_DEXIT( "ProcDomain::FindChipNumber" );
    return(NULL);
}

//Swaps chip at location 0 with a chip at location i_chipToFront
void ProcDomain::MoveToFrontInTheseChips(uint32_t i_chipToFront,
                                         TargetHandleList & i_chips)
{
    for (auto i = i_chips.begin()+i_chipToFront;
         i != i_chips.begin(); i--)
    {
        std::swap((*i), (*(i-1)));
    }
}

} //end namespace PRDF
