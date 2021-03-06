/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/runtime/populate_attributes.C $                       */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2016                        */
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
 *  @file populate_attributes.C
 *
 *  @brief Populate attributes for runtime HostServices code
 */

#include <sys/misc.h>
#include <trace/interface.H>
#include <errl/errlentry.H>
#include <fapi.H>
#include <fapiAttributeIds.H>
#include <targeting/common/target.H>
#include <targeting/common/targetservice.H>
#include <targeting/common/utilFilter.H>
#include <targeting/common/entitypath.H>
#include <runtime/runtime_reasoncodes.H>
#include <runtime/runtime.H>
#include "common/hsvc_attribute_structs.H"
#include <mbox/ipc_msg_types.H>
#include <sys/task.h>
#include <intr/interrupt.H>
#include <errl/errlmanager.H>

trace_desc_t *g_trac_runtime = NULL;
TRAC_INIT(&g_trac_runtime, RUNTIME_COMP_NAME, KILOBYTE);

/**
 * @brief Read a FAPI attribute and stick it into mainstore
 */
#define HSVC_LOAD_ATTR(__fid) \
    fapi::__fid##_Type result_##__fid; \
    _rc = FAPI_ATTR_GET( __fid, _target, result_##__fid ); \
    if( _rc ) { \
        TRACFCOMP( g_trac_runtime, "Error reading 0x%X, rc=0x%X", fapi::__fid, _rc ); \
        _failed_attribute = fapi::__fid; \
        break; \
    } \
    TRACDCOMP( g_trac_runtime, "> %d: 0x%x=%X @ %p", *_num_attr, fapi::__fid, result_##__fid, _output_ptr ); \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = fapi::__fid; \
    _cur_header->sizeBytes = sizeof(fapi::__fid##_Type); \
    _cur_header->offset = (_output_ptr - _beginning); \
    if( _output_ptr + sizeof(fapi::__fid##_Type) > _end_of_area ) { \
        TRACFCOMP( g_trac_runtime, "Not enough space for 0x%X (%p + 0x%X > %p", fapi::__fid, _output_ptr, sizeof(fapi::__fid##_Type), _end_of_area ); \
        _failed_attribute = fapi::__fid; \
        break; \
    } \
    memcpy( _output_ptr, &result_##__fid, sizeof(fapi::__fid##_Type) ); \
    _output_ptr += sizeof(fapi::__fid##_Type); \
    (*_num_attr)++;

/**
 * @brief Read a Privileged FAPI attribute and stick it into mainstore
 */
#define HSVC_LOAD_ATTR_P(__fid) \
    fapi::__fid##_Type result_##__fid; \
    _rc = FAPI_ATTR_GET_PRIVILEGED( __fid, _target, result_##__fid ); \
    if( _rc ) { \
        TRACFCOMP( g_trac_runtime, "Error reading 0x%X, rc=0x%X", fapi::__fid, _rc ); \
        _failed_attribute = fapi::__fid; \
        break; \
    } \
    TRACDCOMP( g_trac_runtime, "> %d: 0x%x=%X @ %p", *_num_attr, fapi::__fid, result_##__fid, _output_ptr ); \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = fapi::__fid; \
    _cur_header->sizeBytes = sizeof(fapi::__fid##_Type); \
    _cur_header->offset = (_output_ptr - _beginning); \
    if( _output_ptr + sizeof(fapi::__fid##_Type) > _end_of_area ) { \
        TRACFCOMP( g_trac_runtime, "Not enough space for 0x%X (%p + 0x%X > %p", fapi::__fid, _output_ptr, sizeof(fapi::__fid##_Type), _end_of_area ); \
        _failed_attribute = fapi::__fid; \
        break; \
    } \
    memcpy( _output_ptr, &result_##__fid, sizeof(fapi::__fid##_Type) ); \
    _output_ptr += sizeof(fapi::__fid##_Type); \
    (*_num_attr)++;

/**
 * @brief Read the HUID attribute from targeting and stick it into mainstore
 */
#define ADD_HUID(__targ) \
    _huid_temp = TARGETING::get_huid(__targ); \
    TRACDCOMP( g_trac_runtime, "> HUID=%.8X @ %p", _huid_temp, _output_ptr ); \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = HSVC_HUID; \
    _cur_header->sizeBytes = sizeof(uint32_t); \
    _cur_header->offset = (_output_ptr - _beginning); \
    memcpy( _output_ptr, &_huid_temp, sizeof(uint32_t) ); \
    _output_ptr += sizeof(uint32_t); \
    (*_num_attr)++;

/**
 * @brief Read the PHYS_PATH attribute from targeting and stick it into mainstore
 */
#define ADD_PHYS_PATH(__targ) \
   { TARGETING::AttributeTraits<TARGETING::ATTR_PHYS_PATH>::Type pathPhys; \
    _rc = !(__targ->tryGetAttr<TARGETING::ATTR_PHYS_PATH>(pathPhys)); \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = HSVC_PHYS_PATH; \
    _cur_header->sizeBytes = sizeof(uint8_t) + (sizeof(pathPhys[0]) * pathPhys.size()); \
    _cur_header->offset = (_output_ptr - _beginning); \
    memcpy( _output_ptr, &pathPhys,  _cur_header->sizeBytes ); \
    _output_ptr +=  _cur_header->sizeBytes; \
    (*_num_attr)++; }

/**
 * @brief Read the ECMD_STRING attribute from targeting and stick it into mainstore
 */
#define ADD_ECMD_STRING() \
   { const char* estring = _target->toEcmdString(); \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = HSVC_ECMD_STRING; \
    _cur_header->sizeBytes = fapi::MAX_ECMD_STRING_LEN; \
    _cur_header->offset = (_output_ptr - _beginning); \
    memcpy( _output_ptr, estring,  _cur_header->sizeBytes ); \
    _output_ptr +=  _cur_header->sizeBytes; \
    (*_num_attr)++; }

/**
 * @brief Read the IBSCOM_BASE attribute from targeting
 *        and stick it into mainstore
 */
#define ADD_IBSCOM_BASE(__targ) \
   { TARGETING::AttributeTraits \
       <TARGETING::ATTR_IBSCOM_PROC_BASE_ADDR>::Type ibscomBase; \
    _rc = !(__targ->tryGetAttr \
       <TARGETING::ATTR_IBSCOM_PROC_BASE_ADDR>(ibscomBase)); \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = HSVC_IBSCOM_BASE; \
    _cur_header->sizeBytes = sizeof(uint64_t); \
    _cur_header->offset = (_output_ptr - _beginning); \
    memcpy( _output_ptr, &ibscomBase,  _cur_header->sizeBytes ); \
    _output_ptr +=  _cur_header->sizeBytes; \
    (*_num_attr)++; }

/**
 * @brief Read the PLID attribute from targeting
 *        and stick it into mainstore
 */
#define ADD_PLID(__targ) \
   { TARGETING::AttributeTraits \
       <TARGETING::ATTR_HOSTSVC_PLID>::Type plid_temp; \
    _rc = !(__targ->tryGetAttr \
       <TARGETING::ATTR_HOSTSVC_PLID>(plid_temp)); \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = HSVC_PLID; \
    _cur_header->sizeBytes = sizeof(uint32_t); \
    _cur_header->offset = (_output_ptr - _beginning); \
    memcpy( _output_ptr, &plid_temp,  _cur_header->sizeBytes ); \
    _output_ptr +=  _cur_header->sizeBytes; \
    (*_num_attr)++; }

/**
 * @brief Insert a terminator into the attribute list
 */
#define EMPTY_ATTRIBUTE \
    _cur_header = &(_all_headers[(*_num_attr)]); \
    _cur_header->id = hsvc_attr_header_t::NO_ATTRIBUTE; \
    _cur_header->sizeBytes = 0; \
    _cur_header->offset = 0;


namespace RUNTIME
{

// This is the data that will be in the 'System Attribute Data'
//  section of HDAT
struct system_data_t
{
    enum {
        MAX_ATTRIBUTES = 100
    };

    // header data that HostServices uses
    hsvc_system_data_t hsvc;

    // actual data content
    hsvc_attr_header_t attrHeaders[MAX_ATTRIBUTES];
    char attributes[MAX_ATTRIBUTES*sizeof(uint32_t)];
};

// This is the data that will be in the 'Node Attribute Data'
//  section of HDAT, there will be 1 of these per physical
//  drawer/book/octant/blade
struct node_data_t
{
    enum {
        MAX_PROCS_RSV = P8_MAX_PROCS+4, //leave space for double
        MAX_EX_RSV = MAX_PROCS_RSV*P8_MAX_EX_PER_PROC,
        NUM_PROC_ATTRIBUTES = 250,
        NUM_EX_ATTRIBUTES = 20,
        MAX_ATTRIBUTES = MAX_PROCS_RSV*NUM_PROC_ATTRIBUTES +
                         MAX_EX_RSV*NUM_EX_ATTRIBUTES
    };

    // header data that HostServices uses
    hsvc_node_data_t hsvc;

    // actual data content
    hsvc_proc_header_t procs[MAX_PROCS_RSV];
    hsvc_ex_header_t ex[MAX_EX_RSV];
    hsvc_attr_header_t procAttrHeaders[MAX_PROCS_RSV][NUM_PROC_ATTRIBUTES];
    hsvc_attr_header_t exAttrHeaders[MAX_EX_RSV][NUM_EX_ATTRIBUTES];
    char attributes[MAX_ATTRIBUTES*sizeof(uint32_t)];
};


/**
 * @brief Populate system attributes for HostServices
 * @param i_nodes  Bit-mask of present nodes
 */
errlHndl_t populate_system_attributes( uint64_t i_nodes )
{
    errlHndl_t errhdl = NULL;
    TRACFCOMP( g_trac_runtime, "populate_system_attributes> i_nodes=%.16X", i_nodes );

    // These variables are used by the HSVC_LOAD_ATTR macros directly
    uint64_t _failed_attribute = 0; //attribute we failed on
    int _rc = 0; //result from FAPI_ATTR_GET

    do {
        TRACDCOMP( g_trac_runtime, "-SYSTEM-" );

        // Wipe out our cache of the NACA/SPIRA pointers
        RUNTIME::rediscover_hdat();

        // find our memory range and fill it with some junk data
        uint64_t sys_data_addr = 0;
        uint64_t sys_data_size = 0;
        errhdl = RUNTIME::get_host_data_section(RUNTIME::HSVC_SYSTEM_DATA,
                                                0,
                                                sys_data_addr,
                                                sys_data_size );
        if( errhdl )
        {
            TRACFCOMP( g_trac_runtime, "Could not find a space for the system data" );
            break;
        }
        if( (sys_data_addr == 0) || (sys_data_size == 0) )
        {
            TRACFCOMP( g_trac_runtime, "Invalid memory values for HSVC_SYSTEM_DATA" );
            /*@
             * @errortype
             * @reasoncode       RUNTIME::RC_INVALID_SECTION
             * @moduleid         RUNTIME::MOD_RUNTIME_POP_SYS_ATTR
             * @userdata1        Returned address: sys_data_addr
             * @userdata2        Returned size: sys_data_size
             * @devdesc          Invalid memory values for HSVC_SYSTEM_DATA
             */
            errhdl = new ERRORLOG::ErrlEntry(
                                          ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                          RUNTIME::MOD_RUNTIME_POP_SYS_ATTR,
                                          RUNTIME::RC_INVALID_SECTION,
                                          sys_data_addr,
                                          sys_data_size );
            // most likely this is a HB code bug
            errhdl->addProcedureCallout(HWAS::EPUB_PRC_HB_CODE,
                                        HWAS::SRCI_PRIORITY_HIGH);
            // but it could also be a FSP bug in setting up the HDAT data
            errhdl->addProcedureCallout(HWAS::EPUB_PRC_SP_CODE,
                                        HWAS::SRCI_PRIORITY_HIGH);
            // save some of the HDAT data for FFDC
            RUNTIME::add_host_data_ffdc( RUNTIME::HSVC_SYSTEM_DATA, errhdl );

            break;
        }
        else if( sizeof(system_data_t) > sys_data_size )
        {
            TRACFCOMP( g_trac_runtime, "Not enough space allocated by HDAT for HostServices System Data" );
            /*@
             * @errortype
             * @reasoncode       RUNTIME::RC_NOT_ENOUGH_SPACE
             * @moduleid         RUNTIME::MOD_RUNTIME_POP_SYS_ATTR
             * @userdata1        Required size
             * @userdata2        Available size
             * @devdesc          Not enough space allocated by HDAT for
             *                   HostServices System Data
             */
            errhdl = new ERRORLOG::ErrlEntry(
                                          ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                          RUNTIME::MOD_RUNTIME_POP_SYS_ATTR,
                                          RUNTIME::RC_NOT_ENOUGH_SPACE,
                                          sizeof(system_data_t),
                                          sys_data_size );
            // need to update the FSP code
            errhdl->addProcedureCallout(HWAS::EPUB_PRC_SP_CODE,
                                        HWAS::SRCI_PRIORITY_HIGH);

            // save some of the HDAT data for FFDC
            RUNTIME::add_host_data_ffdc( RUNTIME::HSVC_SYSTEM_DATA, errhdl );

            break;
        }

        system_data_t* sys_data = reinterpret_cast<system_data_t*>(sys_data_addr);
        memset( sys_data, 'A', sizeof(system_data_t) );

        // These variables are used by the HSVC_LOAD_ATTR macros directly
        uint64_t* _num_attr = NULL; //pointer to numAttr in struct
        char* _output_ptr = NULL; //next memory location to copy attr data into
        char* _beginning = NULL; //position zero for offset calculation
        hsvc_attr_header_t* _all_headers = NULL; //array of attribute headers
        fapi::Target* _target = NULL; //target for FAPI_ATTR_GET
        hsvc_attr_header_t* _cur_header = NULL; //temp variable
        uint32_t _huid_temp = 0; //temp variable
        void* _end_of_area = reinterpret_cast<void*>(sys_data_addr
                                                     + sys_data_size);

        // Prepare the vars for the HSVC_LOAD_ATTR macros
        _beginning = reinterpret_cast<char*>(sys_data);
        _output_ptr = sys_data->attributes;
        _all_headers = sys_data->attrHeaders;
        _num_attr = &(sys_data->hsvc.numAttr);
        _target = NULL; //system queries use NULL target

        // Grab a system object to work with
        TARGETING::Target* sys = NULL;
        TARGETING::targetService().getTopLevelTarget(sys);

        // Fill in the metadata
        sys_data->hsvc.offset =
          reinterpret_cast<uint64_t>(sys_data->attrHeaders)
          - reinterpret_cast<uint64_t>(sys_data);
        sys_data->hsvc.nodePresent = i_nodes;
        sys_data->hsvc.numAttr = 0;

        // Fill up the attributes
        ADD_HUID( sys ); // for debug
        ADD_PHYS_PATH( sys );
        ADD_PLID( sys );
        // Use a generated file for the list of attributes to load
        #include "common/hsvc_sysdata.C"

        // Add an empty attribute header to signal the end
        EMPTY_ATTRIBUTE;

        size_t total_bytes = _output_ptr - sys_data->attributes;
        TRACFCOMP( g_trac_runtime, "populate_system_attributes> numAttr=%d, bytes=%d", sys_data->hsvc.numAttr, total_bytes );

        // Make sure we don't overrun our space
        assert( *_num_attr < system_data_t::MAX_ATTRIBUTES );

        TRACDCOMP( g_trac_runtime, "Run: system_cmp0.memory_ln4->image.save attributes.sys.bin 0x%X %d", sys_data, total_bytes );
    } while(0);

    // Handle any errors from FAPI_ATTR_GET
    if( _rc )
    {
        errlHndl_t rc_errhdl = NULL;

        /*@
         * @errortype
         * @reasoncode       RUNTIME::RC_ATTR_GET_FAIL
         * @moduleid         RUNTIME::MOD_RUNTIME_POP_SYS_ATTR
         * @userdata1        Return code from FAPI_ATTR_GET
         * @userdata2        FAPI Attribute Id that failed
         * @devdesc          Error retrieving FAPI attribute
         */
        rc_errhdl = new ERRORLOG::ErrlEntry( ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                             RUNTIME::MOD_RUNTIME_POP_SYS_ATTR,
                                             RUNTIME::RC_ATTR_GET_FAIL,
                                             _rc,
                                             _failed_attribute,
                                             true /*Add HB Software Callout*/);
        rc_errhdl->collectTrace(TARG_COMP_NAME,KILOBYTE/2);

        // Any previous error is probably more interesting so just commit
        //  the rc error
        if( errhdl )
        {
            errlCommit(rc_errhdl, RUNTIME_COMP_ID);
        }
        else
        {
            errhdl = rc_errhdl;
        }
    }

    return errhdl;
}

/**
 * @brief Populate node attributes for HostServices
 */
errlHndl_t populate_node_attributes( uint64_t i_nodeNum )
{
    errlHndl_t errhdl = NULL;

    // These variables are used by the HSVC_LOAD_ATTR macros directly
    uint64_t _failed_attribute = 0; //attribute we failed on
    int _rc = 0; //result from FAPI_ATTR_GET

    do {
        TRACDCOMP( g_trac_runtime, "-NODE-" );
        TRACFCOMP( g_trac_runtime, "populate_node_attributes(node=%d)", i_nodeNum );

        // Wipe out our cache of the NACA/SPIRA pointers
        RUNTIME::rediscover_hdat();

        // allocate memory and fill it with some junk data
        uint64_t node_data_addr = 0;
        size_t node_data_size = 0;
        errhdl = RUNTIME::get_host_data_section(
                                    RUNTIME::HSVC_NODE_DATA,
                                    i_nodeNum,
                                    node_data_addr,
                                    node_data_size );
        if( errhdl )
        {
            TRACFCOMP( g_trac_runtime, "Could not find a space for the node data" );
            break;
        }
        if( (node_data_addr == 0) || (node_data_size == 0) )
        {
            TRACFCOMP( g_trac_runtime, "Invalid memory values for HSVC_NODE_DATA" );
            /*@
             * @errortype
             * @reasoncode       RUNTIME::RC_INVALID_SECTION
             * @moduleid         RUNTIME::MOD_RUNTIME_POP_NODE_ATTR
             * @userdata1        Returned address: node_data_addr
             * @userdata2        Returned size: node_data_size
             * @devdesc          Invalid memory values for HSVC_NODE_DATA
             */
            errhdl = new ERRORLOG::ErrlEntry(
                                          ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                          RUNTIME::MOD_RUNTIME_POP_NODE_ATTR,
                                          RUNTIME::RC_INVALID_SECTION,
                                          node_data_addr,
                                          node_data_size,
                                          true /*Add HB Software Callout*/ );
            break;
        }
        else if( sizeof(node_data_t) > node_data_size )
        {
            TRACFCOMP( g_trac_runtime, "Not enough space allocated by HDAT for HostServices Node Data" );
            /*@
             * @errortype
             * @reasoncode       RUNTIME::RC_NOT_ENOUGH_SPACE
             * @moduleid         RUNTIME::MOD_RUNTIME_POP_NODE_ATTR
             * @userdata1        Required size
             * @userdata2        Available size
             * @devdesc          Not enough space allocated by HDAT for
             *                   HostServices Node Data
             */
            errhdl = new ERRORLOG::ErrlEntry(
                                          ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                          RUNTIME::MOD_RUNTIME_POP_NODE_ATTR,
                                          RUNTIME::RC_NOT_ENOUGH_SPACE,
                                          sizeof(node_data_t),
                                          node_data_size );
            // need to update the FSP code
            errhdl->addProcedureCallout(HWAS::EPUB_PRC_SP_CODE,
                                        HWAS::SRCI_PRIORITY_HIGH);

            // save some of the HDAT data for FFDC
            RUNTIME::add_host_data_ffdc( RUNTIME::HSVC_NODE_DATA, errhdl );

            break;
        }

        node_data_t* node_data = reinterpret_cast<node_data_t*>(node_data_addr);
        memset( node_data, 'A', sizeof(node_data_t) );

        // These variables are used by the HSVC_LOAD_ATTR macros directly
        uint64_t* _num_attr = NULL; //pointer to numAttr in struct
        char* _output_ptr = NULL; //next memory location to copy attr data into
        char* _beginning = NULL; //position zero for offset calculation
        hsvc_attr_header_t* _all_headers = NULL; //array of attribute headers
        fapi::Target* _target = NULL; //target for FAPI_ATTR_GET
        hsvc_attr_header_t* _cur_header = NULL; //temp variable
        uint32_t _huid_temp = 0; //temp variable
        void* _end_of_area = reinterpret_cast<void*>(node_data_addr
                                                     + node_data_size);

        // Prepare the vars for the HSVC_LOAD_ATTR macros
        _beginning = reinterpret_cast<char*>(node_data);
        _output_ptr = node_data->attributes;

        // indices for ex_header and proc_header
        size_t next_proc = 0;
        size_t next_ex = 0;

        // Fill in the metadata
        node_data->hsvc.numTargets = 0;
        node_data->hsvc.procOffset =
          reinterpret_cast<uint64_t>(node_data->procs)
          - reinterpret_cast<uint64_t>(node_data);

        // Get all proc targets
        //  use PredicateHwas to filter only present procs
        TARGETING::PredicateHwas isPresent;
        isPresent.reset().poweredOn(true).present(true);
        //  filter for Chips/Chiplets
        TARGETING::PredicateCTM isChipProc(TARGETING::CLASS_CHIP,
                                           TARGETING::TYPE_PROC);
        //  declare a postfix expression widget
        TARGETING::PredicatePostfixExpr goodFilter;
        //  is-a-chip  is-present   AND
        goodFilter.push(&isChipProc).push(&isPresent).And();
        //  apply the filter through all targets.
        TARGETING::TargetRangeFilter procIter(
                        TARGETING::targetService().begin(),
                        TARGETING::targetService().end(),
                        &goodFilter );

        // Iterate through the present proc list
        // p is the index of the current procIter
        for ( size_t p = 0; procIter; ++procIter,p++ )
        {
            // Cast to a FAPI type of target.
            fapi::Target fapi_proc( fapi::TARGET_TYPE_PROC_CHIP,
                         reinterpret_cast<void *>
                         (const_cast<TARGETING::Target*>(*procIter)) );

            // Compute the processor id to match what HDAT uses
            uint64_t node_id =
              (*procIter)->getAttr<TARGETING::ATTR_FABRIC_NODE_ID>();
            uint64_t chip_id =
              (*procIter)->getAttr<TARGETING::ATTR_FABRIC_CHIP_ID>();
            uint32_t procid = (node_id << 3) | (chip_id); //NNNCCC
            TRACDCOMP( g_trac_runtime, "PROC:%d (%.8X)", procid, TARGETING::get_huid(*procIter) );

            // Fill in the metadata
            node_data->procs[p].procid = procid;
            node_data->procs[p].offset =
              reinterpret_cast<uint64_t>(&(node_data->procAttrHeaders[p][0]))
              - reinterpret_cast<uint64_t>(node_data);
            node_data->procs[p].numAttr = 0;
            (node_data->hsvc.numTargets)++;

            // Prepare the variables for the HSVC_LOAD_ATTR calls
            _all_headers = &(node_data->procAttrHeaders[p][0]);
            _num_attr = &(node_data->procs[p].numAttr);
            _target = &fapi_proc;

            // Fill up the attributes
            ADD_HUID( (*procIter) ); // for debug
            ADD_PHYS_PATH( (*procIter) );
            ADD_ECMD_STRING();
            ADD_IBSCOM_BASE( (*procIter) );

            // Use a generated file for the list of attributes to load
            #include "common/hsvc_procdata.C"

            // Add an empty attribute header to signal the end
            EMPTY_ATTRIBUTE;

            TRACFCOMP( g_trac_runtime, "populate_node_attributes> PROC:%d (%.8X) : numAttr=%d", procid, TARGETING::get_huid(*procIter), node_data->procs[p].numAttr );

            // Make sure we don't overrun our space
            assert( *_num_attr < node_data_t::NUM_PROC_ATTRIBUTES );

            // Loop around all of the EX chiplets for this proc
            TARGETING::TargetHandleList all_ex;
            TARGETING::getChildChiplets( all_ex, (*procIter),
                                         TARGETING::TYPE_EX, false );
            for( size_t e = 0; e < all_ex.size(); e++ )
            {
                uint32_t chiplet =
                  all_ex[e]->getAttr<TARGETING::ATTR_CHIP_UNIT>();
                TRACDCOMP( g_trac_runtime, "EX:p%d c%d(%.8X)", procid, chiplet, get_huid(all_ex[e]) );

                // Fill in the metadata
                (node_data->hsvc.numTargets)++;
                node_data->ex[next_ex].parent_procid = procid;
                node_data->ex[next_ex].chiplet = chiplet;
                node_data->ex[next_ex].offset =
                  reinterpret_cast<uint64_t>(
                         &(node_data->exAttrHeaders[next_ex][0]))
                  - reinterpret_cast<uint64_t>(node_data);
                node_data->hsvc.exOffset =
                  reinterpret_cast<uint64_t>(node_data->ex)
                  - reinterpret_cast<uint64_t>(node_data);
                node_data->ex[next_ex].numAttr = 0;

                // Cast to a FAPI type of target.
                fapi::Target fapi_ex( fapi::TARGET_TYPE_EX_CHIPLET,
                                      reinterpret_cast<void *>
                                      (const_cast<TARGETING::Target*>(all_ex[e])) );

                // Prepare the variables for the HSVC_LOAD_ATTR calls
                _all_headers = &(node_data->exAttrHeaders[next_ex][0]);
                _num_attr = &(node_data->ex[next_ex].numAttr);
                _target = &fapi_ex;

                // Fill up the attributes
                ADD_HUID( (all_ex[e]) ); // for debug
                ADD_PHYS_PATH( (all_ex[e]) );
                ADD_ECMD_STRING();
                // Use a generated file for the list of attributes to load
                #include "common/hsvc_exdata.C"

                // Add an empty attribute header to signal the end
                EMPTY_ATTRIBUTE;

                TRACFCOMP( g_trac_runtime, "populate_node_attributes> EX:p%d c%d(%.8X) : numAttr=%d", procid, chiplet, get_huid(all_ex[e]), node_data->ex[next_ex].numAttr );

                // Make sure we don't overrun our space
                assert( *_num_attr < node_data_t::NUM_EX_ATTRIBUTES );

                next_ex++;
            }

            next_proc++;
        }

        size_t total_bytes = _output_ptr - node_data->attributes;
        TRACFCOMP( g_trac_runtime, "populate_node_attributes> Total bytes = %d", total_bytes );

        // Add an empty Proc marker at the end
        node_data->procs[next_proc].procid = hsvc_proc_header_t::NO_PROC;
        node_data->procs[next_proc].offset = 0;
        node_data->procs[next_proc].numAttr = 0;
        (node_data->hsvc.numTargets)++;

        // Add an empty EX marker at the end
        node_data->ex[next_ex].parent_procid = hsvc_ex_header_t::NO_PROC;
        node_data->ex[next_ex].chiplet = hsvc_ex_header_t::NO_CHIPLET;
        node_data->ex[next_ex].numAttr = 0;

        TRACDCOMP( g_trac_runtime, "Run: system_cmp0.memory_ln4->image.save attributes.node.bin 0x%X %d", node_data, total_bytes );
    } while(0);

    // Handle any errors from FAPI_ATTR_GET
    if( _rc )
    {
        errlHndl_t rc_errhdl = NULL;

        /*@
         * @errortype
         * @reasoncode       RUNTIME::RC_ATTR_GET_FAIL
         * @moduleid         RUNTIME::MOD_RUNTIME_POP_NODE_ATTR
         * @userdata1        Return code from FAPI_ATTR_GET
         * @userdata2        FAPI Attribute Id that failed
         * @devdesc          Error retrieving FAPI attribute
         */
        rc_errhdl = new ERRORLOG::ErrlEntry( ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                             RUNTIME::MOD_RUNTIME_POP_NODE_ATTR,
                                             RUNTIME::RC_ATTR_GET_FAIL,
                                             _rc,
                                             _failed_attribute,
                                             true /*Add HB Software Callout*/ );
        rc_errhdl->collectTrace(TARG_COMP_NAME,KILOBYTE/2);

        // Any previous error is probably more interesting so just commit
        //  the rc error
        if( errhdl )
        {
            errlCommit(rc_errhdl, RUNTIME_COMP_ID);
        }
        else
        {
            errhdl = rc_errhdl;
        }
    }

    return errhdl;
}

/**
 * @brief Populate attributes for HostServices
 */
errlHndl_t populate_attributes( void )
{
    errlHndl_t errhdl = NULL;

    do {
        TRACFCOMP( g_trac_runtime, "Running populate_attributes" );

        TARGETING::Target * sys = NULL;
        TARGETING::targetService().getTopLevelTarget( sys );
        assert(sys != NULL);

        TARGETING::ATTR_HB_EXISTING_IMAGE_type hb_images =
            sys->getAttr<TARGETING::ATTR_HB_EXISTING_IMAGE>();

        // Figure out which node we are running on
        TARGETING::Target* mproc = NULL;
        TARGETING::targetService().masterProcChipTargetHandle(mproc);

        TARGETING::EntityPath epath =
            mproc->getAttr<TARGETING::ATTR_PHYS_PATH>();

        const TARGETING::EntityPath::PathElement pe =
            epath.pathElementOfType(TARGETING::TYPE_NODE);

        uint64_t nodeid = pe.instance;

        // ATTR_HB_EXISTING_IMAGE only gets set on a multi-drawer system.
        // Currently set up in host_sys_fab_iovalid_processing() which only
        // gets called if there are multiple physical nodes.   It eventually
        // needs to be setup by a hb routine that snoops for multiple nodes.
        uint64_t present_nodes = hb_images;
        if(hb_images == 0)
        {
            // Set the appropriate bit for our node
            present_nodes = ((uint64_t)0x1
                             << ((sizeof(present_nodes) * 8) - 1))
                             >> nodeid;
        }
        else
        {
            // need to left-justify the uint8 into the uint64 we want
            present_nodes = present_nodes <<
              ((sizeof(present_nodes)-sizeof(hb_images))*8);
        }

        // Write the System-level Attributes
        errhdl = populate_system_attributes(present_nodes);
        if( errhdl )
        {
            TRACFCOMP( g_trac_runtime, "populate_attributes failed" );
            break;
        }

        // Single or Multi-node?
        if(hb_images == 0) //Single-node
        {
            // Single node system, call inline and pass in our node number
            errhdl = populate_node_attributes(nodeid);
            if(errhdl != NULL)
            {
                TRACFCOMP( g_trac_runtime, "populate_node_attributes failed" );
            }
            break;
        }

        // continue - multi-node

        // This msgQ catches the responses to populate the attributes
        msg_q_t msgQ = msg_q_create();
        errhdl = MBOX::msgq_register(MBOX::HB_POP_ATTR_MSGQ,msgQ);

        if(errhdl)
        {
            TRACFCOMP( g_trac_runtime, "MBOX::msgq_register failed!" );
            break;
        }


        uint8_t node_map[8];

        sys->tryGetAttr<TARGETING::ATTR_FABRIC_TO_PHYSICAL_NODE_MAP>(node_map);

        uint64_t msg_count = 0;

        // This is a multi-drawer system.
        // The assertion is that the hostboot instance must be equal to
        // the logical node we are running on. The ideal would be to have
        // a function call that would return the HB instance number.
        task_affinity_pin();
        task_affinity_migrate_to_master();
        uint64_t this_node = INTR::PIR_t(task_getcpuid()).nodeId;
        task_affinity_unpin();

        uint64_t payloadBase = sys->getAttr<TARGETING::ATTR_PAYLOAD_BASE>();

        //loop though all possible drawers whether they exist or not
        // An invalid or non-existant logical node number in that drawer
        // indicates that the drawer does not exist.
        for(uint64_t drawer = 0; drawer < sizeof(node_map); ++drawer)
        {
            uint64_t node = node_map[drawer];

            if(node < (sizeof(TARGETING::ATTR_HB_EXISTING_IMAGE_type) * 8))
            {

                // set mask to msb
                TARGETING::ATTR_HB_EXISTING_IMAGE_type mask = 0x1 <<
                    ((sizeof(TARGETING::ATTR_HB_EXISTING_IMAGE_type) * 8) -1);

                if( 0 != ((mask >> node) & hb_images ) )
                {
                    TRACDCOMP( g_trac_runtime,
                               "populate_attributes-sending msg for drawer %d",
                               drawer );
                    ++msg_count;
                    msg_t * msg = msg_allocate();
                    msg->type = IPC::IPC_POPULATE_ATTRIBUTES;
                    msg->data[0] = drawer;     // offset in attribute table
                    msg->data[1] = this_node;  // node to send a msg back to
                    msg->extra_data = reinterpret_cast<uint64_t*>(payloadBase);
                    errhdl = MBOX::send(MBOX::HB_IPC_MSGQ, msg, node);
                    if (errhdl)
                    {
                        TRACFCOMP( g_trac_runtime, "MBOX::send failed");
                        break;
                    }
                }
            }
        }

        if(errhdl == NULL)
        {
            // wait for all hb images to respond
            while(msg_count)
            {
                msg_t* msg = msg_wait(msgQ);
                TRACFCOMP( g_trac_runtime,
                           "populate node attributes. drawer %d completed.",
                           msg->data[0]);
                msg_free(msg);
                --msg_count;
            }
        }

        MBOX::msgq_unregister(MBOX::HB_POP_ATTR_MSGQ);
        msg_q_destroy(msgQ);

    } while(0);

    return errhdl;
}

void setPayloadBaseAddress(uint64_t i_payloadAddress)
{
    TARGETING::Target * sys = NULL;
    TARGETING::targetService().getTopLevelTarget( sys );
    sys->setAttr<TARGETING::ATTR_PAYLOAD_BASE>(i_payloadAddress);
}

} //namespace RUNTIME
