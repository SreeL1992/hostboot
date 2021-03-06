/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/hdat/hdatiplparms.C $                                 */
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


/**
 * @file hdatiplparms.C
 *
 * @brief This file contains the implementation of the HdatIplParms class.
 *
 */

/*------------------------------------------------------------------------*/
/* Includes                                                               */
/*------------------------------------------------------------------------*/
#include <ctype.h>                  // endian testing
#include "hdatiplparms.H"           // HdatIplParms class definition
#include <attributeenums.H>
#include "hdatutil.H"
#include <sys/mm.h>
#include <sys/mmio.h>
#include <vpd/mvpdenums.H>
#include <pnor/pnorif.H>
#include <util/align.H>

#include <devicefw/userif.H>
#include <targeting/common/util.H>

using namespace TARGETING;

namespace HDAT
{

extern trace_desc_t *g_trac_hdat;


/*------------------------------------------------------------------------*/
/* Constants                                                              */
/*------------------------------------------------------------------------*/

/**
 * @brief This routine checks if a certain manufacturing flag is set
 *
 * @pre None
 *
 * @post None
 *
 * @param i_flag - input parameter - Specific mnfg flag to check
 *
 * @return true  : The specified mnfg flag is set
 *         false : The specified mnfg flag is not set
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
static bool isMnfgFlagSet( uint32_t i_flag )
{
    bool o_rc = false;
    TARGETING::ATTR_MNFG_FLAGS_type l_attrValue = 0;
    TARGETING::TargetHandle_t l_pTopTarget= NULL;
    targetService().getTopLevelTarget(l_pTopTarget);

    if(l_pTopTarget)
    {
        l_attrValue = l_pTopTarget->getAttr<ATTR_MNFG_FLAGS>();
        o_rc = l_attrValue & i_flag;
    }
    else
    {
        HDAT_ERR("[isMnfgFlagSet] error finding l_pTopTarget");
    }

    return o_rc;
}


/**
 * @brief This routine gets number of cores in each processor
 *
 * @pre None
 *
 * @post None
 *
 * @param o_numCores - output parameter - Number of cores
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
static void hdatGetNumberOfCores(uint32_t &o_numCores)
{
    uint8_t          l_prData[9];
    size_t           l_prDataSz = 8;
    errlHndl_t       l_err = NULL;

    o_numCores = 0;


    TARGETING::PredicateCTM l_procChipPredicate(TARGETING::CLASS_CHIP,
                                             TARGETING::TYPE_PROC);

    TARGETING::PredicateHwas l_predHwas;
    l_predHwas.present(true);

    TARGETING::PredicatePostfixExpr l_presentProc;
    l_presentProc.push(&l_procChipPredicate).push(&l_predHwas).And();

    TARGETING::TargetRangeFilter l_procFilter(
                                        TARGETING::targetService().begin(),
                                        TARGETING::targetService().end(),
                                        &l_presentProc);

    TARGETING::Target *l_procTarget = (*l_procFilter);

    l_err = deviceRead(l_procTarget,l_prData,l_prDataSz,
                                    DEVICE_MVPD_ADDRESS(MVPD::VINI,MVPD::PR));

    if(l_err)
    {
        HDAT_ERR("Error during VPD PR keyword  read");
    }
    else
    {
        o_numCores = l_prData[2] >> 4;
        HDAT_DBG("Number of Cores: %d",o_numCores);
    }
}

/**
 * @brief This routine gets the information for Enlarged IO Slot Count
 *
 * @pre None
 *
 * @post None
 *
 * @param o_EnlargedSlotCount - output parameter - Enlarged IO Slot Count for
 *                                                 all nodes
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
static void hdatGetEnlargedIOCapacity(uint32_t &o_EnlargedSlotCount)
{
    TARGETING::PredicateCTM l_nodePredicate(TARGETING::CLASS_ENC,
                                             TARGETING::TYPE_NODE);
    TARGETING::PredicateHwas l_predHwas;
    l_predHwas.present(true);

    TARGETING::PredicatePostfixExpr l_presentNode;
    l_presentNode.push(&l_nodePredicate).push(&l_predHwas).And();

    TARGETING::TargetRangeFilter l_nodeFilter(
                                        TARGETING::targetService().begin(),
                                        TARGETING::targetService().end(),
                                        &l_presentNode);

    o_EnlargedSlotCount = 0;
    uint8_t l_nodeindex = 3;

    for (;l_nodeFilter;++l_nodeFilter)
    {
        TARGETING::Target *l_nodeTarget = (*l_nodeFilter);

        TARGETING::ATTR_ENLARGED_IO_SLOT_COUNT_type l_enlargedIOSlotCount;
        if(l_nodeTarget->tryGetAttr<TARGETING::ATTR_ENLARGED_IO_SLOT_COUNT>
                                                        (l_enlargedIOSlotCount))
        {
            o_EnlargedSlotCount |= (uint32_t)l_enlargedIOSlotCount <<
                                                       (8 * l_nodeindex);
        }
        else
        {
            HDAT_ERR("Error in getting ENLARGED_IO_SLOT_COUNT attribute");
        }
        l_nodeindex--;
    }
}

/**
 * @brief This routine gets the information for IPL Parameters
 *
 * @pre None
 *
 * @post None
 *
 * @param o_hdatOTA - output parameter - The structure to update with the
 *                                       other IPL attributes
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
static void hdatPopulateOtherIPLAttributes(hdatOtherIPLAttributes_t &o_hdatOTA)
{
    TARGETING::Target *l_pSysTarget = NULL;
    (void) TARGETING::targetService().getTopLevelTarget(l_pSysTarget);

    if(l_pSysTarget == NULL)
    {
        HDAT_ERR("Error in getting Top Level Target");
        assert(l_pSysTarget != NULL);
    }

    TARGETING::ATTR_IPL_ATTRIBUTES_type l_iplAttributes;
    l_iplAttributes = l_pSysTarget->getAttr<TARGETING::ATTR_IPL_ATTRIBUTES>();

    o_hdatOTA.hdatCreDefPartition = l_iplAttributes.createDefaultPartition;

    o_hdatOTA.hdatCTAState = l_iplAttributes.clickToAcceptState;

    o_hdatOTA.hdatDisVirtIOConn = l_iplAttributes.disableVirtIO;

    o_hdatOTA.hdatResetPCINOs = l_iplAttributes.resetPCINumbers;

    o_hdatOTA.hdatClrPhypNvram = l_iplAttributes.clearHypNVRAM;

    TARGETING::ATTR_PRESERVE_MDC_PARTITION_VPD_type l_preserveMDCPartitionVPD;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_PRESERVE_MDC_PARTITION_VPD>
                                                   (l_preserveMDCPartitionVPD))
    {
        o_hdatOTA.hdatMDCLogPartVPD = l_preserveMDCPartitionVPD;
    }
    else
    {
        HDAT_ERR("Error in getting PRESERVE_MDC_PARTITION_VPD attribute");
    }

    //No CEC CM Capability on these systems
    o_hdatOTA.hdatCECCMCapable = 0;

    //i5/OS not available on this system
    o_hdatOTA.hdati5OSEnable = 0;

    o_hdatOTA.hdatSELFlagsValid = 1;

    o_hdatOTA.hdatDelSELFromHyp = 1;

    o_hdatOTA.hdatDelSELFromHB = 1;

    o_hdatOTA.hdatDelSELFromBMC = 1;

    //Lightpath support available on these systems
    o_hdatOTA.hdatServiceIndMode = 1;

    //RPA AIX/Linux
    o_hdatOTA.hdatDefPartitionType = 1;

}

/**
 * @brief This routine gets the information for IPL Parameters
 *
 * @pre None
 *
 * @post None
 *
 * @param None
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
void  HdatIplParms::hdatGetIplParmsData()
{
    PNOR::SideInfo_t l_sideInfo;
    errlHndl_t l_errl = NULL;

    TARGETING::Target *l_pSysTarget = NULL;
    (void) TARGETING::targetService().getTopLevelTarget(l_pSysTarget);
    if(l_pSysTarget == NULL)
    {
        HDAT_ERR("Error in getting Top Level Target");
        assert(l_pSysTarget != NULL);
    }

    //IPL to hypervisor running
    this->iv_hdatIPLParams->iv_iplParms.hdatIPLDestination = 0x02;

    l_errl = PNOR::getSideInfo(PNOR::WORKING, l_sideInfo);
    if(!l_errl)
    {
        if(l_sideInfo.isGolden)
        {
            this->iv_hdatIPLParams->iv_iplParms.hdatIPLSide =
                                             HDAT_FIRMWARE_SIDE_GOLDEN;
        }
        else
        {
            this->iv_hdatIPLParams->iv_iplParms.hdatIPLSide =
                                             HDAT_FIRMWARE_SIDE_WORKING;
        }
    }
    else
    {
        ERRORLOG::errlCommit(l_errl,HDAT_COMP_ID);
    }
    // Fast IPL Speed
    this->iv_hdatIPLParams->iv_iplParms.hdatIPLSpeed = 0xFF;


    TARGETING::ATTR_IS_MPIPL_HB_type l_mpiplHB;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_IS_MPIPL_HB>(l_mpiplHB))
    {
        if(l_mpiplHB)
        {
            this->iv_hdatIPLParams->iv_iplParms.hdatCECIPLAttributes = 0x2000;
            this->iv_hdatIPLParams->iv_iplParms.hdatIPLMajorType = 0x01;
            this->iv_hdatIPLParams->iv_iplParms.hdatIPLMinorType = 0x0D;
        }
        else
        {
            this->iv_hdatIPLParams->iv_iplParms.hdatCECIPLAttributes = 0x1000;

            this->iv_hdatIPLParams->iv_iplParms.hdatIPLMajorType = 0x00;

            TARGETING::ATTR_CEC_IPL_TYPE_type l_cecIPLType;
            if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_CEC_IPL_TYPE>
                                                                (l_cecIPLType))
            {
                this->iv_hdatIPLParams->iv_iplParms.hdatIPLMinorType =
                                                      l_cecIPLType.MinorIPLType;
                this->iv_hdatIPLParams->iv_iplParms.hdatIPLMinorType = 0xC;
                HDAT_DBG("hdatGetIplParmsData: setting hdatIPLMinorType to 0xC0");
            }
            else
            {
                HDAT_ERR("Error in getting CEC_IPL_TYPE attribute");
            }
        }
    }
    else
    {
        this->iv_hdatIPLParams->iv_iplParms.hdatIPLMinorType = 0x0C;
        HDAT_DBG("hdatGetIplParmsData: setting hdatIPLMinorType to 0xC0");
        HDAT_ERR("Error in getting IS_MPIPL_HB attribute");
    }

    TARGETING::ATTR_OS_IPL_MODE_type l_OSIPLMode;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_OS_IPL_MODE>(l_OSIPLMode))
    {
        this->iv_hdatIPLParams->iv_iplParms.hdatOSIPLMode = l_OSIPLMode;
    }
    else
    {
        HDAT_ERR("Error in getting OS_IPL_MODE attribute");
    }

    this->iv_hdatIPLParams->iv_iplParms.hdatKeyLockPosition =
                                                     HDAT_KEYLOCK_MANUAL;

    TARGETING::ATTR_LMB_SIZE_type l_lmbSize;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_LMB_SIZE>(l_lmbSize))
    {
        this->iv_hdatIPLParams->iv_iplParms.hdatLMBSize = 4;
    }
    else
    {
        HDAT_ERR("Error in getting LMB_SIZE attribute");
    }

    TARGETING::ATTR_MAX_HSL_OPTICONNECT_CONNECTIONS_type l_hslConnections;
    if(l_pSysTarget->tryGetAttr
                            <TARGETING::ATTR_MAX_HSL_OPTICONNECT_CONNECTIONS>
                                                           (l_hslConnections))
    {
        this->iv_hdatIPLParams->iv_iplParms.hdatMaxHSLConns = l_hslConnections;
    }
    else
    {
        HDAT_ERR("Error in getting MAX_HSL_OPTICONNECT_CONNECTIONS attribute");
    }

    hdatPopulateOtherIPLAttributes(this->iv_hdatIPLParams->iv_iplParms.hdatOIA);

    TARGETING::ATTR_HUGE_PAGE_COUNT_type l_hugePageCount;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_HUGE_PAGE_COUNT>
                                                           (l_hugePageCount))
    {
        this->iv_hdatIPLParams->iv_iplParms.hdatHugePageMemCount =
                                                             l_hugePageCount;
    }
    else
    {
        HDAT_ERR("Error in getting HUGE_PAGE_COUNT attribute");
    }

    TARGETING::ATTR_HUGE_PAGE_SIZE_type l_hugePageSize;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_HUGE_PAGE_SIZE>
                                                           (l_hugePageSize))
    {
        this->iv_hdatIPLParams->iv_iplParms.hdatHugePageMemSize =
                                                             l_hugePageSize;
    }
    else
    {
        HDAT_ERR("Error in getting HUGE_PAGE_SIZE attribute");
    }

    TARGETING::ATTR_VLAN_SWITCHES_type l_vlanSwitches;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_VLAN_SWITCHES>
                                                            (l_vlanSwitches))
    {
        this->iv_hdatIPLParams->iv_iplParms.hdatNumVlanSwitches =
                                                              l_vlanSwitches;
    }
    else
    {
        HDAT_ERR("Error in getting VLAN_SWITCHES attribute");
    }

    uint32_t hdatEnlargedIOCapacity = 0;
    hdatGetEnlargedIOCapacity(hdatEnlargedIOCapacity);
    this->iv_hdatIPLParams->iv_iplParms.hdatEnlargedIOCap =
                                                     hdatEnlargedIOCapacity;

}

/**
 * @brief This routine gets the information for SP serial ports
 *
 * @pre None
 *
 * @post None
 *
 * @param o_portArrayHdr - output parameter - Array header
 * @param o_ports        - output parameter - The structure to update with the
 *                                            serial port information
 *
 * @return A null error log handle if successful, else the return code pointed
 *         to by errlHndl_t contains one of:
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
static errlHndl_t hdatGetPortInfo(HDAT::hdatHDIFDataArray_t &o_portArrayHdr,
                                  hdatPortCodes_t o_ports[])
{
    errlHndl_t l_errlHndl = NULL;
    uint32_t   l_loopCnt  = 0;

    o_portArrayHdr.hdatOffset    = sizeof(HDAT::hdatHDIFDataArray_t);
    o_portArrayHdr.hdatAllocSize = sizeof(hdatPortCodes_t);
    o_portArrayHdr.hdatActSize   = sizeof(o_ports);
    o_portArrayHdr.hdatArrayCnt  = 0;

    TARGETING::PredicateCTM l_nodePredicate(TARGETING::CLASS_ENC,
                                             TARGETING::TYPE_NODE);

    TARGETING::PredicateHwas l_predHwas;
    l_predHwas.present(true);

    TARGETING::PredicatePostfixExpr l_presentNode;
    l_presentNode.push(&l_nodePredicate).push(&l_predHwas).And();

    //Get Node targets
    TARGETING::TargetRangeFilter l_nodeFilter(
                                        TARGETING::targetService().begin(),
                                        TARGETING::targetService().end(),
                                        &l_presentNode);

    TARGETING::Target *l_nodeTarget = (*l_nodeFilter);

    TARGETING::PredicateCTM l_serialPortPredicate(TARGETING::CLASS_UNIT,
                                             TARGETING::TYPE_UART);

    TARGETING::PredicatePostfixExpr l_presentSerialPort;
    l_presentSerialPort.push(&l_serialPortPredicate).push(&l_predHwas).And();

    TARGETING::TargetHandleList l_serialPortList;

    //Get Serial Port targets associated with service processor
    TARGETING::targetService().getAssociated(l_serialPortList, l_nodeTarget,
                       TARGETING::TargetService::CHILD,
                       TARGETING::TargetService::ALL,
                       &l_presentSerialPort);

    o_portArrayHdr.hdatArrayCnt = l_serialPortList.size();

    for (uint32_t l_idx = 0; l_idx < l_serialPortList.size(); ++l_idx)
    {
        TARGETING::Target *l_serialportTarget = l_serialPortList[l_idx];
        char l_locCodePrefix[64]={0};
        char l_locCode[64]={0};

        hdatGetLocationCodePrefix(l_locCodePrefix);
        hdatGetLocationCode(l_serialportTarget,l_locCodePrefix,l_locCode);
        HDAT_DBG(" Serial Port Loc Code :%s", l_locCode);

        strncpy((char *)(o_ports[l_loopCnt].hdatLocCode),
                                  l_locCode,
                                   sizeof(o_ports[l_loopCnt].hdatLocCode));

        o_ports[l_loopCnt].hdatResourceId = l_serialportTarget->getAttr
                                              <TARGETING::ATTR_SLCA_RID>();

        // None of the ports are used for callhome
        o_ports[l_loopCnt].hdatCallHome = 0;
        l_loopCnt++;
    }

    return l_errlHndl;
}


/**
 * @brief This routine gets the information for System Parameters
 *
 * @pre None
 *
 * @post None
 *
 * @param None
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 **/
void HdatIplParms::hdatGetSystemParamters()
{

    TARGETING::Target *l_pSysTarget = NULL;
    (void) TARGETING::targetService().getTopLevelTarget(l_pSysTarget);

    if(l_pSysTarget == NULL)
    {
      HDAT_ERR("hdatGetSystemParamters::Top Level Target not found");
      assert(l_pSysTarget != NULL);
    }

    // Get system information - system model
    uint32_t l_sysModel = 0;

    TARGETING::ATTR_RAW_MTM_type l_rawMTM = {0};
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_RAW_MTM>(l_rawMTM))
    {
        //we only want the last three bytes of the raw MTM, preceded by a 0x20
        l_sysModel = *((reinterpret_cast<uint32_t*>(l_rawMTM))+1);
        l_sysModel &= 0x00FFFFFF;
        l_sysModel |= 0x20000000;
        this->iv_hdatIPLParams->iv_sysParms.hdatSysModel = l_sysModel;
    }
    else
    {
        HDAT_ERR("Error in getting RAW_MTM attribute");
    }

    // Get system information - processor feature code
    // Processor Feature Code = CCIN of Anchor Card
    // No Anchor Card in BMC systems
    this->iv_hdatIPLParams->iv_sysParms.hdatProcFeatCode = 0;

    // Get system information - effective PVR
    TARGETING::ATTR_EFFECTIVE_EC_type l_effectiveEc;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_EFFECTIVE_EC>(l_effectiveEc))
    {
        //Convert Ec format to PVR Ec format
        uint32_t l_pvrEc = ( ((l_effectiveEc & 0xF0) << 4) |
                                                 (l_effectiveEc  & 0xF) );

        TARGETING::PredicateCTM l_procFilter(TARGETING::CLASS_CHIP,
                                             TARGETING::TYPE_PROC);

        TARGETING::PredicateHwas l_predHwas;
        l_predHwas.present(true);

        TARGETING::PredicatePostfixExpr l_presentProc;
        l_presentProc.push(&l_procFilter).push(&l_predHwas).And();

        TARGETING::TargetHandleList l_procList;

        //Get all Procs in the system
        TARGETING::targetService().
                  getAssociated(l_procList, l_pSysTarget,
                          TARGETING::TargetService::CHILD,
                          TARGETING::TargetService::ALL, &l_presentProc);

        if(l_procList.size() > 0)
        {
            TARGETING::Target *l_procTarget = l_procList[0];
            ATTR_MODEL_type l_procModel;

            if(l_procTarget->tryGetAttr<ATTR_MODEL>(l_procModel))
            {
                //Effective Processor Version Register (PVR)
                // bits 0-15: Processor version number
                // bits 16-19: Reserved
                // bits 20-23: Full RIT
                // bits 24-27: Reserved
                // bits 28-31: Minor revision level

                if(l_procModel == MODEL_MURANO)
                {
                    this->iv_hdatIPLParams->iv_sysParms.hdatEffectivePvr =
                                                 0x004B0000 | l_pvrEc;
                }
                else if(l_procModel == MODEL_VENICE)
                {
                    this->iv_hdatIPLParams->iv_sysParms.hdatEffectivePvr =
                                                 0x004D0200 ;
                }
                HDAT_DBG(" Effective PVR :0X%x",
                        this->iv_hdatIPLParams->iv_sysParms.hdatEffectivePvr);
            }
            else
            {
                HDAT_ERR("Error reading attribute ATTR_MODEL");
            }
        }
        else
        {
            HDAT_ERR("No Processors found in the system");
            assert(l_procList.size() > 0);
        }
    }
    else
    {
        HDAT_ERR(" Error in getting attribute EFFECTIVE_EC");
    }

    // Get system type
    iv_hdatIPLParams->iv_sysParms.hdatSysType =
             (l_pSysTarget->getAttr<TARGETING::ATTR_PHYP_SYSTEM_TYPE>());

    //Get ABC Bus Speed
    TARGETING::ATTR_FREQ_A_MHZ_type l_ABCBusSpeed;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_FREQ_A_MHZ>(l_ABCBusSpeed))
    {
        this->iv_hdatIPLParams->iv_sysParms.hdatABCBusSpeed = l_ABCBusSpeed;
    }
    else
    {
        HDAT_ERR(" Error in getting attribute FREQ_A_MHZ");
    }

    //Get XYZ Bus Speed
    TARGETING::ATTR_FREQ_X_MHZ_type l_WXYZBusSpeed;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_FREQ_X_MHZ>(l_WXYZBusSpeed))
    {
        this->iv_hdatIPLParams->iv_sysParms.hdatWXYZBusSpeed = l_WXYZBusSpeed;
    }
    else
    {
        HDAT_ERR(" Error in getting attribute FREQ_X_MHZ");
    }

    // NO ECO Support
    this->iv_hdatIPLParams->iv_sysParms.hdatSystemECOMode = 0;

    this->iv_hdatIPLParams->iv_sysParms.hdatSystemAttributes = 0;

    //Populate SMM Enabled/Disabled attribute
    TARGETING::ATTR_PAYLOAD_IN_MIRROR_MEM_type l_payLoadMirrorMem;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_PAYLOAD_IN_MIRROR_MEM>
                                                         (l_payLoadMirrorMem))
    {
        this->iv_hdatIPLParams->iv_sysParms.hdatSystemAttributes |=
             ( static_cast<uint8_t>(l_payLoadMirrorMem) ? HDAT_SMM_ENABLED : 0);
    }
    else
    {
        HDAT_ERR(" Error in getting attribute PAYLOAD_IN_MIRROR_MEM");
    }

    this->iv_hdatIPLParams->iv_sysParms.hdatMemoryScrubbing = 0;

    // Get SPPL information
    uint32_t l_numCores;

    TARGETING::ATTR_OPEN_POWER_TURBO_MODE_SUPPORTED_type l_turboModeSupported;
    if(l_pSysTarget->tryGetAttr
                             <TARGETING::ATTR_OPEN_POWER_TURBO_MODE_SUPPORTED>
                                                        (l_turboModeSupported))
    {
        HDAT::hdatGetNumberOfCores(l_numCores);

        if(l_turboModeSupported == true)
        {
            this->iv_hdatIPLParams->iv_sysParms.hdatCurrentSPPLValue =
                                            HDAT_TURBO_CORE_MODE_PART_SIZE_128;
        }
        else if( l_numCores == 6 )
        {
            this->iv_hdatIPLParams->iv_sysParms.hdatCurrentSPPLValue =
                                          HDAT_NONTURBO_SIX_CORE_PART_SIZE_256;
        }
        else if( l_numCores == 8 )
        {
            this->iv_hdatIPLParams->iv_sysParms.hdatCurrentSPPLValue =
                                        HDAT_NONTURBO_EIGHT_CORE_PART_SIZE_256;
        }
    }
    else
    {
        HDAT_ERR("Error in getting OPEN_POWER_TURBO_MODE_SUPPORTED attribute");
    }

    this->iv_hdatIPLParams->iv_sysParms.usePoreSleep  = 0x01;

    TARGETING::ATTR_VTPM_ENABLED_type l_vTpmEnabled;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_VTPM_ENABLED>
                                                        (l_vTpmEnabled))
    {
        this->iv_hdatIPLParams->iv_sysParms.vTpmEnabled = l_vTpmEnabled;
    }
    else
    {
        HDAT_ERR("Error in getting VTPM_ENABLED attribute");
    }

    //HW Page Table Size : 0x07 : 1/128
    this->iv_hdatIPLParams->iv_sysParms.hdatHwPageTbl = 0x07;

    TARGETING::ATTR_HYP_DISPATCH_WHEEL_type l_hyperDispatchWheel;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_HYP_DISPATCH_WHEEL>
                                                      (l_hyperDispatchWheel))
    {
        if(!l_hyperDispatchWheel)
        {
            l_hyperDispatchWheel = 0x0a;
        }
        this->iv_hdatIPLParams->iv_sysParms.hdatDispWheel =
                                                        l_hyperDispatchWheel;
    }
    else
    {
        HDAT_ERR("Error in getting HYP_DISPATCH_WHEEL attribute");
    }

    TARGETING::ATTR_FREQ_PB_MHZ_type l_nestClockFreq;
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_FREQ_PB_MHZ>
                                                         (l_nestClockFreq))
    {
        this->iv_hdatIPLParams->iv_sysParms.hdatNestFreq =
                                       static_cast<uint32_t>(l_nestClockFreq);
    }
    else
    {
        HDAT_ERR("Error in getting FREQ_PB_MHZ");
    }

    this->iv_hdatIPLParams->iv_sysParms.hdatSplitCoreMode = 1;

    TARGETING::ATTR_SYSTEM_BRAND_NAME_type l_systemBrandName = {0};
    if(l_pSysTarget->tryGetAttr<TARGETING::ATTR_SYSTEM_BRAND_NAME>
                                                         (l_systemBrandName))
    {
        strcpy(reinterpret_cast<char*>
                   (this->iv_hdatIPLParams->iv_sysParms.hdatSystemVendorName),
                                                           l_systemBrandName);
    }
    else
    {
        HDAT_ERR("Error in getting SYSTEM_BRAND_NAME");
    }
}

/**
 * @brief This routine gets the IPL Time Delta Structure
 *
 * @pre None
 *
 * @post None
 *
 * @param o_iplTime - output parameter - IPLTime Data
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
void hdatGetIPLTimeData(hdatIplTimeData_t & o_iplTime)
{
    //RTC and Delta values marked as invalid
    o_iplTime.hdatRTCValidFlags = 0;

    //Cumulative RTC Delta value is reset
    o_iplTime.hdatCumulativeRTCDelta = 0;
}


/**
 * @brief This routine gets the Manufacturing Flags
 *
 * @pre None
 *
 * @post None
 *
 * @param o_hdatManfFlags - output parameter - Manufacturing Flags
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
void hdatGetMnfgFlags(hdatManf_t &o_hdatManfFlags)
{
    o_hdatManfFlags.hdatPolicyFlags[0] = HDAT_MFG_FLAGS_CELL_0;
    o_hdatManfFlags.hdatPolicyFlags[1] = HDAT_MFG_FLAGS_CELL_1;
    o_hdatManfFlags.hdatPolicyFlags[2] = HDAT_MFG_FLAGS_CELL_2;
    o_hdatManfFlags.hdatPolicyFlags[3] = HDAT_MFG_FLAGS_CELL_3;

    if(HDAT::isMnfgFlagSet(TARGETING::MNFG_FLAG_AVP_ENABLE))
        o_hdatManfFlags.hdatPolicyFlags[1] |= HDAT_MFG_FLAG_AVP_ENABLED;

    if(HDAT::isMnfgFlagSet(TARGETING::MNFG_FLAG_HDAT_AVP_ENABLE))
        o_hdatManfFlags.hdatPolicyFlags[2] |= HDAT_MFG_FLAG_HDAT_AVP_ENABLED;

    if(HDAT::isMnfgFlagSet(TARGETING::MNFG_FLAG_SRC_TERM))
        o_hdatManfFlags.hdatPolicyFlags[0] |= HDAT_MNFG_FLAG_SRC_TERM;

    if(HDAT::isMnfgFlagSet(TARGETING::MNFG_FLAG_IPL_MEMORY_CE_CHECKING))
        o_hdatManfFlags.hdatPolicyFlags[2] |=
                                   HDAT_MNFG_IPL_MEM_CE_CHECKING_ACTIVE;

    if(HDAT::isMnfgFlagSet(TARGETING::MNFG_FLAG_FAST_BACKGROUND_SCRUB))
        o_hdatManfFlags.hdatPolicyFlags[1] |= HDAT_MNFG_FAST_BKG_SCRUB_ACTIVE;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_TEST_DRAM_REPAIRS))
        o_hdatManfFlags.hdatPolicyFlags[0] |= HDAT_MNFG_TEST_DRAM_REPAIRS;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_DISABLE_DRAM_REPAIRS))
        o_hdatManfFlags.hdatPolicyFlags[0] |= HDAT_MNFG_DISABLE_DRAM_REPAIRS;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_ENABLE_EXHAUSTIVE_PATTERN_TEST))
        o_hdatManfFlags.hdatPolicyFlags[2] |=
                                 HDAT_MNFG_ENABLE_EXHAUSTIVE_PATTERN_TEST;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_ENABLE_STANDARD_PATTERN_TEST))
        o_hdatManfFlags.hdatPolicyFlags[2] |=
                                 HDAT_MNFG_ENABLE_STANDARD_PATTERN_TEST;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_ENABLE_MINIMUM_PATTERN_TEST))
        o_hdatManfFlags.hdatPolicyFlags[2] |= HDAT_MNFG_ENABLE_MIN_PATTERN_TEST;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_DISABLE_FABRIC_eREPAIR))
        o_hdatManfFlags.hdatPolicyFlags[1] |= HDAT_MNFG_DISABLE_FABRIC_ERPAIR;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_DISABLE_MEMORY_eREPAIR))
        o_hdatManfFlags.hdatPolicyFlags[1] |= HDAT_MNFG_DISABLE_MEMORY_ERPAIR;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_FABRIC_DEPLOY_LANE_SPARES))
        o_hdatManfFlags.hdatPolicyFlags[2] |=
                                          HDAT_MNFG_FABRIC_DEPLOY_LANE_SPARES;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_DMI_DEPLOY_LANE_SPARES))
        o_hdatManfFlags.hdatPolicyFlags[2] |= HDAT_MNFG_DMI_DEPLOY_LANE_SPARES;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_PSI_DIAGNOSTIC))
        o_hdatManfFlags.hdatPolicyFlags[1] |= HDAT_MNFG_PSI_DIAGNOSTIC;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_FSP_UPDATE_SBE_IMAGE))
        o_hdatManfFlags.hdatPolicyFlags[1] |= HDAT_MNFG_FSP_UPDATE_SBE_IMAGE;

    if(isMnfgFlagSet(TARGETING::MNFG_FLAG_UPDATE_BOTH_SIDES_OF_SBE))
        o_hdatManfFlags.hdatPolicyFlags[1] |=
                                         HDAT_MNFG_UPDATE_BOTH_SIDES_OF_SBE;

}

/**
 * @brief This routine populates dump data table
 *
 * @pre None
 *
 * @post None
 *
 * @param o_hdatDump
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
void hdatGetPlatformDumpData(hdatDump_t &o_hdatDump)
{

    o_hdatDump.hdatReserved2 = 0;
    o_hdatDump.hdatHypDumpPolicy = 0;
    memset(o_hdatDump.hdatReserved3, 0 , sizeof(o_hdatDump.hdatReserved3));
    o_hdatDump.hdatMaxHdwSize = 0;
    o_hdatDump.hdatActHdwSize = 0;
    o_hdatDump.hdatMaxSpSize = 0;

    o_hdatDump.hdatFlags = 0;
    o_hdatDump.hdatDumpId = 0;
    o_hdatDump.hdatActPlatformDumpSize = 0;
    o_hdatDump.hdatPlid = 0;

}

/**
 * @brief This routine sets the Header information for IPL
 *        Parameters structure
 *
 * @pre None
 *
 * @post None
 *
 * @param o_iplparams - Output Parameter - IPL Parameter headers
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
static void hdatSetIPLParamsHdrs(hdatIPLParameters_t *o_iplparams)
{

    o_iplparams->hdatHdr.hdatStructId       = HDAT_HDIF_STRUCT_ID;
    o_iplparams->hdatHdr.hdatInstance       = 0;
    o_iplparams->hdatHdr.hdatVersion        = HDAT_IPL_PARAMS_VERSION;
    o_iplparams->hdatHdr.hdatSize           = sizeof(hdatIPLParameters_t);
    o_iplparams->hdatHdr.hdatHdrSize        = sizeof(hdatHDIF_t);
    o_iplparams->hdatHdr.hdatDataPtrOffset  = sizeof(hdatHDIF_t);
    o_iplparams->hdatHdr.hdatDataPtrCnt     = HDAT_IPL_PARAMS_DA_CNT;
    o_iplparams->hdatHdr.hdatChildStrCnt    = 0;
    o_iplparams->hdatHdr.hdatChildStrOffset = 0;

    memcpy(o_iplparams->hdatHdr.hdatStructName, HDAT_IPLP_STRUCT_NAME,
            sizeof(o_iplparams->hdatHdr.hdatStructName));

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_SYS_PARAMS].hdatOffset =
        offsetof(hdatIPLParameters_t,iv_sysParms);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_SYS_PARAMS].hdatSize =
        sizeof(hdatSysParms_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_IPL_PARAMS].hdatOffset =
        offsetof(hdatIPLParameters_t,iv_iplParms);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_IPL_PARAMS].hdatSize =
        sizeof(hdatIPLParams_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_TIME_DATA].hdatOffset =
        offsetof(hdatIPLParameters_t,iv_iplTime);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_TIME_DATA].hdatSize =
        sizeof(hdatIplTimeData_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_SPPL_PARAMS].hdatOffset =
        offsetof(hdatIPLParameters_t,iv_pvt);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_SPPL_PARAMS].hdatSize =
        sizeof(hdatIplSpPvt_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_PDUMP_DATA].hdatOffset =
        offsetof(hdatIPLParameters_t, iv_dump);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_PDUMP_DATA].hdatSize =
        sizeof(hdatDump_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_HMC_CONNS].hdatOffset =
        offsetof(hdatIPLParameters_t, iv_hmc);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_HMC_CONNS].hdatSize =
        sizeof(hdatHmc_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_CUOD_DATA].hdatOffset =
        offsetof(hdatIPLParameters_t, iv_cuod);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_CUOD_DATA].hdatSize =
        sizeof(hdatCuod_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_MFG_DATA].hdatOffset =
        offsetof(hdatIPLParameters_t, iv_manf);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_MFG_DATA].hdatSize =
        sizeof(hdatManf_t);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_SERIAL_PORTS].hdatOffset =
        offsetof(hdatIPLParameters_t, iv_portArrayHdr);

    o_iplparams->hdatIPLParamsIntData[HDAT_IPL_PARAMS_SERIAL_PORTS].hdatSize =
        sizeof(hdatHDIFDataArray_t) + sizeof(hdatPortCodes_t);

}

/**
 * @brief Constructor for IPL Parameters  construction class
 *
 * @pre None
 *
 * @post None
 *
 * @param o_errlHndl - output parameter - Error Handlea
 *        i_msAddr - Mainstore address where IPL Params
 *                   structure is loaded
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
HdatIplParms::HdatIplParms(errlHndl_t &o_errlHndl,
                           const HDAT::hdatMsAddr_t &i_msAddr)
{
    o_errlHndl = NULL;

    // Copy the main store address for the pcia data
    memcpy(&iv_msAddr, &i_msAddr, sizeof(hdatMsAddr_t));

    uint64_t l_base_addr = ((uint64_t) i_msAddr.hi << 32) | i_msAddr.lo;

    void *l_virt_addr = mm_block_map (
                         reinterpret_cast<void*>(ALIGN_PAGE_DOWN(l_base_addr)),
                         (ALIGN_PAGE(sizeof(hdatIPLParameters_t))+ PAGESIZE));

    l_virt_addr = reinterpret_cast<void *>(
                    reinterpret_cast<uint64_t>(l_virt_addr) +
                    (l_base_addr - ALIGN_PAGE_DOWN(l_base_addr)));

    // initializing the space to zero
    memset(l_virt_addr ,0x0, sizeof(hdatIPLParameters_t));

    iv_hdatIPLParams = reinterpret_cast<hdatIPLParameters_t *>(l_virt_addr);

    HDAT_DBG("Ctr iv_hdatIPLParams addr 0x%016llX virtual addr 0x%016llX",
                  (uint64_t) this->iv_hdatIPLParams, (uint64_t)l_virt_addr);

}

/**
 * @brief Load IPL Paramsters to Mainstore
 *
 * @pre None
 *
 * @post None
 *
 * @param o_size - output parameter - Size of IPL Parameters structure
 *        o_count - output parameter - Number of IPL Parameters structures
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
errlHndl_t HdatIplParms::hdatLoadIplParams(uint32_t &o_size, uint32_t &o_count)
{
    errlHndl_t l_errl = NULL;

    //Set IPLParams Headers
    hdatSetIPLParamsHdrs(this->iv_hdatIPLParams);

    //Get the FSP private IPL type
    TARGETING::Target *l_pSysTarget = NULL;
    (void) TARGETING::targetService().getTopLevelTarget(l_pSysTarget);

    if(l_pSysTarget == NULL)
    {
        HDAT_ERR("Error in getting Top Level target");
        assert(l_pSysTarget != NULL);
    }

    //Initializing SP IPL Type to Power On Reset
    this->iv_hdatIPLParams->iv_pvt.hdatIplType = 0x00000801;

    // Get the IPL parameters data
    hdatGetIplParmsData();

    // Get the IPL time data
    hdatGetIPLTimeData(this->iv_hdatIPLParams->iv_iplTime);

    // Get the System Parameters
    hdatGetSystemParamters();

    // Get HMC information
    memset(&this->iv_hdatIPLParams->iv_hmc, 0x00,
                                   sizeof(this->iv_hdatIPLParams->iv_hmc));

    // Get dump information
    hdatGetPlatformDumpData(this->iv_hdatIPLParams->iv_dump);

    // Get CUOD information
    this->iv_hdatIPLParams->iv_cuod.hdatCuodFlags = HDAT_POWER_OFF;

    // Get manufacturing mode information
    memset(&this->iv_hdatIPLParams->iv_manf, 0x00, sizeof(hdatManf_t));
    hdatGetMnfgFlags(this->iv_hdatIPLParams->iv_manf);

    // Get serial port information
    memset(&this->iv_hdatIPLParams->iv_portArrayHdr, 0x00,
                                         sizeof(HDAT::hdatHDIFDataArray_t));
    memset(this->iv_hdatIPLParams->iv_ports, 0x00, sizeof(hdatPortCodes_t) * 2);
    hdatGetPortInfo(this->iv_hdatIPLParams->iv_portArrayHdr,
                                         this->iv_hdatIPLParams->iv_ports);

    HDAT_DBG("HDAT:: IPL Parameters Loaded :: Size : 0x%X",
                                      sizeof(hdatIPLParameters_t));

    o_count = 1;
    o_size  = sizeof(hdatIPLParameters_t);

    return l_errl;
}

/**
 * @brief Destructor for IPL Parameters construction class
 *
 * @pre None
 *
 * @post None
 *
 * @param None
 *
 * @return None
 *
 * @retval HDAT_OTHER_COMP_ERROR
 */
HdatIplParms::~HdatIplParms()
{
    int rc = 0;
    rc =  mm_block_unmap(iv_hdatIPLParams);
    if( rc != 0)
    {
        errlHndl_t l_errl = NULL;
        /*@
         * @errortype
         * @moduleid         HDAT::MOD_IPLPARMS_DESTRUCTOR
         * @reasoncode       HDAT::RC_DEV_MAP_FAIL
         * @devdesc          Unmap a mapped region failed
         * @custdesc         Firmware encountered an internal error.
        */
        hdatBldErrLog(l_errl,
                MOD_PCIA_DESTRUCTOR,
                RC_DEV_MAP_FAIL,
                0,0,0,0,
                ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                HDAT_VERSION1,
                true);
    }

    return;
}
};
