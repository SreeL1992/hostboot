/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/initfiles/p9_fbc_scom.C $  */
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
#include "p9_fbc_scom.H"
#include <stdint.h>
#include <stddef.h>
#include <fapi2.H>
#include <attribute_ids.H>
#include <target_types.H>
#include <fapi2_attribute_service.H>
using namespace fapi2;

#define ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0    0
#define ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0    0
#define LITERAL_PB_IOE_LL1_CONFIG_LINK_PAIR_ON    0x1
#define LITERAL_PB_IOE_LL1_IOEL_FIR_MASK_REG_0xFFFFFFFFFFFFFFFF    0xFFFFFFFFFFFFFFFF
#define LITERAL_PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_ON    0x1
#define LITERAL_PB_IOE_SCOM_PB_CFG_IOE23_IS_LOGICAL_PAIR_ON    0x1
#define LITERAL_PB_IOE_SCOM_PB_CFG_IOE45_IS_LOGICAL_PAIR_ON    0x1
#define LITERAL_PB_IOE_SCOM_PB_IOE_FIR_MASK_REG_0xFFFFFFFFFFFFFFFF    0xFFFFFFFFFFFFFFFF
#define LITERAL_PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_ON    0x1
#define LITERAL_PB_IOO_SCOM_PB_CFG_IOO23_IS_LOGICAL_PAIR_ON    0x1
#define LITERAL_PB_IOO_SCOM_PB_CFG_IOO45_IS_LOGICAL_PAIR_ON    0x1
#define LITERAL_PB_IOO_SCOM_PB_CFG_IOO67_IS_LOGICAL_PAIR_ON    0x1
#define LITERAL_PB_IOO_SCOM_PB_IOO_FIR_MASK_REG_0xFFFFFFFFFFFFFFFF    0xFFFFFFFFFFFFFFFF

fapi2::ReturnCode p9_fbc_scom(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& TGT0)
{
    fapi2::ReturnCode l_rc = 0;

    do
    {
        ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_Type iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG;
        l_rc = FAPI_ATTR_GET(ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG, TGT0, iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG);

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: FAPI_ATTR_GET (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG)");
            break;
        }

        auto iv_def_X0_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG[0] !=
                                  ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);
        fapi2::buffer<uint64_t> PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_scom0;
        l_rc = fapi2::getScom( TGT0, 0x5013423ull, PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: getScom (0x5013423)");
            break;
        }

        if (iv_def_X0_ENABLED)
        {
            PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_ON, 0,
                    1, 63 );
        }

        auto iv_def_X1_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG[1] !=
                                  ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);

        if (iv_def_X1_ENABLED)
        {
            PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOE_SCOM_PB_CFG_IOE23_IS_LOGICAL_PAIR_ON, 1,
                    1, 63 );
        }

        auto iv_def_X2_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG[2] !=
                                  ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);

        if (iv_def_X2_ENABLED)
        {
            PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOE_SCOM_PB_CFG_IOE45_IS_LOGICAL_PAIR_ON, 2,
                    1, 63 );
        }

        fapi2::buffer<uint64_t> PB_IOE_SCOM_PB_IOE_FIR_MASK_REG_scom0;
        l_rc = fapi2::getScom( TGT0, 0x5013403ull, PB_IOE_SCOM_PB_IOE_FIR_MASK_REG_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: getScom (0x5013403)");
            break;
        }

        if (((iv_def_X0_ENABLED || iv_def_X1_ENABLED) || iv_def_X2_ENABLED))
        {
            PB_IOE_SCOM_PB_IOE_FIR_MASK_REG_scom0.insert<uint64_t> (LITERAL_PB_IOE_SCOM_PB_IOE_FIR_MASK_REG_0xFFFFFFFFFFFFFFFF, 0,
                    64, 0 );
        }

        fapi2::buffer<uint64_t> PB_IOE_LL1_CONFIG_LINK_PAIR_scom0;
        l_rc = fapi2::getScom( TGT0, 0x601180aull, PB_IOE_LL1_CONFIG_LINK_PAIR_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: getScom (0x601180a)");
            break;
        }

        if (iv_def_X1_ENABLED)
        {
            PB_IOE_LL1_CONFIG_LINK_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOE_LL1_CONFIG_LINK_PAIR_ON, 0, 1, 63 );
        }

        fapi2::buffer<uint64_t> PB_IOE_LL1_IOEL_FIR_MASK_REG_scom0;
        l_rc = fapi2::getScom( TGT0, 0x6011803ull, PB_IOE_LL1_IOEL_FIR_MASK_REG_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: getScom (0x6011803)");
            break;
        }

        if (iv_def_X1_ENABLED)
        {
            PB_IOE_LL1_IOEL_FIR_MASK_REG_scom0.insert<uint64_t> (LITERAL_PB_IOE_LL1_IOEL_FIR_MASK_REG_0xFFFFFFFFFFFFFFFF, 0, 64,
                    0 );
        }

        ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG_Type iv_TGT0_ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG;
        l_rc = FAPI_ATTR_GET(ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG, TGT0, iv_TGT0_ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG);

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: FAPI_ATTR_GET (iv_TGT0_ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG)");
            break;
        }

        auto iv_def_X3_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG[3] !=
                                  ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);
        auto iv_def_A0_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG[0] !=
                                  ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);
        fapi2::buffer<uint64_t> PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_scom0;
        l_rc = fapi2::getScom( TGT0, 0x5013823ull, PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: getScom (0x5013823)");
            break;
        }

        if ((iv_def_A0_ENABLED || iv_def_X3_ENABLED))
        {
            PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_ON, 0,
                    1, 63 );
        }

        auto iv_def_X4_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG[4] !=
                                  ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);
        auto iv_def_A1_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG[1] !=
                                  ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);

        if ((iv_def_A1_ENABLED || iv_def_X4_ENABLED))
        {
            PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOO_SCOM_PB_CFG_IOO23_IS_LOGICAL_PAIR_ON, 1,
                    1, 63 );
        }

        auto iv_def_X5_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG[5] !=
                                  ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);
        auto iv_def_A2_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG[2] !=
                                  ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);

        if ((iv_def_A2_ENABLED || iv_def_X5_ENABLED))
        {
            PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOO_SCOM_PB_CFG_IOO45_IS_LOGICAL_PAIR_ON, 2,
                    1, 63 );
        }

        auto iv_def_X6_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG[6] !=
                                  ATTR_PROC_FABRIC_X_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);
        auto iv_def_A3_ENABLED = (iv_TGT0_ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG[3] !=
                                  ATTR_PROC_FABRIC_A_ATTACHED_CHIP_CNFG_ATTRIBUTE_VALUE_0);

        if ((iv_def_A3_ENABLED || iv_def_X6_ENABLED))
        {
            PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_scom0.insert<uint64_t> (LITERAL_PB_IOO_SCOM_PB_CFG_IOO67_IS_LOGICAL_PAIR_ON, 3,
                    1, 63 );
        }

        fapi2::buffer<uint64_t> PB_IOO_SCOM_PB_IOO_FIR_MASK_REG_scom0;
        l_rc = fapi2::getScom( TGT0, 0x5013803ull, PB_IOO_SCOM_PB_IOO_FIR_MASK_REG_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: getScom (0x5013803)");
            break;
        }

        if ((((((((iv_def_A0_ENABLED || iv_def_A1_ENABLED) || iv_def_A2_ENABLED) || iv_def_A3_ENABLED) || iv_def_X3_ENABLED)
               || iv_def_X4_ENABLED) || iv_def_X5_ENABLED) || iv_def_X6_ENABLED))
        {
            PB_IOO_SCOM_PB_IOO_FIR_MASK_REG_scom0.insert<uint64_t> (LITERAL_PB_IOO_SCOM_PB_IOO_FIR_MASK_REG_0xFFFFFFFFFFFFFFFF, 0,
                    64, 0 );
        }


        l_rc = fapi2::putScom( TGT0, 0x5013403ull, PB_IOE_SCOM_PB_IOE_FIR_MASK_REG_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: putScom (0x5013403)");
            break;
        }

        l_rc = fapi2::putScom( TGT0, 0x5013423ull, PB_IOE_SCOM_PB_CFG_IOE01_IS_LOGICAL_PAIR_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: putScom (0x5013423)");
            break;
        }

        l_rc = fapi2::putScom( TGT0, 0x5013803ull, PB_IOO_SCOM_PB_IOO_FIR_MASK_REG_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: putScom (0x5013803)");
            break;
        }

        l_rc = fapi2::putScom( TGT0, 0x5013823ull, PB_IOO_SCOM_PB_CFG_IOO01_IS_LOGICAL_PAIR_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: putScom (0x5013823)");
            break;
        }

        l_rc = fapi2::putScom( TGT0, 0x6011803ull, PB_IOE_LL1_IOEL_FIR_MASK_REG_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: putScom (0x6011803)");
            break;
        }

        l_rc = fapi2::putScom( TGT0, 0x601180aull, PB_IOE_LL1_CONFIG_LINK_PAIR_scom0 );

        if (l_rc)
        {
            FAPI_ERR("ERROR executing: putScom (0x601180a)");
            break;
        }

    }
    while(0);

    return l_rc;
}

