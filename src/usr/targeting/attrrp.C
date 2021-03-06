/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/targeting/attrrp.C $                                  */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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
 *  @file targeting/attrrp.C
 *
 *  @brief Attribute resource provider implementation which establishes and
 *      initializes virtual memory ranges for attributes as needed, and works
 *      with other resource providers (such as the PNOR resource provider) to
 *      retrieve attribute data which it connot directly provide.
 */

#include <util/singleton.H>
#include <pnortargeting.H>
#include <pnor/pnorif.H>
#include <sys/mm.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <vmmconst.h>
#include <targeting/adapters/assertadapter.H>
#include <targeting/common/targreasoncodes.H>
#include <targeting/attrrp.H>
#include <targeting/common/trace.H>
#include <initservice/initserviceif.H>
#include <util/align.H>
#include <sys/misc.h>

using namespace INITSERVICE;
using namespace ERRORLOG;

#include "attrrp_common.C"

namespace TARGETING
{
    void* AttrRP::getBaseAddress(const NODE_ID i_nodeIdUnused)
    {
        return reinterpret_cast<void*>(VMM_VADDR_ATTR_RP);
    }

    void* AttrRP::startMsgServiceTask(void* i_pInstance)
    {
        // Call msgServiceTask loop on instance.
        TARG_ASSERT(i_pInstance, "No instance passed to startMsgServiceTask");
        static_cast<AttrRP*>(i_pInstance)->msgServiceTask();
        return NULL;
    }

    void AttrRP::startup(errlHndl_t& io_taskRetErrl, bool i_isMpipl)
    {
        errlHndl_t l_errl = NULL;

        do
        {
            iv_isMpipl = i_isMpipl;
            // Parse PNOR headers.
            l_errl = this->parseAttrSectHeader();
            if (l_errl)
            {
                break;
            }

            // Create corresponding VMM blocks for each section.
            l_errl = this->createVmmSections();
            if (l_errl)
            {
                break;
            }

            // Spawn daemon thread.
            task_create(&AttrRP::startMsgServiceTask, this);

            if(iv_isMpipl)
            {
                populateAttrsForMpipl();
            }



        } while (false);

        // If an error occurred, post to TaskArgs.
        if (l_errl)
        {
            l_errl->setSev(ERRORLOG::ERRL_SEV_UNRECOVERABLE);
        }

        //  return any errlogs to _start()
        io_taskRetErrl = l_errl;
    }

    void AttrRP::msgServiceTask() const
    {
        TRACFCOMP(g_trac_targeting, ENTER_MRK "AttrRP::msgServiceTask");

        // Daemon loop.
        while(1)
        {
            int rc = 0;

            // Wait for message.
            msg_t* msg = msg_wait(iv_msgQ);
            if (!msg) continue;

            // Parse message data members.
            uint64_t vAddr = msg->data[0];
            void*    pAddr = reinterpret_cast<void*>(msg->data[1]);

            TRACFCOMP(g_trac_targeting, INFO_MRK "AttrRP: Message recv'd: "
                      "0x%x, 0x%lx 0x%p", msg->type, vAddr, pAddr);

            // Locate corresponding attribute section for message.
            ssize_t section = -1;
            for (size_t i = 0; i < iv_sectionCount; ++i)
            {
                if ((vAddr >= iv_sections[i].vmmAddress) &&
                    (vAddr < iv_sections[i].vmmAddress + iv_sections[i].size))
                {
                    section = i;
                    break;
                }
            }

            // Return EINVAL if no section was found.  Kernel bug?
            if (section == -1)
            {
                rc = -EINVAL;
                TRACFCOMP(g_trac_targeting,
                          ERR_MRK "AttrRP: Address given outside section "
                                  "ranges: %p",
                          vAddr);
            }
            else // Process request.
            {

                // Determine PNOR offset and page size.
                uint64_t offset = vAddr - iv_sections[section].vmmAddress;
                uint64_t size = std::min(PAGE_SIZE,
                                         iv_sections[section].vmmAddress +
                                         iv_sections[section].size -
                                         vAddr);
                    // We could be requested to read/write less than a page
                    // if the virtual address requested is at the end of the
                    // section and the section size is not page aligned.
                    //
                    // Example: Section size is 6k and vAddr = vmmAddr + 4k,
                    //          we should only operate on 2k of content.


                // Read / Write message behavior.
                switch(msg->type)
                {
                    case MSG_MM_RP_READ:
                        // HEAP_ZERO_INIT should never be requested for read
                        // because kernel should automatically get a zero page.
                        if ( (iv_sections[section].type ==
                              SECTION_TYPE_HEAP_ZERO_INIT) ||
                             (iv_sections[section].type ==
                              SECTION_TYPE_HB_HEAP_ZERO_INIT) )
                        {
                            TRACFCOMP(g_trac_targeting,
                                      ERR_MRK "AttrRP: Read request on "
                                              "HEAP_ZERO section.");
                            rc = -EINVAL;
                            break;
                        }
                        // if we are NOT in mpipl OR if this IS a r/w section,
                        // Do a memcpy from PNOR address into physical page.
                        if(!iv_isMpipl || (iv_sections[section].type == SECTION_TYPE_PNOR_RW)  )
                        {
                            memcpy(pAddr,
                                reinterpret_cast<void*>(
                                        iv_sections[section].pnorAddress + offset),
                                size);
                        }
                        else
                        {
                            // Do memcpy from real memory into physical page.
                            memcpy(pAddr,
                                   reinterpret_cast<void*>(
                                   iv_sections[section].realMemAddress + offset),
                                   size);
                        }
                        break;

                    case MSG_MM_RP_WRITE:
                        // Only PNOR_RW should ever be requested for write-back
                        // because others are not allowed to be pushed back to
                        // PNOR.
                        if (iv_sections[section].type !=
                            SECTION_TYPE_PNOR_RW)
                        {
                            TRACFCOMP(g_trac_targeting,
                                      ERR_MRK "AttrRP: Write request on "
                                              "non-PNOR_RW section.");
                            rc = -EINVAL;
                            break;
                        }
                        // Do memcpy from physical page into PNOR.
                        memcpy(reinterpret_cast<void*>(
                                    iv_sections[section].pnorAddress + offset),
                               pAddr,
                               size);
                        break;

                    default:
                        TRACFCOMP(g_trac_targeting,
                                  ERR_MRK "AttrRP: Unhandled command type %d.",
                                  msg->type);
                        rc = -EINVAL;
                        break;
                }
            }

            // Log an error log if the AttrRP was unable to handle a message
            // for any reason.
            if (rc != 0)
            {
                /*@
                 *   @errortype         ERRORLOG::ERRL_SEV_UNRECOVERABLE
                 *   @moduleid          TARG_MSG_SERVICE_TASK
                 *   @reasoncode        TARG_RC_ATTR_MSG_FAIL
                 *   @userdata1         Virtual Address
                 *   @userdata2         (Msg Type << 32) | Section #
                 *
                 *   @devdesc   The attribute resource provider was unable to
                 *              satisfy a message request from the VMM portion
                 *              of the kernel.  This was either due to an
                 *              address outside a valid range or a message
                 *              request that is invalid for the attribute
                 *              section containing the address.
                 *
                 *   @custdesc  Attribute Resource Provider failed to handle
                 *              request
                 */
                const bool hbSwError = true;
                errlHndl_t l_errl = new ErrlEntry(ERRL_SEV_UNRECOVERABLE,
                                                  TARG_MSG_SERVICE_TASK,
                                                  TARG_RC_ATTR_MSG_FAIL,
                                                  vAddr,
                                                  TWO_UINT32_TO_UINT64(
                                                        msg->type,
                                                        section),
                                                  hbSwError);
                errlCommit(l_errl,TARG_COMP_ID);
            }

            // Respond to kernel request.
            msg->data[1] = rc;
            rc = msg_respond(iv_msgQ, msg);
            if (rc)
            {
                TRACFCOMP(g_trac_targeting,
                          ERR_MRK "AttrRP: Bad rc from msg_respond: %d", rc);
            }
        }
    }

    errlHndl_t AttrRP::parseAttrSectHeader()
    {
        errlHndl_t l_errl = NULL;

        do
        {
            // Locate attribute section in PNOR.
            PNOR::SectionInfo_t l_pnorSectionInfo;
            TargetingHeader* l_header = nullptr;
            l_errl = PNOR::getSectionInfo(PNOR::HB_DATA,
                                          l_pnorSectionInfo);
            if (l_errl) break;
            if(!iv_isMpipl)
            {
                // Find attribute section header.
                l_header =
                reinterpret_cast<TargetingHeader*>(l_pnorSectionInfo.vaddr);
            }
            else
            {
                TRACFCOMP(g_trac_targeting,
                           "Reading attributes from memory, NOT PNOR");
                //Create a block map of the address space we used to store
                //attribute information on the initial IPL
                //Account HRMOR (non 0 base addr)
                uint64_t    l_attr_data_addr =   cpu_spr_value(CPU_SPR_HRMOR)
                                                 + MPIPL_ATTR_DATA_ADDR;
                l_header = reinterpret_cast<TargetingHeader*>(
                mm_block_map(reinterpret_cast<void*>(l_attr_data_addr),
                             MPIPL_ATTR_VMM_SIZE));
                if(l_header == 0)
                {
                    TRACFCOMP(g_trac_targeting,
                              "Failed mapping phys addr: %p for %lx bytes",
                              l_attr_data_addr,
                              MPIPL_ATTR_VMM_SIZE);
                    //Error mm_block_map returned invalid ptr
                    /*@
                    *   @errortype         ERRORLOG::ERRL_SEV_UNRECOVERABLE
                    *   @moduleid          TARG_PARSE_ATTR_SECT_HEADER
                    *   @reasoncode        TARG_RC_MM_BLOCK_MAP_FAIL
                    *   @userdata1         physical address of target info
                    *   @userdata2         size we tried to map
                    *
                    *   @devdesc   While attempting to map a phys addr to a virtual
                    *              addr for our targeting information the kernel
                    *              returned an error
                    *   @custdesc  Kernel failed to block map memory
                    */
                    l_errl = new ErrlEntry(ERRL_SEV_UNRECOVERABLE,
                                           TARG_PARSE_ATTR_SECT_HEADER,
                                           TARG_RC_MM_BLOCK_FAIL,
                                           l_attr_data_addr,
                                           MPIPL_ATTR_VMM_SIZE,
                                           true);
                    break;
                }
                TRACFCOMP(g_trac_targeting,
                          "Mapped phys addr: %p to virt addr: %p",
                          reinterpret_cast<void*>(l_attr_data_addr),
                          l_header);
            }


            // Validate eye catch.
            if (l_header->eyeCatcher != PNOR_TARG_EYE_CATCHER)
            {
                TRACFCOMP(g_trac_targeting,
                          "ATTR_DATA section in pnor header mismatch found"
                          " header: %d expected header: %d",
                          l_header->eyeCatcher,
                          PNOR_TARG_EYE_CATCHER);
                /*@
                 *   @errortype         ERRORLOG::ERRL_SEV_UNRECOVERABLE
                 *   @moduleid          TARG_PARSE_ATTR_SECT_HEADER
                 *   @reasoncode        TARG_RC_BAD_EYECATCH
                 *   @userdata1         Observed Header Eyecatch Value
                 *   @userdata2         Expected Eyecatch Value
                 *
                 *   @devdesc   The eyecatch value observed in PNOR does not
                 *              match the expected value of
                 *              PNOR_TARG_EYE_CATCHER and therefore the
                 *              contents of the Attribute PNOR section are
                 *              unable to be parsed.
                 *   @custdesc  A problem occurred during the IPL of the
                 *              system.
                 *              The eyecatch value observed in memory does not
                 *              match the expected value and therefore the
                 *              contents of the attribute sections are unable
                 *              to be parsed.
                */
                l_errl = new ErrlEntry(ERRL_SEV_UNRECOVERABLE,
                                       TARG_PARSE_ATTR_SECT_HEADER,
                                       TARG_RC_BAD_EYECATCH,
                                       l_header->eyeCatcher,
                                       PNOR_TARG_EYE_CATCHER);
                break;
            }

            // Allocate section structures based on section count in header.
            iv_sectionCount = l_header->numSections;
            iv_sections = new AttrRP_Section[iv_sectionCount];

            TargetingSection* l_section = nullptr;
            if(!iv_isMpipl)
            {
                // Find start to first section:
                //          (PNOR addr + size of header + offset in header).
                l_section =
                        reinterpret_cast<TargetingSection*>(
                                l_pnorSectionInfo.vaddr + sizeof(TargetingHeader) +
                                l_header->offsetToSections
                        );
            }
            else
            {
                // Find start to first section:
                // (header address + size of header + offset in header)
                l_section =
                    reinterpret_cast<TargetingSection*>(
                        reinterpret_cast<uint64_t>(l_header) +
                        sizeof(TargetingHeader) + l_header->offsetToSections
                        );

            }

            //Keep a running offset of how far into our real memory section we are
            uint64_t l_realMemOffset = 0;

            // Parse each section.
            for (size_t i = 0; i < iv_sectionCount; i++, ++l_section)
            {
                iv_sections[i].type = l_section->sectionType;

                // Conversion cast for templated abstract pointer object only
                // works when casting to pointer of the templated type.  Since
                // cache is of a different type, we first cast to extract the
                // real pointer, then recast it into the cache
                iv_sections[i].vmmAddress =
                        static_cast<uint64_t>(
                            TARG_TO_PLAT_PTR(l_header->vmmBaseAddress)) +
                        l_header->vmmSectionOffset*i;


                iv_sections[i].pnorAddress =
                    l_pnorSectionInfo.vaddr + l_section->sectionOffset;

                if(iv_isMpipl)
                {
                    //For MPIPL we are reading from real memory,
                    //not pnor flash. Set the real memory address
                    iv_sections[i].realMemAddress =
                        reinterpret_cast<uint64_t>(l_header) + l_realMemOffset;
                }
                iv_sections[i].size = l_section->sectionSize;

                //Increment our offset variable by the size of this section
                l_realMemOffset += iv_sections[i].size;

                TRACFCOMP(g_trac_targeting,
                          "Decoded Attribute Section: %d, 0x%lx 0x%lx 0x%lx 0x%lx",
                          iv_sections[i].type,
                          iv_sections[i].vmmAddress,
                          iv_sections[i].pnorAddress,
                          iv_sections[i].realMemAddress,
                          iv_sections[i].size);

            }

        } while (false);

        return l_errl;

    }

    errlHndl_t AttrRP::createVmmSections()
    {
        errlHndl_t l_errl = NULL;

        do
        {
            // Allocate message queue for VMM requests.
            iv_msgQ = msg_q_create();

            // Create VMM block for each section, assign permissions.
            for (size_t i = 0; i < iv_sectionCount; ++i)
            {
                uint64_t l_perm = 0;
                switch(iv_sections[i].type)
                {
                    case SECTION_TYPE_PNOR_RO:
                        l_perm = READ_ONLY;
                        break;

                    case SECTION_TYPE_PNOR_RW:
                        l_perm = WRITABLE | WRITE_TRACKED;
                        break;

                    case SECTION_TYPE_HEAP_PNOR_INIT:
                        l_perm = WRITABLE;
                        break;

                    case SECTION_TYPE_HEAP_ZERO_INIT:
                    case SECTION_TYPE_HB_HEAP_ZERO_INIT:
                        l_perm = WRITABLE | ALLOCATE_FROM_ZERO;
                        break;

                    default:

                        /*@
                         *   @errortype  ERRORLOG::ERRL_SEV_UNRECOVERABLE
                         *   @moduleid   TARG_CREATE_VMM_SECTIONS
                         *   @reasoncode TARG_RC_UNHANDLED_ATTR_SEC_TYPE
                         *   @userdata1  Section type
                         *
                         *   @devdesc    Found unhandled attribute section type
                         *   @custdesc   FW error, unexpected Attribute section type
                         */
                        const bool hbSwError = true;
                        l_errl = new ErrlEntry(ERRL_SEV_UNRECOVERABLE,
                                               TARG_CREATE_VMM_SECTIONS,
                                               TARG_RC_UNHANDLED_ATTR_SEC_TYPE,
                                               iv_sections[i].type,
                                               0, hbSwError);
                        break;
                }

                if(l_errl)
                {
                    break;
                }

                int rc = 0;
                msg_q_t l_msgQ = iv_msgQ;

                if ( (iv_sections[i].type == SECTION_TYPE_HEAP_ZERO_INIT) ||
                     (iv_sections[i].type == SECTION_TYPE_HB_HEAP_ZERO_INIT) )
                {
                    l_msgQ = NULL;
                }

                rc = mm_alloc_block(l_msgQ,
                                    reinterpret_cast<void*>(iv_sections[i].vmmAddress),
                                    iv_sections[i].size);

                if (rc)
                {
                    /*@
                     *   @errortype         ERRORLOG::ERRL_SEV_UNRECOVERABLE
                     *   @moduleid          TARG_CREATE_VMM_SECTIONS
                     *   @reasoncode        TARG_RC_MM_BLOCK_FAIL
                     *   @userdata1         vAddress attempting to allocate.
                     *   @userdata2         RC from kernel.
                     *
                     *   @devdesc   While attempting to allocate a virtual
                     *              memory block for an attribute section, the
                     *              kernel returned an error.
                     *   @custdesc  Kernel failed to block memory
                     */
                    const bool hbSwError = true;
                    l_errl = new ErrlEntry(ERRL_SEV_UNRECOVERABLE,
                                           TARG_CREATE_VMM_SECTIONS,
                                           TARG_RC_MM_BLOCK_FAIL,
                                           iv_sections[i].vmmAddress,
                                           rc, hbSwError);
                    break;
                }

                if(iv_sections[i].type == SECTION_TYPE_PNOR_RW)
                {
                    // TODO RTC:164480 For MPIPL we need to map the RW section
                    // in real memory to virtual address of the section in PNOR
                    /*
                     * Register this memory range to be FLUSHed during
                     * a shutdown.
                     */
                    INITSERVICE::registerBlock(
                        reinterpret_cast<void*>(iv_sections[i].vmmAddress),
                        iv_sections[i].size,ATTR_PRIORITY);
                }

                rc = mm_set_permission(reinterpret_cast<void*>(
                                       iv_sections[i].vmmAddress),
                                       iv_sections[i].size,
                                       l_perm);

                if (rc)
                {
                    /*@
                     *   @errortype         ERRORLOG::ERRL_SEV_UNRECOVERABLE
                     *   @moduleid          TARG_CREATE_VMM_SECTIONS
                     *   @reasoncode        TARG_RC_MM_PERM_FAIL
                     *   @userdata1         vAddress attempting to allocate.
                     *   @userdata2         (kernel-rc << 32) | (Permissions)
                     *
                     *   @devdesc   While attempting to set permissions on
                     *              a virtual memory block for an attribute
                     *              section, the kernel returned an error.
                     *
                     *   @custdesc  Kernel failed to set permissions on
                     *              virtual memory block
                     */
                    const bool hbSwError = true;
                    l_errl = new ErrlEntry(ERRL_SEV_UNRECOVERABLE,
                                           TARG_CREATE_VMM_SECTIONS,
                                           TARG_RC_MM_PERM_FAIL,
                                           iv_sections[i].vmmAddress,
                                           TWO_UINT32_TO_UINT64(rc, l_perm),
                                           hbSwError);
                    break;
                }
            } // End iteration through each section

            if(l_errl)
            {
                break;
            }

        } while (false);

        return l_errl;
    }

    void AttrRP::populateAttrsForMpipl()
    {
        do
        {
            // Copy RW, Heap Zero Init sections because we are not
            // running the isteps that set these attrs during MPIPL
            for (size_t i = 0; i < iv_sectionCount; ++i)
            {
                // The volatile sections in MPIPL need to be copied because
                // on the MPIPL flow we will not run the HWPs that set these attrs
                // the RW section of the attribute data must be copied
                // into the vmmAddress in order to make future r/w come
                // from the pnor address, not real memory
                if(((iv_sections[i].type == SECTION_TYPE_HEAP_ZERO_INIT) ||
                    (iv_sections[i].type == SECTION_TYPE_HB_HEAP_ZERO_INIT) ||
                    (iv_sections[i].type == SECTION_TYPE_PNOR_RW)) &&
                    iv_isMpipl)
                {
                    memcpy(reinterpret_cast<void*>(iv_sections[i].vmmAddress),
                        reinterpret_cast<void*>(iv_sections[i].realMemAddress),
                        (iv_sections[i].size));
                }

            }
        }while(0);
    }

    void* AttrRP::save(uint64_t& io_addr)
    {
        // Call save on singleton instance.
        return Singleton<AttrRP>::instance()._save(io_addr);
    }


    uint64_t  AttrRP::maxSize( )
    {
        // Find total size of the sections.
        uint64_t l_size = 0;
        for( size_t i = 0;
             i < Singleton<AttrRP>::instance().iv_sectionCount;
             ++i)
        {
            l_size += ALIGN_PAGE(Singleton<AttrRP>::
                          instance().iv_sections[i].size);
        }

        return(l_size);

    } // end maxSize


    void* AttrRP::_save(uint64_t& io_addr)
    {
        TRACDCOMP(g_trac_targeting, "AttrRP::save: top @ 0x%lx", io_addr);

        // Find total size of the sections.
        uint64_t l_size = maxSize();

        void* region = reinterpret_cast<void*>(io_addr);
        uint8_t* pointer = reinterpret_cast<uint8_t*>(region);

        // For PHYP adjunct partition, use HDAT ptr as is
        bool  l_isPhyp = TARGETING::is_phyp_load();
        if (!l_isPhyp)
        {
            io_addr = ALIGN_PAGE_DOWN(io_addr);
            // Determine bottom of the address region.
            io_addr = io_addr - l_size;
            // Align to 64KB for Opal
            io_addr = ALIGN_DOWN_X(io_addr,64*KILOBYTE);

            // Map in region.
            region = mm_block_map(reinterpret_cast<void*>(io_addr),l_size);
            pointer = reinterpret_cast<uint8_t*>(region);

        } // end if NOT PHYP

        // Copy content.
        for (size_t i = 0; i < iv_sectionCount; ++i)
        {
            memcpy(pointer,
                   reinterpret_cast<void*>(iv_sections[i].vmmAddress),
                   iv_sections[i].size);

            pointer = &pointer[ALIGN_PAGE(iv_sections[i].size)];
        }

        TRACFCOMP(g_trac_targeting, "AttrRP::save: bottom @ 0x%lx", io_addr);
        return region;
    }

};
