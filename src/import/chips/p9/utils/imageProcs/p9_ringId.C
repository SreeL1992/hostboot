/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/utils/imageProcs/p9_ringId.C $            */
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

#include "p9_ringId.H"
#include "p9_ring_identification.H"

namespace PERV
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"perv_fure"                 , 0x00, 0x01, 0x01, NON_VPD_RING, 0, 0x0800000000009000},
    {"perv_gptr"                 , 0x01, 0x01, 0x01, NON_VPD_RING, 0, 0x0800000000002000},
    {"perv_time"                 , 0x02, 0x01, 0x01, NON_VPD_RING, 0, 0x0800000000000100},
    {"occ_fure"                  , 0x03, 0x01, 0x01, NON_VPD_RING, 0, 0x0100000000009000},
    {"occ_gptr"                  , 0x04, 0x01, 0x01, NON_VPD_RING, 0, 0x0100000000002000},
    {"occ_time"                  , 0x05, 0x01, 0x01, NON_VPD_RING, 0, 0x0100000000000100},
    {"perv_ana_func"             , 0x06, 0x01, 0x01, NON_VPD_RING, 0, 0x0080000000008000},
    {"perv_ana_gptr"             , 0x07, 0x01, 0x01, NON_VPD_RING, 0, 0x0080000000002000},
    {"perv_pll_gptr"             , 0x08, 0x01, 0x01, NON_VPD_RING, 0, 0x0002000000002000},
    {"perv_pll_bndy_bucket_1"    , 0x09, 0x01, 0x01, NON_VPD_PLL_RING, 5, 0x0002000000000080},
    {"perv_pll_bndy_bucket_2"    , 0x0a, 0x01, 0x01, NON_VPD_PLL_RING, 5, 0x0002000000000080},
    {"perv_pll_bndy_bucket_3"    , 0x0b, 0x01, 0x01, NON_VPD_PLL_RING, 5, 0x0002000000000080},
    {"perv_pll_bndy_bucket_4"    , 0x0c, 0x01, 0x01, NON_VPD_PLL_RING, 5, 0x0002000000000080},
    {"perv_pll_bndy_bucket_5"    , 0x0d, 0x01, 0x01, NON_VPD_PLL_RING, 5, 0x0002000000000080},
    {"perv_pll_func"             , 0x0e, 0x01, 0x01, NON_VPD_RING, 0, 0x0002000000008000},

};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"perv_repr"        , 0x0f, 0x01, 0x01, NON_VPD_RING, 0, 0x0800000000000200},
    {"occ_repr"         , 0x10, 0x01, 0x01, NON_VPD_RING, 0, 0x0100000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace N0
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"n0_fure"          , 0x00, 0x02, 0x02, NON_VPD_RING, 0, 0x09C0000000009000},
    {"n0_gptr"          , 0x01, 0x02, 0x02, NON_VPD_RING, 0, 0x09C0000000002000},
    {"n0_time"          , 0x02, 0x02, 0x02, NON_VPD_RING, 0, 0x09C0000000000100},
    {"n0_nx_fure"       , 0x03, 0x02, 0x02, NON_VPD_RING, 0, 0x0400000000009000},
    {"n0_nx_gptr"       , 0x04, 0x02, 0x02, NON_VPD_RING, 0, 0x0400000000002000},
    {"n0_nx_time"       , 0x05, 0x02, 0x02, NON_VPD_RING, 0, 0x0400000000000100},
    {"n0_cxa0_fure"     , 0x06, 0x02, 0x02, NON_VPD_RING, 0, 0x0200000000009000},
    {"n0_cxa0_gptr"     , 0x07, 0x02, 0x02, NON_VPD_RING, 0, 0x0200000000002000},
    {"n0_cxa0_time"     , 0x08, 0x02, 0x02, NON_VPD_RING, 0, 0x0200000000000100},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"n0_repr"          , 0x09, 0x02, 0x02, NON_VPD_RING, 0, 0x09C0000000000200},
    {"n0_nx_repr"       , 0x0a, 0x02, 0x02, NON_VPD_RING, 0, 0x0400000000000200},
    {"n0_cxa0_repr"     , 0x0b, 0x02, 0x02, NON_VPD_RING, 0, 0x0200000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace N1
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"n1_fure"          , 0x00, 0x03, 0x03, NON_VPD_RING, 0, 0x0E00000000009000},
    {"n1_gptr"          , 0x01, 0x03, 0x03, NON_VPD_RING, 0, 0x0E00000000002000},
    {"n1_time"          , 0x02, 0x03, 0x03, NON_VPD_RING, 0, 0x0E00000000000100},
    {"n1_ioo0_fure"     , 0x03, 0x03, 0x03, NON_VPD_RING, 0, 0x0100000000009000},
    {"n1_ioo0_gptr"     , 0x04, 0x03, 0x03, NON_VPD_RING, 0, 0x0100000000002000},
    {"n1_ioo0_time"     , 0x05, 0x03, 0x03, NON_VPD_RING, 0, 0x0100000000000100},
    {"n1_ioo1_fure"     , 0x06, 0x03, 0x03, NON_VPD_RING, 0, 0x0080000000009000},
    {"n1_ioo1_gptr"     , 0x07, 0x03, 0x03, NON_VPD_RING, 0, 0x0080000000002000},
    {"n1_ioo1_time"     , 0x08, 0x03, 0x03, NON_VPD_RING, 0, 0x0080000000000100},
    {"n1_mcs23_fure"    , 0x09, 0x03, 0x03, NON_VPD_RING, 0, 0x0040000000009000},
    {"n1_mcs23_gptr"    , 0x0a, 0x03, 0x03, NON_VPD_RING, 0, 0x0040000000002000},
    {"n1_mcs23_time"    , 0x0b, 0x03, 0x03, NON_VPD_RING, 0, 0x0040000000000100},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"n1_repr"          , 0x0c, 0x03, 0x03, NON_VPD_RING, 0, 0x0E00000000000200},
    {"n1_ioo0_repr"     , 0x0d, 0x03, 0x03, NON_VPD_RING, 0, 0x0100000000000200},
    {"n1_ioo1_repr"     , 0x0e, 0x03, 0x03, NON_VPD_RING, 0, 0x0080000000000200},
    {"n1_mcs23_repr"    , 0x0f, 0x03, 0x03, NON_VPD_RING, 0, 0x0040000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace N2
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"n2_fure"          , 0x00, 0x04, 0x04, NON_VPD_RING, 0, 0x0B80000000009000},
    {"n2_gptr"          , 0x01, 0x04, 0x04, NON_VPD_RING, 0, 0x0B80000000002000},
    {"n2_time"          , 0x02, 0x04, 0x04, NON_VPD_RING, 0, 0x0B80000000000100},
    {"n2_cxa1_fure"     , 0x03, 0x04, 0x04, NON_VPD_RING, 0, 0x0400000000009000},
    {"n2_cxa1_gptr"     , 0x04, 0x04, 0x04, NON_VPD_RING, 0, 0x0400000000002000},
    {"n2_cxa1_time"     , 0x05, 0x04, 0x04, NON_VPD_RING, 0, 0x0400000000000100},
    {"n2_psi_fure"      , 0x06, 0x04, 0x04, NON_VPD_RING, 0, 0x0040000000009000},
    {"n2_psi_gptr"      , 0x07, 0x04, 0x04, NON_VPD_RING, 0, 0x0040000000002000},
    {"n2_psi_time"      , 0x08, 0x04, 0x04, NON_VPD_RING, 0, 0x0040000000000100},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"n2_repr"          , 0x09, 0x04, 0x04, NON_VPD_RING, 0, 0x0B80000000000200},
    {"n2_cxa1_repr"     , 0x0a, 0x04, 0x04, NON_VPD_RING, 0, 0x0400000000000200},
    {"n2_psi_repr"      , 0x0b, 0x04, 0x04, NON_VPD_RING, 0, 0x0040000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace N3
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"n3_fure"          , 0x00, 0x05, 0x05, NON_VPD_RING, 0, 0x0CC0000000009000},
    {"n3_gptr"          , 0x01, 0x05, 0x05, NON_VPD_RING, 0, 0x0EC0000000002000},
    {"n3_time"          , 0x02, 0x05, 0x05, NON_VPD_RING, 0, 0x0EC0000000000100},
    {"n3_mcs01_fure"    , 0x03, 0x05, 0x05, NON_VPD_RING, 0, 0x0020000000009000},
    {"n3_mcs01_gptr"    , 0x04, 0x05, 0x05, NON_VPD_RING, 0, 0x0020000000002000},
    {"n3_mcs01_time"    , 0x05, 0x05, 0x05, NON_VPD_RING, 0, 0x0020000000000100},
    {"n3_np_fure"       , 0x06, 0x05, 0x05, NON_VPD_RING, 0, 0x0100000000009000},
    {"n3_np_gptr"       , 0x07, 0x05, 0x05, NON_VPD_RING, 0, 0x0100000000002000},
    {"n3_np_time"       , 0x08, 0x05, 0x05, NON_VPD_RING, 0, 0x0100000000000100},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"n3_repr"          , 0x09, 0x05, 0x05, NON_VPD_RING, 0, 0x0EC0000000000200},
    {"n3_mcs01_repr"    , 0x0a, 0x05, 0x05, NON_VPD_RING, 0, 0x0020000000000200},
    {"n3_np_repr"       , 0x0b, 0x05, 0x05, NON_VPD_RING, 0, 0x0100000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace XB
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"xb_fure"                   , 0x00, 0x06, 0x06, NON_VPD_RING, 0, 0x0880000000009000},
    {"xb_gptr"                   , 0x01, 0x06, 0x06, NON_VPD_RING, 0, 0x0880000000002000},
    {"xb_time"                   , 0x02, 0x06, 0x06, NON_VPD_RING, 0, 0x0880000000000100},
    {"xb_io0_fure"               , 0x03, 0x06, 0x06, NON_VPD_RING, 0, 0x0440000000009000},
    {"xb_io0_gptr"               , 0x04, 0x06, 0x06, NON_VPD_RING, 0, 0x0440000000002000},
    {"xb_io0_time"               , 0x05, 0x06, 0x06, NON_VPD_RING, 0, 0x0440000000000100},
    {"xb_io1_fure"               , 0x06, 0x06, 0x06, NON_VPD_RING, 0, 0x0220000000009000},
    {"xb_io1_gptr"               , 0x07, 0x06, 0x06, NON_VPD_RING, 0, 0x0220000000002000},
    {"xb_io1_time"               , 0x08, 0x06, 0x06, NON_VPD_RING, 0, 0x0220000000000100},
    {"xb_io2_fure"               , 0x09, 0x06, 0x06, NON_VPD_RING, 0, 0x0110000000009000},
    {"xb_io2_gptr"               , 0x0a, 0x06, 0x06, NON_VPD_RING, 0, 0x0110000000002000},
    {"xb_io2_time"               , 0x0b, 0x06, 0x06, NON_VPD_RING, 0, 0x0110000000000100},
    {"xb_pll_gptr"               , 0x0c, 0x06, 0x06, NON_VPD_RING, 0, 0x0002000000002000},
    {"xb_pll_bndy"               , 0x0d, 0x06, 0x06, NON_VPD_RING, 0, 0x0002000000000080},
    {"xb_pll_func"               , 0x0e, 0x06, 0x06, NON_VPD_RING, 0, 0x0002000000008000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"xb_repr"          , 0x13, 0x06, 0x06, NON_VPD_RING, 0, 0x0880000000000200},
    {"xb_io0_repr"      , 0x14, 0x06, 0x06, NON_VPD_RING, 0, 0x0440000000000200},
    {"xb_io1_repr"      , 0x15, 0x06, 0x06, NON_VPD_RING, 0, 0x0220000000000200},
    {"xb_io2_repr"      , 0x16, 0x06, 0x06, NON_VPD_RING, 0, 0x0110000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace MC
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"mc_fure"                   , 0x00, 0x07, 0x07, NON_VPD_RING, 0, 0x0C00000000009000},
    {"mc_gptr"                   , 0x01, 0x07, 0x07, NON_VPD_RING, 0, 0x0C00000000002000},
    {"mc_time"                   , 0x02, 0x07, 0x07, NON_VPD_RING, 0, 0x0C00000000000100},
    {"mc_iom01_fure"             , 0x03, 0x07, 0x07, NON_VPD_RING, 0, 0x0200000000009000},
    {"mc_iom01_gptr"             , 0x04, 0x07, 0x07, NON_VPD_RING, 0, 0x0200000000002000},
    {"mc_iom01_time"             , 0x05, 0x07, 0x07, NON_VPD_RING, 0, 0x0200000000000100},
    {"mc_iom23_fure"             , 0x06, 0x07, 0x07, NON_VPD_RING, 0, 0x0100000000009000},
    {"mc_iom23_gptr"             , 0x07, 0x07, 0x07, NON_VPD_RING, 0, 0x0100000000002000},
    {"mc_iom23_time"             , 0x08, 0x07, 0x07, NON_VPD_RING, 0, 0x0100000000000100},
    {"mc_pll_gptr"               , 0x09, 0x07, 0x07, NON_VPD_RING, 0, 0x0002000000002000},
    {"mc_pll_bndy_bucket_1"      , 0x0a, 0x07, 0x07, NON_VPD_PLL_RING, 0, 0x0002000000000080},
    {"mc_pll_bndy_bucket_2"      , 0x0b, 0x07, 0x07, NON_VPD_PLL_RING, 0, 0x0002000000000080},
    {"mc_pll_bndy_bucket_3"      , 0x0c, 0x07, 0x07, NON_VPD_PLL_RING, 0, 0x0002000000000080},
    {"mc_pll_bndy_bucket_4"      , 0x0d, 0x07, 0x07, NON_VPD_PLL_RING, 0, 0x0002000000000080},
    {"mc_pll_bndy_bucket_5"      , 0x0e, 0x07, 0x07, NON_VPD_PLL_RING, 0, 0x0002000000000080},
    {"mc_pll_func"               , 0x0f, 0x07, 0x07, NON_VPD_RING, 0, 0x0002000000008000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"mc_repr"          , 0x10, 0x07, 0x08, NON_VPD_RING, 0, 0x0C00000000000200},
    {"mc_iom01_repr"    , 0x11, 0x07, 0x08, NON_VPD_RING, 0, 0x0200000000000200},
    {"mc_iom23_repr"    , 0x12, 0x07, 0x08, NON_VPD_RING, 0, 0x0100000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace OB0
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"ob0_fure"                   , 0x00, 0x09, 0x09, NON_VPD_RING, 0, 0x0E00000000009000},
    {"ob0_gptr"                   , 0x01, 0x09, 0x09, NON_VPD_RING, 0, 0x0E00000000002000},
    {"ob0_time"                   , 0x02, 0x09, 0x09, NON_VPD_RING, 0, 0x0E00000000000100},
    {"ob0_pll_gptr"               , 0x03, 0x09, 0x09, NON_VPD_RING, 0, 0x0002000000002000},
    {"ob0_pll_bndy"               , 0x04, 0x09, 0x09, NON_VPD_RING, 0, 0x0002000000000080},
    {"ob0_pll_func"               , 0x05, 0x09, 0x09, NON_VPD_RING, 0, 0x0002000000008000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"ob0_repr"          , 0x0a, 0x09, 0x09, NON_VPD_RING, 0, 0x0E00000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace OB1
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"ob1_fure"                   , 0x00, 0x0a, 0x0a, NON_VPD_RING, 0, 0x0E00000000009000},
    {"ob1_gptr"                   , 0x01, 0x0a, 0x0a, NON_VPD_RING, 0, 0x0E00000000002000},
    {"ob1_time"                   , 0x02, 0x0a, 0x0a, NON_VPD_RING, 0, 0x0E00000000000100},
    {"ob1_pll_gptr"               , 0x03, 0x0a, 0x0a, NON_VPD_RING, 0, 0x0002000000002000},
    {"ob1_pll_bndy"               , 0x04, 0x0a, 0x0a, NON_VPD_RING, 0, 0x0002000000000080},
    {"ob1_pll_func"               , 0x05, 0x0a, 0x0a, NON_VPD_RING, 0, 0x0002000000008000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"ob1_repr"          , 0x0a, 0x0a, 0x0a, NON_VPD_RING, 0, 0x0E00000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace OB2
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"ob2_fure"                   , 0x00, 0x0b, 0x0b, NON_VPD_RING, 0, 0x0E00000000009000},
    {"ob2_gptr"                   , 0x01, 0x0b, 0x0b, NON_VPD_RING, 0, 0x0E00000000002000},
    {"ob2_time"                   , 0x02, 0x0b, 0x0b, NON_VPD_RING, 0, 0x0E00000000000100},
    {"ob2_pll_gptr"               , 0x03, 0x0b, 0x0b, NON_VPD_RING, 0, 0x0002000000002000},
    {"ob2_pll_bndy"               , 0x04, 0x0b, 0x0b, NON_VPD_RING, 0, 0x0002000000000080},
    {"ob2_pll_func"               , 0x05, 0x0b, 0x0b, NON_VPD_RING, 0, 0x0002000000008000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"ob2_repr"          , 0x0a, 0x0b, 0x0b, NON_VPD_RING, 0, 0x0E00000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace OB3
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"ob3_fure"                   , 0x00, 0x0c, 0x0c, NON_VPD_RING, 0, 0x0E00000000009000},
    {"ob3_gptr"                   , 0x01, 0x0c, 0x0c, NON_VPD_RING, 0, 0x0E00000000002000},
    {"ob3_time"                   , 0x02, 0x0c, 0x0c, NON_VPD_RING, 0, 0x0E00000000000100},
    {"ob3_pll_gptr"               , 0x03, 0x0c, 0x0c, NON_VPD_RING, 0, 0x0002000000002000},
    {"ob3_pll_bndy"               , 0x04, 0x0c, 0x0c, NON_VPD_RING, 0, 0x0002000000000080},
    {"ob3_pll_func"               , 0x05, 0x0c, 0x0c, NON_VPD_RING, 0, 0x0002000000008000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"ob3_repr"          , 0x0a, 0x0c, 0x0c, NON_VPD_RING, 0, 0x0E00000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace PCI0
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"pci0_fure"            , 0x00, 0x0d, 0x0d, NON_VPD_RING, 0, 0x0E00000000009000},
    {"pci0_gptr"            , 0x01, 0x0d, 0x0d, NON_VPD_RING, 0, 0x0E00000000002000},
    {"pci0_time"            , 0x02, 0x0d, 0x0d, NON_VPD_RING, 0, 0x0E00000000000100},
    {"pci0_pll_bndy"        , 0x03, 0x0d, 0x0d, NON_VPD_RING, 0, 0x0002000000000080},
    {"pci0_pll_gptr"        , 0x04, 0x0d, 0x0d, NON_VPD_RING, 0, 0x0002000000002000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"pci0_repr"            , 0x05, 0x0d, 0x0d, NON_VPD_RING, 0, 0x0E00000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace PCI1
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"pci1_fure"            , 0x00, 0x0e, 0x0e, NON_VPD_RING, 0, 0x0F00000000009000},
    {"pci1_gptr"            , 0x01, 0x0e, 0x0e, NON_VPD_RING, 0, 0x0F00000000002000},
    {"pci1_time"            , 0x02, 0x0e, 0x0e, NON_VPD_RING, 0, 0x0F00000000000100},
    {"pci1_pll_bndy"        , 0x03, 0x0e, 0x0e, NON_VPD_RING, 0, 0x0002000000000080},
    {"pci1_pll_gptr"        , 0x04, 0x0e, 0x0e, NON_VPD_RING, 0, 0x0002000000002000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"pci1_repr"            , 0x05, 0x0e, 0x0e, NON_VPD_RING, 0, 0x0F00000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace PCI2
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"pci2_fure"            , 0x00, 0x0f, 0x0f, NON_VPD_RING, 0, 0x0F80000000009000},
    {"pci2_gptr"            , 0x01, 0x0f, 0x0f, NON_VPD_RING, 0, 0x0F80000000002000},
    {"pci2_time"            , 0x02, 0x0f, 0x0f, NON_VPD_RING, 0, 0x0F80000000000100},
    {"pci2_pll_bndy"        , 0x03, 0x0f, 0x0f, NON_VPD_RING, 0, 0x0002000000000080},
    {"pci2_pll_gptr"        , 0x04, 0x0f, 0x0f, NON_VPD_RING, 0, 0x0002000000002000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"pci2_repr"            , 0x05, 0x0F, 0x0F, NON_VPD_RING, 0, 0x0F80000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, RL, NOT_VALID};
};


namespace EQ
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    {"eq_fure"                    , 0x00, 0x10, 0x10, NON_VPD_RING, 0, 0x0C10000000009000},
    {"eq_gptr"                    , 0x01, 0x10, 0x10, NON_VPD_RING, 0, 0x0C10000000002000},
    {"eq_time"                    , 0x02, 0x10, 0x10, NON_VPD_RING, 0, 0x0C10000000000100},
    {"eq_mode"                    , 0x03, 0x10, 0x10, NON_VPD_RING, 0, 0x0C10000000004000},
    {"ex_l3_fure"                 , 0x04, 0x10, 0x10, NON_VPD_RING, 0, 0x0200000000009000},
    {"ex_l3_gptr"                 , 0x05, 0x10, 0x10, NON_VPD_RING, 0, 0x0200000000002000},
    {"ex_l3_time"                 , 0x06, 0x10, 0x10, NON_VPD_RING, 0, 0x0200000000000100},
    {"ex_l2_mode"                 , 0x07, 0x10, 0x10, NON_VPD_RING, 0, 0x0080000000004000},
    {"ex_l2_fure"                 , 0x08, 0x10, 0x10, NON_VPD_RING, 0, 0x0080000000009000},
    {"ex_l2_gptr"                 , 0x09, 0x10, 0x10, NON_VPD_RING, 0, 0x0080000000002000},
    {"ex_l2_time"                 , 0x0a, 0x10, 0x10, NON_VPD_RING, 0, 0x0080000000000100},
    {"ex_l3_refr_fure"            , 0x0b, 0x10, 0x10, NON_VPD_RING, 0, 0x0008000000009000},
    {"ex_l3_refr_gptr"            , 0x0c, 0x10, 0x10, NON_VPD_RING, 0, 0x0008000000002000},
    {"eq_ana_func"                , 0x0d, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000008000},
    {"eq_ana_gptr"                , 0x0e, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000002000},
    {"eq_dpll_func"               , 0x0f, 0x10, 0x10, NON_VPD_RING, 0, 0x0002000000008000},
    {"eq_dpll_gptr"               , 0x10, 0x10, 0x10, NON_VPD_RING, 0, 0x0002000000002000},
    {"eq_dpll_mode"               , 0x11, 0x10, 0x10, NON_VPD_RING, 0, 0x0002000000004000},
    {"eq_ana_bndy_bucket_0"       , 0x12, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_1"       , 0x13, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_2"       , 0x14, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_3"       , 0x15, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_4"       , 0x16, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_5"       , 0x17, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_6"       , 0x18, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_7"       , 0x19, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_8"       , 0x1a, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_9"       , 0x1b, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_10"      , 0x1c, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_11"      , 0x1d, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_12"      , 0x1e, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_13"      , 0x1f, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_14"      , 0x20, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_15"      , 0x21, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_16"      , 0x22, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_17"      , 0x23, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_18"      , 0x24, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_19"      , 0x25, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_20"      , 0x26, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_21"      , 0x27, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_22"      , 0x28, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_23"      , 0x29, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_24"      , 0x2a, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_bucket_25"      , 0x2b, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_bndy_l3dcc_bucket_26", 0x2c, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000000080},
    {"eq_ana_mode"                , 0x2d, 0x10, 0x10, NON_VPD_RING, 0, 0x0020000000004000},
    {"ex_l2_fure_1"               , 0x2e, 0x10, 0x10, NON_VPD_RING, 0, 0x0080000000009000},
    {"ex_l3_fure_1"               , 0x2f, 0x10, 0x10, NON_VPD_RING, 0, 0x0200000000009000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    {"eq_repr"          , 0x30, 0x10, 0x1b, NON_VPD_RING, 0, 0x0C10000000000200},
    {"ex_l3_repr"       , 0x31, 0x10, 0x1b, NON_VPD_RING, 0, 0x0200000000000200},
    {"ex_l2_repr"       , 0x32, 0x10, 0x1b, NON_VPD_RING, 0, 0x0080000000000200},
    {"ex_l3_refr_repr"  , 0x33, 0x10, 0x1b, NON_VPD_RING, 0, 0x0008000000000200},
    {"ex_l3_refr_time"  , 0x34, 0x10, 0x1b, NON_VPD_RING, 0, 0x0008000000000100},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, CC, RL};
};


namespace EC
{
const GenRingIdList RING_ID_LIST_COMMON[] =
{
    { "ec_func"         , 0x00, 0x20, 0x20, NON_VPD_RING, 0, 0x0E00000000009000},
    { "ec_gptr"         , 0x01, 0x20, 0x20, NON_VPD_RING, 0, 0x0E00000000002000},
    { "ec_time"         , 0x02, 0x20, 0x20, NON_VPD_RING, 0, 0x0E00000000000100},
    { "ec_mode"         , 0x03, 0x20, 0x20, NON_VPD_RING, 0, 0x0E00000000004000},
};
const GenRingIdList RING_ID_LIST_INSTANCE[] =
{
    { "ec_repr"         , 0x04, 0x20, 0x37, NON_VPD_RING, 0, 0x0E00000000000200},
};
const RingVariantOrder RING_VARIANT_ORDER[] = { BASE, CC, RL};
};

