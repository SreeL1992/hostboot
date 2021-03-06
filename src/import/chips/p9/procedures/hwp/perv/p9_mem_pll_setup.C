/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/perv/p9_mem_pll_setup.C $  */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
//------------------------------------------------------------------------------
/// @file  p9_mem_pll_setup.C
///
/// @brief setup PLL for MBAs
//------------------------------------------------------------------------------
// *HWP HW Owner        : Anusha Reddy Rangareddygari <anusrang@in.ibm.com>
// *HWP HW Backup Owner : Srinivas V Naga <srinivan@in.ibm.com>
// *HWP FW Owner        : Sunil Kumar <skumar8j@in.ibm.com>
// *HWP Team            : Perv
// *HWP Level           : 2
// *HWP Consumed by     : HB
//------------------------------------------------------------------------------


//## auto_generated
#include "p9_mem_pll_setup.H"

#include "p9_perv_scom_addresses.H"
#include <p9_perv_scom_addresses_fld.H>

enum P9_MEM_PLL_SETUP_Private_Constants
{
    NS_DELAY = 5000000, // unit is nano seconds
    SIM_CYCLE_DELAY = 100000, // unit is sim cycles
    OPCG_ALIGN_SCAN_RATIO = 0b00011
};


fapi2::ReturnCode p9_mem_pll_setup(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target_chip)
{
    uint8_t l_read_attr = 0;
    fapi2::buffer<uint64_t> l_data64;
    uint8_t l_mem_bypass;

    FAPI_INF("Entering ...");

    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_NEST_MEM_X_O_PCI_BYPASS, i_target_chip, l_mem_bypass),
             "Error from FAPI_ATTR_GET (ATTR_NEST_MEM_X_O_PCI_BYPASS");

    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_MC_SYNC_MODE, i_target_chip, l_read_attr),
             "Error from FAPI_ATTR_GET (ATTR_MC_SYNC_MODE)");

    if (!l_read_attr )
    {
        for (auto l_chplt_trgt :  i_target_chip.getChildren<fapi2::TARGET_TYPE_PERV>
             (fapi2::TARGET_FILTER_ALL_MC, fapi2::TARGET_STATE_FUNCTIONAL))
        {
            FAPI_DBG("Drop PLDY bypass of Progdelay logic");
            l_data64.flush<1>();
            //NET_CTRL1.TP_MC_PDLY_BYPASS_EN_DC = 0
            l_data64.clearBit<PERV_1_NET_CTRL1_CLK_PDLY_BYPASS_EN>();
            FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_NET_CTRL1_WAND, l_data64));

            FAPI_DBG("Drop DCC bypass of DCC logic");
            l_data64.flush<1>();
            //NET_CTRL1.TP_MC_PDLY_BYPASS_EN_DC = 0
            l_data64.clearBit<PERV_1_NET_CTRL1_CLK_DCC_BYPASS_EN>();
            FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_NET_CTRL1_WAND, l_data64));

            if (l_mem_bypass == 0)
            {
                FAPI_DBG("Drop PLL test enable");
                l_data64.flush<1>();
                //NET_CTRL0.TP_PLL_TEST_EN_DC
                l_data64.clearBit<PERV_1_NET_CTRL0_PLL_TEST_EN>();
                FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_NET_CTRL0_WAND, l_data64));

                FAPI_DBG("Drop PLL reset");
                l_data64.flush<1>();
                //NET_CTRL0.TP_PLLRST_DC
                l_data64.clearBit<PERV_1_NET_CTRL0_PLL_RESET>();
                FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_NET_CTRL0_WAND, l_data64));

                fapi2::delay(NS_DELAY, SIM_CYCLE_DELAY);

                FAPI_DBG("check PLL lock");

                //Getting PLL_LOCK_REG register value
                FAPI_TRY(fapi2::getScom(l_chplt_trgt, PERV_PLL_LOCK_REG,
                                        l_data64)); //l_read_reg = PERV.PLL_LOCK_REG
                FAPI_ASSERT(l_data64.getBit<0>() == 1,
                            fapi2::MEM_PLL_LOCK_ERR()
                            .set_MEM_PLL_READ(l_data64),
                            "ERROR:MEM PLL LOCK NOT SET");

                FAPI_DBG("Drop PLL Bypass");
                l_data64.flush<1>();
                //NET_CTRL0.TP_PLLBYP_DC =  0
                l_data64.clearBit<PERV_1_NET_CTRL0_PLL_BYPASS>();
                FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_NET_CTRL0_WAND, l_data64));

                //OPCG_ALIGN.scan_ratio=0b00011
                FAPI_DBG("Set scan ratio to 4:1");
                FAPI_TRY(fapi2::getScom(l_chplt_trgt, PERV_OPCG_ALIGN, l_data64));
                l_data64.insertFromRight<PERV_1_OPCG_ALIGN_SCAN_RATIO, PERV_1_OPCG_ALIGN_SCAN_RATIO_LEN>
                (OPCG_ALIGN_SCAN_RATIO);
                FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_OPCG_ALIGN, l_data64));
            }

            //Reset PCB Slave error register
            FAPI_DBG("Reset PCB Slave error register");
            l_data64 = 0xFFFFFFFFFFFFFFFF;
            FAPI_TRY(fapi2::putScom(l_chplt_trgt, PERV_ERROR_REG, l_data64));

        }
    }

    FAPI_INF("Exiting ...");

fapi_try_exit:
    return fapi2::current_err;
}
