/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/memory/lib/freq/sync.H $   */
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
///
/// @file synch.H
/// @brief Synchronous function implementations
///
// *HWP HWP Owner: Andre Marin <aamarin@us.ibm.com>
// *HWP HWP Backup: Brian Silver <bsilver@us.ibm.com>
// *HWP Team: Memory
// *HWP Level: 2
// *HWP Consumed by: HB:FSP

#ifndef _MSS_SYNC_H_
#define _MSS_SYNC_H_

#include <map>
#include <vector>

#include <fapi2.H>

namespace mss
{

enum class speed_equality : uint8_t
{
    NOT_EQUAL_DIMM_SPEEDS = 0,
    EQUAL_DIMM_SPEEDS = 1,
};

///
/// @brief Retrieves a mapping of MSS frequency values per mcbist target
/// @param[in] i_targets vector of controller targets
/// @param[out] o_freq_map dimm speed map <key, value> = (mcbist target, frequency)
/// @param[out] o_is_speed_equal holds whether map dimm speed is equal
/// @return FAPI2_RC_SUCCESS iff successful
///
fapi2::ReturnCode dimm_speed_map(const std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCBIST> >& i_targets,
                                 std::map< fapi2::Target<fapi2::TARGET_TYPE_MCBIST>, uint64_t >& o_freq_map,
                                 speed_equality& o_is_speed_equal);

///
/// @brief Helper function to deconfigure MCS targets connected to MCBIST
/// @param[in] i_target the controller target
/// @param[in] i_dimm_speed dimm speed in MT/s
/// @param[in] i_nest_freq nest freq in MHz
/// @return true if hardware was deconfigured
///
bool deconfigure(const fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_target,
                 const uint64_t i_dimm_speed,
                 const uint32_t i_nest_freq);

///
/// @brief Selects synchronous mode and performs requirements enforced by ATTR_REQUIRED_SYNCH_MODE
/// @param[in] i_freq_map dimm speed mapping
/// @param[in] i_equal_dimm_speed tracks whether map has equal dimm speeds
/// @param[in] i_nest_freq nest frequency
/// @param[in] i_required_sync_mode system policy to enforce synchronous mode
/// @param[out] o_selected_sync_mode final synchronous mode
/// @param[out] o_selected_freq final freq selected, only valid if final sync mode is in-sync
/// @return FAPI2_RC_SUCCESS iff successful
///
fapi2::ReturnCode select_sync_mode(const std::map< fapi2::Target<fapi2::TARGET_TYPE_MCBIST>, uint64_t >& i_freq_map,
                                   const speed_equality i_equal_dimm_speed,
                                   const uint32_t i_nest_freq,
                                   const uint8_t i_required_sync_mode,
                                   uint8_t& o_selected_sync_mode,
                                   uint64_t& o_selected_freq);

///
/// @brief Create and sort a vector of supported MT/s (freq) - helper for testing purposes
/// @param[in] MCS target for which to get the DIMM configs
/// @param[in] vector of MVPD freqs
/// @param[in] vector of max allowed freqs
/// @param[in] bool whether or not we're forced into sync mode
/// @param[in,out] reference to a std::vector<uint32_t> space to put the sorted vector
/// @return FAPI2_RC_SUCCESS iff ok
/// @note Taken from ATTR_MSS_MRW_SUPPORTED_FREQ. The result is sorted so such that the min
/// supported freq is std::vector<>.begin and the max is std::vector<>.end - 1. You can
/// search the resulting vector for valid frequencies as it is sorted.
///
fapi2::ReturnCode supported_freqs_helper(const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_target,
        const std::vector<uint32_t>& i_freqs,
        const std::vector<uint32_t>& i_max_freqs,
        const bool i_req_sync_mode,
        std::vector<uint32_t>& io_freqs);

///
/// @brief Create and sort a vector of supported MT/s (freq)
/// @param[in] MCS target for which to get the DIMM configs
/// @param[in,out] reference to a std::vector<uint32_t> space to put the sorted vector
/// @return FAPI2_RC_SUCCESS iff ok
/// @note Taken from ATTR_MSS_MRW_SUPPORTED_FREQ. The result is sorted so such that the min
/// supported freq is std::vector<>.begin and the max is std::vector<>.end - 1. You can
/// search the resulting vector for valid frequencies as it is sorted.
///
fapi2::ReturnCode supported_freqs(const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_target,
                                  std::vector<uint32_t>& io_freqs);

///
/// @brief Return whether a given freq is supported
/// @param[in] a freq to check for
/// @param[in] reference to a std::vector<uint32_t> of freqs
/// @return bool, true iff input freq is supported
///
fapi2::ReturnCode is_freq_supported(const uint32_t i_freq, const std::vector<uint32_t>& i_freqs);


}// mss

#endif
