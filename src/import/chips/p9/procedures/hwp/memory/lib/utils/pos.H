/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/memory/lib/utils/pos.H $   */
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

///
/// @file pos.H
/// @brief Tools to return target's position from a fapi2 target
///
// *HWP HWP Owner: Brian Silver <bsilver@us.ibm.com>
// *HWP HWP Backup: Andre Marin <aamarin@us.ibm.com>
// *HWP Team: Memory
// *HWP Level: 2
// *HWP Consumed by: HB:FSP

#ifndef _MSS_POS_H_
#define _MSS_POS_H_

#include <fapi2.H>
#include <lib/shared/mss_const.H>

namespace mss
{

///
/// @brief Trait classes for the mss::pos functions
///
template<fapi2::TargetType T>
class posTraits
{
    public:
        // Needed as some targets have chip_unit_pos attributes,
        // uint8_t and some attr_pos, uint32_t. We don't really care
        // but the type checking in the macros sure does ...
        typedef uint8_t pos_type;
};

///
/// @brief DIMM Trait class for the mss::pos functions
///
template<>
class posTraits<fapi2::TARGET_TYPE_DIMM>
{
    public:
        typedef uint32_t pos_type;
};

///
/// @brief Processor Trait class for the mss::pos functions
///
template<>
class posTraits<fapi2::TARGET_TYPE_PROC_CHIP>
{
    public:
        typedef uint32_t pos_type;
};

///
/// @brief Return a target's position from a fapi2 target
/// @tparam T the fapi2::TargetType
/// @param[in] i_target a target representing the target in question
/// @return The position relative to the chip
///
template< fapi2::TargetType T, typename TT = posTraits<T> >
inline typename TT::pos_type pos(const fapi2::Target<T>& i_target)
{
    typename TT::pos_type i_pos = 0;

    // Don't use FAPI_TRY as you'll mess up fapi2::current_err which
    // lmits where this can be used.
    if (FAPI_ATTR_GET(fapi2::ATTR_CHIP_UNIT_POS, i_target, i_pos) != fapi2::FAPI2_RC_SUCCESS)
    {
        goto fapi_try_exit;
    }

    return i_pos;

fapi_try_exit:
    // If we can't get our unit position, we're in other trouble
    FAPI_ERR("can't get our chip unit position");

    // Cronusflex doesn't support Assert.
    // Enable once move to dev complete - AAM
#if MOVED_AWAY_FROM_CRONUSFLEX
    fapi2::Assert(false);
#endif
    return 0;
}

///
/// @brief Return a DIMM's position from a fapi2 target
/// @param[in] i_target a target representing the target in question
/// @return The position relative to the chip
///
template<>
inline posTraits<fapi2::TARGET_TYPE_DIMM>::pos_type pos(const fapi2::Target<fapi2::TARGET_TYPE_DIMM>& i_target)
{
    posTraits<fapi2::TARGET_TYPE_DIMM>::pos_type i_pos = 0;

    if (FAPI_ATTR_GET(fapi2::ATTR_POS, i_target, i_pos) != fapi2::FAPI2_RC_SUCCESS)
    {
        goto fapi_try_exit;
    }

    return i_pos;

fapi_try_exit:
    // If we can't get our position, we're in other trouble
    FAPI_ERR("can't get our position");
#if MOVED_AWAY_FROM_CRONUSFLEX
    fapi2::Assert(false);
#endif
    return 0;

}

///
/// @brief Return a processor's position from a fapi2 target
/// @param[in] i_target a target representing the target in question
/// @return The position relative to the chip
///
template<>
inline posTraits<fapi2::TARGET_TYPE_PROC_CHIP>::pos_type pos(
    const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target)
{
    posTraits<fapi2::TARGET_TYPE_PROC_CHIP>::pos_type i_pos = 0;

    if (FAPI_ATTR_GET(fapi2::ATTR_POS, i_target, i_pos) != fapi2::FAPI2_RC_SUCCESS)
    {
        goto fapi_try_exit;
    }

    return i_pos;

fapi_try_exit:
    // If we can't get our position, we're in other trouble
    FAPI_ERR("can't get our position");
#if MOVED_AWAY_FROM_CRONUSFLEX
    fapi2::Assert(false);
#endif
    return 0;

}


///
/// @brief Return a target's relative position from a fapi2 target
/// @tparam T the fapi2::TargetType
/// @tparam R the fapi2::TargetType we want the position relative to
/// @param[in] i_target a target representing the target in question
/// @return The position relative to chiplet R
///
template< fapi2::TargetType R, fapi2::TargetType T, typename TT = posTraits<T> >
inline typename TT::pos_type relative_pos(const fapi2::Target<T>& i_target);

///
/// @brief Return a MCA's relative position from an MCBIST
/// @param[in] i_target a target representing the target in question
/// @return The position relative to chiplet R
///
template<>
inline posTraits<fapi2::TARGET_TYPE_MCA>::pos_type
relative_pos<fapi2::TARGET_TYPE_MCBIST>(const fapi2::Target<fapi2::TARGET_TYPE_MCA>& i_target)
{
    return pos(i_target) % PORTS_PER_MCBIST;
}

///
/// @brief Return a DIMM's relative position from an MCS
/// @param[in] i_target a target representing the target in question
/// @return The position relative to chiplet R
///
template<>
inline posTraits<fapi2::TARGET_TYPE_DIMM>::pos_type
relative_pos<fapi2::TARGET_TYPE_MCS>(const fapi2::Target<fapi2::TARGET_TYPE_DIMM>& i_target)
{
    constexpr uint64_t MAX_DIMM_PER_MCS = PORTS_PER_MCS * MAX_DIMM_PER_PORT;
    return pos(i_target) % MAX_DIMM_PER_MCS;
}

///
/// @brief Return an MCS's relative position from a processor
/// @param[in] i_target a target representing the target in question
/// @return The position relative to chiplet R
///
template<>
inline posTraits<fapi2::TARGET_TYPE_MCS>::pos_type
relative_pos<fapi2::TARGET_TYPE_PROC_CHIP>(const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_target)
{
    constexpr uint64_t MCS_PER_PROC = MC_PER_MODULE * MCS_PER_MC;
    return pos(i_target) % MCS_PER_PROC;
}

///
/// @brief Return an MCA's relative position from an MCS
/// @param[in] i_target a target representing the target in question
/// @return The position relative to chiplet R
///
template<>
inline posTraits<fapi2::TARGET_TYPE_MCA>::pos_type
relative_pos<fapi2::TARGET_TYPE_MCS>(const fapi2::Target<fapi2::TARGET_TYPE_MCA>& i_target)
{
    return pos(i_target) % PORTS_PER_MCS;
}



///
/// @brief Return a target's fapi position from a fapi2 target
/// @tparam T the fapi2::TargetType
/// @param[in] i_target a target representing the target in question
/// @return The position relative to the system
///
template< fapi2::TargetType T, typename TT = posTraits<T> >
inline uint32_t fapi_pos(const fapi2::Target<T>& i_target)
{
    uint32_t i_pos = 0;

    // Don't use FAPI_TRY as you'll mess up fapi2::current_err which
    // lmits where this can be used.
    if (FAPI_ATTR_GET(fapi2::ATTR_FAPI_POS, i_target, i_pos) != fapi2::FAPI2_RC_SUCCESS)
    {
        goto fapi_try_exit;
    }

    return i_pos;

fapi_try_exit:
    // If we can't get our fapi position, we're in other trouble
    FAPI_ERR("can't get our fapi position");
#if MOVED_AWAY_FROM_CRONUSFLEX
    fapi2::Assert(false);
#endif
    return 0;
}


}
#endif
