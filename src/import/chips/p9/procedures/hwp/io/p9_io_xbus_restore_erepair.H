/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/io/p9_io_xbus_restore_erepair.H $ */
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
///----------------------------------------------------------------------------
/// *HWP HWP Owner        : Chris Steffen <cwsteffen@us.ibm.com>
/// *HWP HWP Backup Owner : Gary Peterson <garyp@us.ibm.com>
/// *HWP FW Owner         : Jamie Knight <rjknight@us.ibm.com>
/// *HWP Team             : IO
/// *HWP Level            : 1
/// *HWP Consumed by      : FSP:HB
///----------------------------------------------------------------------------

#ifndef _P9_IO_XBUS_RESTORE_EREPAIR_H_
#define _P9_IO_XBUS_RESTORE_EREPAIR_H_

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include <fapi2.H>

// function pointer typedef definition for HWP call support
typedef fapi2::ReturnCode (*p9_io_xbus_restore_erepair_FP_t) (
    const fapi2::Target < fapi2::TARGET_TYPE_XBUS >&,
    const uint8_t&,
    const std::vector< uint8_t >&,
    const std::vector< uint8_t >&);

extern "C"
{

    /**
     * @brief A HWP that runs Restore eRepair.  This procedure should update the
     * bad lane vector and power down the bad lanes.  The rx bad lanes will update
     * the bad lane vector of the passed target.  The lanes specified in the
     * i_rx_bad_lanes and in the i_tx_bad_lanes vectors will be powered down on the
     * target that is passed in.  Note: This procedure does not power down any
     * bad lanes on the connected target.
     * @param[in] i_target        FAPI2 Target
     * @param[in] i_group         Clock Group of Target
     * @param[in] i_rx_bad_lanes  Vector of Rx Bad Lanes
     * @param[in] i_tx_bad_lanes  Vector of Tx Bad Lanes
     * @retval    ReturnCode
     */
    fapi2::ReturnCode
    p9_io_xbus_restore_erepair(
        const fapi2::Target< fapi2::TARGET_TYPE_XBUS >& i_target,
        const uint8_t&                                  i_group,
        const std::vector< uint8_t >&                   i_rx_bad_lanes,
        const std::vector< uint8_t >&                   i_tx_bad_lanes );

} // extern "C"

#endif // _P9_IO_XBUS_RESTORE_EREPAIR_H_
