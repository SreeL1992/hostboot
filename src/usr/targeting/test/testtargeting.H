/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/targeting/test/testtargeting.H $                      */
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

#ifndef __TARGETING_TESTTARGETING_H
#define __TARGETING_TESTTARGETING_H

/**
 *  @file targeting/test/testtargeting.H
 *
 *  @brief All unit tests which test targeting in a platform specific way
 */

//******************************************************************************
// Includes
//******************************************************************************

// STD
#include <stdio.h>
#include <sys/time.h>

// CXXTEST
#include <cxxtest/TestSuite.H>

#include <errl/errlmanager.H>
#include <sys/sync.h>
#include <sys/task.h>
#include <sys/time.h>

// This component
#include <targeting/common/attributes.H>
#include <targeting/common/entitypath.H>
#include <targeting/common/target.H>
#include <targeting/common/targetservice.H>
#include <targeting/common/utilFilter.H>
#include <targeting/common/iterators/rangefilter.H>
#include <targeting/common/predicates/predicatectm.H>
#include <targeting/common/predicates/predicatepostfixexpr.H>
#include <targeting/common/targreasoncodes.H>
#include <errl/errludtarget.H>
#include <targeting/common/trace.H>
#include <kernel/console.H>

/**
 *  @brief Struct to hold pointers to a mutex / protected value
 */
struct MutexTestData_t
{
    mutex_t* pMutex;         // Pointer to mutex
    barrier_t* pBarrier;     // Pointer to barrier
    volatile uint32_t* pVar; // Pointer to value protected by mutex
};

/**
 *  @brief Function which attempts to write a protected variable
 *
 *  @param[in] i_pData Pointer to mutex pointer/value pointer structure
 *
 *  @return NULL
 */
void* funcTestMutex(void* i_pData)
{
    MutexTestData_t* l_pData = static_cast<MutexTestData_t*>(i_pData);
    barrier_wait(l_pData->pBarrier);
    mutex_lock(l_pData->pMutex);
    *(l_pData->pVar) = 1;
    mutex_unlock(l_pData->pMutex);
    barrier_wait(l_pData->pBarrier);
    return NULL;
}

class TargetingTestSuite : public CxxTest::TestSuite
{
  public:

    /**
     *  @brief Test Hostboot specific mutex attribute support
     */
    void testHbMutexAttr()
    {
        TS_TRACE(ENTER_MRK "testHbMutexAttr" );

        using namespace TARGETING;

        do {

        // Get a reference to the target service
        TargetService& l_targetService = targetService();

        // Get the system target containing the test mutex
        TARGETING::Target* l_pTarget = NULL;
        (void) l_targetService.getTopLevelTarget(l_pTarget);
        if (l_pTarget == NULL)
        {
            TS_FAIL("Top level target handle is NULL");
            break;
        }

        // Get the mutex attribute (actually a mutex_t* which points to
        // a mutex)
        HB_MUTEX_TEST_LOCK_ATTR l_pLock
            = l_pTarget->getHbMutexAttr<TARGETING::ATTR_HB_MUTEX_TEST_LOCK>();

        // Test: Verify the value pointed to by the mutex_t* is zero
        if ( (*reinterpret_cast<uint64_t*>(l_pLock)) != 0)
        {
            TS_FAIL("Mutex attribute must be initialized to zero, but got %ld",
                *reinterpret_cast<uint64_t*>(l_pLock));
            break;
        }

        // Try to get the attribute, and ensure it's the same
        HB_MUTEX_TEST_LOCK_ATTR l_pLockTry = NULL;
        if(l_pTarget->tryGetHbMutexAttr<TARGETING::ATTR_HB_MUTEX_TEST_LOCK>
                (l_pLockTry))
        {
            if(l_pLockTry != l_pLock)
            {
                TS_FAIL("Mutex attributes should match, but dont.  "
                        "l_pLockTry = %ld, l_pLock = %ld",l_pLockTry,
                        l_pLock);
                break;
            }
        }
        else
        {
            TS_FAIL("Mutex attribute tryGet failed, even though it exists");
            break;
        }

        // Create a structue holding pointers to the mutex and a protected value
        volatile uint32_t l_var = 0;
        (void)mutex_lock(l_pLock);
        barrier_t l_barrier;
        (void)barrier_init(&l_barrier, 2);
        MutexTestData_t l_mutexTestData = { l_pLock, &l_barrier, &l_var };

        // Spawn off a function which tries to write the protected value to
        // something unexpected.  If the mutex is working, the for loop will
        // always poll the expected value.
        task_create(funcTestMutex, static_cast<void*>(&l_mutexTestData));

        // Guarantee the child process runs and blocks on the mutex prior to
        // modifying the protected value.  isync to ensure the processor doesn't
        // speculatively perform the comparison prior to the sleep completing
        barrier_wait(&l_barrier);
        nanosleep(0,TEN_CTX_SWITCHES_NS); isync();

        if(l_var != 0)
        {
            TS_FAIL("Protected value must be 0, was %d instead",l_var);
            break;
        }

        // Now unlock the mutex, allowing the other thread to overwrite the
        // protected value; which should happen within 100,000 reads of the
        // var.  This will confirm the other thread was actively trying to
        // write the controlled value
        (void)mutex_unlock(l_pLock);

        // Guarantee the child process acquires the mutex and modifies the
        // protected value.
        barrier_wait(&l_barrier);

        if(l_var != 1)
        {
             TS_FAIL("Protected value must now be 1, was %d instead",l_var);
             break;
        }

        barrier_destroy(&l_barrier);

        } while(0);

        TS_TRACE(EXIT_MRK "testHbMutexAttr");
    }

    /**
     *  @brief Test Hostboot specific error target FFDC support
     */
    void testErrlTargetFFDC()
    {
        TS_TRACE(ENTER_MRK "testErrlTargetFFDC" );

        using namespace ERRORLOG;
        using namespace TARGETING;

        // Get a reference to the target service
        TargetService& l_service = targetService();

        // Get the master proc target
        TARGETING::Target* l_pTarget1 = NULL;
        TARGETING::Target* l_pTarget2 = MASTER_PROCESSOR_CHIP_TARGET_SENTINEL;
        l_service.masterProcChipTargetHandle( l_pTarget1);

        // Create an errorlog to test FFDC capture of targets
        /*@
         * @errortype
         * @severity ERRORLOG_SEV_INFORMATIONAL
         * @moduleid TARG_MOD_TEST
         * @reasoncode TARG_RC_TEST_TARGET_FFDC
         * @userdata1 Test data 1
         * @userdata2 Test data 2
         * @devdesc User Details unit test - create target user detail data
         */
        errlHndl_t l_err;
        l_err = new ErrlEntry(ERRL_SEV_INFORMATIONAL,
                              TARG_MOD_TEST,
                              TARG_RC_TEST_TARGET_FFDC,
                              0x0011223344556677,
                              0x8899aabbccddeeff);

        ErrlUserDetailsTarget(l_pTarget1).addToLog(l_err);
        ErrlUserDetailsTarget(l_pTarget2).addToLog(l_err);

        errlCommit(l_err, CXXTEST_COMP_ID);

        TS_TRACE(EXIT_MRK "testErrlTargetFFDC");
    }

    /**
     *  @brief Test Hostboot specific L4 Target support
     */
    void testL4Target()
    {
        TS_TRACE(ENTER_MRK "testL4Target" );
#if 0
        // Disabling test until ecid reg setup in simics
        // is resolved
        using namespace ERRORLOG;
        using namespace TARGETING;

        TargetHandleList l_targs;

        getAllChiplets( l_targs, TYPE_L4);

        TARGETING::EntityPath phys_path_ptr;

        if( l_targs.size() )
        {

            phys_path_ptr =  l_targs[0]->getAttr<TARGETING::ATTR_PHYS_PATH>();

            TS_TRACE("%s",  phys_path_ptr.toString());

        }
        else
        {
            TS_FAIL("no L4 targets found in system");
        }

#endif
        TS_TRACE(EXIT_MRK "testL4Target" );
    }

    /**
     *  @brief Testing Hostboot support for negative attributes
     */
    void testSignedAttribute()
    {
        TS_TRACE(ENTER_MRK "testSignedAttribute");

        // TODO RTC 144142

        TARGETING::Target* l_sys = NULL;
        TARGETING::targetService().getTopLevelTarget(l_sys);
        int8_t signedAttr = -5;

        // Attempt to set the attribute to a negative number
        if(l_sys->trySetAttr<TARGETING::ATTR_TEST_NEGATIVE_FCN>(signedAttr))
        {
            TS_TRACE("testSignedAttribute: Attribute is %d",signedAttr);
        }
        else
        {
            TS_FAIL("testSignedAttribute: Attribute failed during set");
        }

        signedAttr = 0;

        l_sys->tryGetAttr<TARGETING::ATTR_TEST_NEGATIVE_FCN>(signedAttr);
        if(signedAttr < 0)
        {
            TS_TRACE("testSignedAttribute: Attribute is %d",signedAttr);
        }
        else
        {
            TS_FAIL("testSignedAttribute: Attribute is incorrectly positive");
        }

        TS_TRACE(EXIT_MRK "testSignedAttribute");
    }

    void testPciPhbTarget()
    {
        TS_TRACE(ENTER_MRK "testPciPhbTarget" );

        using namespace ERRORLOG;
        using namespace TARGETING;

        TargetHandleList l_targs;

        getAllChiplets( l_targs, TYPE_PCI);

        // There is no PCI target in P9, ensure we got 0 back
        if(l_targs.size() != 0)
        {
            TS_FAIL("TYPE_PCIE target incorrectly returned non-zero size: %d",
                    l_targs.size());
        }

        // The PCI target is actually the PHB target in P9
        getAllChiplets( l_targs, TYPE_PHB);
        if(l_targs.size() == 0)
        {
            TS_FAIL("TYPE_PHB target incorrectly returned zero size");
        }
        else
        {
            TS_TRACE("Correctly returned non-zero number of PHB units: %d",
                     l_targs.size());
        }

        for (auto l_targ : l_targs)
        {
            // Do a basic validation of PHB target by reading HUID
            uint32_t l_huid = get_huid(l_targ);
            if(l_huid == 0)
            {
                TS_FAIL("Failed to read HUID for a PHB target");
            }
            else
            {
                TS_TRACE("PHB HUID:0x%.8X",l_huid);
            }
        }

        TS_TRACE(EXIT_MRK "testPciPhbTarget");
    }

    void testNvTarget()
    {
        TS_TRACE(ENTER_MRK "testNvTarget" );

        using namespace ERRORLOG;
        using namespace TARGETING;

        TargetHandleList l_targs;

        getAllChiplets( l_targs, TYPE_NV);
        if(l_targs.size() == 0)
        {
            TS_FAIL("TYPE_NV target incorrectly returned zero size");
        }
        else
        {
            TS_TRACE("Correctly returned non-zero number of NV units: %d",
                     l_targs.size());
        }

        for (auto l_targ : l_targs)
        {
            // Do a basic validation of NV target by reading HUID
            uint32_t l_huid = get_huid(l_targ);
            if(l_huid == 0)
            {
                TS_FAIL("Failed to read HUID for a NV target");
            }
            else
            {
                TS_TRACE("NV HUID:0x%.8X",l_huid);
            }
        }

        TS_TRACE(EXIT_MRK "testNvTarget");
    }
};

#endif // End __TARGETING_TESTTARGETING_H
