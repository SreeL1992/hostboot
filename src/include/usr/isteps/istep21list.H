/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/isteps/istep21list.H $                        */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2016                        */
/* [+] Google Inc.                                                        */
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

#ifndef __ISTEPS_ISTEP21LIST_H
#define __ISTEPS_ISTEP21LIST_H

/**
 * @file    istep21list.H
 *
 *  IStep 21    Start Payload
 *  IPL FLow Doc        v0.60 (06/02/15)
 *
 *    21.1    host_runtime_setup
 *          :  Start Payload
 *    21.2    host_verify_hdat
 *          :  Start Payload
 *    21.3    host_start_payload
 *          :  Start Payload
 *
 *
 *  Please see the note in initsvcstructs.H for description of
 *      the ISTEPNAME macro.
 *
 */

#include    <initservice/initsvcstructs.H>
#include    <initservice/initsvcreasoncodes.H>
#include    <config.h>

namespace   ISTEP_21
{

/**
 *  @brief   host_runtime_setup
 *
 *  param[in,out]   -   pointer to any arguments, usually NULL
 *
 *  return any errlogs to istep
 *
 */
void*  call_host_runtime_setup( void    *io_pArgs );



/**
 *  @brief   host_verify_hdat
 *
 *  param[in,out]   -   pointer to any arguments, usually NULL
 *
 *  return  any errlogs to istep
 *
 */
void*    call_host_verify_hdat( void    *io_pArgs );



/**
 *  @brief   host_start_payload
 *
 *  param[in,out]   -   pointer to any arguments, usually NULL
 *
 *  return  any errlogs to istep
 *
 */
void*    call_host_start_payload( void    *io_pArgs );


};


namespace   INITSERVICE
{
    const   TaskInfo    g_istep21[]  =   {
        {
                "",         // dummy, index 0
                NULL,
                { NONE, EXT_IMAGE, IPL_NOOP, false }
        },
        {
                ISTEPNAME(21,01,"host_runtime_setup"),
                ISTEP_21::call_host_runtime_setup,
                { START_FN, EXT_IMAGE, MPIPL_OP | NORMAL_IPL_OP, false }
        },
        {
                ISTEPNAME(21,02,"host_verify_hdat"),
                ISTEP_21::call_host_verify_hdat,
                { START_FN, EXT_IMAGE, MPIPL_OP | NORMAL_IPL_OP, false }
        },
        {
                ISTEPNAME(21,03,"host_start_payload"),
                ISTEP_21::call_host_start_payload,
                { START_FN, EXT_IMAGE, MPIPL_OP | NORMAL_IPL_OP, false }
        },

};

const DepModInfo  g_istep21Dependancies = {
    {
        DEP_LIB(libistep21.so),
        DEP_LIB(libdevtree.so),
        DEP_LIB(libruntime.so),
        DEP_LIB(libp9_cpuWkup.so),
        NULL
    }
};

const ExtTaskInfo g_istep21TaskList = {
    &(g_istep21[0]),
    ( sizeof(g_istep21)/sizeof(TaskInfo) ),
    &g_istep21Dependancies
};

};  // end namespace

#endif
